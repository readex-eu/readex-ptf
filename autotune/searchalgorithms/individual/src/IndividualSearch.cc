/**
   @file    IndividualSearch.cc
   @ingroup IndividualSearch
   @brief   Individual search algorithm
   @author  Michael Gerndt
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include "IndividualSearch.h"
#include "autotune_services.h"

/**
 * @brief Constructor
 * @ingroup IndividualSearch
 *
 **/
IndividualSearch::IndividualSearch() : ISearchAlgorithm() {
}

/**
 * @brief Destructor
 * @ingroup IndividualSearch
 *
 **/
IndividualSearch::~IndividualSearch() {
}

void IndividualSearch::addObjectiveFunction(ObjectiveFunction *obj){
   objectiveFunctions.push_back(obj);
}


/**
 * @brief Initializes individual search
 * @ingroup IndividualSearch
 *
 * This method initializes the search and allocates exhaustive search
 * to be used in the search steps.
 *
 */
void IndividualSearch::initialize( DriverContext*   context,
                                   ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualSearch: call to initialize()\n" );

    this->pool_set = pool_set;

    std::string search_algorithm;
    int         major, minor;
    string      name, description;

    individual_keep = 1;
    bestScenario    = -1;
    worstScenario   = -1;
    searchStepCount = 0;
    tpEliminated    = 0;

    search_algorithm = "exhaustive";
    context->loadSearchAlgorithm( search_algorithm, &major, &minor, &name, &description );
    searchAlgorithm = context->getSearchAlgorithmInstance( search_algorithm );

    if( searchAlgorithm != NULL ) {
        print_loaded_search( major, minor, name, description );
        searchAlgorithm->initialize( context, pool_set );
    }
    else {
        psc_errmsg( "Could not load exhaustive search algorithm in individual search\n" );
        throw 0;
    }
}

/**
 * @brief Adds the search space.
 * @ingroup IndividualSearch
 *
 * The individual search startegy supports a single tuning space. The tuning parameters are tuned
 * in the order they appear in the tuning space.
 *
 * @param searchSpace Space of tuning parameters.
 * @todo Extension for multiple search spaces.
 **/
void IndividualSearch::addSearchSpace( SearchSpace* searchSpace ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualSearch: call to addSearchSpace()\n" );
    if( searchSpaces.size() == 0 ) {
        searchSpaces.push_back( searchSpace );
        tuningParameters = searchSpace->getVariantSpace()->getTuningParameters();
        regions          = searchSpace->getRegions();
        rtsVector        = searchSpace->getRts();
    }
    else {
        psc_errmsg( "Cannot add another search space. The individual search algorithm has no support for multiple search spaces.\n" );
        throw 0;
    }
}

/**
 * @brief Creates the scenarios for the next tuning parameter to explore.
 * @ingroup IndividualSearch
 *
 * It constructs a new search space from the current best settings for the already explored
 * tuning parameters and the next tuning parameter. This search space is forwarded to exhaustive
 * search which then constructs all the scenarios in this search space.
 *
 **/
void IndividualSearch::createScenarios() {
    SearchSpace searchSpace;

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualSearch: call to createScenarios()\n" );
    searchAlgorithm->clear();
    selection.clear();
    variantSpace.clear();


    if (objectiveFunctions.size()==0){
   	 addObjectiveFunction(new PTF_minObjective(""));
    }

	 for (int i = 0; i < objectiveFunctions.size(); i++) {
		 searchAlgorithm->addObjectiveFunction(objectiveFunctions[i]);
	 }



    searchStepCount++;
    //add new search space to exhaustive search
    for( int i = 0; i < tuningParameters.size() && i < searchStepCount - tpEliminated; i++ ) {
        variantSpace.addTuningParameter( tuningParameters[ i ] );
    }
    searchSpace.setVariantSpace( &variantSpace );
    if(withRtsSupport()){
   	 if (rtsVector.empty()){
   		 searchSpace.addRts(new Rts);
   	 } else {
          for( vector<Rts*>::iterator rts = rtsVector.begin(); rts != rtsVector.end(); rts++ ) {
              searchSpace.addRts(*rts);
          }
   	 }
    }else{
		 if( regions.empty() ) {
			  searchSpace.addRegion( new Region() );
		 }
		 else {
			  for( vector<Region*>::iterator region = regions.begin(); region != regions.end(); region++ ) {
					searchSpace.addRegion( *region );
			  }
		 }
    }


    searchAlgorithm->addSearchSpace( &searchSpace );

    searchAlgorithm->createScenarios();

    map<int, Scenario*>::iterator scenario_iter;
    for( scenario_iter = pool_set->csp->getScenarios()->begin(); scenario_iter != pool_set->csp->getScenarios()->end();
         scenario_iter++ ) {
        Scenario* scenario = scenario_iter->second;

        if( scenario != NULL ) {
            //scenario->print();

            const Variant*             v      = scenario->getTuningSpecifications()->front()->getVariant();
            map<TuningParameter*, int> values = v->getValue();
            selection[ scenario->getID() ] = values[ tuningParameters[ searchStepCount - 1 - tpEliminated ] ];
        }
    }
}

/**
 * @brief Checks whether all the tuning parameters were explored.
 * @ingroup IndividualSearch
 *
 * It analyzes the search path for the scenarios created and explored in this search step.
 * It determines a single or multiple scenarios that are better than the best one found in previous search steps.
 * If none is better then those found in the
 * previous search step, it discards this tuning parameter. If new better scenarios are found, it
 * creates a vector restriction for that tuning parameter that includes all settings that improved
 * the previous best scenario.
 * @return Returns true if the search finished.
 **/
