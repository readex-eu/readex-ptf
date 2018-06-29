#include "EnergyPlugin.h"
#include "search_common.h"




const std::string EnergyPlugin::ENERGY = "Energy";
const std::string EnergyPlugin::TIME   = "Time";


EnergyPlugin::EnergyPlugin() : app( Application::instance() ), numberOfProcsTP( NULL ), numberOfThreadsTP( NULL ), frequencyTP( NULL ), searchAlgorithm( NULL ), nextScenarioNumProcs( -1 ) {
}


/*
 * Initialize the plugin's data structures.
 *
 * The tuning parameter list needs to be created.
 *
 * Search algorithms are loaded here when required. This can be done as follows:
 *
 *    searchAlgorithm = loadSearchAlgorithm("name");
 *
 * where "name" refers to one of the available search algorithms (currently only exhaustive).
 */
void EnergyPlugin::initialize( DriverContext*   context,
                               ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ENERGY: call to initialize()\n" );
    this->context  = context;
    this->pool_set = pool_set;

    parseCPUFreqs();

    numberOfProcsTP = new TuningParameter();
    numberOfProcsTP->setId( 0 );
    numberOfProcsTP->setName( "NUMPROCS" );
    numberOfProcsTP->setPluginType( Energy );
    numberOfProcsTP->setRange( 1, context->getMPINumProcs(), 1 );
    numberOfProcsTP->setRuntimeActionType( TUNING_ACTION_NONE );

    numberOfThreadsTP = new TuningParameter();
    numberOfThreadsTP->setId( 1 );
    numberOfThreadsTP->setName( "NUMTHREADS" );
    numberOfThreadsTP->setPluginType( Energy );
    numberOfThreadsTP->setRange( 1, context->getOmpnumthreads(), 1 );
    numberOfThreadsTP->setRuntimeActionType( TUNING_ACTION_FUNCTION_POINTER );

    frequencyTP = new TuningParameter();
    frequencyTP->setId( 2 );
    frequencyTP->setName( "CPU_FREQ" );
    frequencyTP->setPluginType( Energy );
    frequencyTP->setRange( minFreq, maxFreq, freqStep );
    frequencyTP->setRuntimeActionType( TUNING_ACTION_FUNCTION_POINTER );
}


/*
 * Defines a pre-analysis strategy.
 *
 * Found properties of the pre-analysis strategy are stored in the arp.
 */
bool EnergyPlugin::analysisRequired( StrategyRequest** ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ENERGY: call to analysisRequired()\n" );
    return false;
}


/*
 * Operations to be done at the start of a tuning step.
 *
 * The tuning parameters that were created at initialization are used to create a variant space.
 * The variant space and the regions are then used to create a search space that is passed to the
 * search algorithm.
 */
void EnergyPlugin::startTuningStep() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ENERGY: call to startTuningStep()\n" );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ENERGY: Create a SearchSpace from the tuning parameters.\n" );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "obtain getSearchInstance\n" );

    int                major, minor;
    std::string        name, description;

    char const* selected_search = getenv( "PSC_SEARCH_ALGORITHM" );
    if( selected_search != NULL ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "User specified search algorithm: %s\n", selected_search );
        std::string selected_search_string = std::string( selected_search );
        context->loadSearchAlgorithm( selected_search_string, &major, &minor, &name, &description );
        searchAlgorithm = context->getSearchAlgorithmInstance( selected_search_string );
    }
    else {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Selecting default search algorithm: exhaustive\n" );
        context->loadSearchAlgorithm( "exhaustive", &major, &minor, &name, &description );
        searchAlgorithm = context->getSearchAlgorithmInstance( "exhaustive" );
        searchAlgorithm->addObjectiveFunction(new EDPObjective(""));
    }

    if( searchAlgorithm ) {
        print_loaded_search( major, minor, name, description );
        searchAlgorithm->initialize( context, pool_set );
    }
    else {
        perror( "NULL pointer in searchAlgorithm\n" );
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }

    Region* phase = app.get_phase_region();
    if( !phase ) {
        perror( "Phase region not specified, cannot begin tuning!\n" );
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "searchAlgorithm instance obtained\n" );

    // search space for number of MPI tasks
    VariantSpace* variantSpace = new VariantSpace();
    variantSpace->addTuningParameter( numberOfProcsTP );
    SearchSpace*  searchspace  = new SearchSpace();
    searchspace->setVariantSpace( variantSpace );
    searchspace->addRegion( phase );
    searchAlgorithm->addSearchSpace( searchspace );

    // search space for number of OMP threads
    variantSpace = new VariantSpace();
    variantSpace->addTuningParameter( numberOfThreadsTP );
    searchspace  = new SearchSpace();
    searchspace->setVariantSpace( variantSpace );
    searchspace->addRegion( phase );
    searchAlgorithm->addSearchSpace( searchspace );

    // search space for CPU frequency
    variantSpace = new VariantSpace();
    variantSpace->addTuningParameter( frequencyTP );
    searchspace  = new SearchSpace();
    searchspace->setVariantSpace( variantSpace );
    searchspace->addRegion( phase );
    searchAlgorithm->addSearchSpace( searchspace );
}


