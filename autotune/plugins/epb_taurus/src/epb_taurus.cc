/**
   @file    epb_taurus.cc
   @ingroup EpbTaurusPlugin
   @brief   EPB on the taurus system
   @author  Andreas Gocht
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
    Copyright (c) 2016, Technische Universitaet Dresden, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include "../include/epb_taurus.h"

/**
 * @brief Initialize the plugin's data structures.
 *
 * The tuning parameter list needs to be created.
 *
 * Search algorithms are loaded here when required. This can be done as follows:
 *
 *    searchAlgorithm = loadSearchAlgorithm("name");
 *
 * where "name" refers to one of the available search algorithms (currently only exhaustive).
 *
 * @ingroup EpbTaurusPlugin
 *
 * @param context a pointer to a context for a plugin
 * @param pool_set a pointer to a set of pools for a plugin
 */
void EpbTaurusPlugin::initialize( DriverContext*   context,
                                   ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: call to initialize()\n" );


    this->context = context;
    this->pool_set = pool_set;

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: got pool and context\n" );
    /*
     * parse command line options
     */
    int argc = context->getArgc();
    char **argv = context->getArgv();

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: got argc and argv\n" );

    int c;
    optind = 1; // ptf parses opts with get_opt as well. So set optind to default, to avoid trouble
    while (1)
    {
         int option_index = 0;
         static struct option long_options[] = {
             {"min_epb",   required_argument, 0,  0 },
             {"max_epb",   required_argument, 0,  1 },
             {"epb_step",  required_argument, 0,  2 },
             {0,            0,                 0,  0 }
         };

         c = getopt_long(argc, argv, "",
                  long_options, &option_index);
         if (c == -1)
             break;

         switch (c) {
         case 0:
             this->min_epb = atoi(optarg);
             break;
         case 1:
             this->max_epb = atoi(optarg);
             break;
         case 2:
             this->epb_step = atoi(optarg);
             break;
         case '?':
             break;
         default:
             break;
         }
    }
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: got options\n" );

    TuningParameter* epbTuningParameter = new TuningParameter();
    epbTuningParameter->setId( 0 );
    epbTuningParameter->setName( "EPB" );
    epbTuningParameter->setPluginType( UNKOWN_PLUGIN );
    epbTuningParameter->setRange( min_epb, max_epb, epb_step );
    epbTuningParameter->setRuntimeActionType( TUNING_ACTION_FUNCTION_POINTER );
    tuningParameters.push_back(epbTuningParameter);
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: set epb\n" );

    string results = epbTuningParameter->toString();
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin plugin: Tuning parameter is: %s\n\n",
                results.c_str() );

    int    major, minor;
    string name, description;
    context->loadSearchAlgorithm( "exhaustive", &major, &minor, &name, &description );
    searchAlgorithm = context->getSearchAlgorithmInstance( "exhaustive" );
    if( searchAlgorithm ) {
        print_loaded_search( major, minor, name, description );
        searchAlgorithm->initialize( context, pool_set );
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: initialize() finisehd \n\n");
}

/**
 * @brief Operations to be done at the start of a tuning step.
 *
 * The tuning parameters that were created at initialization are used to create a variant space.
 * The variant space and the regions are then used to create a search space that is passed to the
 * search algorithm.
 *
 * @ingroup EpbTaurusPlugin
 *
 */
void EpbTaurusPlugin::startTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: call to startTuningStep()\n" );

    VariantSpace* variantSpace = new VariantSpace();
    SearchSpace*  searchSpace  = new SearchSpace();
    for( int i = 0; i < tuningParameters.size(); i++ ) {
        variantSpace->addTuningParameter( tuningParameters[ i ] );
    }
    searchSpace->setVariantSpace( variantSpace );
    searchSpace->addRegion( appl->get_phase_region() );
    searchAlgorithm->addSearchSpace( searchSpace );

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: call to startTuningStep() finished\n" );
}

