/**
   @file    TutObjFunc.cc
   @ingroup TutObjFunc
   @brief   Tutorial Plugin - customized objective functions
   @author  Shrikant Vinchurkar
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

#include "TutObjFunc.h"
#include "search_common.h"

/**
 * @brief Initialize the plugin's data structures.
 *
 * This plugin has one tuning paramter NUMTHREADS. It takes values in the
 * power of 2, starting from 1 to number of threads given by --ompnumthreads
 * on the commandline.
 *
 * It is based on a function tuning action. The function is set_omp_num_threads(int).
 *
 * The initialization also copies the context and the pool set to the plugin attributes.
 *
 * It loads the exhaustive search algorithm and initializes it with the context and the
 * pool set of the plugin.
 *
 * @ingroup TutObjFunc
 *
 * @param context a pointer to a context for the plugin
 * @param pool_set a pointer to a set of pools for the plugin
 */
void TutObjFunc::initialize( DriverContext*   context,
                             ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutObjFunc: call to initialize()\n" );

    this->context  = context;
    this->pool_set = pool_set;

    // make sure that the regions to be tuned are mentioned in the list of regionse
    if( appl->get_regions().empty() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutObjFunc plugin : No Region found. Exiting.\n" );
        throw 0;
    }

    TuningParameter* numberOfThreadsTP = new TuningParameter();
    numberOfThreadsTP->setId( 0 );
    numberOfThreadsTP->setName( "NUMTHREADS" );
    numberOfThreadsTP->setPluginType( TUTMULTIRANK );
    numberOfThreadsTP->setRange( 1, context->getOmpnumthreads(), 1 );
    Restriction* r = new Restriction();
    for( int i = 1; i <= context->getOmpnumthreads(); i = i * 2 ) {
        r->addElement( i );
    }
    r->setRegion( NULL );
    r->setType( 2 );
    numberOfThreadsTP->setRestriction( r );
    numberOfThreadsTP->setRuntimeActionType( TUNING_ACTION_FUNCTION_POINTER );
    tuningParameters.push_back( numberOfThreadsTP );

    string results = numberOfThreadsTP->toString();
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutObjFunc plugin: Before exiting to initialize, Tuning parameter is: %s\n\n",
                results.c_str() );

//! [initialize]
    int    major, minor;
    string name, description;
    context->loadSearchAlgorithm( "exhaustive", &major, &minor, &name, &description );
    searchAlgorithm = context->getSearchAlgorithmInstance( "exhaustive" );
    if( searchAlgorithm ) {
        print_loaded_search( major, minor, name, description );
        searchAlgorithm->initialize( context, pool_set );
        searchAlgorithm->addObjectiveFunction(new Inverse_speedupObjective(""));
    }
//! [initialize]
}

/**
 * @brief Operations to be done at the start of a tuning step.
 *
 * The functions creates the variant space and the search space.
 * The variant space consists only of the NUMTHREADS tuning parameter.
 * For the search space, the regions are selected from list of regions
 * collected during initial run of the application and tuning action
 * is applied to the regions that are selected.

   The regions are selected from list of regions
 *
 */
void TutObjFunc::startTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutObjFunc: call to startTuningStep()\n" );

    // Tuning Parallel Regions
    std::list<Region*> code_regions;
    code_regions = appl->get_regions();
    std::list<Region*>::iterator region;

    // iterating over all regions
    for( region = code_regions.begin(); region != code_regions.end(); region++ ) {
        if( ( *region )->get_type() == PARALLEL_REGION || ( *region )->get_type() == DO_REGION ) {
            if( ( *region )->getFileName() == "z_solve.f" ) { // Tuning parallel region from z_solve.f
                SearchSpace*  searchspace  = new SearchSpace();
                VariantSpace* variantSpace = new VariantSpace();

                for( int i = 0; i < tuningParameters.size(); i++ ) {
                    variantSpace->addTuningParameter( tuningParameters[ i ] );
                }
                searchspace->setVariantSpace( variantSpace );

                searchspace->addRegion( *region );
                searchAlgorithm->addSearchSpace( searchspace );
            }
        }
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutObjFunc: Created a SearchSpace from TP NUMTHREADS.\n" );
}

/**
 * @brief No pre-analysis is required.
 *
 *
 * @ingroup TutObjFunc
 *
 * @return false
 *
 */
bool TutObjFunc::analysisRequired( StrategyRequest** strategy ) {
    return false;
}

