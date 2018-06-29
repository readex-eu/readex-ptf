/**
   @file    TutMultiTuning.cc
   @ingroup TutMultiTuning
   @brief   Tuning significant regions in multiple tuning steps
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

#include "TutMultiTuning.h"
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
 * Also initializes the number of tuning steps that the plugin should execute
 *
 * @ingroup TutMultiTuning
 *
 * @param context a pointer to a context for the plugin
 * @param pool_set a pointer to a set of pools for the plugin
 */
void TutMultiTuning::initialize( DriverContext*   context,
                                 ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiTuning: call to initialize()\n" );

    this->context  = context;
    this->pool_set = pool_set;

    if( appl->get_regions().empty() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiTuning plugin : No Region found. Exiting.\n" );
        throw 0;
    }

    TuningParameter* numberOfThreadsTP = new TuningParameter();
    numberOfThreadsTP->setId( 0 );
    numberOfThreadsTP->setName( "NUMTHREADS" );
    numberOfThreadsTP->setPluginType( TUTMULTITUNE );
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
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiTuning plugin: Before exiting to initialize, Tuning parameter is: %s\n\n",
                results.c_str() );

    int    major, minor;
    string name, description;
    context->loadSearchAlgorithm( "exhaustive", &major, &minor, &name, &description );
    searchAlgorithm = context->getSearchAlgorithmInstance( "exhaustive" );
    if( searchAlgorithm ) {
        print_loaded_search( major, minor, name, description );
        searchAlgorithm->initialize( context, pool_set );
    }

//! [initialize]
// sets the number of tuning steps plugin should execute
    maxTuningSteps = 2;
    curTuningStep  = 0;
//! [initialize]
}

/**
 * @brief Operations to be done at the start of a tuning step.
 *
 * All parallel and do parallel regions are collected for preanalysis
 *
 */
//! [startTuningStep]
void TutMultiTuning::startTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiTuning: call to startTuningStep()\n" );

    if( curTuningStep == 0 ) {
        std::list<Region*> code_regions;
        code_regions = appl->get_regions();
        std::list<Region*>::iterator region;

        // iterating over all regions
        int count = 0, parallel_regions = 0;
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Processing all regions...\n" );
        for( region = code_regions.begin(); region != code_regions.end(); region++ ) {
            if( ( *region )->get_type() == PARALLEL_REGION || ( *region )->get_type() == DO_REGION ) {
                parallel_regions++;
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Parallel region found:\n" );
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "\tFile name: %s\n", ( *region )->getFileName().c_str() );
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "\tFirst line: %d\n", ( *region )->getFirstLine() );
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "\tLast line: %d\n", ( *region )->getLastLine() );
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "\tMatcher key: %s\n", ( *region )->getIdForPropertyMatching().c_str() );
                code_region_candidates[ ( *region )->getIdForPropertyMatching() ] = *region;
            }
        }
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                    "TutMultiTuning: found %d parallel regions (out of %d total).\n", parallel_regions, count );
    }
}
//! [startTuningStep]

/**
 * @brief Pre-analysis is required.
 *
 * Preanalysis is requested for collected paralle regions
 * The preanalysis is requested only during first tuning step
 *
 * @ingroup TutMultiTuning
 *
 * @return false
 *
 */
//! [analysisRequired]
bool TutMultiTuning::analysisRequired( StrategyRequest** strategy ) {
    if( curTuningStep == 0 ) {
        StrategyRequestGeneralInfo*         analysisStrategyGeneralInfo = new StrategyRequestGeneralInfo;
        std::map<string, Region*>::iterator region;

        analysisStrategyGeneralInfo->strategy_name     = "ConfigAnalysis";
        analysisStrategyGeneralInfo->pedantic          = 0;
        analysisStrategyGeneralInfo->delay_phases      = 0;
        analysisStrategyGeneralInfo->delay_seconds     = 0;
        analysisStrategyGeneralInfo->analysis_duration = 1;

        PropertyRequest* req = new PropertyRequest();

        req->addPropertyID( EXECTIME );
        for( region = code_region_candidates.begin(); region != code_region_candidates.end(); region++ ) {
            req->addRegion( region->second );
        }
        req->addAllProcesses();

        list<PropertyRequest*>* reqList = new list<PropertyRequest*>;
        reqList->push_back( req );

        *strategy = new StrategyRequest( reqList, analysisStrategyGeneralInfo );
        return true;
    }

    return false;
}
//! [analysisRequired]

