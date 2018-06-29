/**
   @file    IndividualATPSearch.cc
   @ingroup IndividualATPSearch
   @brief   Individual search algorithm
   @author  Anamika Chowdhury
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2017, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include "IndividualATPSearch.h"
#include "ExhaustiveATPSearch.h"
#include "autotune_services.h"

/**
 * @brief Constructor
 * @ingroup IndividualATPSearch
 *
 **/
IndividualATPSearch::IndividualATPSearch() : ISearchAlgorithm() {
}

/**
 * @brief Destructor
 * @ingroup IndividualATPSearch
 *
 **/
IndividualATPSearch::~IndividualATPSearch() {
}

void IndividualATPSearch::addObjectiveFunction(ObjectiveFunction *obj){
   objectiveFunctions.push_back(obj);
}


/**
 * @brief Initializes individual search
 * @ingroup IndividualATPSearch
 *
 * This method initializes the search and allocates exhaustive search
 * to be used in the search steps.
 *
 */
void IndividualATPSearch::initialize( DriverContext*   context,
                                   ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualATPSearch: call to initialize()\n" );

    this->pool_set = pool_set;

    std::string search_algorithm;
    int         major, minor;
    string      name, description;

    individual_keep = 1;
    bestScenario    = -1;
    worstScenario    = -1;
    searchStepCount = 0;
    tpEliminated    = 0;

    search_algorithm = "exhaustive_atp";
    context->loadSearchAlgorithm( search_algorithm, &major, &minor, &name, &description );
    searchAlgorithm = context->getSearchAlgorithmInstance( search_algorithm );
    ((ExhaustiveATPSearch*)searchAlgorithm)->setATPService (atpServc);

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
 * @ingroup IndividualATPSearch
 *
 * The individual search strategy supports a single tuning space. The tuning parameters are tuned
 * in the order they appear in the tuning space.
 *
 * @param searchSpace Space of tuning parameters.
 * @todo Extension for multiple search spaces.
 **/
void IndividualATPSearch::addSearchSpace( SearchSpace* searchSpace ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualATPSearch: call to addSearchSpace()\n" );
    searchSpaces.push_back( searchSpace );
}

/**
 * @brief Creates the scenarios for the next tuning parameter to explore.
 * @ingroup IndividualATPSearch
 *
 * It constructs a new search space from the current best settings for the already explored
 * tuning parameters of a domain and constructs another search space for a new domain. Then add these two searchspaces to exhasutive_atp search algorithm.
 * This search space is forwarded to exhaustive
 * search which then constructs all the scenarios in this search space.
 *
 **/
void IndividualATPSearch::createScenarios() {
    SearchSpace searchSpaceDomain;
    SearchSpace searchSpace;

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualATPSearch: call to createScenarios()\n" );
    searchAlgorithm->clear();
    selection.clear();
    variantSpaceDomain.clear();
    variantSpace.clear();
    tuningParameters.clear();


    if (objectiveFunctions.size()==0){
   	 addObjectiveFunction(new PTF_minObjective(""));
    }

	 for (int i = 0; i < objectiveFunctions.size(); i++) {
		 searchAlgorithm->addObjectiveFunction(objectiveFunctions[i]);
	 }

    //add a new search space  with respect to each domain to exhaustive search
    std::string domain_name = searchSpaces[ searchStepCount ]->getDomain();
    tuningParameters = searchSpaces[ searchStepCount ]->getVariantSpace()->getTuningParameters();
    regions          = searchSpaces[ searchStepCount ]->getRegions();
    rtsVector        = searchSpaces[ searchStepCount ]->getRts();

    for( int i = 0; i < tuningParameters.size(); i++ )
        variantSpaceDomain.addTuningParameter( tuningParameters[ i ] );

    searchSpaceDomain.setVariantSpace( &variantSpaceDomain );
    if(withRtsSupport()){
   	 if (rtsVector.empty()){
         searchSpaceDomain.addRts(new Rts);
   	 } else {
          for( vector<Rts*>::iterator rts = rtsVector.begin(); rts != rtsVector.end(); rts++ ) {
              searchSpaceDomain.addRts(*rts);
          }
   	 }
    }else{
		 if( regions.empty() ) {
             searchSpaceDomain.addRegion( new Region() );
		 }
		 else {
			  for( vector<Region*>::iterator region = regions.begin(); region != regions.end(); region++ ) {
                  searchSpaceDomain.addRegion( *region );
			  }
		 }
    }

    searchSpaceDomain.addDomain(domain_name);
    searchAlgorithm->addSearchSpace( &searchSpaceDomain );

    if (searchSpaces.size() > 1 && exploredTuningParameters.size() > 0)
    {
        if(withRtsSupport()){
            if (rtsVector.empty()){
                searchSpace.addRts(new Rts);
            } else {
                for( vector<Rts*>::iterator rts = rtsVector.begin(); rts != rtsVector.end(); rts++ ) {
                    searchSpace.addRts(*rts);
                }
            }
        }
        else{
            if( regions.empty() ) {
                searchSpace.addRegion( new Region() );
            }
            else {
                for( vector<Region*>::iterator region = regions.begin(); region != regions.end(); region++ ) {
                    searchSpace.addRegion( *region );
                }
            }
        }
        // now add a new search from the current best settings for the already explored tuning parameters of a domain
        for( int i = 0; i < exploredTuningParameters.size(); i++ ) {
            //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualATPSearch: Already explored tuning  %s \n", exploredTuningParameters[ i ]->getName().c_str() );//
            variantSpace.addTuningParameter( exploredTuningParameters[ i ] );
        }
        searchSpace.setVariantSpace( &variantSpace );

        searchSpace.addDomain(std::string());
        searchAlgorithm->addSearchSpace( &searchSpace );
    }

    searchAlgorithm->createScenarios();

    searchStepCount++;

    map<int, Scenario*>::iterator scenario_iter;
    for( scenario_iter = pool_set->csp->getScenarios()->begin(); scenario_iter != pool_set->csp->getScenarios()->end();
         scenario_iter++ ) {
        Scenario* scenario = scenario_iter->second;

        if( scenario != NULL ) {
            //scenario->print();
            int idx;
            const Variant*             v      = scenario->getTuningSpecifications()->front()->getVariant();
            map<TuningParameter*, int> values = v->getValue();
            //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualATPSearch: scenario->getID()  %d \n", scenario->getID() );//
            // take all new tuning parameters for a domain of a scenario
            for (std::map<TuningParameter*, int>::iterator it=values.begin(); it!=values.end(); ++it){
                if ( getTPIndex(it->first->getName(), tuningParameters) != -1 )
                {
                    idx = getTPIndex(it->first->getName(), tuningParameters);
                    selection[ scenario->getID() ].push_back( values[tuningParameters[ idx ]] );
                    //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualATPSearch: NM %s selection[ scenario->getID() ]  %d \n", it->first->getName().c_str(), it->second );
                }
            }
        }
    }
}

/**
 * @brief Checks whether all the tuning parameters were explored.
 * @ingroup IndividualATPSearch
 *
 * It analyzes the search path for the scenarios created and explored in this search step.
 * It determines a single or multiple scenarios that are better than the best one found in previous search steps.
 * If none is better then those found in the
 * previous search step, it discards all tuning parameters of the current domain. If new better scenarios are found, it
 * creates a vector restriction for that tuning parameter that includes all settings that improved
 * the previous best scenario.
 * @return Returns true if the search finished.
 **/
bool IndividualATPSearch::searchFinished() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualATPSearch: call to searchFinished()\n" );

    results.clear();

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
    if( worstScenario == -1 || results.back().second > worstEnergy  ) {
        worstScenario = results.back().first;
        worstEnergy     = results.back().second;

        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                    "Individual search: New worst scenario in step %d is scenario %d, %f\n", searchStepCount, worstScenario,
                    worstEnergy );
    }

    if( bestScenario == -1 || results.begin()->second < bestEnergy ) {
        bestScenario = results[ 0 ].first;
        bestEnergy     = results[ 0 ].second;

        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                    "Individual search: New best scenario in step %d is scenario %d, %f\n", searchStepCount, bestScenario,
                    bestEnergy );

        /*individual_keep=1. So go over all the tuning parameters of */
        if(individual_keep==1 ) {
            vector<int> tp_values = selection[ results[ 0 ].first ];
            for (int i = 0; i < tp_values.size() ; ++i) {
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "--->Kept TP: %s value: %d from scenario: %d\n",
                            tuningParameters[i]->getName().c_str(), tp_values[i], results[ 0 ].first );
            }
            for (int i = 0; i < tp_values.size() ; ++i) {
                Restriction* r = new Restriction();
                r->setType( 2 );
                r->addElement(tp_values[i]);
                //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "Individual search: TP NAME %s tp_values[i] %d \n", tuningParameters[i]->getName().c_str(), tp_values[i] );
                tuningParameters[ i ]->setRestriction( r );
                exploredTuningParameters.push_back(tuningParameters[ i ]);
            }
        }
    }
    else {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "The best energy %f from previous step/domain is better than the best "
                                                                 "energy %f of current step/domain \n", bestEnergy, results[ 0 ].second );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "All the tuning Parameters from the current domain are discarded.\n" );
        tuningParameters.clear();
        //tpEliminated++;
    }

    return searchStepCount - tpEliminated == searchSpaces.size(); //no. of domains===searchSpaces.size()
}

