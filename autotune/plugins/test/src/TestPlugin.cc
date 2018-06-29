/**
   @file    TestPlugin.cc
   @ingroup TestPlugin
   @brief   Test Plugin
   @author  Isaias Compres
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

#include "TestPlugin.h"

/**
 * @brief Initialize the plugin's data structures.
 *
 * The tuning parameter list needs to be created.
 *
 *    tuningParameters = extractTuningParameters();
 *
 * Search algorithms are loaded here when required. This can be done as follows:
 *
 *    searchAlgorithm = loadSearchAlgorithm("name");
 *
 * where "name" refers to one of the available search algorithms (currently only exhaustive).
 *
 * @ingroup TestPlugin
 *
 */
// TODO every throw should have a code, instead of just zero
void TestPlugin::initialize( DriverContext*   context,
                             ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TestPlugin: call to initialize()\n" );
    this->context    = context;
    this->pool_set   = pool_set;
    search_algorithm = NULL;
    int    major, minor;
    string name, description;
    int    count;
    //const char * search_algorithm_names[] = {"exhaustive", "activeharmony", "compilerflagssearch", "mpiparameterssearch"};
    const char* search_algorithm_names[] = { "exhaustive", "individual", "gde3" };
    int         total_search_algorithms  = ( sizeof( search_algorithm_names ) / sizeof( search_algorithm_names[ 0 ] ) );
    proc_count   = 2;
    just_started = true;

    if( !context->applUninstrumented() ) {
        // Should be adapted to ScoreP
        psc_infomsg( "This application is instrumented; loading tuning parameters...\n" );
        try {
            tuning_parameters = extractTuningParameters();
        }
        catch( ... ) {
            psc_errmsg( "Error while loading tuning parameters.\n" );
            throw 0;
        }

        if( tuning_parameters.empty() ) {
            psc_errmsg( "No tuning parameters found.\n" );
            throw PTF_PLUGIN_ERROR( TUNING_PARAMETERS_NOT_FOUND );
        }

        psc_infomsg( "Tuning parameters loaded successfully." );
    }
    else {
        psc_infomsg( "This application is NOT instrumented; no tuning parameters...\n" );
        psc_infomsg( "The test plugin currently cannot handle this case; aborting...\n" );
        throw 0;
    }

    psc_infomsg( "Attempting to load and unload all available search algorithms...\n" );

    for( count = 0; count < total_search_algorithms; count++ ) {
        try {
            context->loadSearchAlgorithm( search_algorithm_names[ count ], &major, &minor, &name, &description );
            search_algorithm = context->getSearchAlgorithmInstance( search_algorithm_names[ count ] );
            print_loaded_search( major, minor, name, description );
        }
        catch( ... ) {
            psc_errmsg( "Error while loading search algorithm: %s.\n", search_algorithm_names[ count ] );
            throw 0;
        }

        try {
            context->unloadSearchAlgorithms();
        }
        catch( ... ) {
            psc_errmsg( "Error while unloading search algorithm: %s.\n", search_algorithm_names[ count ] );
            throw 0;
        }
    }

    psc_infomsg( "Search algorithms loaded successfully. They are currently not used by the test plugin "
                 "and are already unloaded.\n" );

    psc_infomsg( "Loading Exhaustive search for testing...\n" );
    context->loadSearchAlgorithm( "exhaustive", &major, &minor, &name, &description );
    search_algorithm = context->getSearchAlgorithmInstance( "exhaustive" );
    search_algorithm->initialize( search_context, pool_set );
    print_loaded_search( major, minor, name, description );

    psc_infomsg( "Done testing in initialize()\n" );
}

/**
 * @brief Defines a pre-analysis strategy.
 *
 * Found properties of the pre-analysis strategy are stored in the arp.
 *
 * @ingroup TestPlugin
 *
 */
bool TestPlugin::analysisRequired( StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TestPlugin: call to analysisRequired()\n" );
    //return false;

    StrategyRequestGeneralInfo* analysisStrategyGeneralInfo =
        new StrategyRequestGeneralInfo;

    analysisStrategyGeneralInfo->strategy_name     = "MPI";
    analysisStrategyGeneralInfo->pedantic          = 1;
    analysisStrategyGeneralInfo->delay_phases      = 0;
    analysisStrategyGeneralInfo->delay_seconds     = 0;
    analysisStrategyGeneralInfo->analysis_duration = 1;

    *strategy = new StrategyRequest( analysisStrategyGeneralInfo );
    ( *strategy )->printStrategyRequest();

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TestPlugin: returning true in analysisRequired()\n" );
    return true;
}

/**
 * @brief Operations to be done at the start of a tuning step.
 *
 * The tuning parameters that were created at initialization are used to create a variant space.
 * The variant space and the regions are then used to create a search space that is passed to the
 * search algorithm.
 *
 * @ingroup TestPlugin
 *
 */
