#define NCOEFFICIENTS 9 // a, b, c, d, e, f, g, h, i
#define NFREQ 8
#define Fref 2000000
#define SET_NODE_FREQ 1
#define SET_CORE_FREQ 0

#include "DVFSPlugin.h"
#include "model_inc.h"
#include <cmath>

static const double NANO = 1e9;




DVFSPlugin::DVFSPlugin() :
    app( Application::instance() ),
    define_params( false ),
    fRef( static_cast<float>( Fref ) / 1000000.0f ),
    searchAlgorithm( NULL ),
    tuningRegion( NULL ),
    model_method( MODEL_ENERGY1 ),
    set_freq_node( SET_NODE_FREQ ),
    freq_neighbours( 1 ) {
}


/**
 * @brief Initialize the plugin's data structures.
 *
 * This method initializes the structures needed to store the data
 * relative to the DVFS plugin. The tuning parameter list needs to
 * be created.
 *
 * The plugin uses 2 tuning parameters: the available frequencies
 * and governors of the machine. There are 16 possible frequencies
 * [1.2GHz, 1.3GHz, 1.4GHz, ... 2.6GHz, 2.7GHz] and 5 governors
 * [conservative, ondemand, performance, powersave, userspace]. For
 * more information related to the available frequencies and governors
 * of a certain platform, refer to:
 * https://www.kernel.org/doc/Documentation/cpu-freq/governors.txt
 *
 * @ingroup DVFSPlugin
 * @param context Context.
 * @param pool_set Set of Scenario Pools.
 */
void DVFSPlugin::initialize( DriverContext*   context,
                             ScenarioPoolSet* pool_set ) {
    int                     major, minor;
    string                  name, description;
    list<Region*>           regions;
    list<Region*>::iterator reg;

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DVFSPlugin: call to initialize()\n" );
    this->context  = context;
    this->pool_set = pool_set;

    regions = app.get_regions();
    if( regions.empty() ) {
        psc_abort( "DVFSPlugin: No code regions are known! Exiting.\n" );
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DVFSPlugin: obtain getSearchInstance\n" );
    char const* selected_search = getenv( "PSC_SEARCH_ALGORITHM" );
    if( selected_search != NULL ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                    "DVFSPlugin: User specified search algorithm: %s\n",
                    selected_search );
        string selected_search_string = string( selected_search );
        context->loadSearchAlgorithm( selected_search_string, &major, &minor,
                                      &name, &description );
        searchAlgorithm = context->getSearchAlgorithmInstance(
            selected_search_string );
    }
    else {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                    "DVFSPlugin: Selecting default search algorithm: exhaustive\n" );
        context->loadSearchAlgorithm( "exhaustive", &major, &minor, &name,
                                      &description );
        searchAlgorithm = context->getSearchAlgorithmInstance( "exhaustive" );
    }

    if( searchAlgorithm != NULL ) {
        searchAlgorithm->initialize( context, pool_set );
    }
    else {
        perror( "NULL pointer in searchAlgorithm\n" );
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }

    if( !searchAlgorithm ) {
        perror( "DVFSPlugin: Search algorithm not instantiated\n" );
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }

    //model method
    char* env_PSC_DVFS_MODEL = getenv( "PSC_DVFS_TUNING_OBJECTIVE" );
    int   model_method_tmp   = 0;
    if( env_PSC_DVFS_MODEL != NULL ) {
        model_method_tmp = atoi( env_PSC_DVFS_MODEL );
    }
    if( model_method_tmp >= MODEL_ENERGY1 && model_method_tmp <= MODEL_POLICY4 ) {
        model_method = model_method_tmp;
        cout << "DVFSPlugin: User specified model:" << model_method << endl;
    }
    else {
        cout << "DVFSPlugin: No model specified, using default: MODEL_ENERGY1" << endl;
    }

    char* env_PSC_FREQ_TO_ALL_NODE = getenv( "PSC_FREQ_TO_ALL_NODE" );
    int   set_freq_node_tmp        = 0;
    if( env_PSC_FREQ_TO_ALL_NODE != NULL ) {
        set_freq_node_tmp = atoi( env_PSC_FREQ_TO_ALL_NODE );
    }
    if( set_freq_node_tmp == SET_NODE_FREQ ) {
        set_freq_node = SET_NODE_FREQ;
        cout << "DVFSPlugin: Sets frequency to all the node" << endl;
    }

    char* env_PSC_FREQ_NEIGHBORS = getenv( "PSC_FREQ_NEIGHBORS" );
    int   freq_neighbours_tmp    = 0;
    if( env_PSC_FREQ_NEIGHBORS != NULL ) {
        freq_neighbours_tmp = atoi( env_PSC_FREQ_NEIGHBORS );
    }
    if( freq_neighbours_tmp > 1 && freq_neighbours_tmp < 8 ) {
        freq_neighbours = freq_neighbours_tmp;
        cout << "DVFSPlugin: will investigate " <<  freq_neighbours << " neighbouring frequencies (higher and lower)" << endl;
    }
}


/**
 * @brief Preparation before starting a tuning step.
 *
 * The tuning parameters that were created at initialization are used to create a variant space.
 * The variant space and the regions are then used to create a search space that is passed to the
 * search algorithm.
 *
 * @ingroup DVFSPlugin
 */
void DVFSPlugin::startTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DVFSPlugin: call to startTuningStep()\n" );

    tuningRegion = app.get_phase_region();

    if( !tuningRegion ) {
        tuningRegion = app.get_main_region();
    }
    if( !tuningRegion ) {
        printf( "TuningRegion is NULL in startTuningStep\n" );
    }
}


/**
 * @brief Defines a pre-analysis strategy.
 *
 * In the case that an analysis strategy procedure is needed, it is
 * done in this method. For this plugin, the strategy loaded is the one
 * called "EnergyGranularity"
 *
 * The method is used to found properties of the pre-analysis strategy
 * being later stored in the arp.
 *
 * @ingroup DVFSPlugin
 * @param **strategy Address where the pre-analysis strategy information should be stored
 */
