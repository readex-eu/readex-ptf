/**
   @file    TutMultiObj.cc
   @ingroup TutMultiObj
   @brief   Tutorial of a Plugin
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

#include "TutMultiObj.h"

/**
 * @brief Initialize the plugin's data structures.
 *
 * This plugin has one tuning paramter NUMTHREADS. It takes values in the
 * power of 2, starting from 1 to number of threads given by --ompnumthreads
 * on the commandline.
 *
 * It is based on a function tuning action. The function is set_omp_num_threads(int).
 * It is executed when the phase region is started.
 *
 * The initialization also copies the context and the pool set to the plugin attributes.
 *
 * It loads the exhaustive search algorithm and initializes it with the context and the
 * pool set of the plugin.
 *
 * @ingroup TutMultiObj
 *
 * @param context a pointer to a context for the plugin
 * @param pool_set a pointer to a set of pools for the plugin
 */
//! [initialize]
void TutMultiObj::initialize( DriverContext*   context,
                              ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiObj: call to initialize()\n" );

    this->context  = context;
    this->pool_set = pool_set;

    if( appl->get_regions().empty() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutGenSearch plugin : No Region found. Exiting.\n" );
        throw 0;
    }

    TuningParameter* numberOfThreadsTP = new TuningParameter();
    numberOfThreadsTP->setId( 0 );
    numberOfThreadsTP->setName( "NUMTHREADS" );
    numberOfThreadsTP->setPluginType( TUTVECTORRANGE );
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
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiObj plugin: Before exiting to initialize, Tuning parameter is: %s\n\n",
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
 * For the search space, the phase region is given as region to
 * which the tuning action is applied.
 *
 */
//! [startTuningStep]
void TutMultiObj::startTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiObj: call to startTuningStep()\n" );
    VariantSpace* variantSpace = new VariantSpace();
    SearchSpace*  searchSpace  = new SearchSpace();

    for( int i = 0; i < tuningParameters.size(); i++ ) {
        variantSpace->addTuningParameter( tuningParameters[ i ] );
    }
    searchSpace->setVariantSpace( variantSpace );
    searchSpace->addRegion( appl->get_phase_region() );

    searchAlgorithm->addSearchSpace( searchSpace );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiObj: Created a SearchSpace from TP NUMTHREADS.\n" );
}
//! [startTuningStep]

/**
 * @brief No pre-analysis is required.
 *
 *
 * @ingroup TutMultiObj
 *
 * @return false
 *
 */
//! [analysisRequired]
bool TutMultiObj::analysisRequired( StrategyRequest** strategy ) {
    return false;
}
//! [analysisRequired]

/**
 * @brief All scenarios are created.
 *
 * For each number of threads a single scenario is created by the search
 * algorithm and pushed to the Created Scenario Pool (csp).
 *
 * @ingroup TutMultiObj
 *
 */
//! [createScenarios]
void TutMultiObj::createScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiObj: call to createScenarios()\n" );
    searchAlgorithm->createScenarios();
}
//! [createScenarios]

/**
 * @brief Copies all scenarios to the Prepared Scenario Pool.
 *
 * There are no preparation done. All the scenarios are copied to the
 * Prepared Scenario Pool (psp).
 *
 * @ingroup TutMultiObj
 *
 */
//! [prepareScenarios]
void TutMultiObj::prepareScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiObj: call to prepareScenarios()\n" );
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
 * @ingroup TutMultiObj
 *
 * @param numprocs         Number of processes required for the experiment
 * @param analysisRequired Defines is per-experiment analysis required
 * @param strategy         Strategy request for per-experiment analysis
 *
 */
//! [defineExperiment]
void TutMultiObj::defineExperiment( int               numprocs,
                                    bool&             analysisRequired,
                                    StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiObj: call to defineExperiment()\n" );
    Scenario* scenario = pool_set->psp->pop();

    list<PropertyRequest*>* propertyRequestList = new list<PropertyRequest*>();
    list<int>*              propertyIds         = new list<int>;

    propertyIds->push_back( ENERGY_CONSUMPTION );
    propertyIds->push_back( EXECTIME );
    propertyRequestList->push_back( new PropertyRequest( propertyIds ) );
    scenario->setPropertyRequests( propertyRequestList );
    scenario->setTunedRegion( appl->get_phase_region() );

    pool_set->esp->push( scenario );
}
//! [defineExperiment]

/**
 * @brief This experiment does not require a restart.
 *
 * Instead of a restart the next phase execution is used to evaluated
 * the scenario.
 *
 * @ingroup TutMultiObj
 *
 * @return false
 *
 */
//! [restartRequired]
bool TutMultiObj::restartRequired( std::string& env,
                                   int&         numprocs,
                                   std::string& command,
                                   bool&        is_instrumented ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiObj: call to restartRequired()\n" );
    return false;
}
//! [restartRequired]

