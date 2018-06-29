/**
   @file    TutMultipleRegions.cc
   @ingroup TutMultipleRegions
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

#include "TutMultipleRegions.h"

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
 * @ingroup TutMultipleRegions
 *
 * @param context a pointer to a context for the plugin
 * @param pool_set a pointer to a set of pools for the plugin
 */
//! [initialize]
void TutMultipleRegions::initialize( DriverContext*   context,
                                     ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultipleRegions: call to initialize()\n" );

    this->context  = context;
    this->pool_set = pool_set;

    if( appl->get_regions().empty() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultipleRegions plugin : No Region found. Exiting.\n" );
        throw 0;
    }

    TuningParameter* numberOfThreadsTP = new TuningParameter();
    numberOfThreadsTP->setId( 0 );
    numberOfThreadsTP->setName( "NUMTHREADS" );
    numberOfThreadsTP->setPluginType( TUTREGIONS );
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
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultipleRegions plugin: Before exiting to initialize, Tuning parameter is: %s\n\n",
                results.c_str() );

    int    major, minor;
    string name, description;
    context->loadSearchAlgorithm( "exhaustive", &major, &minor, &name, &description );
    searchAlgorithm = context->getSearchAlgorithmInstance( "exhaustive" );
    if( searchAlgorithm ) {
        print_loaded_search( major, minor, name, description );
        searchAlgorithm->initialize( context, pool_set );
    }
}
//! [initialize]

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
void TutMultipleRegions::startTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultipleRegions: call to startTuningStep()\n" );
    std::list<Region*> code_regions;
    code_regions = appl->get_regions();
    std::list<Region*>::iterator region;

    // iterating over all regions
    int count = 0, parallel_regions = 0;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Processing all regions...\n" );
    for( region = code_regions.begin(); region != code_regions.end(); region++ ) {
        if( ( *region )->get_type() == PARALLEL_REGION ) {
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
                "TutMultipleRegions: found %d parallel regions (out of %d total).\n", parallel_regions, count );
}
//! [startTuningStep]

/**
 * @brief No pre-analysis is required.
 *
 *
 * @ingroup TutMultipleRegions
 *
 * @return false
 *
 */
//! [analysisRequired]
bool TutMultipleRegions::analysisRequired( StrategyRequest** strategy ) {
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
 * @brief All scenarios are created.
 *
 * For each number of threads a single scenario is created by the search
 * algorithm and pushed to the Created Scenario Pool (csp).
 *
 * @ingroup TutMultipleRegions
 *
 */
//! [createScenarios]
void TutMultipleRegions::createScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultipleRegions: call to createScenarios()\n" );
    list<MetaProperty>::iterator property;
    MetaProperty                 longest_running_property;
    list<MetaProperty>           found_properties = pool_set->arp->getPreAnalysisProperties( 0 );
    double                       severity         = 0;

    SearchSpace*  searchspace  = new SearchSpace();
    VariantSpace* variantSpace = new VariantSpace();

    for( int i = 0; i < tuningParameters.size(); i++ ) {
        variantSpace->addTuningParameter( tuningParameters[ i ] );
    }
    searchspace->setVariantSpace( variantSpace );

    for( property = found_properties.begin(); property != found_properties.end(); property++ ) {
        if( property->getSeverity() > severity ) {
            severity                 = property->getSeverity();
            longest_running_property = *property;
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                        "TutMultipleRegions: current max. (changed) severity (%f) in region: %d\n",
                        property->getSeverity(), static_cast<int>( property->getRegionType() ) );
        }
        else {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                        "TutMultipleRegions: current max. (not changed) severity (%f) in region: %d\n",
                        property->getSeverity(), static_cast<int>( property->getRegionType() ) );
        }
    }
    selected_region = code_region_candidates[ longest_running_property.getRegionId() ];
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                "TutMultipleRegions: selected region with: runtime: %f; ID: %s; file: %s; first line: %d; last line: %d;\n",
                longest_running_property.getSeverity(), selected_region->getIdForPropertyMatching().c_str(), selected_region->getFileName().c_str(),
                selected_region->getFirstLine(), selected_region->getLastLine() );

    searchspace->addRegion( selected_region );

    searchAlgorithm->addSearchSpace( searchspace );
    searchAlgorithm->createScenarios();
}
//! [createScenarios]

/**
 * @brief Copies all scenarios to the Prepared Scenario Pool.
 *
 * There are no preparation done. All the scenarios are copied to the
 * Prepared Scenario Pool (psp).
 *
 * @ingroup TutMultipleRegions
 *
 */
//! [prepareScenarios]
void TutMultipleRegions::prepareScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultipleRegions: call to prepareScenarios()\n" );
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
 * @ingroup TutMultipleRegions
 *
 * @param numprocs         Number of processes required for the experiment
 * @param analysisRequired Defines is per-experiment analysis required
 * @param strategy         Strategy request for per-experiment analysis
 *
 */