bool DVFSPlugin::analysisRequired( StrategyRequest** strategy ) {
    StrategyRequestGeneralInfo* analysisStrategyGeneralInfo;
    TuningParameter*            preconfigureTP;
    map<TuningParameter*, int>  variantMap;
    list<Region*>*              location;
    TuningSpecification*        preconfigureTS;
    list<TuningSpecification*>* listPreconfigureTS;
    StrategyRequest*            analysisStrategy;

    analysisStrategyGeneralInfo                    = new StrategyRequestGeneralInfo;
    analysisStrategyGeneralInfo->strategy_name     = "EnergyGranularityBF";
    analysisStrategyGeneralInfo->pedantic          = 1;
    analysisStrategyGeneralInfo->delay_phases      = 0;
    analysisStrategyGeneralInfo->delay_seconds     = 0;
    analysisStrategyGeneralInfo->analysis_duration = 1;




    preconfigureTP = new TuningParameter;
    if( set_freq_node ) {
        preconfigureTP->setName( "FREQNODE" );
    }
    else {
        preconfigureTP->setName( "FREQCORE" );
    }
    preconfigureTP->setPluginType( Power );
    preconfigureTP->setRuntimeActionType( TUNING_ACTION_FUNCTION_POINTER );
    preconfigureTP->setId( 1000 );
    preconfigureTP->setRange( NFREQ, NFREQ, 1 ); // REf Freq

    variantMap[ preconfigureTP ] = NFREQ;        // REf Freq

//Carla
    preconfigureTP = new TuningParameter;
    if( set_freq_node ) {
        preconfigureTP->setName( "GOVNODE" );
    }
    else {
        preconfigureTP->setName( "GOVCORE" );
    }
    preconfigureTP->setPluginType( Power );
    preconfigureTP->setRuntimeActionType( TUNING_ACTION_FUNCTION_POINTER );
    preconfigureTP->setId( 1001 );
    preconfigureTP->setRange( 4, 4, 1 ); // Userspace governor

    variantMap[ preconfigureTP ] = 4;    // Userspace governor

    location = new list<Region*>;
    location->push_back( tuningRegion );

    preconfigureTS     = new TuningSpecification( new Variant( variantMap ), location );
    listPreconfigureTS = new list<TuningSpecification*>;
    listPreconfigureTS->push_back( preconfigureTS );

    analysisStrategy = new StrategyRequest( listPreconfigureTS, analysisStrategyGeneralInfo );
    *strategy        = analysisStrategy;
    return true;
}


/**
 * @brief Population of the "Created Scenario Pool" (csp)
 *
 * The scenarios need to be created and added to the first pool (created
 * scenario pool). In order to create the scenarios, a search algorithm can
 * be used or it can be done directly by the plugin.
 *
 * After this step, the Periscope will verify that scenarios were added to the csp.
 *
 * @ingroup DVFSPlugin
 */