/*
 * The Created Scenario Pool (csp) is populated here.
 *
 * The scenarios need to be created and added to the first pool. To create the scenarios, a
 * search algorithm can be used or it can be done directly by the plugin.
 *
 * After this step, the Periscope will verify that scenarios were added to the csp.
 */
void EnergyPlugin::createScenarios() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ENERGY: call to createScenarios()\n" );
    if( !searchAlgorithm )
        psc_abort( "Search algorithm not instantiated\n" );

    searchAlgorithm->createScenarios();
}


void EnergyPlugin::prepareScenarios() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ENERGY: call to prepareScenarios()\n" );

    // Here, we select the next scenarios from the "created scenario pool" to execute, by moving them to the "prepared
    // scenario pool".
    if( !pool_set->csp->empty() ) {
        Scenario* scenario;
        scenario = pool_set->csp->pop();
        pool_set->psp->push( scenario );
    }
}


/*
 * Populate the Experiment Scenario Pool (esp) for the next experiment.
 *
 * This is the final step before the experiments are executed. Scenarios are moved from the
 * psp to the esp, depending on the number of processes and whether they can be executed
 * in parallel.
 *
 * After this step, the Periscope will verify that scenarios were added to the esp.
 */
void EnergyPlugin::defineExperiment( int               numprocs,
                                     bool&             analysisRequired,
                                     StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ENERGY: call to defineExperiment()\n" );

    Scenario* scenario = NULL;
    for( int i = 0; !pool_set->psp->empty()   &&   i < numprocs; i++ ) {
        scenario = pool_set->psp->pop();

        std::list<PropertyRequest*>* propertyRequestList = new std::list<PropertyRequest*>();
        std::list<int>*              propertyIds         = new std::list<int>();

        propertyIds->push_back( HDEEM_ENERGY_CONSUMTION_BLADE );
        propertyIds->push_back( EXECTIME );
        propertyRequestList->push_back( new PropertyRequest( propertyIds ) );
        scenario->setPropertyRequests( propertyRequestList );
        scenario->setTunedRegion( app.get_phase_region() );

        pool_set->esp->push( scenario );
    }

    StrategyRequestGeneralInfo* info = new StrategyRequestGeneralInfo();
    info->strategy_name     = "OMP";
    info->pedantic          = 1;
    info->delay_phases      = 0;
    info->delay_seconds     = 0;
    info->analysis_duration = 1;
    *strategy               = NULL;

    assert( scenario );
    TuningSpecification& spec = *scenario->getTuningSpecifications()->front();
    nextScenarioNumProcs = spec.getVariant()->getValue().begin()->second;
}


bool EnergyPlugin::restartRequired( std::string& env,
                                    int&         numprocs,
                                    std::string& command,
                                    bool&        is_instrumented ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ENERGY: call to restartRequired()\n" );

    if( numprocs != nextScenarioNumProcs ) {
        numprocs = nextScenarioNumProcs;
        return true;
    }
    else
        return false;
}


bool EnergyPlugin::searchFinished() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ENERGY: call to searchFinished()\n" );
    return searchAlgorithm->searchFinished();
}


void EnergyPlugin::finishTuningStep() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ENERGY: call to finishTuningStep()\n" );
}


bool EnergyPlugin::tuningFinished() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ENERGY: call to tuningFinished()\n" );
    return true;
}