/**
 * custom compare function for sorting the pair of vectors
 **/
bool pairCompare( const std::pair<string, double> firstPair,
                  const std::pair<string, double> secondPair ) {
    return firstPair.second > secondPair.second;
}

/**
 * @brief All scenarios are created.
 *
 * During first tuning step, the pre-analysis results of collected
 * parallel regions are sorted according to execution time. The regions
 * with higher exeution time are considered significant
 *
 * The functions creates the variant space and the search space.
 * The variant space consists only of the NUMTHREADS tuning parameter.
 * For the search space, the regions are selected from sorted list of
 * preanalysis results and tuning action is applied to the regions
 * that are selected.
 *
 * For each number of threads a single scenario is created by the search
 * algorithm and pushed to the Created Scenario Pool (csp).
 *
 * It loads the exhaustive search algorithm and initializes it with the context and the
 * pool set of the plugin.
 * @ingroup TutMultiTuning
 *
 */
//! [createScenarios]
void TutMultiTuning::createScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiTuning: call to createScenarios()\n" );

    if( curTuningStep == 0 ) {
        list<MetaProperty>::iterator property;
        list<MetaProperty>           found_properties = pool_set->arp->getPreAnalysisProperties( 0 );
        double                       severity         = 0;

        for( property = found_properties.begin(); property != found_properties.end(); property++ ) {
            regionExectimeMap.push_back( make_pair( property->getRegionId(), property->getSeverity() ) );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                        "TutMultiTuning: Severity (%f) in region: %d added to vector\n",
                        property->getSeverity(), static_cast<int>( property->getRegionType() ) );
        }

        // sort the vector according to decreasing execution time
        std::sort( regionExectimeMap.begin(), regionExectimeMap.end(), pairCompare );
    }

    selected_region = code_region_candidates[ regionExectimeMap[ curTuningStep ].first ];
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                "TutMultiTuning- Tuning Step:%d ; selected region with: runtime: %f; ID: %s; file: %s; first line: %d; last line: %d;\n",
                curTuningStep, regionExectimeMap[ curTuningStep ].second, selected_region->getIdForPropertyMatching().c_str(), selected_region->getFileName().c_str(),
                selected_region->getFirstLine(), selected_region->getLastLine() );

    SearchSpace*  searchspace  = new SearchSpace();
    VariantSpace* variantSpace = new VariantSpace();

    for( int i = 0; i < tuningParameters.size(); i++ ) {
        variantSpace->addTuningParameter( tuningParameters[ i ] );
    }
    searchspace->setVariantSpace( variantSpace );
    searchspace->addRegion( selected_region );

    searchAlgorithm->addSearchSpace( searchspace );
    searchAlgorithm->createScenarios();
}
//! [createScenarios]

/**
 * @brief Copies all scenarios to the Prepared Scenario Pool.
 *
 * There is no preparation to be done. All the scenarios are copied to the
 * Prepared Scenario Pool (psp).
 *
 * @ingroup TutMultiTuning
 *
 */
//! [prepareScenarios]
void TutMultiTuning::prepareScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiTuning: call to prepareScenarios()\n" );
    while( !pool_set->csp->empty() ) {
        pool_set->psp->push( pool_set->csp->pop() );
    }
}
//! [prepareScenarios]


/**
 * @brief Select a single scenario for the next experiment.
 *
 * A single scenario is popped from the psp and prepared for the experiment.
 * The phase region is defined as the tuned region for which the EXECTIME
 * property is requested in process 0.
 *
 * @ingroup TutMultiTuning
 *
 * @param numprocs         Number of processes required for the experiment
 * @param analysisRequired Defines is per-experiment analysis required
 * @param strategy         Strategy request for per-experiment analysis
 *
 */
