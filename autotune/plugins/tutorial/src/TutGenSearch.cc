/**
   @file    TutGenSearch.cc
   @ingroup TutGenSearch
   @brief   Tutorial of a Plugin (Tuning Regions)
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

#include "TutGenSearch.h"
#include "GDE3Search.h"

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
 * @ingroup TutGenSearch
 *
 * @param context a pointer to a context for the plugin
 * @param pool_set a pointer to a set of pools for the plugin
 */
void TutGenSearch::initialize( DriverContext*   context,
                               ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutGenSearch: call to initialize()\n" );

    this->context  = context;
    this->pool_set = pool_set;

    if( appl->get_regions().empty() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutGenSearch plugin : No Region found. Exiting.\n" );
        throw 0;
    }

    TuningParameter* numberOfThreadsTP = new TuningParameter();
    numberOfThreadsTP->setId( 0 );
    numberOfThreadsTP->setName( "NUMTHREADS" );
    numberOfThreadsTP->setPluginType( TUTGENSRCH );
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
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutGenSearch plugin: Before exiting to initialize, Tuning parameter is: %s\n\n",
                results.c_str() );

    int    major, minor;
    string name, description;

//! [initialize]
    string searchAlgoName = "gde3";
    context->loadSearchAlgorithm( searchAlgoName, &major, &minor, &name, &description );
    searchAlgorithm = context->getSearchAlgorithmInstance( searchAlgoName );
    if( searchAlgorithm ) {
        print_loaded_search( major, minor, name, description );
        GDE3Search* alg = reinterpret_cast<GDE3Search*>( searchAlgorithm );
        alg->setTimer( 10 );
        searchAlgorithm->initialize( context, pool_set );
    }
//! [initialize]
    noSigRegions = 4;
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
void TutGenSearch::startTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutGenSearch: call to startTuningStep()\n" );

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
                "TutGenSearch: found %d parallel regions (out of %d total).\n", parallel_regions, count );
}
//! [startTuningStep]

/**
 * @brief No pre-analysis is required.
 *
 *
 * @ingroup TutGenSearch
 *
 * @return false
 *
 */
//! [analysisRequired]
bool TutGenSearch::analysisRequired( StrategyRequest** strategy ) {
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
 * For each number of threads a single scenario is created by the search
 * algorithm and pushed to the Created Scenario Pool (csp).
 *
 * @ingroup TutGenSearch
 *
 */
//! [createScenarios]
void TutGenSearch::createScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutGenSearch: call to createScenarios()\n" );

    list<MetaProperty>::iterator property;
    list<MetaProperty>           found_properties = pool_set->arp->getPreAnalysisProperties( 0 );
    double                       severity         = 0;

    for( property = found_properties.begin(); property != found_properties.end(); property++ ) {
        regionExectimeMap.push_back( make_pair( property->getRegionId(), property->getSeverity() ) );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                    "TutGenSearch: Severity (%f) in region: %d added to vector\n",
                    property->getSeverity(), static_cast<int>( property->getRegionType() ) );
    }

    // sort the vector according to decreasing execution time
    std::sort( regionExectimeMap.begin(), regionExectimeMap.end(), pairCompare );

    for( int i = 0; i < noSigRegions; i++ ) {
        selected_region = code_region_candidates[ regionExectimeMap[ i ].first ];
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                    "TutGenSearch- selected region with: runtime: %f; ID: %s; file: %s; first line: %d; last line: %d;\n",
                    regionExectimeMap[ i ].second, selected_region->getIdForPropertyMatching().c_str(), selected_region->getFileName().c_str(),
                    selected_region->getFirstLine(), selected_region->getLastLine() );

        SearchSpace*  searchspace  = new SearchSpace();
        VariantSpace* variantSpace = new VariantSpace();

        for( int i = 0; i < tuningParameters.size(); i++ ) {
            variantSpace->addTuningParameter( tuningParameters[ i ] );
        }
        searchspace->setVariantSpace( variantSpace );
        searchspace->addRegion( selected_region );

        searchAlgorithm->addSearchSpace( searchspace );
    }
    searchAlgorithm->createScenarios();
}
//! [createScenarios]

/**
 * @brief Copies all scenarios to the Prepared Scenario Pool.
 *
 * There are no preparation done. All the scenarios are copied to the
 * Prepared Scenario Pool (psp).
 *
 * @ingroup TutGenSearch
 *
 */
//! [prepareScenarios]
void TutGenSearch::prepareScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutGenSearch: call to prepareScenarios()\n" );
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
 * @ingroup TutGenSearch
 *
 * @param numprocs         Number of processes required for the experiment
 * @param analysisRequired Defines is per-experiment analysis required
 * @param strategy         Strategy request for per-experiment analysis
 *
 */
//! [defineExperiment]
void TutGenSearch::defineExperiment( int               numprocs,
                                     bool&             analysisRequired,
                                     StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutGenSearch: call to defineExperiment()\n" );
    Scenario* scenario = pool_set->psp->pop();
    scenario->setSingleTunedRegionWithPropertyRank( selected_region, EXECTIME, 0 );
    pool_set->esp->push( scenario );
}
//! [defineExperiment]

/**
 * @brief This experiment does not require a restart.
 *
 * Instead of a restart the next phase execution is used to evaluated
 * the scenario.
 *
 * @ingroup TutGenSearch
 *
 * @return false
 *
 */