/**
 * @brief All scenarios are created.
 *
 * For each number of threads a single scenario is created by the search
 * algorithm and pushed to the Created Scenario Pool (csp).
 *
 * @ingroup TutObjFunc
 *
 */
void TutObjFunc::createScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutObjFunc: call to createScenarios()\n" );

    searchAlgorithm->createScenarios();
}

/**
 * @brief Copies all scenarios to the Prepared Scenario Pool.
 *
 * There are no preparation done. All the scenarios are copied to the
 * Prepared Scenario Pool (psp).
 *
 * @ingroup TutObjFunc
 *
 */
void TutObjFunc::prepareScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutObjFunc: call to prepareScenarios()\n" );

    while( !pool_set->csp->empty() ) {
        pool_set->psp->push( pool_set->csp->pop() );
    }
}


/**
 * @brief Select a single scenario for the next experiment.
 *
 * Two scenarios are popped up from psp and prepared for the experiment. One of
 * popped scenario is assigned to MPI process with rank 0 and other one is
 * assigned to MPI process with rank 1.
 * The tuned region is the parallel region for which the EXECTIME
 * property is requested.
 *
 * @ingroup TutObjFunc
 *
 * @param numprocs         Number of processes required for the experiment
 * @param analysisRequired Defines is per-experiment analysis required
 * @param strategy         Strategy request for per-experiment analysis
 *
 */
void TutObjFunc::defineExperiment( int               numprocs,
                                   bool&             analysisRequired,
                                   StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutObjFunc: call to defineExperiment()\n" );

    for( int i = 0; i < numprocs; i++ ) {
        //break if the scenario pool is empty
        if( pool_set->psp->empty() ) {
            break;
        }

        // pop scenario from prepared scenario pool and assign to MPI process with rank `i`
        Scenario* scenario = pool_set->psp->pop();

        // Tuning Parallel Regions
        std::list<Region*> code_regions;
        code_regions = appl->get_regions();
        std::list<Region*>::iterator region;
//! [defineExperiment]

        // iterating over all regions
        for( region = code_regions.begin(); region != code_regions.end(); region++ ) {
            if( ( *region )->get_type() == PARALLEL_REGION || ( *region )->get_type() == DO_REGION ) {
                if( ( *region )->getFileName() == "z_solve.f" ) { // Requesting property for parallel region from z_solve.f
                    if( scenario->getID() == 0 ) { // push first scenario for all MPI processes
                        scenario->setSingleTunedRegionWithPropertyALLRanks( ( *region ), EXECTIME );
                        pool_set->esp->push( scenario );
                        return;
                    }
                    else {
                        const list<TuningSpecification*>* ts = scenario->getTuningSpecifications();
                        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                                    "setting single rank (%d) in scenario with id: %d\n", i, scenario->getID() );
                        ts->front()->setSingleRank( i );
                        scenario->setSingleTunedRegionWithPropertyRank( ( *region ), EXECTIME, i ); // Assigning to MPI process i
                    }
                }
            }
        }
        pool_set->esp->push( scenario );
    }
//! [defineExperiment]
}

/**
 * @brief This experiment does not require a restart.
 *
 * Instead of a restart the next phase execution is used to evaluated
 * the scenario.
 *
 * @ingroup TutObjFunc
 *
 * @return false
 *
 */
bool TutObjFunc::restartRequired( std::string& env,
                                  int&         numprocs,
                                  std::string& command,
                                  bool&        is_instrumented ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutObjFunc: call to restartRequired()\n" );

    return false;
}

/**
 * @brief Returns the status of the current search iteration.
 *
 * Returns always true since exhaustive search creates all the scenarios
 * in the first createScenario call.
 *
 * @ingroup TutObjFunc
 *
 * @return true
 *
 */
bool TutObjFunc::searchFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutObjFunc: call to searchFinished()\n" );

    return searchAlgorithm->searchFinished();
}

/**
 * @brief Nothing to be done in this step.
 *
 *
 * @ingroup TutObjFunc
 *
 */
void TutObjFunc::finishTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutObjFunc: call to processResults()\n" );
}

/**
 * @brief Returns true since no other tuning step is required.
 *
 * @ingroup TutObjFunc
 *
 * @return true
 *
 */
bool TutObjFunc::tuningFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutObjFunc: call to tuningFinished()\n" );

    return true;
}