void DVFSPlugin::createScenarios( void ) {
    typedef map<int, list<MetaProperty> >::const_iterator Iterator;

    int                           propertyCount            = 0;
    map<int, list<MetaProperty> > preAnalysisPropertiesMap = pool_set->arp->getAllPreAnalysisProperties();
    list<MetaProperty>                            properties;
    list<MetaProperty>::iterator                  property;
    TuningParameter*                              tpf          = 0;
    TuningParameter*                              tpg          = 0;
    int                                           index_freq   = 0;
    double                                        l2           = 0.0;
    double                                        l3           = 0.0;
    double                                        inst         = 0.0;
    double                                        cores        = 0.0;
    double                                        dram         = 0.0;
    double                                        cyc          = 0.0;
    double                                        t            = 0.0;
    double                                        measuredFreq = 0.0;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DVFSPlugin: call to createScenarios()\n" );

    for( Iterator i = preAnalysisPropertiesMap.begin();  i != preAnalysisPropertiesMap.end();  ++i ) {
        properties = i->second;
        cout << "Properties returned from the pre-analysis" << endl;

        for( property = properties.begin(); property != properties.end();
             property++, propertyCount++ ) {
            if( property->getName().compare( "SuitedForEnergyConfiguration" ) == 0 ) {
                if( app.getRegionByID( property->getRegionId() ) == app.get_phase_region() ) {
                    // check measured frequency
                    std::cout <<  "Property for phase region identified:"  <<  std::endl;
                    double temp_cyc = atof( property->getExtraInfo().at( "Cycles" ).c_str() );
                    double temp_t   = atof( property->getExtraInfo().at( "ExecutionTime" ).c_str() );
//Calculate MODEL here:
                    if( temp_t ) {
                        //try to extract energy values (not really necessary for calculating the factors in the model)
                        double temp_cores = atof( property->getExtraInfo().at( "Nanojoules" ).c_str() ) / NANO;
                        double temp_dram  = atof( property->getExtraInfo().at( "DRAM" ).c_str() ) / NANO;
                        if( temp_cores > 0 ) {
                            cores = temp_cores;
                            dram  = temp_dram;
                        }

                        // use the best cyc/t relation for the model
                        // (that is, the relation that is closest to the reference frequency)
                        double temp_measuredFreq = temp_cyc / temp_t;
                        if( fabs( Fref * 1000 - temp_measuredFreq ) < fabs( Fref * 1000 - measuredFreq ) ) {
                            // Get values from the properties for the model.
                            t            = temp_t;
                            cyc          = temp_cyc;
                            l2           = atof( property->getExtraInfo().at( "L2CacheMisses" ).c_str() );
                            l3           = atof( property->getExtraInfo().at( "L3CacheMisses" ).c_str() );
                            inst         = atof( property->getExtraInfo().at( "Instructions" ).c_str() );
                            measuredFreq = temp_measuredFreq;
                        }
                        //if we have a better measurement of frequency we recalculate the model.
                    }
                    else {
                        psc_abort("ERROR: The variable t in function DVFSPlugin::createScenarios is zero.\n");
                    }
                }                 //if phase and model

//                cout << propertyCount << ". Property: " << endl;
//                cout << " - Cluster: " << property->getCluster() << endl;
//                cout << " - Confidence: " << property->getConfidence() << endl;
//                cout << " - Configuration: " << property->getConfiguration() << endl;
//                cout << " - File id: " << property->getFileId() << endl;
//                cout << " - File name: " << property->getFileName() << endl;
//                cout << " - Property id: " << property->getId() << endl;
//                cout << " - Max processes: " << property->getMaxProcs() << endl;
//                cout << " - Max threads: " << property->getMaxThreads() << endl;
//                cout << " - Name: " << property->getName() << endl;
//                cout << " - Process: " << property->getProcess() << endl;
//                cout << " - Region id: " << property->getRegionId() << endl;
//                cout << " - Region name: " << property->getRegionName() << endl;
//                cout << " - Region type: " << property->getRegionType() << endl;
//                cout << " - Severity: " << property->getSeverity() << endl;
//                cout << " - Start position: " << property->getStartPosition() << endl;
//                cout << " - Thread: " << property->getThread() << endl;
//                cout << " - Time: " << property->getExtraInfo().at( "ExecutionTime" ) << endl;

                suited_regions.push_back( app.getRegionByID( property->getRegionId() ) );
            } // if suited
        }     // for property
    }         // for propertyMapIter
    std::cout << "Model data to work with: " << "cycles=" << cyc << ", inst=" << inst << ", l2=" << l2 << ", l3=" << l3
              << ", energy=" << ( cores + 4 * dram ) << ", t=" << t << ", pwr=" << ( ( cores + 4 * dram ) / t ) << endl;
    float optFreq = 0.0, optval = 0.0;

    index_freq = model_prediction( Fref * 1000, inst / t, l2 / t, l3 / t, cores + 4 * dram, t, ( cores + 4 * dram ) / t, NULL, optFreq, optval );
    cout << "Found optimal frequency for phase region="     << optFreq << ", optimal value = " << optval << ", index=" << index_freq << endl;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DVFSPlugin: There are %d regions.\n", suited_regions.size() );

    double measured  = measuredFreq;
    double reference = Fref * 1000;
    double maxDelta  = 0.05e9;
    if( abs( measured - reference ) > maxDelta ) {
        psc_errmsg( "DVFS: Cycles measurement is too different from reference frequency. (measured = %e Hz, reference = %e Hz)\n", static_cast<double>( measured ), static_cast<double>( reference ) );
        psc_errmsg( "DVFS: Maybe the phase region is too short-lived (t = %f sec).\n", t );
        psc_errmsg( "DVFS: Model data is not valid. Aborting...\n" );
        //abort();
    }

    tpg = new TuningParameter;
    if( set_freq_node ) {
        tpg->setName( "GOVNODE" );
    }
    else {
        tpg->setName( "GOVCORE" );
    }
    tpg->setPluginType( Power );
    tpg->setRuntimeActionType( TUNING_ACTION_FUNCTION_POINTER );
    tpg->setId( 0 );
    tpg->setRange( 4, 4, 1 );   // Governors: only userspace

    tpf = new TuningParameter;
    if( set_freq_node ) {
        tpf->setName( "FREQNODE" );
    }
    else {
        tpf->setName( "FREQCORE" );
    }
    tpf->setPluginType( Power );
    tpf->setRuntimeActionType( TUNING_ACTION_FUNCTION_POINTER );
    tpf->setId( 0 );
    int freq_left  = ( index_freq - freq_neighbours ) > 0 ? ( index_freq - freq_neighbours ) : 0;
    int freq_right = ( index_freq + freq_neighbours ) < 16 ?   ( index_freq + freq_neighbours ) : 15;
    tpf->setRange( freq_left, freq_right, 1 );

    tuningParameters.push_back( tpf );
    tuningParameters.push_back( tpg );

    for( int i = 0; i < tuningParameters.size(); ++i ) {
        variantSpace.addTuningParameter( tuningParameters[ i ] );
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DVFSPlugin: Create a SearchSpace from the tuning parameters.\n" );
    searchSpace.setVariantSpace( &variantSpace );
    searchSpace.addRegion( tuningRegion );
    searchAlgorithm->addSearchSpace( &searchSpace );
    searchAlgorithm->createScenarios();
}


/**
 * @brief Method to prepare the scenarios.
 *
 * Since there are no preparatory steps required by the scenarios in
 * the csp, the method just only move all the scenarios from the cps
 * to the psp (Prepared Scenario Pool).
 *
 * After this step, the Periscope will verify that scenarios were added to the psp.
 *
 * @ingroup DVFSPlugin
 */
void DVFSPlugin::prepareScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DVFSPlugin: call to prepareScenarios()\n" );

    //no preparation is necessary, just move the elements on the pools
    while( !pool_set->csp->empty() ) {
        pool_set->psp->push( pool_set->csp->pop() );
    }
}


/**
 * @brief Method to populate the Experiment Scenario Pool (esp)
 * for the next experiment.
 *
 * The final steps before the experiments are executed is performed in this
 * method. The scenarios are now moved from the psp to the Experiment
 * Scenario Pool (esp), depending on the number of processes and whether
 * they can be executed in parallel.
 *
 * After this step, the Periscope will verify that scenarios were added to the esp.
 *
 * @ingroup DVFSPlugin
 * @param numprocs Ignored
 * @param *analysisRequired Ignored
 * @param **strategy Ignored
 */
