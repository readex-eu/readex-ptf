/**
   @file    TutCombineScen.cc
   @ingroup TutCombineScen
   @brief   Combine scenarios from different regions in single experiment
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

#include "TutCombineScen.h"
#include <cmath>

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
 * It loads the exhaustive search algorithm. The array of search algorithm instances are
 * initialized with the context and the pool set of the plugin.
 *
 * @ingroup TutCombineScen
 *
 * @param context a pointer to a context for the plugin
 * @param pool_set a pointer to a set of pools for the plugin
 */
void TutCombineScen::initialize( DriverContext*   context,
                                 ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutCombineScen: call to initialize()\n" );

    this->context  = context;
    this->pool_set = pool_set;

    if( appl->get_regions().empty() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutCombineScen plugin : No Region found. Exiting.\n" );
        throw 0;
    }

    TuningParameter* numberOfThreadsTP = new TuningParameter();
    numberOfThreadsTP->setId( 0 );
    numberOfThreadsTP->setName( "NUMTHREADS" );
    numberOfThreadsTP->setPluginType( TUTCOMBINESCEN );
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
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutCombineScen plugin: Before exiting to initialize, Tuning parameter is: %s\n\n",
                results.c_str() );

//! [initialize]
// initialize number of significant regions to be probed
    noSigRegions       = 4;
    arrSearchAlgorithm = new ISearchAlgorithm*[ noSigRegions ];

    int    major, minor;
    string name, description;
    context->loadSearchAlgorithm( "exhaustive", &major, &minor, &name, &description );

    for( int i = 0; i < noSigRegions; i++ ) {
        arrSearchAlgorithm[ i ] = context->getSearchAlgorithmInstance( "exhaustive" );
        if( arrSearchAlgorithm[ i ] ) {
            print_loaded_search( major, minor, name, description );
            arrSearchAlgorithm[ i ]->initialize( context, pool_set );
        }
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
 *
 * The regions are selected from list of regions
 *
 */
//! [startTuningStep]
void TutCombineScen::startTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutCombineScen: call to startTuningStep()\n" );

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
                "TutCombineScen: found %d parallel regions (out of %d total).\n", parallel_regions, count );
}
//! [startTuningStep]

/**
 * @brief No pre-analysis is required.
 *
 *
 * @ingroup TutCombineScen
 *
 * @return false
 *
 */
//! [analysisRequired]
bool TutCombineScen::analysisRequired( StrategyRequest** strategy ) {
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
 * Searchspaces are initialized for all search algorithm instances and every
 * instance creates a single scenario for each number of thread.
 * The scenarios are then pushed to the Created Scenario Pool (csp).
 *
 * @ingroup TutCombineScen
 *
 */
//! [createScenarios]
void TutCombineScen::createScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutCombineScen: call to createScenarios()\n" );

    list<MetaProperty>::iterator property;
    list<MetaProperty>           found_properties = pool_set->arp->getPreAnalysisProperties( 0 );
    double                       severity         = 0;

    for( property = found_properties.begin(); property != found_properties.end(); property++ ) {
        regionExectimeMap.push_back( make_pair( property->getRegionId(), property->getSeverity() ) );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                    "TutCombineScen: Severity (%f) in region: %d added to vector\n",
                    property->getSeverity(), static_cast<int>( property->getRegionType() ) );
    }

    // sort the vector according to decreasing execution time
    std::sort( regionExectimeMap.begin(), regionExectimeMap.end(), pairCompare );

    // adding search space for every search algorithm instance
    for( int i = 0; i < noSigRegions; i++ ) {
        selected_region = code_region_candidates[ regionExectimeMap[ i ].first ];
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                    "TutCombineScen- selected region with: runtime: %f; ID: %s; file: %s; first line: %d; last line: %d;\n", regionExectimeMap[ i ].second, selected_region->getIdForPropertyMatching().c_str(), selected_region->getFileName().c_str(), selected_region->getFirstLine(), selected_region->getLastLine() );

        SearchSpace*  searchspace  = new SearchSpace();
        VariantSpace* variantSpace = new VariantSpace();

        for( int j = 0; j < tuningParameters.size(); j++ ) {
            variantSpace->addTuningParameter( tuningParameters[ j ] );
        }
        searchspace->setVariantSpace( variantSpace );
        searchspace->addRegion( selected_region );
        arrSearchAlgorithm[ i ]->addSearchSpace( searchspace );
        arrSearchAlgorithm[ i ]->createScenarios();
    }
}
//! [createScenarios]

