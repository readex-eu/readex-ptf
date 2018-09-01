#include "MPICAPPlugin.h"
#include "search_common.h"
#include <boost/foreach.hpp>
#include <fstream>
#include <iostream>




const std::string MPICAPPlugin::ENERGY = "Energy";
const std::string MPICAPPlugin::TIME   = "Time";


MPICAPPlugin::MPICAPPlugin() : app( Application::instance() ), numberOfProcsTP( NULL ), searchAlgorithm( NULL ), nextScenarioNumProcs( -1 ) {
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
void MPICAPPlugin::initialize( DriverContext*   context,
                               ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPICAP: call to initialize()\n" );
    this->context  = context;
    this->pool_set = pool_set;

    numberOfProcsTP = new TuningParameter();
    numberOfProcsTP->setId( 0 );
    numberOfProcsTP->setName( "NUM_MPI_PROCS" );
    numberOfProcsTP->setPluginType( MPICAP );
    numberOfProcsTP->setRange( 1, context->getMPINumProcs(), 1 );
    numberOfProcsTP->setRuntimeActionType( TUNING_ACTION_NONE );

    std::string results = numberOfProcsTP->toString();
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPICAP: Before exiting to initialize, string is: %s\n\n", results.c_str() );
}


/*
 * Defines a pre-analysis strategy.
 *
 * Found properties of the pre-analysis strategy are stored in the arp.
 */
bool MPICAPPlugin::analysisRequired( StrategyRequest** ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPICAP: call to analysisRequired()\n" );
    return false;
}


/*
 * Operations to be done at the start of a tuning step.
 *
 * The tuning parameters that were created at initialization are used to create a variant space.
 * The variant space and the regions are then used to create a search space that is passed to the
 * search algorithm.
 */
void MPICAPPlugin::startTuningStep() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPICAP: call to startTuningStep()\n" );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPICAP: Create a SearchSpace from the tuning parameters.\n" );
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
    SearchSpace*  searchspace  = new SearchSpace();
    VariantSpace* variantSpace = new VariantSpace();
    variantSpace->addTuningParameter( numberOfProcsTP );
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
void MPICAPPlugin::createScenarios() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPICAP: call to createScenarios()\n" );
    if( !searchAlgorithm ) {
        perror( "Search algorithm not instantiated\n" );
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }

    searchAlgorithm->createScenarios();
}


void MPICAPPlugin::prepareScenarios() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPICAP: call to prepareScenarios()\n" );

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
void MPICAPPlugin::defineExperiment( int               numprocs,
                                     bool&             analysisRequired,
                                     StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPICAP: call to defineExperiment()\n" );

    Scenario* scenario = NULL;
    //TODO: We only have one scenario at a time. We should remove this for loop. -RM
    for( int i = 0; !pool_set->psp->empty()   &&   i < numprocs; i++ ) {
        scenario = pool_set->psp->pop();

        std::list<PropertyRequest*>* propertyRequestList = new list<PropertyRequest*>();
        std::list<int>*              propertyIds         = new std::list<int>();

        propertyIds->push_back( ENERGY_CONSUMPTION );
        propertyIds->push_back( EXECTIME );
        propertyRequestList->push_back( new PropertyRequest( propertyIds ) );
        scenario->setPropertyRequests( propertyRequestList );
        scenario->setTunedRegion( app.get_phase_region() );

        pool_set->esp->push( scenario );
    }

    StrategyRequestGeneralInfo* strategyRequestGeneralInfo = new StrategyRequestGeneralInfo();
    strategyRequestGeneralInfo->strategy_name     = "OMP";
    strategyRequestGeneralInfo->pedantic          = 1;
    strategyRequestGeneralInfo->delay_phases      = 0;
    strategyRequestGeneralInfo->delay_seconds     = 0;
    strategyRequestGeneralInfo->analysis_duration = 1;
    *strategy                                     = NULL;

    TuningSpecification& spec = *scenario->getTuningSpecifications()->front();
    nextScenarioNumProcs = spec.getVariant()->getValue().begin()->second;
}


bool MPICAPPlugin::restartRequired( std::string& env,
                                    int&         numprocs,
                                    std::string& command,
                                    bool&        is_instrumented ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPICAP: call to restartRequired()\n" );
    numprocs = nextScenarioNumProcs;
    return true;
}


bool MPICAPPlugin::searchFinished() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPICAP: call to searchFinished()\n" );
    return searchAlgorithm->searchFinished();
}


void MPICAPPlugin::finishTuningStep() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPICAP: call to finishTuningStep()\n" );
}


bool MPICAPPlugin::tuningFinished() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPICAP: call to tuningFinished()\n" );
    return true;
}