//! [defineExperiment]
void TutMultiTuning::defineExperiment( int               numprocs,
                                       bool&             analysisRequired,
                                       StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiTuning: call to defineExperiment()\n" );
    Scenario* scenario = pool_set->psp->pop();
    scenario->setSingleTunedRegionWithPropertyRank( selected_region, EXECTIME, 0 );
    pool_set->esp->push( scenario );
}
//! [defineExperiment]

/**
 * @brief This experiment does not require a restart.
 *
 * @ingroup TutMultiTuning
 *
 * @return false
 *
 */
//! [restartRequired]
bool TutMultiTuning::restartRequired( std::string& env,
                                      int&         numprocs,
                                      std::string& command,
                                      bool&        is_instrumented ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiTuning: call to restartRequired()\n" );
    return false;
}
//! [restartRequired]

/**
 * @brief Returns the status of the current search iteration.
 *
 * Returns always true since exhaustive search creates all the scenarios
 * in the first createScenario call.
 *
 * @ingroup TutMultiTuning
 *
 * @return true
 *
 */
//! [searchFinished]
bool TutMultiTuning::searchFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiTuning: call to searchFinished()\n" );
    return searchAlgorithm->searchFinished();
}
//! [searchFinished]

/**
 * @brief Nothing to be done in this step.
 *
 *
 * @ingroup TutMultiTuning
 *
 */
//! [finishTuningStep]
void TutMultiTuning::finishTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiTuning: call to processResults()\n" );
    curTuningStep++;
    // Get optimum for this tuning step
    map<int, double>                     timeForScenario = searchAlgorithm->getSearchPath();
    int                                  optimum         = searchAlgorithm->getOptimum();
    list<TuningSpecification*>::iterator ts;

    // copy search path to local search path
    map<int, double>::iterator iter;
    for( iter = timeForScenario.begin(); iter != timeForScenario.end(); ++iter ) {
        localSearchPath[ iter->first ]     = timeForScenario[ iter->first ];
        scenIdTuningStepMap[ iter->first ] = curTuningStep;
    }

    std::ostringstream result_oss;
    result_oss << "Evaluated source code region:" << endl <<
    "File name:\t" << selected_region->getFileName() << endl <<
    "Start line:\t" << selected_region->getFirstLine() << endl <<
    "End line:\t" <<  selected_region->getLastLine() << endl;

    Scenario*                   best_scenario              = ( *pool_set->fsp->getScenarios() )[ optimum ];
    list<TuningSpecification*>* best_tuning_specifications = best_scenario->getTuningSpecifications();
    result_oss << "Threads: ";
    for( ts = best_tuning_specifications->begin(); ts != best_tuning_specifications->end(); ts++ ) {
        map<TuningParameter*, int> best_thread_count = ( *ts )->getVariant()->getValue();
        result_oss << best_thread_count.begin()->second << " ";
    }
    result_oss << endl << "Optimum Scenario: " << optimum << endl << endl;
    cout << result_oss.str() << endl;

    searchAlgorithm->clear();     // clear searchspaces in search algorithm
}
//! [finishTuningStep]

/**
 * @brief Returns false since multiple tuning steps are needed
 *
 * Current tuning step ends here. The optimum results for region
 * tuned during this step are printed. The instance of search algorithm
 * is destroyed to be recreated during next tuning step
 *
 * Returns true if all tuning steps are finished otherwise returns false
 *
 * @ingroup TutMultiTuning
 *
 * @return false
 */
//! [tuningFinished]
bool TutMultiTuning::tuningFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiTuning: call to tuningFinished()\n" );
    if( curTuningStep >= maxTuningSteps ) {
        return true;
    }
    return false;
}
//! [tuningFinished]

/**
 * @brief Prints the scenarios, their execution time, and speedup.
 *
 * Prints the results and creates the advice file.
 *
 * @ingroup TutMultiTuning
 */