Advice* EnergyPlugin::getAdvice() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ENERGY: call to getAdvice()\n" );
    if( searchAlgorithm == NULL ) {
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }

    const int num_scenarios = pool_set->fsp->size();
    for( int scenario_id = 0; scenario_id != num_scenarios; ++scenario_id ) {
        Scenario* scenario = ( *pool_set->fsp->getScenarios() )[ scenario_id ];

        // accumulate the individual property values from each agent
        std::list<MetaProperty> properties = pool_set->srp->getScenarioResultsByID( scenario_id );
        for( MetaProperty& property: properties ) {
            if( atoi( property.getId().c_str() ) == HDEEM_ENERGY_CONSUMTION_BLADE ) {
                double sum;
                bool   was_set = scenario->getResult( ENERGY, &sum );
                scenario->addResult( ENERGY, ( was_set ? sum : 0.0 ) + property.getSeverity() );
            }
            if( atoi( property.getId().c_str() ) == EXECTIME ) {
                double maximum;
                bool   was_set = scenario->getResult( TIME, &maximum );
                double current = property.getSeverity();
                if( !was_set   ||   current > maximum ) {
                    scenario->addResult( TIME, current );
                }
            }
        }

        // calculate derived EDP property
        double EDP = objectiveFunction_EDP( scenario_id, pool_set->srp );
        scenario->addResult( "EDP", EDP );
    }

    writeResults();

    return new Advice( getName(), ( *pool_set->fsp->getScenarios() )[ searchAlgorithm->getOptimum() ], searchAlgorithm->getSearchPath(), "Time",
                       pool_set->fsp->getScenarios() );
}


void EnergyPlugin::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ENERGY: call to finalize()\n" );
    cleanup();
}


void EnergyPlugin::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ENERGY: call to terminate()\n" );
    cleanup();
}


void EnergyPlugin::cleanup() {
    if( searchAlgorithm ) {
        searchAlgorithm->finalize();
        delete searchAlgorithm;
    }

    context->unloadSearchAlgorithms();
}


/*
 * The following are C functions that belong to the Plugin Management Interface, and not to the
 * IPlugin class. Currently the interface includes information and instance creation functions.
 *
 * These are defined in PluginManagement.h .
 */

/**
 * @brief Returns an instance of this particular plugin implementation.
 * @ingroup EnergyPlugin
 * @return A pointer to an instance of this particular plugin implementation.
 */
IPlugin* getPluginInstance() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ENERGY: call to getComponentInstance()\n" );
    return new EnergyPlugin();
}


/**
 * @brief Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 * @ingroup EnergyPlugin
 * @return The major plugin interface version used by the plugin
 */
int getVersionMajor() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ENERGY: call to getInterfaceVersionMajor()\n" );
    return 1;
}


/**
 * @brief Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 * @ingroup EnergyPlugin
 * @return The minor plugin interface version used by the plugin
 */
int getVersionMinor() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ENERGY: call to getInterfaceVersionMinor()\n" );
    return 0;
}


/**
 * @brief Returns a string with the name of the plugin.
 * @ingroup EnergyPlugin
 * @return A string with the name of the plugin
 */
std::string getName() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ENERGY: call to getName()\n" );
    return "Energy plugin";
}


/**
 * @brief Returns a string with a short description of the plugin.
 * @ingroup EnergyPlugin
 * @return A string with a short description of the plugin
 */
std::string getShortSummary() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ENERGY: call to getShortSummary()\n" );
    return "Tunes the Energy-Delay-Product of parallel applications by systematically trying out a varying number of MPI tasks, threads and CPU frequencies.";
}


