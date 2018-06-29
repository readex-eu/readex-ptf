/**
   @file    LoadSearch.cc
   @ingroup LoadSearch
   @brief   Load search algorithm
   @author  Author's name
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2015, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include "../include/LoadSearch.h"
#include "search_common.h"

/**
 * @brief Constructor description
 * @ingroup LoadSearch
 *
 **/
LoadSearch::LoadSearch() : ISearchAlgorithm() {
    optimumValue = std::numeric_limits<double>::max();
}

/**
 * @brief Destructor description
 * @ingroup LoadSearch
 *
 **/
LoadSearch::~LoadSearch() {
}

/**
 * @brief Initialize description
 * @ingroup LoadSearch
 *
 * Long description.
 *
 */
void LoadSearch::initialize( DriverContext*   context,
                             ScenarioPoolSet* poolSet ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "LoadSearch: call to initialize()\n" );

    this->pool_set = poolSet;

    parseLoadConfig( "load_config.cfg", scenarioMap );
}

/**
 * @brief Search Finished description
 * @ingroup LoadSearch
 *
 * Long description.
 *
 */
bool LoadSearch::searchFinished() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "LoadSearch: call to searchFinished()\n" );

 	while (!scenarioIds.empty()) {
 		int scenario_id = scenarioIds.front();
 		scenarioIds.pop();
 		for (int i = 0; i < objectiveFunctions.size(); i++) {
 			double objValue = objectiveFunctions[i]->objective(scenario_id, pool_set->srp);
 			path[scenario_id] = objValue;
 			Scenario* scenario= pool_set->fsp->getScenarioByScenarioID(scenario_id);
 			scenario->addResult(objectiveFunctions[i]->getName(), objValue);

 			if (objValue < optimumValue) {
 				optimum = scenario_id;
 				optimumValue = objValue;
 			}
 		}
 	}
   return true;
}


void LoadSearch::addObjectiveFunction(ObjectiveFunction *obj){
   objectiveFunctions.push_back(obj);
}


/**
 * @brief Get Optimum description
 * @ingroup LoadSearch
 *
 * Long description.
 *
 */
int LoadSearch::getOptimum() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "LoadSearch: call to getOptimum()\n" );

    return optimum;
}

/**
 * @brief Clear description
 * @ingroup LoadSearch
 *
 * Long description.
 *
 */
void LoadSearch::clear() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "LoadSearch: call to clear()\n" );
    optimumValue = std::numeric_limits<double>::max();
    path.clear();
    searchSpaces.clear();
}

/**
 * @brief Get Search Path description
 * @ingroup LoadSearch
 *
 * Long description.
 *
 */
map<int, double> LoadSearch::getSearchPath() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "LoadSearch: call to getSearchPath()\n" );
    return path;
}

/**
 * @brief Add Search Space description
 * @ingroup LoadSearch
 *
 * Long description.
 *
 */
void LoadSearch::addSearchSpace( SearchSpace* searchSpace ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "LoadSearch: call to addSearchSpace()\n" );
    searchSpaces.push_back( searchSpace );
}

/**
 * @brief Create Scenarios description
 * @ingroup LoadSearch
 *
 * Long description.
 *
 */
void LoadSearch::createScenarios() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "LoadSearch: call to createScenarios()\n" );


    if (objectiveFunctions.size()==0){
   	 addObjectiveFunction(new PTF_minObjective(""));
    }


// Works
/******************************************************************************/
    list<TuningSpecification*>* ts  = new list<TuningSpecification*>();
    vector<TuningParameter*>    tps = searchSpaces[ 0 ]->getVariantSpace()->getTuningParameters();

    for( vector<unsigned int>::iterator sId = scenarioMap.begin();
         sId != scenarioMap.end(); sId++ ) {
//    printf("scenarioId = %d\n", *sId);

        map<TuningParameter*, int>* variant_map = new map<TuningParameter*, int>;

        //Here we set up values for tuning parameters and create a variant
        for( std::vector<TuningParameter*>::reverse_iterator rtp = tps.rbegin();
             rtp != tps.rend(); ++rtp ) {
            unsigned int values = ( ( *rtp )->getRangeTo() - ( *rtp )->getRangeFrom() ) / ( *rtp )->getRangeStep() + 1;
            ( *variant_map )[ *rtp ] = ( *sId ) % values;

            printf( "Tuning parameter %s has value %d\n", ( *rtp )->getName().c_str(), ( *sId ) % values );

            ( *sId ) = ( *sId ) / values;
        }

        Variant* variant = new Variant( *variant_map );
        cout << variant->toString( 1, "\t" );

        list<Region*>* regions;
        regions = new list<Region*>;
        regions->push_back( searchSpaces[ 0 ]->getRegions()[ 0 ] );
        ts->push_back( new TuningSpecification( variant, regions ) );

        Scenario*                            scenario;
        list<TuningSpecification*>*          ts1 = new list<TuningSpecification*> ();
        list<TuningSpecification*>::iterator TS_iterator;

        printf( "ts.size() = %d\n", ts->size() );
        for( TS_iterator = ts->begin(); TS_iterator != ts->end(); TS_iterator++ ) {
            TuningSpecification* tuningSpec = ( *TS_iterator );
            VariantContext       context    = tuningSpec->getVariantContext();
            Variant*             var        = const_cast<Variant*>( tuningSpec->getVariant() );
            TuningSpecification* temp       = new TuningSpecification( var, context.context_union.entity_list );
            ts1->push_back( temp );
        }

        scenario = new Scenario( ts1 );
        ts->pop_back();
//    printf("ScenarioId = %d\n", scenario->getID());
        scenarioIds.push( scenario->getID() );
//    scenario->print();
        pool_set->csp->push( scenario );
    }
/******************************************************************************/
}


/**
 * @brief Terminate description
 * @ingroup LoadSearch
 *
 * Long description.
 *
 */
void LoadSearch::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "LoadSearch: call to terminate()\n" );
}

/**
 * @brief Finalize description
 * @ingroup LoadSearch
 *
 * Long description.
 *
 */
void LoadSearch::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "LoadSearch: call to finalize()\n" );

    terminate();
}

/**
 * @brief Get Search Algorithm Instance description
 * @ingroup LoadSearch
 *
 * Long description.
 *
 */
ISearchAlgorithm* getSearchAlgorithmInstance( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "LoadSearch: call to getSearchAlgorithmInstance()\n" );

    return new LoadSearch();
}

/**
 * @brief Return the current major version number.
 * @ingroup LoadSearch
 *
 **/
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "LoadSearch: call to getVersionMajor()\n" );

    return 1;
}

/**
 * @brief Return the current minor version number.
 * @ingroup LoadSearch
 *
 **/
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "LoadSearch: call to getVersionMinor()\n" );

    return 0;
}

/**
 * @brief Return the name of the search algorithm.
 * @ingroup LoadSearch
 *
 **/
string getName( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "LoadSearch: call to getName()\n" );

    return "Load Search";
}

/**
 * @brief Return a short description.
 * @ingroup LoadSearch
 *
 **/
string getShortSummary( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "LoadSearch: call to getShortSummary()\n" );

    return "Explores the search space specified in the configuration file.";
}
