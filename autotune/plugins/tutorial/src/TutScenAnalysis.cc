/**
   @file    TutScenAnalysis.cc
   @ingroup TutScenAnalysis
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

#include "TutScenAnalysis.h"

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
 * @ingroup TutScenAnalysis
 *
 * @param context a pointer to a context for the plugin
 * @param pool_set a pointer to a set of pools for the plugin
 */
//! [initialize]
void TutScenAnalysis::initialize( DriverContext*   context,
                                  ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutScenAnalysis: call to initialize()\n" );

    this->context  = context;
    this->pool_set = pool_set;

    if( appl->get_regions().empty() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutScenAnalysis plugin : No Region found. Exiting.\n" );
        throw 0;
    }

    TuningParameter* numberOfThreadsTP = new TuningParameter();
    numberOfThreadsTP->setId( 0 );
    numberOfThreadsTP->setName( "NUMTHREADS" );
    numberOfThreadsTP->setPluginType( TUTSCENANALYSIS );
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
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutScenAnalysis plugin: Before exiting to initialize, Tuning parameter is: %s\n\n",
                results.c_str() );

    int    major, minor;
    string name, description;
    context->loadSearchAlgorithm( "exhaustive", &major, &minor, &name, &description );
    searchAlgorithm = context->getSearchAlgorithmInstance( "exhaustive" );
    if( searchAlgorithm ) {
        print_loaded_search( major, minor, name, description );
        searchAlgorithm->initialize( context, pool_set );
    }
    noSigRegions = 4;
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
void TutScenAnalysis::startTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutScenAnalysis: call to startTuningStep()\n" );

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
                "TutScenAnalysis: found %d parallel regions (out of %d total).\n", parallel_regions, count );
}
//! [startTuningStep]

/**
 * @brief No pre-analysis is required.
 *
 *
 * @ingroup TutScenAnalysis
 *
 * @return false
 *
 */
//! [analysisRequired]
bool TutScenAnalysis::analysisRequired( StrategyRequest** strategy ) {
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
 * @ingroup TutScenAnalysis
 *
 */
//! [createScenarios]
void TutScenAnalysis::createScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutScenAnalysis: call to createScenarios()\n" );

    list<MetaProperty>::iterator property;
    list<MetaProperty>           found_properties = pool_set->arp->getPreAnalysisProperties( 0 );
    double                       severity         = 0;

    for( property = found_properties.begin(); property != found_properties.end(); property++ ) {
        regionExectimeMap.push_back( make_pair( property->getRegionId(), property->getSeverity() ) );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                    "TutScenAnalysis: Severity (%f) in region: %d added to vector\n",
                    property->getSeverity(), static_cast<int>( property->getRegionType() ) );
    }

    // sort the vector according to decreasing execution time
    std::sort( regionExectimeMap.begin(), regionExectimeMap.end(), pairCompare );

    SearchSpace*  searchspace  = new SearchSpace();
    VariantSpace* variantSpace = new VariantSpace();

    for( int i = 0; i < tuningParameters.size(); i++ ) {
        variantSpace->addTuningParameter( tuningParameters[ i ] );
    }
    searchspace->setVariantSpace( variantSpace );
    searchspace->addRegion( appl->get_phase_region() );

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
 * @ingroup TutScenAnalysis
 *
 */
//! [prepareScenarios]
void TutScenAnalysis::prepareScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutScenAnalysis: call to prepareScenarios()\n" );
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
 * @ingroup TutScenAnalysis
 *
 * @param numprocs         Number of processes required for the experiment
 * @param analysisRequired Defines is per-experiment analysis required
 * @param strategy         Strategy request for per-experiment analysis
 *
 */
//! [defineExperiment]
void TutScenAnalysis::defineExperiment( int               numprocs,
                                        bool&             analysisRequired,
                                        StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutScenAnalysis: call to defineExperiment()\n" );
    Scenario* scenario = pool_set->psp->pop();
    scenario->setSingleTunedRegionWithPropertyRank( appl->get_phase_region(), EXECTIME, 0 );
    pool_set->esp->push( scenario );

    // requesting region specific properties
    StrategyRequestGeneralInfo* analysisStrategyRequest = new StrategyRequestGeneralInfo;
    analysisStrategyRequest->strategy_name     = "ConfigAnalysis";
    analysisStrategyRequest->pedantic          = 1;
    analysisStrategyRequest->delay_phases      = 0;
    analysisStrategyRequest->delay_seconds     = 0;
    analysisStrategyRequest->analysis_duration = 1;

    list<PropertyRequest*>* reqList = new list<PropertyRequest*>;
    for( int i = 0; i < noSigRegions; i++ ) {
        PropertyRequest* req = new PropertyRequest();
        req->addPropertyID( EXECTIME );
        req->addRegion( code_region_candidates[ regionExectimeMap[ i ].first ] );
        req->addSingleProcess( 0 );
        reqList->push_back( req );
    }

    StrategyRequest* sub_strategy = new StrategyRequest( reqList, analysisStrategyRequest );
    ( *strategy ) = sub_strategy;
    ( *strategy )->printStrategyRequest();
    analysisRequired = true;
}
//! [defineExperiment]