//! [getAdvice]
Advice* TutMultiTuning::getAdvice( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiTuning: call to getAdvice()\n" );

    list<TuningSpecification*>::iterator ts;
    std::ostringstream                   result_oss;
    int                                  optimum     = 0;
    double                               minExecTime = FLT_MAX;

    result_oss << endl << endl << "All Results:" << endl;
    result_oss << "Scenario\t|  TuningStep\t|  (Region,Threads)\t|  Time\t" << endl;

    for( int scenario_id = 0; scenario_id < pool_set->fsp->size(); scenario_id++ ) {
        Scenario*                   sc          = ( *pool_set->fsp->getScenarios() )[ scenario_id ];
        list<TuningSpecification*>* tuningSpecs = sc->getTuningSpecifications();
        result_oss << scenario_id << "\t\t|  " << scenIdTuningStepMap[ scenario_id ] << "\t\t|  ";

        for( ts = tuningSpecs->begin(); ts != tuningSpecs->end(); ts++ ) {
            map<TuningParameter*, int> tpValues = ( *ts )->getVariant()->getValue();
            int                        nthr     = tpValues.begin()->second;
            list<string>::iterator     region_iter;
            for( region_iter = ( *ts )->getVariantContext().context_union.entity_list->begin();
                 region_iter != ( *ts )->getVariantContext().context_union.entity_list->end(); region_iter++ ) {
                result_oss << "(" << ( *region_iter ) << "," << nthr << ") ";
            }
        }

        double time = localSearchPath[ scenario_id ];

        // optimum in this context is min Exec time
        if( time < minExecTime ) {
            minExecTime = time;
            optimum     = scenario_id;
        }

        result_oss << "\t\t|  " << time << endl;
    }

    cout << result_oss.str() << endl;

    map<int, Scenario*>::iterator scenario_iter;
    for( scenario_iter = pool_set->fsp->getScenarios()->begin(); scenario_iter != pool_set->fsp->getScenarios()->end(); scenario_iter++ ) {
        Scenario* sc = scenario_iter->second;
        sc->addResult( "Time", localSearchPath[ scenario_iter->first ] );
    }

    Scenario* bestScenario = ( *pool_set->fsp->getScenarios() )[ optimum ];
    return new Advice( getName(), bestScenario, localSearchPath, "Time", pool_set->fsp->getScenarios() );
}
//! [getAdvice]

/**
 * @brief Nothing done here.
 *
 *
 * @ingroup TutMultiTuning
 *
 */
//! [finalize]
void TutMultiTuning::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiTuning: call to finalize()\n" );
}
//! [finalize]

/**
 * @brief Nothing done here.
 *
 *
 *
 * @ingroup TutMultiTuning
 *
 */
void TutMultiTuning::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiTuning: call to terminate()\n" );
}


/**
 * @brief Returns an instance of this particular plugin implementation.
 *
 *
 * @ingroup TutMultiTuning
 *
 * @return A pointer to a new TutMultiTuning
 */
IPlugin* getPluginInstance( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiTuning: call to getComponentInstance()\n" );

    return new TutMultiTuning();
}

/**
 * @brief Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 *
 * @ingroup TutMultiTuning
 *
 * @return The major plugin interface version used by the plugin
 */
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiTuning: call to getInterfaceVersionMajor()\n" );

    return 1;
}

/**
 * @brief Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 *
 * @ingroup TutMultiTuning
 *
 * @return The minor plugin interface version used by the plugin
 *
 */
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiTuning: call to getInterfaceVersionMinor()\n" );

    return 0;
}

/**
 * @brief Returns the plugin name TutMultiTuning.
 *
 * @ingroup TutMultiTuning
 *
 * @return Returns TutMultiTuning
 *
 */
string getName( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiTuning: call to getName()\n" );

    return "TutMultiTuning";
}

/**
 * @brief Returns a short description of the plugin.
 *
 * @ingroup TutMultiTuning
 *
 * @return A string with a short description of the plugin.
 *
 */
string getShortSummary( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiTuning: call to getShortSummary()\n" );

    return "Multiple Tuning steps for tuning different regions";
}
