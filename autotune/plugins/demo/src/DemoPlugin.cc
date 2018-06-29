/**
   @file    DemoPlugin.cc
   @ingroup DemoPlugin
   @brief   Demo Plugin
   @author  Houssam Haitof
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include "DemoPlugin.h"
#include "application.h"
#include "PropertyConfiguration.h" //TOROMOVE

/**
 * @brief Initialize the plugin's data structures.
 *
 * This method initializes the structures needed to store the data
 * relative to the Demo plugin. The tuning parameter list needs to
 * be created.
 *
 * The plugin uses 1 tuning parameters: the number of variants. This plugin is
 * used only as a proof of concept.
 *
 * @ingroup DemoPlugin
 *
 * @param context a pointer to a context for a plugin
 * @param pool_set a pointer to pools for a plugin
 **/
void DemoPlugin::initialize( DriverContext*   context,
                             ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DemoPlugin: Call to initialize\n" );
    this->context  = context;
    this->pool_set = pool_set;

    vector<TuningParameter*> tps = extractTuningParameters();
    if( tps.empty() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                    "DemoPlugin: No tuning parameters found. Exiting.\n" );
        throw PTF_PLUGIN_ERROR( TUNING_PARAMETERS_NOT_FOUND );
    }
    tuningParameters = tps;

    // later on we should have a max or min and a function that evaluates the objective
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DemoPlugin: obtain getSearchInstance\n" );
    int    major, minor;
    string name, description;

    char const* selected_search = getenv( "PSC_SEARCH_ALGORITHM" );
    if( selected_search != NULL ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DemoPlugin: User specified search algorithm: %s\n",
                    selected_search );
        string selected_search_string = string( selected_search );
        context->loadSearchAlgorithm( selected_search_string, &major, &minor, &name, &description );
        searchAlgorithm = context->getSearchAlgorithmInstance( selected_search_string );
    }
    else {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DemoPlugin: Selecting default search algorithm: exhaustive\n" );
        context->loadSearchAlgorithm( "exhaustive", &major, &minor, &name, &description );
        searchAlgorithm = context->getSearchAlgorithmInstance( "exhaustive" );
    }

    if( searchAlgorithm != NULL ) {
        print_loaded_search( major, minor, name, description );
        searchAlgorithm->initialize( context, pool_set );
    }
    else {
        perror( "NULL pointer in searchAlgorithm\n" );
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }

    if( context->applUninstrumented() ) {
        psc_infomsg( "Running an application that is not instrumented.\n" );
    }
    else {
        psc_infomsg( "Running an instrumented application.\n" );
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DemoPlugin: searchAlgorithm instance obtained\n" );
}

void DemoPlugin::startTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DemoPlugin: Create a VariantSpace from the tuning parameters.\n" );

    for( int i = 0; i < tuningParameters.size(); i++ ) {
        variantSpace.addTuningParameter( tuningParameters[ i ] );
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DemoPlugin: Create a SearchSpace from the tuning parameters.\n" );
    searchSpace.setVariantSpace( &variantSpace );
    searchSpace.addRegion( tuningParameters[ 0 ]->getRestriction()->getRegion() );
    searchAlgorithm->addSearchSpace( &searchSpace );
}

/**
 * @brief Defines a pre-analysis strategy.
 *
 * Found properties of the pre-analysis strategy are stored in the arp.
 *
 * @ingroup DemoPlugin
 *
 **/
bool DemoPlugin::analysisRequired( StrategyRequest** strategy ) {
    return false;
}

void DemoPlugin::createScenarios() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DemoPlugin: Call to createScenarios\n" );

    if( searchAlgorithm == NULL ) {
        perror( "Search algorithm not instantiated\n" );
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }

    searchAlgorithm->createScenarios();
}

void DemoPlugin::prepareScenarios() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                "DemoPlugin: Preparing the scenarios.\n" );

//  if (!csp->empty())
//    csp->print();
//  else
//    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "DemoPlugin: Created Scenario Pool was empty in prepare scenarios.\n");

    //no preparation is necessary, just move the elements on the pools
    while( !pool_set->csp->empty() ) {
        pool_set->psp->push( pool_set->csp->pop() );
    }

    // if compilation is necessary, a compile() method can be called here
}