/**
 * @brief This experiment does not require a restart.
 *
 * Instead of a restart the next phase execution is used to evaluated
 * the scenario.
 *
 * @ingroup TutScenAnalysis
 *
 * @return false
 *
 */
//! [restartRequired]
bool TutScenAnalysis::restartRequired( std::string& env,
                                       int&         numprocs,
                                       std::string& command,
                                       bool&        is_instrumented ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutScenAnalysis: call to restartRequired()\n" );
    return false;
}
//! [restartRequired]

/**
 * @brief Returns the status of the current search iteration.
 *
 * Returns always true since exhaustive search creates all the scenarios
 * in the first createScenario call.
 *
 * @ingroup TutScenAnalysis
 *
 * @return true
 *
 */
//! [searchFinished]
bool TutScenAnalysis::searchFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutScenAnalysis: call to searchFinished()\n" );
    return searchAlgorithm->searchFinished();
}
//! [searchFinished]

/**
 * @brief Nothing to be done in this step.
 *
 *
 * @ingroup TutScenAnalysis
 *
 */
//! [finishTuningStep]
void TutScenAnalysis::finishTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutScenAnalysis: call to processResults()\n" );
}
//! [finishTuningStep]

/**
 * @brief Returns true since no other tuning step is required.
 *
 * @ingroup TutScenAnalysis
 *
 * @return true
 *
 */
//! [tuningFinished]
bool TutScenAnalysis::tuningFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutScenAnalysis: call to tuningFinished()\n" );
    return true;
}
//! [tuningFinished]

/**
 * @brief Prints the scenarios, their execution time, and speedup.
 *
 * Prints the results and creates the advice file.
 *
 * @ingroup TutScenAnalysis
 */