/**
 * @brief Returns the scenario id of the best scenario.
 * @ingroup IndividualATPSearch
 *
 *
 **/
int IndividualATPSearch::getOptimum() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualATPSearch: call to getOptimum()\n" );
    return bestScenario;
}

/**
 * @brief Returns the scenario id of the worst scenario.
 * @ingroup IndividualATPSearch
 *
 *
 **/
int IndividualATPSearch::getWorst() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualATPSearch: call to getWorst()\n" );
    return worstScenario;
}


/**
 * @brief Returns the search path taken by the individual search algorithm.
 * @ingroup IndividualATPSearch
 *
 * The search path is a list of scenario ids and the double value of the objective function.
 **/
map<int, double> IndividualATPSearch::getSearchPath() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualATPSearch: call to getSearchPath()\n" );
    return individualSearchPath;
}


/**
 * @brief Clears the data structures so that the same search object can be used again.
 * @ingroup IndividualATPSearch
 *
 **/
void IndividualATPSearch::clear() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualATPSearch: call to clear()\n" );
    searchSpaces.clear();
}

/**
 * @brief Terminates the search algorithm.
 * @ingroup IndividualATPSearch
 *
 **/
void IndividualATPSearch::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualATPSearch: call to terminate()\n" );
    delete searchAlgorithm;
    //unloadSearchAlgorithm();
}