void DVFSPlugin::defineExperiment( int               numprocs,
                                   bool&             analysisRequired,
                                   StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DVFSPlugin: call to defineExperiment()\n" );

    int experimentId;
    int propertyCount = 0;
    //map<int, list<MetaProperty> > perExperimentPropertiesMap;
    map<int, list<MetaProperty> >::const_iterator propertyMapIter;
    list<MetaProperty>                            properties;
    list<MetaProperty>::iterator                  property;

    Scenario*                         scenario;
    const list<TuningSpecification*>* ts;
    list<PropertyRequest*>*           propRequestList = new list<PropertyRequest*>;
    PropertyRequest*                  propRequest;
    PropertyRequest*                  objPropRequest;
    list<int>*                        property_ids = new list<int>;
    list<unsigned int>*               rank_list    = new list<unsigned int>;
    std::list<Region*>::iterator      it;
    int                               i = 0;


    //perExperimentPropertiesMap = pool_set->arp->getAllExperimentProperties();

    scenario = pool_set->psp->pop();
    ts       = scenario->getTuningSpecifications();
    if( ts->size() != 1 ) {
        psc_abort( "DVFSPlugin can't currently handle multiple TuningSpecifications\n" );
    }

    ts->front()->setALLRanks();

    list<PropertyRequest*>* objPropReqList = new list<PropertyRequest*>();

    objPropRequest = new PropertyRequest();
    objPropRequest->addPropertyID( ENERGY_CONSUMPTION );
    //objPropRequest->addPropertyID(EXECTIME);
    objPropRequest->addSingleProcess( 0 );
    //objPropRequest->addAllProcesses();
    objPropReqList->push_back( objPropRequest );
    objPropRequest->addRegion( appl->get_phase_region() );

    scenario->setPropertyRequests( objPropReqList );
    scenario->setTunedRegion( tuningRegion );

    pool_set->esp->push( scenario );
    pool_set->esp->print();
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DVFSPlugin: Added 1 scenario in the experiment.\n" );

    propRequest = new PropertyRequest();
    propRequest->addPropertyID( ENERGY_CONSUMPTION );
    propRequest->addPropertyID( EXECTIME );
    //propRequest->addSingleProcess(0);
    propRequest->addAllProcesses();

    suited_regions.sort();
    suited_regions.unique();
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DVFSPlugin: There are %d regions.\n", suited_regions.size() );

    for( it = suited_regions.begin(); it != suited_regions.end(); ++it ) {
        //regions from suited regions
        propRequest->addRegion( *it );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DVFSPlugin: Region %d added\n", i );
        ( *it )->print( true );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "\n" );
        i++;
    }

    propRequestList->push_back( propRequest );

    StrategyRequestGeneralInfo* strategyRequestGeneralInfo =
        new StrategyRequestGeneralInfo;
    strategyRequestGeneralInfo->strategy_name     = "ConfigAnalysis";
    strategyRequestGeneralInfo->pedantic          = 1;
    strategyRequestGeneralInfo->delay_phases      = 0;
    strategyRequestGeneralInfo->delay_seconds     = 0;
    strategyRequestGeneralInfo->analysis_duration = 1;

    *strategy = new StrategyRequest( propRequestList, strategyRequestGeneralInfo );
    ( *strategy )->printStrategyRequest();
    analysisRequired = true;
}


bool DVFSPlugin::restartRequired( std::string& env,
                                  int&         numprocs,
                                  std::string& command,
                                  bool&        is_instrumented ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DVFSPlugin: call to restartRequired()\n" );
    return false;
}


/**
 * @brief Method to know if the search procedure is finished or not.
 *
 * Calls the searchFinished method of the searchAlgorithm class and returns its
 * value, in case the csp pool still has scenarios.
 *
 * @ingroup DVFSPlugin
 */
bool DVFSPlugin::searchFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DVFSPlugin: call to searchFinished()\n" );

    if( !pool_set->csp->empty() ) {
        return false;
    }
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                "DVFSPlugin: Search is not yet finished (pool size: %d)\n",
                pool_set->csp->size() );

    return searchAlgorithm->searchFinished();
}


/**
 * @brief The method performs the final operation of a tuning step.
 *
 * This method does nothing since no post-processing is needed before
 * entering the next tuning iteration.
 *
 * @ingroup DVFSPlugin
 */
void DVFSPlugin::finishTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                "DVFSPlugin: call to processResults()\n" );
}


/**
 * @brief Returns true whether the plugin finished the tuning process.
 *
 * This method always returns true.
 *
 * @ingroup DVFSPlugin
 */
bool DVFSPlugin::tuningFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                "DVFSPlugin: call to tuningFinished()\n" );
    return true;
}