/**
 * @brief Prints the scenarios, their execution time, and speedup.
 *
 * Prints the results and creates the advice file.
 *
 * @ingroup TutObjFunc
 */
Advice* TutObjFunc::getAdvice( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutObjFunc: call to getAdvice()\n" );

    std::ostringstream result_oss;
    map<int, double>   invSpeedupScenario = searchAlgorithm->getSearchPath();
    double             baseInvSpeedup     = invSpeedupScenario[ 0 ];

    int optimum = searchAlgorithm->getOptimum();
    result_oss << "Optimum Scenario: " << optimum << endl << endl;


    result_oss << "\nAll Results:\n";
    result_oss << "Scenario\t|  Process\t|  Threads\t|  InverseSpeedup|  Speedup\t\n";

    for( int scenario_id = 0; scenario_id < pool_set->fsp->size(); scenario_id++ ) {
        Scenario*                   sc         = ( *pool_set->fsp->getScenarios() )[ scenario_id ];
        list<TuningSpecification*>* tuningSpec = sc->getTuningSpecifications();
        map<TuningParameter*, int>  tpValues   = tuningSpec->front()->getVariant()->getValue();
        int                         threads    = tpValues[ tuningParameters[ 0 ] ];
        double                      invSpeedup = invSpeedupScenario[ scenario_id ];

        list<MetaProperty> properties = pool_set->srp->getScenarioResultsByID( scenario_id );
        int                process    = properties.front().getProcess();

        if( scenario_id == 0 ) {
            result_oss << scenario_id << "\t\t|  " << "ALL\t\t|  " << threads << "\t\t|  " << invSpeedup << "\t|  " << baseInvSpeedup / invSpeedup << endl;
        }
        else {
            result_oss << scenario_id << "\t\t|  " << process << "\t\t|  " << threads << "\t\t|  " << invSpeedup << "\t|  " << baseInvSpeedup / invSpeedup << endl;
        }
    }
    result_oss << "\n------------------------" << endl << endl;

    cout << result_oss.str();

    //reset tuning specifications and sets default description in the scenario and execution time.
    map<int, Scenario*>::iterator scenario_iter;
    for( scenario_iter = pool_set->fsp->getScenarios()->begin(); scenario_iter != pool_set->fsp->getScenarios()->end(); scenario_iter++ ) {
        Scenario* sc = scenario_iter->second;
        sc->addResult( "invSpeedup", invSpeedupScenario[ sc->getID() ] );
    }

    Scenario* bestScenario = ( *pool_set->fsp->getScenarios() )[ optimum ];
    return new Advice( getName(), bestScenario, invSpeedupScenario, "invSpeedup", pool_set->fsp->getScenarios() );
}

/**
 * @brief Nothing done here.
 *
 *
 * @ingroup TutObjFunc
 *
 */
void TutObjFunc::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutObjFunc: call to finalize()\n" );
}

/**
 * @brief Nothing done here.
 *
 *
 *
 * @ingroup TutObjFunc
 *
 */
void TutObjFunc::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutObjFunc: call to terminate()\n" );
}


/**
 * @brief Returns an instance of this particular plugin implementation.
 *
 *
 * @ingroup TutObjFunc
 *
 * @return A pointer to a new TutObjFunc
 */
IPlugin* getPluginInstance( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutObjFunc: call to getComponentInstance()\n" );

    return new TutObjFunc();
}

/**
 * @brief Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 *
 * @ingroup TutObjFunc
 *
 * @return The major plugin interface version used by the plugin
 */
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutObjFunc: call to getInterfaceVersionMajor()\n" );

    return 1;
}

/**
 * @brief Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 *
 * @ingroup TutObjFunc
 *
 * @return The minor plugin interface version used by the plugin
 *
 */
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutObjFunc: call to getInterfaceVersionMinor()\n" );

    return 0;
}

/**
 * @brief Returns the plugin name TutObjFunc.
 *
 * @ingroup TutObjFunc
 *
 * @return Returns TutObjFunc
 *
 */
string getName( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutObjFunc: call to getName()\n" );

    return "TutObjFunc";
}

/**
 * @brief Returns a short description of the plugin.
 *
 * @ingroup TutObjFunc
 *
 * @return A string with a short description of the plugin.
 *
 */
string getShortSummary( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutObjFunc: call to getShortSummary()\n" );

    return "Tunes parallel region. Scenario execution is done in parallel by assigning them to different MPI processes";
}