/**
 * @brief Copies all scenarios to the Prepared Scenario Pool.
 *
 * There are no preparation done. All the scenarios are copied to the
 * Prepared Scenario Pool (psp).
 *
 * @ingroup TutCombineScen
 *
 */
//! [prepareScenarios]
void TutCombineScen::prepareScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutCombineScen: call to prepareScenarios()\n" );
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
 * @ingroup TutCombineScen
 *
 * @param numprocs         Number of processes required for the experiment
 * @param analysisRequired Defines is per-experiment analysis required
 * @param strategy         Strategy request for per-experiment analysis
 *
 */
//! [defineExperiment]
void TutCombineScen::defineExperiment( int               numprocs,
                                       bool&             analysisRequired,
                                       StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutCombineScen: call to defineExperiment()\n" );

    Scenario* scenario = pool_set->psp->pop();
    scenario->setSingleTunedRegionWithPropertyRank( code_region_candidates[ regionExectimeMap[ 0 ].first ], EXECTIME, 0 );
    pool_set->esp->push( scenario );

    int offset = log2( context->getOmpnumthreads() ) + 1;

    for( int i = 1; i < noSigRegions; i++ ) {
        Scenario* scenarioNew = pool_set->psp->pop( scenario->getID() + i * offset );
        scenarioNew->setSingleTunedRegionWithPropertyRank( code_region_candidates[ regionExectimeMap[ i ].first ], EXECTIME, 0 );
        pool_set->esp->push( scenarioNew );
    }
}
//! [defineExperiment]

/**
 * @brief This experiment does not require a restart.
 *
 * Instead of a restart the next phase execution is used to evaluated
 * the scenario.
 *
 * @ingroup TutCombineScen
 *
 * @return false
 *
 */
//! [restartRequired]
bool TutCombineScen::restartRequired( std::string& env,
                                      int&         numprocs,
                                      std::string& command,
                                      bool&        is_instrumented ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutCombineScen: call to restartRequired()\n" );
    return false;
}
//! [restartRequired]

/**
 * @brief Returns the status of the current search iteration.
 *
 * Returns true only if all search algorithms have finished searching.
 *
 * Always returns true for exhaustive search since exhaustive search creates all the scenarios
 * in the first createScenario call.
 *
 * @ingroup TutCombineScen
 *
 * @return true
 *
 */
//! [searchFinished]
bool TutCombineScen::searchFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutCombineScen: call to searchFinished()\n" );

    bool retStatus = true;
    for( int i = 0; i < noSigRegions; i++ ) {
        retStatus = retStatus && arrSearchAlgorithm[ i ]->searchFinished();
    }
    return retStatus;
}
//! [searchFinished]

/**
 * @brief Nothing to be done in this step.
 *
 *
 * @ingroup TutCombineScen
 *
 */
//! [finishTuningStep]
void TutCombineScen::finishTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutCombineScen: call to processResults()\n" );
}
//! [finishTuningStep]

/**
 * @brief Returns true since no other tuning step is required.
 *
 * @ingroup TutCombineScen
 *
 * @return true
 *
 */
//! [tuningFinished]
bool TutCombineScen::tuningFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutCombineScen: call to tuningFinished()\n" );
    return true;
}
//! [tuningFinished]

/**
 * @brief Prints the scenarios, their execution time, and speedup.
 *
 * Prints the results and creates the advice file.
 *
 * @ingroup TutCombineScen
 */