/** Prints the tuning advice at the end the whole tuning process. */
Advice* DVFSPlugin::getAdvice() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DVFSPlugin: call to getAdvice()\n" );

    std::map< int, std::list<MetaProperty> >                 arpExperiments;
    std::map< int, std::list<MetaProperty> >::const_iterator arpIterator;

    std::list<MetaProperty>           arpProperties, srpProperties;
    std::list<MetaProperty>::iterator arpProperty, srpIterator;
    int                               experimentId = 0, scenarioId = 0;

    std::map<TuningParameter*, int>           tp;
    std::map<TuningParameter*, int>::iterator tpit;

    // a map to organize the output with key: scenario-regionnameregionid
    std::map<std::string, outputDVFS*> outputValues;
    outputDVFS*                        outputElement;

    if( !searchAlgorithm ) {
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }

    arpExperiments = pool_set->arp->getAllExperimentProperties();
    for( arpIterator = arpExperiments.begin();
         arpIterator != arpExperiments.end(); arpIterator++ ) {
        experimentId  = arpIterator->first;
        arpProperties = arpIterator->second;
        for( arpProperty = arpProperties.begin(); arpProperty != arpProperties.end(); ( arpProperty++ )++ ) {      //2 properties
            scenarioId = atoi( arpProperty->getExtraInfo().at( "ScenarioID" ).c_str() );

            stringstream key;
            key << scenarioId << "-";

            key << arpProperty->getFileName() << ":"
                << arpProperty->getStartPosition();

            //find key
            if( outputValues.find( key.str() ) != outputValues.end() ) {
                //key found, get object.
                outputElement = outputValues[ key.str() ];
            }
            else {
                //key not found make new object.
                outputElement = new outputDVFS;
                //insert in map
                outputValues[ key.str() ] = outputElement;
                outputElement->energy     = 0;
                outputElement->energynode = 0;
                outputElement->time       = 0;
            }

            outputElement->scenarioId = scenarioId;
            outputElement->fileId     = arpProperty->getFileId();
            outputElement->rfl        = arpProperty->getRFL();
            stringstream ss2;
            ss2 << " " << arpProperty->getRegionType() << " ("
                << arpProperty->getFileName() << ":"
                << arpProperty->getStartPosition() << ")";
            outputElement->regionFileNameStartPos = ss2.str();

            const std::list<TuningSpecification*>* ts = pool_set->fsp->getTuningSpecificationByScenarioID( scenarioId );
            for( std::list<TuningSpecification*>::const_iterator tsit = ts->begin(); tsit != ts->end(); ++tsit ) {
                tp = ( *tsit )->getVariant()->getValue();
                for( tpit = tp.begin(); tpit != tp.end(); ++tpit ) {
                    // Governor tuning parameter
                    string setFrequencyNodeString;
                    string setGovNodeString;
                    if( set_freq_node ) {
                        setFrequencyNodeString = "FREQNODE";
                        setGovNodeString       = "GOVNODE";
                    }
                    else {
                        setFrequencyNodeString = "FREQCORE";
                        setGovNodeString       = "GOVCORE";
                    }
                    if( !tpit->first->getName().compare( 0, 7, setGovNodeString.c_str() ) ) {
                        switch( tpit->second ) {
                        case 0:
                            outputElement->governor = " Conservative  |";
                            break;
                        case 1:
                            outputElement->governor = " Ondemand      |";
                            break;
                        case 2:
                            outputElement->governor = " Performance   |";
                            break;
                        case 3:
                            outputElement->governor = " Powersave     |";
                            break;
                        case 4:
                            outputElement->governor = " Userspace     |";
                            break;
                        default:
                            cout << "DVFS ERROR:unrecognized governor!" << endl;
                            break;
                        }                         //switch
                    }

                    // Frequency tuning parameter
                    else if( !tpit->first->getName().compare( 0, 8, setFrequencyNodeString.c_str() ) ) {
                        outputElement->freq = 1200 + ( tpit->second * 100 );
                    }
                }         // for tpit
            }             // for tsit

            // Region name and location
            if( !arpProperty->getName().compare( "ExecTime" ) && arpProperty->getSeverity() > outputElement->time ) {
                outputElement->time = arpProperty->getSeverity();
            }

            if( !arpProperty->getName().compare( "Energy Consumption" ) ) {
                outputElement->energy += arpProperty->getSeverity();
            }

            if( !arpProperty->getName().compare( "Energy Consumption" ) && arpProperty->getSeverity() > outputElement->energynode ) {
                outputElement->energynode = arpProperty->getSeverity();
            }

            if( outputElement->time > 0 ) {
                outputElement->powernode   = outputElement->energynode / outputElement->time;
                outputElement->tco         = model_TCO( outputElement->powernode, outputElement->time, 1, 1 );
                outputElement->energydelay = outputElement->energy * outputElement->time;
            }
        } //for arpProperty
    }     //for arpIterator

    map<string, outputDVFS*> optimumFileIdRFL2Output;
    createEnoptAdvice( outputValues, optimumFileIdRFL2Output );

    //output
    cout << "AutoTune Results:" << endl;
    map<string, outputDVFS*>::iterator optIt;
    for( optIt = optimumFileIdRFL2Output.begin(); optIt != optimumFileIdRFL2Output.end(); ++optIt ) {
        cout << " Region id:" << optIt->second->regionFileNameStartPos;
        cout << "\t Optimum Scenario:" << optIt->second->scenarioId;
        cout << "\t Frequency: " << optIt->second->freq << endl;
    }
    cout << "Search Path:\n";
    cout
    << "Scenario  | Governor      | Freq (MHz)   | Energy (J)   | Runtime (s)  | EDP          | Region \n";
    cout
    << "----------+---------------+--------------+--------------+--------------+--------------+--------------------------------\n";
    for( map<string, outputDVFS*>::iterator outIt = outputValues.begin();
         outIt != outputValues.end(); ++outIt ) {
        printf( "%-10d|", outIt->second->scenarioId );
        cout << outIt->second->governor;
        printf( " %-12d |", outIt->second->freq );
        printf( " %-12.3f |", outIt->second->energy );
        printf( " %-12.3f |", outIt->second->time );
        printf( " %-12.3f |", outIt->second->time * outIt->second->energy );
        cout << outIt->second->regionFileNameStartPos << std::endl;
    }

    cout << "Other data:" << endl;
    cout << "Scenario  | Governor      | Freq (MHz)   | Avrg Power (W/node) | TCO (EUR/node)|  EDDP        | Region \n";
    cout << "----------+---------------+--------------+---------------------+---------------+--------------+-----------------------------\n";

    for( map<string, outputDVFS*>::iterator outIt = outputValues.begin(); outIt != outputValues.end(); ++outIt ) {
        printf( "%-10d|", outIt->second->scenarioId );
        cout << outIt->second->governor;
        printf( " %-12d |", outIt->second->freq );
        printf( " %-12.3f        |", outIt->second->powernode );
        printf( " %-12.3f  |", outIt->second->tco );
        printf( " %-12.3f |",    outIt->second->time * outIt->second->time * outIt->second->energy );
        cout << outIt->second->regionFileNameStartPos << std::endl;
    }


    //delete map
    for( map<string, outputDVFS*>::iterator outIt = outputValues.begin();
         outIt != outputValues.end(); ++outIt ) {
        delete outIt->second;
    }


    return new Advice( getName(),
                       ( *pool_set->fsp->getScenarios() )[ searchAlgorithm->getOptimum() ],
                       searchAlgorithm->getSearchPath(), "Energy",
                       pool_set->fsp->getScenarios() );
}


/**
 * @brief create enopt advice to implement on stand alone version of optimized code
 * This method produces a file containing the optimal frequencies per file id and region first line (rfl)
 * @param outputvalues
 * @param optimum
 */