/**
 * @brief Returns the status of the current search iteration.
 *
 * Returns always true since exhaustive search creates all the scenarios
 * in the first createScenario call.
 *
 * @ingroup TutMultiObj
 *
 * @return true
 *
 */
//! [searchFinished]
bool TutMultiObj::searchFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiObj: call to searchFinished()\n" );
    return searchAlgorithm->searchFinished();
}
//! [searchFinished]

/**
 * @brief Nothing to be done in this step.
 *
 *
 * @ingroup TutMultiObj
 *
 */
//! [finishTuningStep]
void TutMultiObj::finishTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiObj: call to processResults()\n" );
}
//! [finishTuningStep]

/**
 * @brief Returns true since no other tuning step is required.
 *
 * @ingroup TutMultiObj
 *
 * @return true
 *
 */
//! [tuningFinished]
bool TutMultiObj::tuningFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiObj: call to tuningFinished()\n" );
    return true;
}
//! [tuningFinished]

/**
 * @brief Prints the scenarios, their execution time, and speedup.
 *
 * Prints the results and creates the advice file.
 *
 * @ingroup TutMultiObj
 */
//! [getAdvice]
Advice* TutMultiObj::getAdvice( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiObj: call to getAdvice()\n" );

    std::ostringstream result_oss;
    map<int, double>   timeForScenario = searchAlgorithm->getSearchPath();

    int optimum = searchAlgorithm->getOptimum();
    result_oss << "Optimum Scenario: " << optimum << endl << endl;


    result_oss << "\nAll Results:\n";
    result_oss << "Scenario\t|  Threads\t|  Time\t|  Energy Consumption\t|  Energy Delay Product\t\n";

    for( int scenario_id = 0; scenario_id < pool_set->fsp->size(); scenario_id++ ) {
        Scenario* sc = ( *pool_set->fsp->getScenarios() )[ scenario_id ];

        list<TuningSpecification*>* tuningSpec = sc->getTuningSpecifications();
        map<TuningParameter*, int>  tpValues   = tuningSpec->front()->getVariant()->getValue();
        int                         threads    = tpValues[ tuningParameters[ 0 ] ];

        double time              = 0;
        double energyConsumption = 0;

        list<MetaProperty> properties = pool_set->srp->getScenarioResultsByID( scenario_id );
        for( std::list<MetaProperty>::iterator iterator = properties.begin(),
             end = properties.end(); iterator != end; ++iterator ) {
            if( atoi( iterator->getId().c_str() ) == ENERGY_CONSUMPTION ) {
                energyConsumption = iterator->getSeverity();
                sc->addResult( "Energy", iterator->getSeverity() );
            }
            if( atoi( iterator->getId().c_str() ) == EXECTIME ) {
                time = iterator->getSeverity();
                sc->addResult( "Time", iterator->getSeverity() );
            }
        }

        result_oss << scenario_id << "\t\t|  " << threads << "\t\t|  " << time << "\t|  " << energyConsumption << "\t|  " << energyConsumption * time << endl;
    }
    result_oss << "\n------------------------" << endl << endl;

    cout << result_oss.str();

    Scenario* bestScenario = ( *pool_set->fsp->getScenarios() )[ optimum ];
    return new Advice( getName(), bestScenario, timeForScenario, "Time", pool_set->fsp->getScenarios() );
}
//! [getAdvice]

/**
 * @brief Nothing done here.
 *
 *
 * @ingroup TutMultiObj
 *
 */
//! [finalize]
void TutMultiObj::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiObj: call to finalize()\n" );
}
//! [finalize]

/**
 * @brief Nothing done here.
 *
 *
 *
 * @ingroup TutMultiObj
 *
 */
void TutMultiObj::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiObj: call to terminate()\n" );
}


/**
 * @brief Returns an instance of this particular plugin implementation.
 *
 *
 * @ingroup TutMultiObj
 *
 * @return A pointer to a new TutMultiObj
 */
IPlugin* getPluginInstance( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiObj: call to getComponentInstance()\n" );

    return new TutMultiObj();
}

/**
 * @brief Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 *
 * @ingroup TutMultiObj
 *
 * @return The major plugin interface version used by the plugin
 */
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiObj: call to getInterfaceVersionMajor()\n" );

    return 1;
}

/**
 * @brief Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 *
 * @ingroup TutMultiObj
 *
 * @return The minor plugin interface version used by the plugin
 *
 */
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiObj: call to getInterfaceVersionMinor()\n" );

    return 0;
}

/**
 * @brief Returns the plugin name TutMultiObj.
 *
 * @ingroup TutMultiObj
 *
 * @return Returns TutMultiObj
 *
 */
string getName( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiObj: call to getName()\n" );

    return "TutMultiObj";
}

/**
 * @brief Returns a short description of the plugin.
 *
 * @ingroup TutMultiObj
 *
 * @return A string with a short description of the plugin.
 *
 */
string getShortSummary( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "TutMultiObj: call to getShortSummary()\n" );

    return "Explores scalability of OpenMP codes with exhaustive search. Tuning parameter has vector range restriction.";
}