/**
 * @brief Finalizes the search algorithms by calling terminate().
 * @ingroup IndividualATPSearch
 *
 **/
void IndividualATPSearch::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndividualATPSearch: call to finalize()\n" );
    terminate();
}

/**
 * @brief Return a pointer to the search object.
 * @ingroup IndividualATPSearch
 *
 **/
ISearchAlgorithm* getSearchAlgorithmInstance( void ) {
    return new IndividualATPSearch();
}

/**
 * @brief Return the current major version number.
 * @ingroup IndividualATPSearch
 *
 **/
int getVersionMajor() {
    return 1;
}

/**
 * @brief Return the current minor version number.
 * @ingroup IndividualATPSearch
 *
 **/
int getVersionMinor() {
    return 0;
}

/**
 * @brief Return the name of the search algorithm.
 * @ingroup IndividualATPSearch
 *
 **/
string getName() {
    return "Individual ATP Search";
}

/**
 * @brief Return a short description.
 * @ingroup IndividualATPSearch
 *
 **/
string getShortSummary() {
    return "Searches best values per domain in order.";
}

/**
 * @brief get TP index
 */

int getTPIndex( string name, vector<TuningParameter*> tps )
{
    for (int i = 0; i < tps.size() ; ++i) {
        if(name == tps[i]->getName())
            return i;
    }
    return -1;
}