void DVFSPlugin::createEnoptAdvice( const map<string, outputDVFS*>& outputvalues,
                                    map<string, outputDVFS*>&       optimum ) {
    if( model_method == MODEL_POWERCAPPING ) {
        optimizeForPower( outputvalues, optimum );
    }
    else {
        minimize( outputvalues, optimum );
    }
    //produced tab file
    //create advice file for enopt
    char filename[ 1000 ];
    sprintf( filename, "advice_enopt_%d.txt", ( int )getpid() );
    ofstream enoptAdvice( filename );
    if( !enoptAdvice ) {
        char errorMessage[ 1000 ];
        sprintf( errorMessage, "Can not create %s", filename );
        perror( errorMessage );
        return;
    }
    map<string, outputDVFS*>::const_iterator fileRFLit;
    for( fileRFLit = optimum.begin(); fileRFLit != optimum.end(); ++fileRFLit ) {
        enoptAdvice << fileRFLit->second->fileId << "\t" << fileRFLit->second->rfl << "\t" << ( fileRFLit->second->freq * 1000 ) << endl;
    }
    enoptAdvice.close();
}


/**
 * Optimizes to the highest power without trespassing the POWERCAP.
 * @param outputvalues all the values
 * @param optimum optimized scenario
 */
void DVFSPlugin::optimizeForPower( const map<string, outputDVFS*>& outputvalues,
                                   map<string, outputDVFS*>&       optimum ) {
    map<string, outputDVFS*>::const_iterator it;
    for( it = outputvalues.begin(); it != outputvalues.end(); ++it ) {
        stringstream key;
        key << it->second->fileId << "-" << it->second->rfl;
        //find key
        if( optimum.find( key.str() ) != optimum.end() ) {
            outputDVFS* currentOptimum = optimum[ key.str() ];
            double      power          = it->second->powernode;
            if( power <= POWERCAP && it->second->freq > currentOptimum->freq ) {
                //we found new optimum
                optimum[ key.str() ] = it->second;
            }
        }
        else {                   //key not found insert this object in map
            optimum[ key.str() ] = it->second;
        }
    }
}


void DVFSPlugin::minimize( const map<string, outputDVFS*>& outputvalues,
                           map<string, outputDVFS*>&       optimum ) {
    map<string, outputDVFS*>::const_iterator it;
    for( it = outputvalues.begin(); it != outputvalues.end(); ++it ) {
        stringstream key;
        key << it->second->fileId << "-" << it->second->rfl;
        //find key
        if( optimum.find( key.str() ) != optimum.end() ) {
            //key found, get object.
            double objFuncValueFromOutput = getObjectiveFunctionValue( it->second ); //for example energy
            double objFuncValueFromOpt    = getObjectiveFunctionValue( optimum[ key.str() ] );
            if( objFuncValueFromOutput < objFuncValueFromOpt ) {                     //minimizing condition
                                                                                     //replace with this object
                optimum[ key.str() ] = it->second;
            }
            else if( objFuncValueFromOutput == objFuncValueFromOpt ) { //check time
                                                                      //check time
                if( it->second->time < optimum[ key.str() ]->time ) {
                    optimum[ key.str() ] = it->second;
                }
            }
        }
        else {
            //key not found insert this object in map
            optimum[ key.str() ] = it->second;
        }
    }
}


/**
 * Depending on the objective function specified, the right data to be compared is returned.
 * @param dvfsData
 * @return the objective function value
 */
double DVFSPlugin::getObjectiveFunctionValue( outputDVFS* dvfsData ) {
    switch( model_method ) {
    case MODEL_ENERGY1:
        return dvfsData->energy;
    case MODEL_ENERGY2:
        return dvfsData->energy;
    case MODEL_ENERGYDELAY:
        return dvfsData->energydelay;
    case MODEL_TCO:
        return dvfsData->tco;
    case MODEL_POLICY1:
    case MODEL_POLICY2:
    case MODEL_POLICY3:
    case MODEL_POLICY4:
        return dvfsData->energy;
    default:
        return dvfsData->energy;
    }
}


/**
 * @brief Method to finalize the plugin normally.
 *
 * The method performs a normal finalization of the plugin, removing
 * any allocated memory, objects, file descriptors, etc. by calling the
 * terminate procedure.
 *
 * @ingroup DVFSPlugin
 */
void DVFSPlugin::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                "DVFSPlugin: call to finalize()\n" );
    terminate();
}


/**
 * @brief Standard termination of the plugi
 *
 * This method performs a safely remove of any allocated memory, objects,
 * file descriptors, etc. It also unloads the search algorithm.
 *
 * @ingroup DVFSPlugin
 */
void DVFSPlugin::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                "DVFSPlugin: call to terminate()\n" );
    if( searchAlgorithm ) {
        searchAlgorithm->finalize();
        delete searchAlgorithm;
    }

    context->unloadSearchAlgorithms();
}


/**
 * Model Predictions based on energy, time (performance), and power.
 * @param cycles		cycles per second
 * @param instructions	instructions per second
 * @param cache2		cache2 per second
 * @param cache3		cache3 per second
 * @param eRef			reference energy
 * @param tRef			reference time
 * @param pwrRef		reference power
 * @param outvec
 * @param optFreq
 * @param optval
 * @return returns the optimal index from the range of frequencies
 */