void DemoPlugin::defineExperiment( int               numprocs,
                                   bool&             analysisRequired,
                                   StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DemoPlugin: Call to defineExperiment.\n" );
    int       i;
    Scenario* scenario;

    for( i = 0; !pool_set->psp->empty() && i < numprocs; i++ ) {
        //select scenario of this rank
        scenario = pool_set->psp->pop();
        const list<TuningSpecification*>* ts = scenario->getTuningSpecifications();
        if( ts->size() != 1 ) {
            perror( "DemoPlugin: can't currently handle multiple TuningSpecifications\n" );
            throw 0;
        }
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                    "setting single rank (%d) in scenario with id: %d\n", i, scenario->getID() );
        //define rank in the tuning specification
        ts->front()->setSingleRank( i );

        //define tuned region, property, and rank for the objective evaluation
        scenario->setSingleTunedRegionWithPropertyRank( appl->get_phase_region(), EXECTIME, i );
        // PropertyRequest *req = scenario->getPropertyRequests()->back();
        pool_set->esp->push( scenario );
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DemoPlugin: Added %d scenarios in the experiment.\n", i );

    //StrategyRequestGeneralInfo* strategyRequestGeneralInfo = new StrategyRequestGeneralInfo;

    //strategyRequestGeneralInfo->strategy_name = "SC_BF";
    //strategyRequestGeneralInfo->pedantic = 1;
    //strategyRequestGeneralInfo->delay_phases = 0;
    //strategyRequestGeneralInfo->delay_seconds = 0;

    //*strategy = new StrategyRequest(strategyRequestGeneralInfo);


    StrategyRequestGeneralInfo* strategyRequestGeneralInfo = new StrategyRequestGeneralInfo;

    strategyRequestGeneralInfo->strategy_name     = "ConfigAnalysis";
    strategyRequestGeneralInfo->pedantic          = 1;
    strategyRequestGeneralInfo->delay_phases      = 0;
    strategyRequestGeneralInfo->delay_seconds     = 0;
    strategyRequestGeneralInfo->analysis_duration = 1;

    PropertyRequest* req = new PropertyRequest();

    req->addRegion( appl->get_phase_region() );
    req->addPropertyID( EXECTIME );
    const char* env_qualexpr = getenv( "PSC_QUALEXPR" );
    req->addAllProcesses();
    list<PropertyRequest*>* reqList = new list<PropertyRequest*>;
    reqList->push_back( req );
    *strategy = new StrategyRequest( reqList, strategyRequestGeneralInfo );
    ( *strategy )->printStrategyRequest();
    analysisRequired = true;
}

bool DemoPlugin::restartRequired( std::string& env,
                                  int&         numprocs,
                                  std::string& command,
                                  bool&        is_instrumented ) {
    char const* force_restart = getenv( "PSC_FORCE_RESTART" );
    if( force_restart != NULL ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Forced restart.\n" );
        char const* restart_command = getenv( "PSC_RESTART_COMMAND" );
        if( restart_command != NULL ) {
            command.append( string( restart_command ) );
        }
        char const* process_count = getenv( "PSC_PROCESS_COUNT" );
        if( process_count != NULL ) {
            numprocs = atoi( process_count );
        }
        char const* env_is_instrumented = getenv( "PSC_NOT_INSTRUMENTED" );
        if( env_is_instrumented != NULL ) {
            is_instrumented = false;
        }
        return true;
    }
    return false; // no restart required
}


bool DemoPlugin::searchFinished() {
    return searchAlgorithm->searchFinished();
}


void DemoPlugin::finishTuningStep() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DemoPlugin: Call to finishTuningStep.\n" );
}


bool DemoPlugin::tuningFinished() {
    return true;
}


Advice* DemoPlugin::getAdvice() {
    if( searchAlgorithm == NULL ) {
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }

    cout << "\n-----------------------\n";
    cout << "AutoTune Results:" << endl;
    cout << "Found Optimum Scenario: " << searchAlgorithm->getOptimum() << endl;
    map<int, double > path = searchAlgorithm->getSearchPath();
    cout << "Search Steps: " << path.size() << endl;
    cout << "Search Path:\n";
    cout << "Step      |  Best\n";
    for( int step = 0; step < path.size(); step++ ) {
        printf( "%-10d|\t%f\n", step, path[ step ] );
    }
    cout << "All Results:\n";
    cout << "Scenario  |  Severity\n";
    for( int scenario_id = 0; scenario_id < pool_set->fsp->size(); scenario_id++ ) {
        if( !pool_set->srp->getScenarioResultsByID( scenario_id ).empty() ) {
            printf( "%-10d|", scenario_id );
            list<MetaProperty> properties = pool_set->srp->getScenarioResultsByID( scenario_id );
            for( std::list<MetaProperty>::iterator iterator = properties.begin(),
                 end = properties.end(); iterator != end; ++iterator ) {
                printf( "\t%f", iterator->getSeverity() );
            }
            printf( "\n" );
        }
    }
    cout << "----------|-------------" << endl;

    return new Advice( getName(), ( *pool_set->fsp->getScenarios() )[ searchAlgorithm->getOptimum() ],
                       searchAlgorithm->getSearchPath(), "Time", pool_set->fsp->getScenarios() );
}

void DemoPlugin::terminate() {
    if( searchAlgorithm ) {
        searchAlgorithm->finalize();
        delete searchAlgorithm;
    }
    context->unloadSearchAlgorithms();
}

void DemoPlugin::finalize() {
    terminate();
}

/**
 * @brief Returns plugin instance.
 * @ingroup DemoPlugin
 *
 * @return pointer to a new plugin object.
 **/
IPlugin* getPluginInstance( void ) {
    return new DemoPlugin();
}

/**
 * @brief Returns the major version number.
 * @ingroup DemoPlugin
 *
 * @return Major version number.
 **/
int getVersionMajor( void ) {
    return 1;
}

/**
 * @brief Returns the minor version number.
 * @ingroup DemoPlugin
 *
 * @return Minor version number.
 **/
int getVersionMinor( void ) {
    return 0;
}

/**
 * @brief Returns plugin name.
 * @ingroup DemoPlugin
 *
 * @return Plugin name.
 **/
string getName( void ) {
    return "Demonstrator Plugin";
}

/**
 * @brief Returns a short description.
 * @ingroup DemoPlugin
 *
 * @return description.
 **/
string getShortSummary( void ) {
    return "Simple plugin to demonstrate Autotune.";
}