//! [getAdvice]
Advice* TutCombineScen::getAdvice( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutCombineScen: call to getAdvice()\n" );

    list<TuningSpecification*>::iterator ts;
    std::ostringstream                   result_oss;

    list<Scenario* > bestScenarios;

    int    optimum     = 0;
    double minExecTime = std::numeric_limits<double>::max();

    for( int i = 0; i < noSigRegions; i++ ) {
        // copy the path of searchAlgorithm to local Search Path
        map<int, double> pathMap = arrSearchAlgorithm[ i ]->getSearchPath();
        localSearchPath.insert( pathMap.begin(), pathMap.end() );

        result_oss << endl << "Optimal Scenario for region:" << endl;
        result_oss << "Region Name: " << code_region_candidates[ regionExectimeMap[ i ].first ]->getFileName().c_str() << endl;
        result_oss << "Optimal Scenario Id:" << arrSearchAlgorithm[ i ]->getOptimum() << endl << endl;
    }

    result_oss << endl << "All Results:" << endl;
    result_oss << "Scenario\t|  (Region,Threads)\t|  Time\t" << endl;

    for( int scenario_id = 0; scenario_id < pool_set->fsp->size(); scenario_id++ ) {
        Scenario*                   sc          = ( *pool_set->fsp->getScenarios() )[ scenario_id ];
        list<TuningSpecification*>* tuningSpecs = sc->getTuningSpecifications();
        result_oss << scenario_id << "\t\t|  ";

        for( ts = tuningSpecs->begin(); ts != tuningSpecs->end(); ts++ ) {
            map<TuningParameter*, int> tpValues = ( *ts )->getVariant()->getValue();
            int                        nthr     = tpValues.begin()->second;
            list<string>::iterator     region_iter;
            for( region_iter = ( *ts )->getVariantContext().context_union.entity_list->begin();
                 region_iter != ( *ts )->getVariantContext().context_union.entity_list->end(); region_iter++ ) {
                result_oss << "(" << ( *region_iter ) << "," << nthr << ") ";
            }
        }
        double time = localSearchPath[ sc->getID() ];

        // optimum in this context is min Exec time
        if( time < minExecTime ) {
            minExecTime = time;
            optimum     = scenario_id;
        }

        result_oss << "\t\t|  " << time << endl;
    }

    Scenario* best_scenario = ( *pool_set->fsp->getScenarios() )[ optimum ];
    bestScenarios.push_back( best_scenario );

    cout << endl << result_oss.str() << endl;

    // reset tuning specifications and sets default description in the scenario and execution time.
    map<int, Scenario*>::iterator scenario_iter;
    for( scenario_iter = pool_set->fsp->getScenarios()->begin(); scenario_iter != pool_set->fsp->getScenarios()->end(); scenario_iter++ ) {
        Scenario* sc = scenario_iter->second;
        sc->addResult( "Time", localSearchPath[ sc->getID() ] );
    }

    return new Advice( getName(), bestScenarios, localSearchPath, "Time", pool_set->fsp->getScenarios() );
}
//! [getAdvice]

/**
 * @brief Nothing done here.
 *
 *
 * @ingroup TutCombineScen
 *
 */
//! [finalize]
void TutCombineScen::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutCombineScen: call to finalize()\n" );
    for( int i = 0; i < noSigRegions; i++ ) {
        delete arrSearchAlgorithm[ i ];
    }
}
//! [finalize]

/**
 * @brief Nothing done here.
 *
 *
 *
 * @ingroup TutCombineScen
 *
 */
void TutCombineScen::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutCombineScen: call to terminate()\n" );
}


/**
 * @brief Returns an instance of this particular plugin implementation.
 *
 *
 * @ingroup TutCombineScen
 *
 * @return A pointer to a new TutCombineScen
 */
IPlugin* getPluginInstance( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutCombineScen: call to getComponentInstance()\n" );

    return new TutCombineScen();
}

/**
 * @brief Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 *
 * @ingroup TutCombineScen
 *
 * @return The major plugin interface version used by the plugin
 */
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutCombineScen: call to getInterfaceVersionMajor()\n" );

    return 1;
}

/**
 * @brief Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 *
 * @ingroup TutCombineScen
 *
 * @return The minor plugin interface version used by the plugin
 *
 */
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutCombineScen: call to getInterfaceVersionMinor()\n" );

    return 0;
}

/**
 * @brief Returns the plugin name TutCombineScen.
 *
 * @ingroup TutCombineScen
 *
 * @return Returns TutCombineScen
 *
 */
string getName( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutCombineScen: call to getName()\n" );

    return "TutCombineScen";
}

/**
 * @brief Returns a short description of the plugin.
 *
 * @ingroup TutCombineScen
 *
 * @return A string with a short description of the plugin.
 *
 */
string getShortSummary( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutCombineScen: call to getShortSummary()\n" );

    return "Scenario analysis to get significant regions in one experiment";
}