Advice* TutScenAnalysis::getAdvice( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutScenAnalysis: call to getAdvice()\n" );

    list<TuningSpecification*>::iterator ts;
    std::ostringstream                   result_oss;
    map<int, double>                     timeForScenario = searchAlgorithm->getSearchPath();
    double                               serialTime      = timeForScenario[ 0 ];

    list<Scenario* > bestScenarios;
    int              optimum = searchAlgorithm->getOptimum();
    result_oss << endl << "Optimum Scenario: " << optimum << endl;

    Scenario* best_scenario = ( *pool_set->fsp->getScenarios() )[ optimum ];
    bestScenarios.push_back( best_scenario );

    list<TuningSpecification*>* best_tuning_specifications = best_scenario->getTuningSpecifications();
    result_oss << "Threads: ";
    for( ts = best_tuning_specifications->begin(); ts != best_tuning_specifications->end(); ts++ ) {
        map<TuningParameter*, int> best_thread_count = ( *ts )->getVariant()->getValue();
        int                        nthr              = best_thread_count.begin()->second;
        result_oss << nthr << " " << endl;
    }

    result_oss << endl << "All Results:" << endl;
    result_oss << "Scenario\t|  Threads\t|  Time\t\t|  Speedup\t" << endl;

    for( int scenario_id = 0; scenario_id < pool_set->fsp->size(); scenario_id++ ) {
        Scenario*                   sc          = ( *pool_set->fsp->getScenarios() )[ scenario_id ];
        list<TuningSpecification*>* tuningSpecs = sc->getTuningSpecifications();
        result_oss << scenario_id << "\t\t|  ";
        int nthr = 0;
        for( ts = tuningSpecs->begin(); ts != tuningSpecs->end(); ts++ ) {
            map<TuningParameter*, int> tpValues = ( *ts )->getVariant()->getValue();
            nthr = tpValues.begin()->second;
        }

        double time = timeForScenario[ scenario_id ];
        result_oss << nthr <<  "\t\t|  " << time << "\t|  " << serialTime / time << endl;
    }

    map<int, list<MetaProperty> >                 scenarioAnalysisPropertiesMap;
    map<int, list<MetaProperty> >::const_iterator propertyMapIter;
    list<MetaProperty>                            properties;
    list<MetaProperty>::iterator                  p;

    string regionName[ noSigRegions ];
    int    regionBestThreads[ noSigRegions ];
    int    regionBestScenario[ noSigRegions ];
    double regionExecTime[ noSigRegions ];

    // init
    for( int i = 0; i < noSigRegions; i++ ) {
        regionBestThreads[ i ] = 0;
        regionExecTime[ i ]    = 100000; // init to minimize
    }

    scenarioAnalysisPropertiesMap = pool_set->arp->getAllExperimentProperties();
    int numExp = scenarioAnalysisPropertiesMap.size();
    int cntr   = 0;
    result_oss << endl << "Results for individual regions:" << endl << endl;
    for( int exp = 0; exp < numExp; exp++ ) {
        properties = pool_set->arp->getExperimentProperties( exp );
        cntr       = 0;

        for( p = properties.begin(); p != properties.end(); p++ ) {
            Region* reg = appl->getRegionByID( p->getRegionId() );
            result_oss << "Region: " << reg->getFileName() << " Time: " << p->getSeverity() << "\t";

            int                        scenarioID = atoi( p->getExtraInfo().at( "ScenarioID" ).c_str() );
            Scenario*                  sc         = ( *pool_set->fsp->getScenarios() )[ scenarioID ];
            map<TuningParameter*, int> tpValues   = sc->getTuningSpecifications()->front()->getVariant()->getValue();
            int                        noThread   = tpValues.begin()->second;

            if( p->getSeverity() < regionExecTime[ cntr ] ) {
                regionExecTime[ cntr ]     = p->getSeverity();
                regionBestThreads[ cntr ]  = noThread;
                regionBestScenario[ cntr ] = scenarioID;
            }

            if( exp == 0 ) {
                regionName[ cntr ] = reg->getFileName();
            }
            cntr++;
        }
        result_oss << endl;
    }

//! [getAdvice]
    result_oss << endl << "Best configurations for individual regions:" << endl << endl;

    for( int i = 0; i < noSigRegions; i++ ) {
        result_oss << "Region: " << regionName[ i ] << " ";
        result_oss << "Threads: " << regionBestThreads[ i ] << " ";
        result_oss << "ExecTime: " << regionExecTime[ i ] << endl;

        // accumulating best scenario description
        Scenario*         sc              = ( *pool_set->fsp->getScenarios() )[ regionBestScenario[ i ] ];
        string            scenDescription = sc->getDescription();
        std::stringstream strStream;
        strStream << "Best scenario for region: " << regionName[ i ];
        if( scenDescription.empty() ) {
            sc->setDescription( strStream.str() );
        }
        else {
            scenDescription = scenDescription + " | " + strStream.str();
            sc->setDescription( scenDescription );
        }
    }
//! [getAdvice]

    cout << endl << result_oss.str() << endl;

    //reset tuning specifications and sets default description in the scenario and execution time.
    map<int, Scenario*>::iterator scenario_iter;
    for( scenario_iter = pool_set->fsp->getScenarios()->begin(); scenario_iter != pool_set->fsp->getScenarios()->end(); scenario_iter++ ) {
        Scenario* sc = scenario_iter->second;
        sc->addResult( "Time", timeForScenario[ sc->getID() ] );
    }

    return new Advice( getName(), bestScenarios, timeForScenario, "Time", pool_set->fsp->getScenarios() );
}

/**
 * @brief Nothing done here.
 *
 *
 * @ingroup TutScenAnalysis
 *
 */
//! [finalize]
void TutScenAnalysis::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutScenAnalysis: call to finalize()\n" );
}
//! [finalize]

/**
 * @brief Nothing done here.
 *
 *
 *
 * @ingroup TutScenAnalysis
 *
 */
void TutScenAnalysis::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutScenAnalysis: call to terminate()\n" );
}


/**
 * @brief Returns an instance of this particular plugin implementation.
 *
 *
 * @ingroup TutScenAnalysis
 *
 * @return A pointer to a new TutScenAnalysis
 */
IPlugin* getPluginInstance( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutScenAnalysis: call to getComponentInstance()\n" );

    return new TutScenAnalysis();
}

/**
 * @brief Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 *
 * @ingroup TutScenAnalysis
 *
 * @return The major plugin interface version used by the plugin
 */
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutScenAnalysis: call to getInterfaceVersionMajor()\n" );

    return 1;
}

/**
 * @brief Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 *
 * @ingroup TutScenAnalysis
 *
 * @return The minor plugin interface version used by the plugin
 *
 */
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutScenAnalysis: call to getInterfaceVersionMinor()\n" );

    return 0;
}

/**
 * @brief Returns the plugin name TutScenAnalysis.
 *
 * @ingroup TutScenAnalysis
 *
 * @return Returns TutScenAnalysis
 *
 */
string getName( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutScenAnalysis: call to getName()\n" );

    return "TutScenAnalysis";
}

/**
 * @brief Returns a short description of the plugin.
 *
 * @ingroup TutScenAnalysis
 *
 * @return A string with a short description of the plugin.
 *
 */
string getShortSummary( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutScenAnalysis: call to getShortSummary()\n" );

    return "Scenario analysis to get significant regions in one experiment";
}