int DVFSPlugin::model_prediction( float   cycles,
                                  float   instructions,
                                  float   cache2,
                                  float   cache3,
                                  float   eRef,
                                  float   tRef,
                                  float   pwrRef,
                                  float** outvec,
                                  float&  optFreq,
                                  float&  optval ) {
    if( !define_params ) {
        defineParams();
        define_params = true;
    }
    bool deleteoutvec = false;
    if( outvec == NULL ) {
        //if NULL it will only be used internally
        outvec = new float*[ NUMBER_OF_FREQ ];
        for( int j = 0; j < NUMBER_OF_FREQ; ++j ) {
            outvec[ j ] = new float[ 4 ];
        }
        deleteoutvec = true;
    }

    int   iref = NUMBER_OF_FREQ;
    float xmin = 1.e99;

    for( int i = 0; i < NUMBER_OF_FREQ; ++i ) {
        if( abs( fRef - frequency[ i ] ) < xmin ) {
            iref = i;
            xmin = abs( fRef - frequency[ i ] );
        }
    }

    float invec[ NUMBER_OF_COEF ];
    //! Influcence of Frequency
    //! All benchmarks are performec as Giga! therefore convert
    //! remove this in later version
    invec[ 0 ] = 1.0;
    //! Influcence of INSTRUCTIONS
    invec[ 1 ] = instructions * 1.E-9;
    invec[ 2 ] = 1. / ( instructions * 1.E-9 );
    //! Influence of CPI
    invec[ 3 ] = cycles / instructions;
    invec[ 4 ] = instructions / cycles;
    //! Influence of Cache
    invec[ 5 ] = cache3 * 1.E-9;
    invec[ 6 ] = cache3 / instructions;
    invec[ 7 ] = cache2 * 1.E-9;
    invec[ 8 ] = cache2 / instructions;
    //! Dummy for later usage
    invec[ 9 ]  = 0.;
    invec[ 10 ] = 0.;
    cout
    << "Frequency         (Fact)Time      (Fact)Ener      (Fact)Perf       (Fact)PWR       CostFunct"
    << endl;
    for( int i = 0; i < NUMBER_OF_FREQ; ++i ) {
        float ciref[ NUMBER_OF_FREQ ];
        for( int t = 0; t < NUMBER_OF_COEF; ++t ) {
            ciref[ t ] = coeffEnergy[ t ][ i ][ iref ];
        }
        outvec[ i ][ 0 ] = model_compute( ciref, energyMin[ i ][ iref ], energyMax[ i ][ iref ], invec );
        for( int t = 0; t < NUMBER_OF_COEF; ++t ) {
            ciref[ t ] = coeffPerformance[ t ][ i ][ iref ];
        }
        outvec[ i ][ 1 ] = model_compute( ciref, performanceMin[ i ][ iref ], performanceMax[ i ][ iref ], invec );
        for( int t = 0; t < NUMBER_OF_COEF; ++t ) {
            ciref[ t ] = coeffPower[ t ][ i ][ iref ];
        }
        outvec[ i ][ 2 ] = model_compute( ciref, powerMin[ i ][ iref ], powerMax[ i ][ iref ], invec );

        switch( model_method ) {
        case ( MODEL_ENERGY1 ):
            outvec[ i ][ 3 ] = model_energy1( eRef, outvec[ i ][ 0 ] );
            break;
        case ( MODEL_ENERGY2 ):
            outvec[ i ][ 3 ] = model_energy2( pwrRef, tRef, outvec[ i ][ 2 ],
                                              1. / outvec[ i ][ 1 ] );
            break;
        case ( MODEL_ENERGYDELAY ):
            outvec[ i ][ 3 ] = model_delay( pwrRef, tRef, outvec[ i ][ 2 ],
                                            1. / outvec[ i ][ 1 ], 2 );
            break;
        case ( MODEL_TCO ):
            outvec[ i ][ 3 ] = model_TCO( pwrRef, tRef, outvec[ i ][ 2 ],
                                          1. / outvec[ i ][ 1 ] );
            break;
        case ( MODEL_POWERCAPPING ):
            outvec[ i ][ 3 ] = model_powercapping( pwrRef, outvec[ i ][ 2 ] );
            break;
        case ( MODEL_POLICY1 ):
            outvec[ i ][ 3 ] = model_policy1( frequency[ iref ], frequency[ i ],
                                              outvec[ i ][ 0 ], outvec[ i ][ 1 ] );
            break;
        case ( MODEL_POLICY2 ):
            outvec[ i ][ 3 ] = model_policy2( frequency[ iref ], frequency[ i ],
                                              outvec[ i ][ 2 ], outvec[ i ][ 1 ] );
            break;
        case ( MODEL_POLICY3 ):
            outvec[ i ][ 3 ] = model_policy3( pwrRef, outvec[ i ][ 2 ], outvec[ i ][ 1 ] );
            break;
        case ( MODEL_POLICY4 ):
            outvec[ i ][ 3 ] = model_policy4( outvec[ i ][ 1 ], frequency[ i ] );
            break;
        default:
            cout << "WARNING: Energy Cost Model not defined using ENOPT_MODEL_ENERGY1" << endl;
            outvec[ i ][ 3 ] = model_energy1( eRef, outvec[ i ][ 0 ] );
            break;
        }
        if( 1 ) {        //for debugging purposes
            printf( " %-12.10f  ", frequency[ i ] );
            printf( " %-12.10f ", 1. / outvec[ i ][ 1 ] );
            printf( " %-12.10f ", outvec[ i ][ 0 ] );
            printf( " %-12.10f ", outvec[ i ][ 1 ] );
            printf( " %-12.10f ", outvec[ i ][ 2 ] );
            printf( " %-12.10f \n", outvec[ i ][ 3 ] );
        }
    }
    int optIndx = NUMBER_OF_FREQ - 1;
    optval = outvec[ optIndx ][ 3 ];
    for( int i = NUMBER_OF_FREQ - 2; i >= 0; --i ) {
        if( outvec[ i ][ 3 ] < optval ) {
            optval  = outvec[ i ][ 3 ];
            optIndx = i;
        }
    }
    optFreq = frequency[ optIndx ];
    optval  = outvec[ optIndx ][ 3 ];
    if( deleteoutvec ) {
        for( int j = 0; j < NUMBER_OF_FREQ; ++j ) {
            delete[] outvec[ j ];
        }
        delete[] outvec;
    }
    return optIndx;
}


float DVFSPlugin::model_compute( float* c,
                                 float  ymin,
                                 float  ymax,
                                 float* invec ) {
    float y = 0;
    float x = 0;
    for( int i = 0; i < NUMBER_OF_COEF; ++i ) {
// safeguard input
        x = invec[ i ];       //5
        if( invec[ i ] < xminval[ i ] ) {
            x = xminval[ i ];
        }
        if( invec[ i ] > xmaxval[ i ] ) {
            x = xmaxval[ i ];
        }
        y = y + c[ i ] * invec[ i ];     // x;
    }

// safeguard output
    if( y < ymin ) {
        y = ymin;
    }
    if( y > ymax ) {
        y = ymax;
    }
    return y;
}