/**
 * @brief Defines a pre-analysis strategy.
 *
 * Found properties of the pre-analysis strategy are stored in the arp.
 *
 * @ingroup EpbTaurusPlugin
 *
 * @return true if pre-analysis is required false otherwise
 *
 */
bool EpbTaurusPlugin::analysisRequired( StrategyRequest** strategy ) {
    return false;
}

/**
 * @brief The Created Scenario Pool (csp) is populated here.
 *
 * The scenarios need to be created and added to the first pool. To create the scenarios, a
 * search algorithm can be used or it can be done directly by the plugin.
 *
 * After this step, the Periscope will verify that scenarios were added to the csp.
 *
 * @ingroup EpbTaurusPlugin
 *
 */
void EpbTaurusPlugin::createScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: call to createScenarios()\n" );

    searchAlgorithm->createScenarios();
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: call to createScenarios() finished\n" );
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
 * @ingroup EpbTaurusPlugin
 *
 */
void EpbTaurusPlugin::prepareScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: call to prepareScenarios()\n" );

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
 * @ingroup EpbTaurusPlugin
 *
 * @param numprocs         Number of processes required for the experiment
 * @param analysisRequired Defines is per-experiment analysis required
 * @param strategy         Strategy request for per-experiment analysis
 *
 */
void EpbTaurusPlugin::defineExperiment( int               numprocs,
                                         bool&             analysisRequired,
                                         StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: call to defineExperiment()\n" );

    Scenario* scenario = pool_set->psp->pop();
    scenario->setSingleTunedRegionWithPropertyRank( appl->get_phase_region(), HDEEM_ENERGY_CONSUMTION_BLADE, 0 );
//    scenario->setSingleTunedRegionWithPropertyRank( appl->get_phase_region(), EXECTIME, 0 );
    pool_set->esp->push( scenario );

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: call to defineExperiment() finished\n" );

}

/**
 * @brief Returns does the application has to be restarted for the next experiment.
 *
 * Return true if a restart of the application is required for the next experiment,
 * false otherwise.
 *
 * @ingroup EpbTaurusPlugin
 *
 * @return true if an application has to be restarted false otherwise
 *
 */
bool EpbTaurusPlugin::restartRequired( std::string& env,
                                        int&         numprocs,
                                        std::string& command,
                                        bool&        is_instrumented ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: call to restartRequired()\n" );
    return false; // no restart required
}

/**
 * @brief Returns the status of the current search iteration.
 *
 * Returns true if the current search iteration is finished (by either the plugin itself or as
 * a result of the search algorithm), false otherwise.
 *
 * @ingroup EpbTaurusPlugin
 *
 * @return true if the plugin has finished search false otherwise
 *
 */
bool EpbTaurusPlugin::searchFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: call to searchFinished()\n" );
    return searchAlgorithm->searchFinished();
}

/**
 * @brief Final operation of a tuning step.
 *
 * If any post-processing is necessary before entering the next tuning iteration, it is to be
 * done here.
 *
 * @ingroup EpbTaurusPlugin
 *
 */
void EpbTaurusPlugin::finishTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: call to processResults()\n" );
}

/**
 * @brief Returns true if the plugin finished the tuning process, false otherwise.
 *
 * @ingroup EpbTaurusPlugin
 *
 * @return true if the plugin has finished tuning false otherwise
 *
 */
bool EpbTaurusPlugin::tuningFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: call to tuningFinished()\n" );
    return true;
}

/**
 * @brief Prints to the screen (and to a file, where necessary) the tuning advice.
 *
 * @ingroup EpbTaurusPlugin
 */