Advice* MPICAPPlugin::getAdvice() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPICAP: call to getAdvice()\n" );
    if( searchAlgorithm == NULL ) {
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }

    const int num_scenarios = pool_set->fsp->size();
    for( int scenario_id = 0; scenario_id != num_scenarios; ++scenario_id ) {
        Scenario* scenario = ( *pool_set->fsp->getScenarios() )[ scenario_id ];

        // accumulate the individual property values from each agent
        std::list<MetaProperty> properties = pool_set->srp->getScenarioResultsByID( scenario_id );
        BOOST_FOREACH( MetaProperty & property, properties ) {
            if( atoi( property.getId().c_str() ) == ENERGY_CONSUMPTION ) {
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


void MPICAPPlugin::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPICAP: call to finalize()\n" );
    cleanup();
}


void MPICAPPlugin::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPICAP: call to terminate()\n" );
    cleanup();
}


void MPICAPPlugin::cleanup() {
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
 * @ingroup MPICAPPlugin
 * @return A pointer to an instance of this particular plugin implementation.
 */
IPlugin* getPluginInstance() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPICAP: call to getComponentInstance()\n" );
    return new MPICAPPlugin();
}


/**
 * @brief Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 * @ingroup MPICAPPlugin
 * @return The major plugin interface version used by the plugin
 */
int getVersionMajor() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPICAP: call to getInterfaceVersionMajor()\n" );

    return MPICAP_VERSION_MAJOR;
}


/**
 * @brief Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 * @ingroup MPICAPPlugin
 * @return The minor plugin interface version used by the plugin
 */
int getVersionMinor() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPICAP: call to getInterfaceVersionMinor()\n" );

    return MPICAP_VERSION_MINOR;
}


/**
 * @brief Returns a string with the name of the plugin.
 * @ingroup MPICAPPlugin
 * @return A string with the name of the plugin
 */
std::string getName() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPICAP: call to getName()\n" );
    return "MPICAP plugin";
}


/**
 * @brief Returns a string with a short description of the plugin.
 * @ingroup MPICAPPlugin
 * @return A string with a short description of the plugin
 */
std::string getShortSummary() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPICAP: call to getShortSummary()\n" );
    return "Tunes the Energy-Delay-Product of MPI applications by systematically trying out a varying number of tasks.";
}


void MPICAPPlugin::writeResults() {
    int num_scenarios = pool_set->fsp->size();

    std::cout << "\nAutoTune Results:" << endl;
    std::cout << "-----------------------\n\n";
    std::cout << "Scenario                           | Processes | EDP           | Energy (J)    | Execution Time (sec)"  <<  std::endl;

    std::ofstream result_file( "mpicap_results.txt" );
    if( !result_file ) {
        psc_errmsg( "PCAP: Unable to open result file. No results will be written.\n" );
    }
    result_file << "Scenario;Processes;EDP;Energy;Time"  <<  std::endl;

    for( int scenario_id = 0; scenario_id != num_scenarios; ++scenario_id ) {
        Scenario* scenario = ( *pool_set->fsp->getScenarios() )[ scenario_id ];
        std::cout <<  std::setw( 4 )  <<  scenario_id  <<  ' ';
        result_file <<  scenario_id  <<  ";";

        // get the (only) tuning specification
        if( scenario->getTuningSpecifications()->size() != 1 ) {
            int size = static_cast<int>( scenario->getTuningSpecifications()->size() );
            psc_errmsg( "PCAP: Unexpected number of tuning specifications: %i\n", size );
            abort();
        }
        TuningSpecification& ts = *scenario->getTuningSpecifications()->front();

        // get the (only) tuning parameter to read the number of processes used
        if( ts.getVariant()->getValue().size() != 1 ) {
            int size = static_cast<unsigned int>( ts.getVariant()->getValue().size() );
            psc_errmsg( "PCAP: Unexpected number of tuning parameters: %i\n", size );
            abort();
        }
        const int numProcs = ts.getVariant()->getValue().begin()->second;

        // for each region, print the results
        BOOST_FOREACH( const std::string & region_id, *ts.getVariantContext().context_union.entity_list ) {
            Region*            region = Application::instance().getRegionByID( region_id );
            std::ostringstream temp;
            temp <<  "(" << region->getFileName()  <<  ","  <<  region->getFirstLine()  <<  ") ";
            std::string tempStr = temp.str();

            std::cout <<  std::setw( 30 )  <<  tempStr;
            std::cout <<  std::setw( 11 )  <<  numProcs;
            result_file <<  numProcs;
        }
        result_file <<  ";";

        std::map<std::string, double>           results = scenario->getResults();
        std::map<std::string, double>::iterator res_iter;
        for( res_iter = results.begin(); res_iter != results.end(); res_iter++ ) {
            std::cout <<  std::setw( 16 )  <<  res_iter->second;
            result_file <<  std::setiosflags( ios::fixed )  <<  res_iter->second  <<  ";";
        }
        std::cout <<  std::endl;
        result_file <<  std::endl;
    }

    std::cout <<  std::endl;
}


double MPICAPPlugin::objectiveFunction_EDP( int                  scenario_id,
                                            ScenarioResultsPool* srp ) {
    std::list<MetaProperty> properties = srp->getScenarioResultsByID( scenario_id );
    double                  energy     = 0;
    double                  time       = 0;

    BOOST_FOREACH( MetaProperty & property, properties ) {
        if( atoi( property.getId().c_str() ) == ENERGY_CONSUMPTION ) {
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
    return energy * time;
}