void EnergyPlugin::parseCPUFreqs() {
    const int argc = context->getArgc();
    char** const argv = context->getArgv();

    int c;
    optind = 1; // ptf parses opts with get_opt as well. So set optind to default, to avoid trouble
    while( true ) {
        int option_index = 0;
        static struct option long_options[] = {
                {"min_freq",   required_argument, 0,  0 },
                {"max_freq",   required_argument, 0,  1 },
                {"freq_step",  required_argument, 0,  2 },
                {0,            0,                 0,  0 }
        };

        c = getopt_long(argc, argv, "",
                        long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 0:
                minFreq = atoi(optarg);
                break;
            case 1:
                maxFreq = atoi(optarg);
                break;
            case 2:
                freqStep = atoi(optarg);
                break;
            case '?':
                break;
            default:
                break;
        }
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ENERGY: Frequencies will be checked in the interval [%i, %i], step size %i\n", minFreq, maxFreq, freqStep );
}


void EnergyPlugin::writeResults() {
    int num_scenarios = pool_set->fsp->size();

    std::cout << "\nAutoTune Results:" << endl;
    std::cout << "-----------------------\n\n";
    std::cout << "Scenario    | Processes | Threads |   Frequency | EDP           | Energy (mJ)   | Execution Time (sec)"  <<  std::endl;

    std::ofstream result_file( "energy_results.csv" );
    if( !result_file ) {
        psc_errmsg( "ENERGY: Unable to create the results file. No results will be written.\n" );
    }
    result_file << "Scenario,Processes,Threads,Frequency,EDP,Energy,Time"  <<  std::endl;

    for( int scenario_id = 0; scenario_id != num_scenarios; ++scenario_id ) {
        std::cout << std::setw(4) << scenario_id << ' ';
        result_file << scenario_id << ",";

        Scenario* scenario = (*pool_set->fsp->getScenarios())[scenario_id];
        if( !scenario )
            psc_abort("Error: Found a nullptr scenario!\n");

        assert( scenario );
        writeResults(result_file, *scenario);
        std::cout << std::endl;
        result_file << std::endl;
    }

    std::cout << std::endl;
}


void EnergyPlugin::writeResults(std::ofstream& resultFile, Scenario& scenario) {
    // check preconditions
    if( scenario.getTuningSpecifications()->size() != 3 ) {
        int size = static_cast<int>( scenario.getTuningSpecifications()->size());
        psc_errmsg("ENERGY: Unexpected number of tuning specifications: %i\n", size);
        abort();
    }

    // print the 3 tuning specifications (processes, threads, frequency)
    for( TuningSpecification* ts: *scenario.getTuningSpecifications() ) {
        if( !ts )
            psc_abort("Error: nullptr tuning specification detected.\n");

        assert(ts);
        writeResults(resultFile, scenario, *ts);
    }

    // print the 3 results (EDP, energy, time)
    writeResults(resultFile,scenario, scenario.getResults());
}


void EnergyPlugin::writeResults(std::ofstream& resultFile, Scenario& scenario, TuningSpecification& ts) {
    // check if there is exactly 1 tuning variant
    if( ts.getVariant()->getValue().size() != 1 ) {
        int size = static_cast<unsigned int>( ts.getVariant()->getValue().size() );
        psc_errmsg( "ENERGY: Unexpected number of tuning parameters: %i\n", size );
        abort();
    }

    // print the value of the variant
    int variantValue = ts.getVariant()->getValue().begin()->second;
    std::cout << std::setw( 14 ) << variantValue;
    resultFile << variantValue << ",";
}


void EnergyPlugin::writeResults(std::ofstream& resultFile, Scenario& scenario, const std::map<std::string, double>& results) {
    if( results.size() != 3 )
        psc_abort("ENERGY: Unexpected number of results!");

    for( auto i = results.begin(); true;  ) {
        std::cout << std::setw( 16 ) << i->second;
        resultFile << std::setiosflags( ios::fixed ) << i->second;
        if( ++i == results.end() )
            break;

        resultFile << ",";
    }
}


double EnergyPlugin::objectiveFunction_EDP(int scenario_id, ScenarioResultsPool* srp) {
    static double normalizationBase = std::numeric_limits<double>::min();

    std::list<MetaProperty> properties = srp->getScenarioResultsByID( scenario_id );
    double                  energy     = 0;
    double                  time       = 0;

    for( MetaProperty& property: properties ) {
        if( atoi( property.getId().c_str() ) == HDEEM_ENERGY_CONSUMTION_BLADE ) {
            energy += property.getSeverity();
        }
        else if( atoi( property.getId().c_str() ) == EXECTIME ) {
            double current = property.getSeverity();
            if( current > time ) {
                time = current;
            }
        }
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "EDP: %f;\n", energy * time );
    double result = energy * time;
    if( normalizationBase == std::numeric_limits<double>::min() )
        normalizationBase = result;

    return result / normalizationBase;
}