Advice* EpbTaurusPlugin::getAdvice( void ) {

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: call to getAdvice()\n" );
    std::ostringstream result_oss;
    map<int, double>   timeForScenario = searchAlgorithm->getSearchPath();
    double             serialTime      = timeForScenario[ 0 ];
    int optimum = searchAlgorithm->getOptimum();
    result_oss << "Optimum Scenario: " << optimum << endl << endl;
    result_oss << "\nAll Results:\n";
    result_oss << "Scenario\t|  EPB\t|  Energy consumption\t\n";
    for( int scenario_id = 0; scenario_id < pool_set->fsp->size(); scenario_id++ ) {
        Scenario*                   sc         = ( *pool_set->fsp->getScenarios() )[ scenario_id ];
        list<TuningSpecification*>* tuningSpec = sc->getTuningSpecifications();
        map<TuningParameter*, int>  tpValues   = tuningSpec->front()->getVariant()->getValue();
        int                         threads    = tpValues[ tuningParameters[ 0 ] ];
        double                      time       = timeForScenario[ scenario_id ];
        result_oss << scenario_id << "\t\t|  " << threads << "\t\t|  " << time << "\t  "<< endl;
    }
    result_oss << "\n------------------------" << endl << endl;
    cout << result_oss.str();
    map<int, Scenario*>::iterator scenario_iter;
    for( scenario_iter = pool_set->fsp->getScenarios()->begin(); scenario_iter != pool_set->fsp->getScenarios()->end(); scenario_iter++ ) {
        Scenario* sc = scenario_iter->second;
        sc->addResult( "Time", timeForScenario[ sc->getID() ] );
    }
    Scenario* bestScenario = ( *pool_set->fsp->getScenarios() )[ optimum ];
    return new Advice( getName(), bestScenario, timeForScenario, "Time", pool_set->fsp->getScenarios() );
}

/**
 * @brief Finalize the plugin normally.
 *
 * Remove any allocated memory, objects, file descriptors, etc.
 *
 * @ingroup EpbTaurusPlugin
 *
 */
void EpbTaurusPlugin::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: call to finalize()\n" );

    delete searchAlgorithm;
    for( int i = 0; i < tuningParameters.size(); i++ ) {
        delete tuningParameters[ i ];
    }
}

/**
 * @brief Terminate the plugin due to error.
 *
 * Safely remove any allocated memory, objects, file descriptors, etc. This method should
 * be able to be executed safely at any point.
 *
 * @ingroup EpbTaurusPlugin
 *
 */
void EpbTaurusPlugin::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: call to terminate()\n" );
}

/*
 * The following are C functions that belong to the Plugin Management Interface, and not to the
 * IPlugin class. Currently the interface includes information and instance creation functions.
 *
 * These are defined in PluginManagement.h .
 */

/**
 * @brief Returns an instance of this particular plugin implementation.
 *
 * Typically, a simple return with new is enough. For example:
 *
 * return a pointer to a new EpbTaurusPlugin();
 *
 * @ingroup EpbTaurusPlugin
 *
 * @return A pointer to a new EpbTaurusPlugin
 */
IPlugin* getPluginInstance( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: call to getPluginInstance()\n" );
    return new EpbTaurusPlugin();
}

/**
 * @brief Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 *
 * @ingroup EpbTaurusPlugin
 *
 * @return The major plugin interface version used by the plugin
 */
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: call to getInterfaceVersionMajor()\n" );
    return 1;
}

/**
 * @brief Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 *
 * @ingroup EpbTaurusPlugin
 *
 * @return The minor plugin interface version used by the plugin
 *
 */
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: call to getInterfaceVersionMinor()\n" );

    return 0;
}

/**
 * @brief Returns a string with the name of the plugin.
 *
 * @ingroup EpbTaurusPlugin
 *
 * @return Returns a string with the name of the plugin
 *
 */
string getName( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: call to getName()\n" );
    return "EpbTaurusPlugin";
}

/**
 * @brief Returns a string with a short description of the plugin.
 *
 * @ingroup EpbTaurusPlugin
 *
 * @return A string with a short description of the plugin
 *
 */
string getShortSummary( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "EpbTaurusPlugin: call to getShortSummary()\n" );
    return "EPB plugin for taurus";
}