//! [defineExperiment]
void TutMultipleRegions::defineExperiment( int               numprocs,
                                           bool&             analysisRequired,
                                           StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultipleRegions: call to defineExperiment()\n" );
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
 * @ingroup TutMultipleRegions
 *
 * @return false
 *
 */
//! [restartRequired]
bool TutMultipleRegions::restartRequired( std::string& env,
                                          int&         numprocs,
                                          std::string& command,
                                          bool&        is_instrumented ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultipleRegions: call to restartRequired()\n" );
    return false;
}
//! [restartRequired]

/**
 * @brief Returns the status of the current search iteration.
 *
 * Returns always true since exhaustive search creates all the scenarios
 * in the first createScenario call.
 *
 * @ingroup TutMultipleRegions
 *
 * @return true
 *
 */
//! [searchFinished]
bool TutMultipleRegions::searchFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultipleRegions: call to searchFinished()\n" );
    return searchAlgorithm->searchFinished();
}
//! [searchFinished]

/**
 * @brief Nothing to be done in this step.
 *
 *
 * @ingroup TutMultipleRegions
 *
 */
//! [finishTuningStep]
void TutMultipleRegions::finishTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultipleRegions: call to processResults()\n" );
}
//! [finishTuningStep]

/**
 * @brief Returns true since no other tuning step is required.
 *
 * @ingroup TutMultipleRegions
 *
 * @return true
 *
 */
//! [tuningFinished]
bool TutMultipleRegions::tuningFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultipleRegions: call to tuningFinished()\n" );
    return true;
}
//! [tuningFinished]

/**
 * @brief Prints the scenarios, their execution time, and speedup.
 *
 * Prints the results and creates the advice file.
 *
 * @ingroup TutMultipleRegions
 */
//! [getAdvice]
Advice* TutMultipleRegions::getAdvice( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultipleRegions: call to getAdvice()\n" );

    list<TuningSpecification*>::iterator ts;
    std::ostringstream                   result_oss;
    map<int, double>                     timeForScenario = searchAlgorithm->getSearchPath();
    double                               serialTime      = timeForScenario[ 0 ];

    int optimum = searchAlgorithm->getOptimum();
    result_oss << endl << "Optimum Scenario: " << optimum << endl << endl;
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

    result_oss << endl << endl << "All Results:" << endl;
    result_oss << "Scenario\t|  Threads\t|  Time\t\t|  Speedup\t" << endl;

    for( int scenario_id = 0; scenario_id < pool_set->fsp->size(); scenario_id++ ) {
        Scenario*                   sc          = ( *pool_set->fsp->getScenarios() )[ scenario_id ];
        list<TuningSpecification*>* tuningSpecs = sc->getTuningSpecifications();
        result_oss << scenario_id << "\t\t|  ";

        for( ts = tuningSpecs->begin(); ts != tuningSpecs->end(); ts++ ) {
            map<TuningParameter*, int> tpValues = ( *ts )->getVariant()->getValue();
            result_oss << tpValues.begin()->second << " ";
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
 * @ingroup TutMultipleRegions
 *
 */
//! [finalize]
void TutMultipleRegions::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultipleRegions: call to finalize()\n" );
}
//! [finalize]

/**
 * @brief Nothing done here.
 *
 *
 *
 * @ingroup TutMultipleRegions
 *
 */
void TutMultipleRegions::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultipleRegions: call to terminate()\n" );
}


/**
 * @brief Returns an instance of this particular plugin implementation.
 *
 *
 * @ingroup TutMultipleRegions
 *
 * @return A pointer to a new TutMultipleRegions
 */
IPlugin* getPluginInstance( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultipleRegions: call to getComponentInstance()\n" );

    return new TutMultipleRegions();
}

/**
 * @brief Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 *
 * @ingroup TutMultipleRegions
 *
 * @return The major plugin interface version used by the plugin
 */
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultipleRegions: call to getInterfaceVersionMajor()\n" );

    return 1;
}

/**
 * @brief Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 *
 * @ingroup TutMultipleRegions
 *
 * @return The minor plugin interface version used by the plugin
 *
 */
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultipleRegions: call to getInterfaceVersionMinor()\n" );

    return 0;
}

/**
 * @brief Returns the plugin name TutMultipleRegions.
 *
 * @ingroup TutMultipleRegions
 *
 * @return Returns TutMultipleRegions
 *
 */
string getName( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultipleRegions: call to getName()\n" );

    return "TutMultipleRegions";
}

/**
 * @brief Returns a short description of the plugin.
 *
 * @ingroup TutMultipleRegions
 *
 * @return A string with a short description of the plugin.
 *
 */
string getShortSummary( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultipleRegions: call to getShortSummary()\n" );

    return "Tunes parallel regions as listed in region lists";
}