bool IndividualSearch::searchFinished() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualSearch: call to searchFinished()\n" );

    results.clear();

    Restriction* r = new Restriction();
    r->setType( 2 );

    bool             flag = searchAlgorithm->searchFinished();
    map<int, double> path = searchAlgorithm->getSearchPath();
    individualSearchPath.insert( path.begin(), path.end() );

    map<int, double>::iterator it;
    for( it = path.begin(); it != path.end(); it++ ) {
        MyPair p;
        p.first  = ( *it ).first;
        p.second = ( *it ).second;
        //printf("added to results: %d %f\n", (*it).first, (*it).second); fflush(stdout);
        results.push_back( p );
    }

    //printf("Before sort:\n=========\n"); fflush(stdout);
    //for (int i = 0; i < results.size(); i++) {
    //  printf("results[%d] %d %f\n", i, results[i].first, results[i].second); fflush(stdout);
    //}
    std::sort( results.begin(), results.end(), CompareByValue() );
    if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ) ) > 0 ) {
        printf( "After sort:\nSorted INDIVIDUAL SEARCH scenarios of last search step.\n" );
        for( int i = 0; i < results.size(); i++ ) {
            printf( "results[%d] %d %f\n", i, results[ i ].first, results[ i ].second );
            fflush( stdout );
        }
    }
    if( worstScenario == -1 || results.back().second > worstTime  ) {
        worstScenario = results.back().first;
        worstTime     = results.back().second;

        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                    "Individual search: New worst scenario in step %d is scenario %d, %f\n", searchStepCount, worstScenario,
                    worstTime );
    }

    if( bestScenario == -1 || results.begin()->second < bestTime ) {
        bestScenario = results[ 0 ].first;
        bestTime     = results[ 0 ].second;

        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                    "Individual search: New best scenario in step %d is scenario %d, %f\n", searchStepCount, bestScenario,
                    bestTime );

        int from = tuningParameters[ searchStepCount - 1 - tpEliminated ]->getRangeFrom();
        int to   = tuningParameters[ searchStepCount - 1 - tpEliminated ]->getRangeTo();
        int step = tuningParameters[ searchStepCount - 1 - tpEliminated ]->getRangeStep();

        int sizeRange = ( to - from ) / step + 1;

        for( int i = 0; i < sizeRange && i < individual_keep; i++ ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "--->Kept value %d from scenario %d\n",
                        selection[ results[ i ].first ], results[ i ].first );
            bool        found = false;
            vector<int> vect;
            vect = r->getElements();
            for( int k = 0; k < vect.size(); k++ ) {
                if( vect[ k ] == selection[ results[ i ].first ] ) {
                    found = true;
                    //           psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotuneSearch), "---->value already registered.\n");
                    break;
                }
            }
            if( !found ) {
                r->addElement( selection[ results[ i ].first ] );
            }
        }

        tuningParameters[ searchStepCount - 1 - tpEliminated ]->setRestriction( r );
    }
    else {
     psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "Tuning Parameter eliminated.\n" );

        tuningParameters.erase( tuningParameters.begin() + searchStepCount - 1 - tpEliminated );
        tpEliminated++;
    }

    return searchStepCount - tpEliminated == tuningParameters.size();
}

/**
 * @brief Returns the scenario id of the best scenario.
 * @ingroup IndividualSearch
 *
 *
 **/
int IndividualSearch::getOptimum() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualSearch: call to getOptimum()\n" );
    return bestScenario;
}

/**
 * @brief Returns the scenario id of the worst scenario.
 * @ingroup IndividualSearch
 *
 *
 **/
int IndividualSearch::getWorst() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualSearch: call to getWorst()\n" );
    return worstScenario;
}


/**
 * @brief Returns the search path taken by the individual search algorithm.
 * @ingroup IndividualSearch
 *
 * The search path is a list of scenario ids and the double value of the objective function.
 **/
map<int, double> IndividualSearch::getSearchPath() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualSearch: call to getSearchPath()\n" );
    return individualSearchPath;
}


/**
 * @brief Clears the data structures so that the same search object can be used again.
 * @ingroup IndividualSearch
 *
 **/
void IndividualSearch::clear() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualSearch: call to clear()\n" );
    searchSpaces.clear();
}

/**
 * @brief Terminates the search algorithm.
 * @ingroup IndividualSearch
 *
 **/
void IndividualSearch::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualSearch: call to terminate()\n" );
    delete searchAlgorithm;
    //unloadSearchAlgorithm();
}

/**
 * @brief Finalizes the search algorithms by calling terminate().
 * @ingroup IndividualSearch
 *
 **/
void IndividualSearch::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualSearch: call to finalize()\n" );
    terminate();
}

/**
 * @brief Return a pointer to the search object.
 * @ingroup IndividualSearch
 *
 **/
ISearchAlgorithm* getSearchAlgorithmInstance( void ) {
    return new IndividualSearch();
}

/**
 * @brief Return the current major version number.
 * @ingroup IndividualSearch
 *
 **/
int getVersionMajor() {
    return 1;
}

/**
 * @brief Return the current minor version number.
 * @ingroup IndividualSearch
 *
 **/
int getVersionMinor() {
    return 0;
}

/**
 * @brief Return the name of the search algorithm.
 * @ingroup IndividualSearch
 *
 **/
string getName() {
    return "Individual Search";
}

/**
 * @brief Return a short description.
 * @ingroup IndividualSearch
 *
 **/
string getShortSummary() {
    return "Searches best values per individual parameters in order.";
}