void TestPlugin::startTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TestPlugin: call to startTuningStep()\n" );

    try {
        psc_infomsg( "Verifying that the tuning parameter list is not empty...\n" );
        if( !tuning_parameters.empty() ) {
            for( int i = 0; i < tuning_parameters.size(); i++ ) {
                psc_infomsg( "Adding tuning parameter %d (%s) to the variant space...\n", tuning_parameters[ i ]->getId(), tuning_parameters[ i ]->getName().c_str() );
                variantSpace.addTuningParameter( tuning_parameters[ i ] );
            }

            psc_infomsg( "Adding the variant space to the full search space...\n" );
            searchSpace.setVariantSpace( &variantSpace );
            searchSpace.addRegion( tuning_parameters[ 0 ]->getRestriction()->getRegion() );
            if( search_algorithm != NULL ) {
                psc_infomsg( "Adding the search space in the search algorithm...\n" );
                search_algorithm->addSearchSpace( &searchSpace );
            }
            else {
                psc_errmsg( "NULL pointer in TestPlugin::search_algorithm.\n" );
                throw 0;
            }
        }
        else {
            psc_errmsg( "TestPlugin::startTuningStep: the tuning parameter list is empty; aborting...\n" );
            throw 0;
        }
    }
    catch( ... ) {
        psc_errmsg( "Error in TestPlugin::startTuningStep().\n" );
        throw 0;
    }
}

/**
 * @brief The Created Scenario Pool (csp) is populated here.
 *
 * The scenarios need to be created and added to the first pool. To create the scenarios, a
 * search algorithm can be used or it can be done directly by the plugin.
 *
 * After this step, the Periscope will verify that scenarios were added to the csp.
 *
 * @ingroup TestPlugin
 *
 */
void TestPlugin::createScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TestPlugin: call to createScenarios()\n" );

    try {
        search_algorithm->createScenarios();
    }
    catch( ... ) {
        psc_errmsg( "Error in while calling createScenarios() from the TestPlugin::search_algorithm.\n" );
        throw 0;
    }
}

/**
 * @brief Preparatory steps for the scenarios are done here.
 *
 * If there are any preparatory steps required by some or all scenarios in the csp (for example:
 * the project may need to be re-compiled), then they are to be performed here. After each
 * scenario is prepared, they are migrated from the csp to the Prepared Scenario Pool (psp).
 *
 * In some cases, no preparation may be necessary and the plugin can simply move all scenarios
 * from the csp to the psp.
 *
 * After this step, the Periscope will verify that scenarios were added to the psp.
 *
 * @ingroup TestPlugin
 *
 */
void TestPlugin::prepareScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TestPlugin: call to prepareScenarios()\n" );

    while( !pool_set->csp->empty() ) {
        pool_set->psp->push( pool_set->csp->pop() );
    }
}

/**
 * @brief Populate the Experiment Scenario Pool (esp) for the next experiment.
 *
 * This is the final step before the experiments are executed. Scenarios are moved from the
 * psp to the esp, depending on the number of processes and whether they can be executed
 * in parallel.
 *
 * After this step, the Periscope will verify that scenarios were added to the esp.
 *
 * @ingroup TestPlugin
 *
 * @param numprocs         Number of processes required for the experiment
 * @param analysisRequired Defines is per-experiment analysis required
 * @param strategy         Strategy request for per-experiment analysis
 *
 */
void TestPlugin::defineExperiment( int               numprocs,
                                   bool&             analysisRequired,
                                   StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TestPlugin: call to defineExperiment()\n" );
    int       i;
    Scenario* scenario;

    // determine how many processes to use for this experiment
    if( just_started ) {
        psc_infomsg( "application started with %d processes. Changing this to 2...\n", numprocs );
        max_procs    = numprocs;
        proc_count   = 2;       // start with 2 processes
        just_started = false;
    }
    else if( ( proc_count * 2 ) <= max_procs ) {
        proc_count *= 2;         // next process count
        psc_infomsg( "changing process count to: %d\n", proc_count );
    }

    psc_infomsg( "defining experiments for %d processes ...\n", proc_count );
    //select scenario of this rank
    scenario = pool_set->psp->pop();
    const list<TuningSpecification*>* ts = scenario->getTuningSpecifications();
    if( ts->size() != 1 ) {
        perror( "TestPlugin can't currently handle multiple TuningSpecifications\n" );
        throw 0;
    }
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                "setting single rank (%d) in scenario with id: %d\n", i, scenario->getID() );


    if( tuning_parameters.empty() ) {
        psc_errmsg( "the tuning parameter list is empty; aborting...\n" );
        throw 0;
    }

    scenario->setSingleTunedRegionWithPropertyRank( tuning_parameters[ 0 ]->getRestriction()->getRegion(), EXECTIME, i );

    //define tuned region, property, and rank for the objective evaluation
    pool_set->esp->push( scenario );


    StrategyRequestGeneralInfo* analysisStrategyGeneralInfo = new StrategyRequestGeneralInfo;
    analysisStrategyGeneralInfo->strategy_name     = "MPI";
    analysisStrategyGeneralInfo->pedantic          = 1;
    analysisStrategyGeneralInfo->delay_phases      = 0;
    analysisStrategyGeneralInfo->delay_seconds     = 0;
    analysisStrategyGeneralInfo->analysis_duration = 1;

    StrategyRequest* analysisStrategy = new StrategyRequest( analysisStrategyGeneralInfo );

    psc_dbgmsg( 3, "Strategy requests in the plugin:\n" );
    if( psc_get_debug_level() >= 3 ) {
        analysisStrategy->printStrategyRequest();
    }
    *strategy = analysisStrategy;

    analysisRequired = true;


    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "[#### AUTOTUNE ####]: Added %d scenarios in the experiment.\n", i );
}