/**
 * Energy model using only energy
 * @param eRef
 * @param factor
 * @return
 */
float DVFSPlugin::model_energy1( float eRef,
                                 float factor ) {
    return eRef * factor;
}


/**
 * Energy model using power * time
 * @param pwrRef
 * @param tRef
 * @param factor1
 * @param factor2
 * @return
 */
float DVFSPlugin::model_energy2( float pwrRef,
                                 float tRef,
                                 float factor1,
                                 float factor2 ) {
    return pwrRef * factor1 * tRef * factor2;
}


/**
 * EDDP
 * @param pwrRef
 * @param tRef
 * @param factor1
 * @param factor2
 * @param iexp
 * @return
 */
float DVFSPlugin::model_delay( float pwrRef,
                               float tRef,
                               float factor1,
                               float factor2,
                               int   iexp ) {
// ! ED =             PWR            *T            *T
    return pwrRef * factor1 * pow( tRef * factor2, iexp );
}


/**
 * Total Cost of Ownership
 * Uses:  0.18 Cent/Kwh and 50 Mio/System/5Jahre/Node
 *              Preis pro wATT*SEC = J
 * @param pwrRef
 * @param tRef
 * @param factor1
 * @param factor2
 * @return
 */
float DVFSPlugin::model_TCO( float pwrRef,
                             float tRef,
                             float factor1,
                             float factor2 ) {
    float KWh          = 0.18 / 3600. / 1000. * 1.5;
    float abschreibung = 45.E6 / ( 5. * 24. * 365 * 3600. ) / ( 18. * 512 );

    return ( KWh * pwrRef * factor1 * tRef * factor2
             + abschreibung * tRef * factor2 ) * 3600.;
}


/**
 * Power capping
 * @param pwrRef
 * @param factor1
 * @return
 */
float DVFSPlugin::model_powercapping( float pwrRef,
                                      float factor1 ) {
    float pow = pwrRef * factor1;
    return pow > POWERCAP ? pow : POWERCAP;
}


/**
 * Model policy 1: Increase of Performance > increase of Energy
 * @param freq1
 * @param freq2
 * @param factor1
 * @param factor2
 * @return
 */
float DVFSPlugin::model_policy1( float freq1,
                                 float freq2,
                                 float factor1,
                                 float factor2 ) {
    float enopt_model_policy1 = 0.;
    if( factor2 < factor1 ) {
        enopt_model_policy1 = 1.;
    }
    if( freq2 - freq1 < 1.E-3 ) {
        enopt_model_policy1 = 0.;
    }
    return enopt_model_policy1;
}


/**
 * Model policy 2: Increase of Performance > increase of Power
 * @param freq1
 * @param freq2
 * @param factor1
 * @param factor2
 * @return
 */
float DVFSPlugin::model_policy2( float freq1,
                                 float freq2,
                                 float factor1,
                                 float factor2 ) {
    float enopt_policy2 = 0.;
    if( factor2 < factor1 ) {
        enopt_policy2 = 1.;
    }
    if( freq2 - freq1 < 1.E-3 ) {
        enopt_policy2 = 0.;
    }
    return enopt_policy2;
}


/**
 * Model policy 3: stay below powercap or have significant Performance increase
 * @param pwrRef
 * @param factor1
 * @param factor2
 * @return
 */
float DVFSPlugin::model_policy3( float pwrRef,
                                 float factor1,
                                 float factor2 ) {
    return ( pwrRef * factor1 < POWERCAP || factor2 > 1.1 ) ? 0 : 1;
}


/**
 * Stay below powercap or have significant Performance increase
 * @param factor1
 * @param Freq
 * @return
 */
float DVFSPlugin::model_policy4( float factor1,
                                 float Freq ) {
    return factor1 > 0.9 ? Freq : 1.e33;
}


/**
 * The following are C functions that belong to the Plugin Management Interface, and not to the
 * IPlugin class.  Currently the interface includes information and instance creation functions.
 *
 * These are defined in PluginManagement.h .
 */

/**
 * @brief Returns an instance of the DVFS plugin.
 *
 * This methods call the DVFS constructor method.
 *
 * @ingroup DVFSPlugin
 *
 * @return An instance of the DVFS plugin.
 */
IPlugin* getPluginInstance( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                "DVFSPlugin: call to getComponentInstance()\n" );
    return new DVFSPlugin();
}


/**
 * @brief Returns the major plugin interface version used by this plugin.
 *
 * Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 *
 * @ingroup DVFSPlugin
 *
 * @return The major plugin interface version used by this plugin.
 */
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                "DVFSPlugin: call to getInterfaceVersionMajor()\n" );
    return DVFS_VERSION_MAJOR;
}


/**
 * @brief Returns the minor plugin interface version used by this plugin.
 *
 * Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 *
 * @ingroup DVFSPlugin
 *
 * @return The minor plugin interface version used by this plugin.
 */
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                "DVFSPlugin: call to getInterfaceVersionMinor()\n" );
    return DVFS_VERSION_MINOR;
}


/**
 * @brief Returns the particular name of the plugin.
 *
 * Returns a string with the name of the plugin "DVFS Plugin".
 *
 * @ingroup DVFSPlugin
 *
 * @return The particular name of the plugin.
 *
 */
string getName( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                "DVFSPlugin: call to getName()\n" );
    return "DVFS Plugin";
}


/**
 * @brief Returns the description of the DVFS plugin.
 *
 * Returns a string with the short description of the DVFS plugin: "Energy plugin for AutoTune".
 *
 * @ingroup DVFSPlugin
 *
 * @return The description of the DVFS plugin.
 */
string getShortSummary( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                "DVFSPlugin: call to getShortSummary()\n" );
    return "Energy plugin for Autotune";
}