//! [restartRequired]
bool TutGenSearch::restartRequired( std::string& env,
                                    int&         numprocs,
                                    std::string& command,
                                    bool&        is_instrumented ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutGenSearch: call to restartRequired()\n" );
    return false;
}
//! [restartRequired]

/**
 * @brief Returns the status of the current search iteration.
 *
 * Returns always true since exhaustive search creates all the scenarios
 * in the first createScenario call.
 *
 * @ingroup TutGenSearch
 *
 * @return true
 *
 */
//! [searchFinished]
bool TutGenSearch::searchFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutGenSearch: call to searchFinished()\n" );
    return searchAlgorithm->searchFinished();
}
//! [searchFinished]

/**
 * @brief Nothing to be done in this step.
 *
 *
 * @ingroup TutGenSearch
 *
 */
//! [finishTuningStep]
void TutGenSearch::finishTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutGenSearch: call to processResults()\n" );
}
//! [finishTuningStep]

/**
 * @brief Returns true since no other tuning step is required.
 *
 * @ingroup TutGenSearch
 *
 * @return true
 *
 */
//! [tuningFinished]
bool TutGenSearch::tuningFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutGenSearch: call to tuningFinished()\n" );
    return true;
}
//! [tuningFinished]

/**
 * @brief Prints the scenarios, their execution time, and speedup.
 *
 * Prints the results and creates the advice file.
 *
 * @ingroup TutGenSearch
 */
//! [getAdvice]
Advice* TutGenSearch::getAdvice( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutGenSearch: call to getAdvice()\n" );

    list<TuningSpecification*>::iterator ts;
    std::ostringstream                   result_oss;
    map<int, double>                     timeForScenario = searchAlgorithm->getSearchPath();
    double                               serialTime      = timeForScenario[ 0 ];

    int optimum = searchAlgorithm->getOptimum();
    result_oss << endl << "Optimum Scenario: " << optimum << endl << endl;
    Scenario*                   best_scenario              = ( *pool_set->fsp->getScenarios() )[ optimum ];
    list<TuningSpecification*>* best_tuning_specifications = best_scenario->getTuningSpecifications();
    result_oss << "(Region,Threads): " << endl;
    for( ts = best_tuning_specifications->begin(); ts != best_tuning_specifications->end(); ts++ ) {
        map<TuningParameter*, int> best_thread_count = ( *ts )->getVariant()->getValue();
        int                        nthr              = best_thread_count.begin()->second;
        list<string>::iterator     region_iter;
        for( region_iter = ( *ts )->getVariantContext().context_union.entity_list->begin();
             region_iter != ( *ts )->getVariantContext().context_union.entity_list->end(); region_iter++ ) {
            result_oss << "(" << ( *region_iter ) << "," << nthr << ") ";
        }
    }

    result_oss << endl << endl << "All Results:" << endl;
    result_oss << "Scenario\t|  (Region,Threads)\t|  Time\t\t|  Speedup\t" << endl;

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

        double time = timeForScenario[ scenario_id ];
        result_oss << "\t\t|  " << time << "\t|  " << serialTime / time << endl;
    }

    cout << result_oss.str() << endl;

    //reset tuning specifications and sets default description in the scenario and execution time.
    map<int, Scenario*>::iterator scenario_iter;
    for( scenario_iter = pool_set->fsp->getScenarios()->begin(); scenario_iter != pool_set->fsp->getScenarios()->end(); scenario_iter++ ) {
        Scenario* sc = scenario_iter->second;
        sc->addResult( "Time", timeForScenario[ sc->getID() ] );
    }

    Scenario* bestScenario = ( *pool_set->fsp->getScenarios() )[ optimum ];
    return new Advice( getName(), bestScenario, timeForScenario, "Time", pool_set->fsp->getScenarios() );
}
//! [getAdvice]

/**
 * @brief Nothing done here.
 *
 *
 * @ingroup TutGenSearch
 *
 */
//! [finalize]
void TutGenSearch::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutGenSearch: call to finalize()\n" );
}
//! [finalize]

/**
 * @brief Nothing done here.
 *
 *
 *
 * @ingroup TutGenSearch
 *
 */
void TutGenSearch::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutGenSearch: call to terminate()\n" );
}


/**
 * @brief Returns an instance of this particular plugin implementation.
 *
 *
 * @ingroup TutGenSearch
 *
 * @return A pointer to a new TutGenSearch
 */
IPlugin* getPluginInstance( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutGenSearch: call to getComponentInstance()\n" );

    return new TutGenSearch();
}

/**
 * @brief Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 *
 * @ingroup TutGenSearch
 *
 * @return The major plugin interface version used by the plugin
 */
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutGenSearch: call to getInterfaceVersionMajor()\n" );

    return 1;
}

/**
 * @brief Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 *
 * @ingroup TutGenSearch
 *
 * @return The minor plugin interface version used by the plugin
 *
 */
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutGenSearch: call to getInterfaceVersionMinor()\n" );

    return 0;
}

/**
 * @brief Returns the plugin name TutGenSearch.
 *
 * @ingroup TutGenSearch
 *
 * @return Returns TutGenSearch
 *
 */
string getName( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutGenSearch: call to getName()\n" );

    return "TutGenSearch";
}

/**
 * @brief Returns a short description of the plugin.
 *
 * @ingroup TutGenSearch
 *
 * @return A string with a short description of the plugin.
 *
 */
string getShortSummary( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutGenSearch: call to getShortSummary()\n" );

    return "Using genetic search with time limit";
}