/**
 * @brief Returns does the application has to be restarted for the next experiment.
 *
 * Return true if a restart of the application is required for the next experiment,
 * false otherwise.
 *
 * @ingroup TestPlugin
 *
 */
bool TestPlugin::restartRequired( std::string& env,
                                  int&         numprocs,
                                  std::string& command,
                                  bool&        is_instrumented ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TestPlugin: call to restartRequired()\n" );
    psc_infomsg( "Received from the frontend: environment: \"%s\"; "
                 " process count: %d; command: \"%s\"; instrumented: %s;\n",
                 env.c_str(), numprocs, command.c_str(), is_instrumented ? "true" : "false"
                 );
    if( proc_count != numprocs ) {
        numprocs = proc_count;
        return true;
    }
    else {
        return false;         // the process count is already what we need
    }
}

/**
 * @brief Returns the status of the current search iteration.
 *
 * Returns true if the current search iteration is finished (by either the plugin itself or as
 * a result of the search algorithm), false otherwise.
 *
 * @ingroup TestPlugin
 */
bool TestPlugin::searchFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TestPlugin: call to searchFinished()\n" );

    return search_algorithm->searchFinished();
}

/**
 * @brief Final operation of a tuning step.
 *
 * If any post-processing is necessary before entering the next tuning iteration, it is to be
 * done here.
 *
 * @ingroup TestPlugin
 *
 */
void TestPlugin::finishTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TestPlugin: call to processResults()\n" );
}

/**
 * @brief Returns true if the plugin finished the tuning process, false otherwise.
 *
 * @ingroup TestPlugin
 *
 */
bool TestPlugin::tuningFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TestPlugin: call to tuningFinished()\n" );

    return true;
}

/**
 * @brief Prints to the screen (and to a file, where necessary) the tuning advice.
 *
 * @ingroup TestPlugin
 */
Advice* TestPlugin::getAdvice( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TestPlugin: call to getAdvice()\n" );
    return new Advice( getName(), ( *pool_set->fsp->getScenarios() )[ search_algorithm->getOptimum() ],
                       search_algorithm->getSearchPath(), "Time", pool_set->fsp->getScenarios() );
    return new Advice();
}

/**
 * @brief Finalize the plugin normally.
 *
 * Remove any allocated memory, objects, file descriptors, etc.
 *
 * @ingroup TestPlugin
 *
 */
void TestPlugin::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TestPlugin: call to finalize()\n" );

    terminate();
}

/*
 * @brief Terminate the plugin due to error.
 *
 * Safely remove any allocated memory, objects, file descriptors, etc. This method should
 * be able to be executed safely at any point.
 *
 * @ingroup TestPlugin
 *
 */
void TestPlugin::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TestPlugin: call to terminate()\n" );
    context->unloadSearchAlgorithms();
}

/*
 * The following are C functions that belong to the Plugin Management Interface, and not to the
 * IPlugin class.  Currently the interface includes information and instance creation functions.
 *
 * These are defined in PluginManagement.h .
 */

/**
 * @brief Returns an instance of this particular plugin implementation.
 *
 * @ingroup TestPlugin
 *
 * @return A pointer to a new TestPlugin
 */

IPlugin* getPluginInstance( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TestPlugin: call to getComponentInstance()\n" );
    return ( IPlugin* )new TestPlugin();
}

/**
 * @brief Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 *
 * @ingroup TestPlugin
 *
 * @return The major plugin interface version used by the plugin
 */
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TestPlugin: call to getVersionMajor()\n" );
    return 1;
}

/**
 * @brief Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 *
 * @ingroup TestPlugin
 *
 * @return The minor plugin interface version used by the plugin
 *
 */
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TestPlugin: call to getVersionMinor()\n" );
    return 0;
}

/**
 * @brief Returns a string with the name of the plugin.
 *
 * @ingroup TestPlugin
 *
 * @return Returns a string with the name of the plugin
 *
 */
string getName( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TestPlugin: call to getName()\n" );
    return "Plugin Interface Test";
}

/**
 * @brief Returns a string with a short description of the plugin.
 *
 * @ingroup TestPlugin
 *
 * @return A string with a short description of the plugin
 *
 */
string getShortSummary( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TestPlugin: call to getShortSummary()\n" );
    return "Tests the AutoTune plugin interface and services.";
}
