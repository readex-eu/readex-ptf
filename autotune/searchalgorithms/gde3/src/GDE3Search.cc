/**
   @file    GDE3Search.cc
   @ingroup GDE3Search
   @brief   GDE3 Search
   @author  Shrikant Vinchurkar
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

#include "GDE3Search.h"
#include "search_common.h"

#define DBG 1
#define BOOSTRANDOM 1

/**
 * @brief Constructor
 * @ingroup GDE3Search
 *
 * Initializes all default parameters like populationSize that can be overridden
 * by explicit methods
 **/
GDE3Search::GDE3Search() : ISearchAlgorithm() {
    GenerationNo = 0;

    // Minimization problem
    optimalObjVal = 100000.0;

    samePopulationVector = 0;
    optimalScenarioId    = -1;

    // Set defaults (if not overrided by set functions)
    populationSize   = 10;
    maxNoGenerations = 20;
    maxAttempts      = 10000;
    timer            = -1;
    singleObjective  = false;
    noObjectives     = 0; // default, for resetting in compareVariants()

    noAttempts = 0;
    CR         = 0.5;
    F          = 0.5;

    // initialize random seed
    struct timeval tv;
    gettimeofday( &tv, NULL );
    srand( tv.tv_sec * tv.tv_usec );
}


/**
 * @brief Destructor
 * @ingroup GDE3Search
 *
 **/
GDE3Search::~GDE3Search() {
}

void GDE3Search::addObjectiveFunction(ObjectiveFunction *obj){
   objectiveFunctions.push_back(obj);
}



/**
 * @brief Returns a random number between 0 and 1
 * @ingroup GDE3Search
 *
 */
double uniform() {
    static boost::mt19937                    gen( static_cast<unsigned int>( std::time( 0 ) ) );
    static boost::uniform_01<boost::mt19937> dist( gen );
    return dist();
}


/**
 * @brief Checks if the variant is inside allowed search space
 * @ingroup GDE3Search
 *
 * Checks whether the variant is inside allowed search space or not
 * As well as , it checks whether the variant is valid point in the search space with respect to step size
 *
 */
bool GDE3Search::checkFeasible( int      indexSS,
                                Variant& variant ) {
    vector<TuningParameter*>   tuningParVec = this->searchSpaces[ indexSS ]->getVariantSpace()->getTuningParameters();
    map<TuningParameter*, int> vectorMap    = variant.getValue();

    for( size_t i = 0; i < tuningParVec.size(); i++ ) {
        Restriction* r = tuningParVec[ i ]->getRestriction();

        if( r == NULL || r->getType() != 2 ) {
            if( vectorMap[ tuningParVec[ i ] ] < tuningParVec[ i ]->getRangeFrom() ) {
                return false;
            }

            if( vectorMap[ tuningParVec[ i ] ] > tuningParVec[ i ]->getRangeTo() ) {
                return false;
            }

            // need to find if tp value is correct w.r.t to stepsize
            bool valid = false;
            for( size_t iter = tuningParVec[ i ]->getRangeFrom(); iter <= tuningParVec[ i ]->getRangeTo();
                 iter += tuningParVec[ i ]->getRangeStep() ) {
                if( iter == vectorMap[ tuningParVec[ i ] ] ) {
                    valid = true;
                    break;
                }
            }

            if( !valid ) {
                return false;
            }
        }
        else {
            bool        valid = false;
            vector<int> v     = reinterpret_cast<Restriction*>( r )->getElements();
            for( int j = 0; j < v.size(); j++ ) {
                if( vectorMap[ tuningParVec[ i ] ] == v[ j ] ) {
                    valid = true;
                    break;
                }
            }
            if( !valid ) {
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief Callback function. called after time is expired
 * @ingroup GDE3Search
 *
 * If the timer is registered for gde3 search algorithm, this callback function will be called
 * by periscope frontend state machine. Flag 'timerExceeded' is set here such that searchFinished() called henceforth
 * will return true.
 *
 */
void timerFunction( double curTime ) {
    timerExceeded = true;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: Timer exceed signal received from context\n" );
}

/**
 * @brief Initialize pool set, register timer
 * @ingroup GDE3Search
 *
 * If timer is set explicitly, the timer is registered with Periscope frontend statemachine.
 *
 */
void GDE3Search::initialize( DriverContext*   context,
                             ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: call to initialize()\n" );
    this->pool_set = pool_set;

    if( timer != -1 ) { // overrided by set function
        void (* funcPtr)( double );
        funcPtr = &timerFunction;
        context->register_timer( funcPtr, timer );
    }
}

/**
 * @brief internal function for lexical sorting of vectors just like radix sort
 * @ingroup GDE3Search
 *
 */
struct coord { int* arr; };

struct Comparer : std::binary_function<coord, coord, bool>{
    Comparer( int base ) : m_base( base ) {
    }
    bool operator()( vector<double> vec1, vector<double> vec2 ) {
        return vec1[ m_base ] < vec2[ m_base ];
    }
private:
    int m_base;
};


/**
 * @brief finds crowding distance for a scenario
 * @ingroup GDE3Search
 *
 * finds distance with neighbours on either side of pareto curve, normalizes them, sums them and
 * returns them as crowding distance
 */
double findDistance( vector<double> vec,
                     vector<double> prevVec,
                     vector<double> nextVec,
                     double*        minObj,
                     double*        maxObj ) {
    double distance1 = 0;

    for( int i = 0; i < vec.size(); i++ ) {
        double absDist =  abs( vec[ i ] - prevVec[ i ] );

        // normalize here
        double range = maxObj[ i ] - minObj[ i ];
        if( range != 0 ) {
            absDist = absDist / range;
        }

        distance1 += ( absDist * absDist );
    }

    distance1 = sqrt( distance1 );

    double distance2 = 0;

    for( int i = 0; i < vec.size(); i++ ) {
        double absDist =  abs( vec[ i ] - nextVec[ i ] );

        // normalize here
        double range = maxObj[ i ] - minObj[ i ];
        if( range != 0 ) {
            absDist = absDist / range;
        }

        distance2 += ( absDist * absDist );
    }

    distance2 = sqrt( distance2 );

    return distance1 + distance2;
}

/**
 * @brief internal data structure for maintaining priority queue of pair
 * @ingroup GDE3Search
 *
 */
class comparepq {
public:
    bool operator()( pair<int, double> pair1, pair<int, double> pair2 ) {
        return pair1.second > pair2.second;
    }
};

/**
 * @brief Removes scenarioId from the list of optimal ids
 * @ingroup GDE3Search
 *
 */
void GDE3Search::removeOptimalId( int scenarioId ) {
    vector<int>::iterator iter;
    for( iter = optimalIds.begin(); iter != optimalIds.end(); ++iter ) {
        if( *iter == scenarioId ) {
            optimalIds.erase( iter );
        }
    }
}

/**
 * @brief Add scenario Id to the list of optimal Ids
 * @ingroup GDE3Search
 *
 */
void GDE3Search::addOptimalId( int scenarioId ) {
    vector<int>::iterator iter;
    for( iter = optimalIds.begin(); iter != optimalIds.end(); ++iter ) {
        if( *iter == scenarioId ) {
            return;
        }
    }

    optimalIds.push_back( scenarioId );
}

/**
 * @brief Checks if the algorithm has finished searching
 * @ingroup GDE3Search
 *
 * This function does following things:
 * - compares parent scenario with child scenario to decide who dominates other with respect to objective function
 * - removes dominated solutions from population
 * - pruns the population if number of non-dominating solutions exceed population size
 * - terminates the search if 1. timer is exceeded 2. search algorithm cannot generate more scenarios 3. 3 consecutive population retains the same scenarios 4. Number of generations exceeded
 *
 */
bool GDE3Search::searchFinished() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: call to searchFinished()\n" );
    bool searchFinish = false;

    // evaluate all configurations in population
    for( std::size_t i = 0; i < population.size(); i++ ) {
        if( parentChildMap.count( i ) > 0 ) { // parent
            int parent = i;
            int child  = parentChildMap[ i ];

            // send for evaluation
            int resultComp = compareScenarios( population[ parent ], population[ child ] );

            if( resultComp == 0 ) { // both dominant
                                   // keep both parent & child, prune the population
                int parentScenarioId = population[ parent ]->getID();
                int childScenarioId  = population[ child ]->getID();
                addOptimalId( parentScenarioId );
                addOptimalId( childScenarioId );
            }
            else if( resultComp < 0 ) { //Parent Dominant
                tobeDropped.insert( child );
                removeOptimalId( population[ child ]->getID() );
                int parentScenarioId = population[ parent ]->getID();
                addOptimalId( parentScenarioId );
            }
            else if( resultComp > 0 ) { // Child Dominant
                tobeDropped.insert( parent );
                removeOptimalId( population[ parent ]->getID() );
                int childScenarioId = population[ child ]->getID();
                addOptimalId( childScenarioId );
            }
        }
    }

    cleanupPopulation();

    // if the population has more elements than population size, clear depending on Crowding distance
    if( population.size() > populationSize ) {
        if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ) ) > 0 ) {
            std::stringstream strStream;
            strStream << "GDE3Search: Optimal Scenario Ids- ";
            for( int i = 0; i < optimalIds.size(); i++ ) {
                strStream << optimalIds[ i ] << " ";
            }
            strStream << endl;
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), strStream.str().data() );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: Population Oversize, cleaning by dominance !\n" );
        }

        bool resizingDone = false;

        for( std::size_t i = 0; i < population.size() - 1; i++ ) {
            for( std::size_t j = i + 1; j < population.size(); j++ ) {
                // send for evaluation
                int resultComp = compareScenarios( population[ i ], population[ j ] );

                if( resultComp == 0 ) {    // both non-dominant
                                           // do nothing, cannot lose non-dominant vectors
                }
                else if( resultComp < 0 ) { //Parent Dominant
                    tobeDropped.insert( j );
                    // remove scenarioId from optimalIds
                    removeOptimalId( population[ j ]->getID() );
                }
                else if( resultComp > 0 ) { // Child Dominant
                    tobeDropped.insert( i );
                    // remove scenarioId from optimalIds
                    removeOptimalId( population[ i ]->getID() );
                }

                if( population.size() - tobeDropped.size() == populationSize ) {
                    resizingDone = true;
                    break;
                }
            }
            if( resizingDone ) {
                break;
            }
        }

        if( !resizingDone ) { // all non-dominant vector, need to clean by crowding distance
                             // Logic for crowding distance (for multiple Objective function)
            if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ) ) > 0 ) {
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: In manipulations for crowding distance !\n" );
            }

            vector< vector<double> > mainVector;
            for( std::size_t i = 0; i < population.size(); i++ ) {
                vector<double> scenarioVec;
                int            scenarioId = population[ i ]->getID();

                // storing all objectives values in scenario vector
                list<MetaProperty> properties = pool_set->srp->getScenarioResultsByID( scenarioId );
                for( std::list<MetaProperty>::iterator iterator = properties.begin(),
                     end = properties.end(); iterator != end; ++iterator ) {
                    double value = iterator->getSeverity();
                    scenarioVec.push_back( value );
                }

                mainVector.push_back( scenarioVec );
            }

            // imposing lexical ordering on the scenarios depending on number of objectives, sort only once
            for( int i = noObjectives - 1; i >= 0; i-- ) {
                std::sort( mainVector.begin(), mainVector.end(), Comparer( i ) );
            }

            if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ) ) > 0 ) {
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: Inserting vectors in priority queue keying on their crowding distance !\n" );
            }

            // find maximum, minimum of any objective for normalization ahead
            double minObj[ noObjectives ], maxObj[ noObjectives ];
            for( int i = 0; i < noObjectives; i++ ) {
                minObj[ i ] = FLT_MAX;
                maxObj[ i ] = 0;
            }

            for( int i = 0; i < mainVector.size(); i++ ) {
                vector<double> currVector = mainVector[ i ];

                for( int j = 0; j < currVector.size(); j++ ) {
                    if( currVector[ j ] < minObj[ j ] ) {
                        minObj[ j ] = currVector[ j ];
                    }

                    if( currVector[ j ] > maxObj[ j ] ) {
                        maxObj[ j ] = currVector[ j ];
                    }
                }
            }

            // remove members with lowest CD and reduce population to default size
            int overHead = population.size() - populationSize;
            if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ) ) > 0 ) {
                std::stringstream strStream;
                strStream << "GDE3Search: No of vectors that are to be trimmed by picking lowest crowding distance : " << overHead << endl;
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), strStream.str().data() );
            }

            for( int k = 0; k < overHead; k++ ) {
                // Current logic: calculate crowding distance after every removal of crowded vector
                // Can be made efficient by decrease key operation or thinking of another way !
                priority_queue<std::pair<int, double>, vector<std::pair<int, double> >, comparepq > crowdPq;
                // calculate distance between neighbours on each side, sum up = CD, order by CD
                for( int i = 0; i < mainVector.size(); i++ ) {
                    vector<double> currVector = mainVector[ i ];
                    vector<double> prevVector;
                    vector<double> nextVector;

                    if( i == 0 || i == mainVector.size() - 1 ) {
                        continue;
                    }

                    prevVector = mainVector[ i - 1 ];
                    nextVector = mainVector[ i + 1 ];
                    double crowdDistance = findDistance( currVector, prevVector, nextVector, minObj, maxObj );

                    // get the scenario
                    int                                 scenarioId;
                    map<int, vector<double> >::iterator it;
                    for( it = scenarioObjValMap.begin(); it != scenarioObjValMap.end(); ++it ) {
                        if( it->second == currVector ) {
                            scenarioId = it->first;
                            break;
                        }
                    }

                    // push to priority queue
                    string newString;
                    for( int j = 0; j < noObjectives; j++ ) {
                        stringstream newS;
                        newS << currVector[ j ];
                        newString.append( newS.str() );
                        newString.append( "," );
                    }

                    if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ) ) > 0 ) {
                        std::stringstream strStream;
                        strStream << "GDE3Search: scenario Id: " << scenarioId << " objVal: " <<  newString << " and CD = " << crowdDistance << " pushed to pq !" << endl;
                        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), strStream.str().data() );
                    }

                    crowdPq.push( make_pair( scenarioId, crowdDistance ) );
                }

                pair<int, double> popPair;
                popPair = crowdPq.top();
                crowdPq.pop();

                if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ) ) > 0 ) {
                    std::stringstream strStream;
                    strStream << "### popped from PQ: " << popPair.first << " , " << popPair.second << endl;
                    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), strStream.str().data() );
                }

                for( std::size_t i = 0; i < population.size(); i++ ) {
                    if( population[ i ]->getID() == popPair.first ) {
                        tobeDropped.insert( i );

                        int scenarioId = population[ i ]->getID();
                        removeOptimalId( scenarioId );

                        vector<double> vecRemove = scenarioObjValMap[ scenarioId ];
                        // Remove from mainVector as well
                        vector<vector<double> >::iterator it;
                        for( it = mainVector.begin(); it != mainVector.end(); ++it ) {
                            if( ( *it ) == vecRemove ) {
                                mainVector.erase( it );
                                break;
                            }
                        }
                        break;
                    }
                }
            }
        } // crowding distance manipulation ends here

        // clear elements from population vector
        cleanupPopulation();
    }

    // Print Population to logString after cleaning
    logString.append( "\nCleaned Population: \n { " );
    for( vector<Scenario*>::iterator iter = population.begin(); iter != population.end(); ++iter ) {
        logString.append( "(" );
        logString.append( boost::lexical_cast<std::string>( ( *iter )->getID() ) );
        logString.append( ") , " );
    }
    logString.append( "}\n" );

    // Priting evaluation of this generation, later this code can be removed
    logString.append( "Objective values after generation:\n {" );
    for( vector<Scenario*>::iterator iter = population.begin(); iter != population.end(); ++iter ) {
        Scenario*      currSc = *iter;
        vector<double> objVal = scenarioObjValMap[ currSc->getID() ];

        logString.append( "(" );
        for( size_t i = 0; i < objVal.size(); i++ ) {
            stringstream ss1;
            ss1 << objVal[ i ];
            logString.append( ss1.str() + "," );
        }
        logString.append( ")" );
    }

    logString.append( "}\n" );

    if( recentPopulation.empty() ) { // first time
        vector<Scenario*>::iterator iter;
        for( iter = population.begin(); iter != population.end(); ++iter ) {
            recentPopulation.push_back( *iter );
        }
    }

    bool generationEqual = true;

    // compare two population vectors
    if( population.size() == recentPopulation.size() ) {
        for( std::size_t i = 0; i < recentPopulation.size(); i++ ) {
            if( population[ i ]->getID() != recentPopulation[ i ]->getID() ) {
                generationEqual = false;
                break;
            }
        }

        if( generationEqual ) {
            samePopulationVector++;
        }
        else {
            samePopulationVector = 1;
        }
    }

    // replace replacePopulation by newest one
    recentPopulation.clear();
    vector<Scenario*>::iterator iter;
    for( iter = population.begin(); iter != population.end(); ++iter ) {
        recentPopulation.push_back( *iter );
    }

    // check if search is finished
    if( samePopulationVector == 3        ||
        GenerationNo == maxNoGenerations ||
        noAttempts >= maxAttempts        ||
        timerExceeded ) {
        searchFinish = true;
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), logString.data() );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: Application Completed after %d generations !!!\n", GenerationNo );
        if( timerExceeded ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: TERMINATION due to TIMEOUT\n" );
        }
        if( noAttempts >= maxAttempts ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: TERMINATION as no of attempts exceeded !\n" );
        }
    }

    if( singleObjective ) {
        this->getOptimum();
    }
    else {
        vector<int>       optimals = this->getOptima();
        std::stringstream strStream;
        strStream << "GDE3Search: Optimal Scenario Ids: ";
        for( int i = 0; i < optimals.size(); i++ ) {
            strStream << optimals[ i ] << " ";
        }
        strStream << endl;
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), strStream.str().data() );
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: samePopulationVector value - %d\n", samePopulationVector );

    return searchFinish;
}

/**
 * @brief returns optimal scenario id
 * @ingroup GDE3Search
 *
 * returns optimal scenario id in case of single objective process.
 * In case of multiple objective process,returns one of the points on pareto curve
 *
 */
int GDE3Search::getOptimum() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: call to getOptimum()\n" );

    if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ) ) > 0 ) {
        if( singleObjective ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: Optimal Value till now - %f\n", optimalObjVal );
        }
    }

    if( !singleObjective ) {
        if( optimalIds.size() > 0 ) {
            optimalScenarioId = optimalIds[ 0 ];
        }
        else {
            optimalScenarioId = 0;     // Special case: No childrens are created
        }
    }

    return optimalScenarioId;
}

/**
 * @brief Returns the scenario id of the worst scenario.
 * @ingroup IndividualSearch
 *
 *
 **/
int GDE3Search::getWorst() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GED3Search: call to getWorst()\n" );
//NOTE: Please note that this might not be proper implementation. Just one that resolves undefined reference problem.
    return optimalIds.back();
}

/**
 * @brief Clears the data structures so that the same search object can be used again.
 * @ingroup IndividualSearch
 *
 **/
void GDE3Search::clear() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: call to clear()\n" );
}

/**
 * @brief Get Search Path description
 * @ingroup GDE3Search
 *
 * Long description.
 *
 */
map<int, double> GDE3Search::getSearchPath() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: call to getSearchPath()\n" );
    return path;
}

/**
 * @brief Adds Search Space
 * @ingroup GDE3Search
 *
 * Long description.
 *
 */
void GDE3Search::addSearchSpace( SearchSpace* searchSpace ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: call to addSearchSpace()\n" );
    this->searchSpaces.push_back( searchSpace );
}

/**
 * @brief iterates through all search spaces recursively
 * @ingroup GDE3Search
 *
 * Long description.
 *
 */
void GDE3Search::iterateSS( int index,
                            list<TuningSpecification*>& ts,
                            map<TuningParameter*, int>& newMap,
                            string uniqueConfig ) {
    if( index >= searchSpaces.size() ) {
        if( populElem.count( uniqueConfig ) == 1 ) { //Already in population
            return;
        }
        populElem.insert( uniqueConfig );

        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: New scenario with value = %s created\n", uniqueConfig.c_str() );

        std::list<TuningSpecification*>*          tsList = new list<TuningSpecification*>();
        std::list<TuningSpecification*>::iterator iter;
        for( iter = ts.begin(); iter != ts.end(); ++iter ) {
            TuningSpecification* tuningSpec;
            tuningSpec = *iter;
            VariantContext       context = tuningSpec->getVariantContext();
            Variant*             var     = const_cast<Variant*>( tuningSpec->getVariant() );
            TuningSpecification* temp    = new TuningSpecification( var, context.context_union.entity_list );
            tsList->push_back( temp );
        }

        memberCreated++;
        Scenario* scenario;
        scenario = new Scenario( tsList );

        population.push_back( scenario );
        pool_set->csp->push( scenario );

        if( memberCreated == populationSize ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: No of attempts in creating scenarios INITIAL-%d\n", noAttempts );
            noAttempts = 0;
        }
        return;
    }

    vector<TuningParameter*> tuningParVec = this->searchSpaces[ index ]->getVariantSpace()->getTuningParameters();

    iterateTP( 0, tuningParVec.size(), index, ts, newMap, uniqueConfig );
}

/**
 * @brief Iterates through all tuning parameters recursively
 * @ingroup GDE3Search
 *
 * Long description.
 *
 */
void GDE3Search::iterateTP( int index,
                            int size,
                            int indexSS,
                            list<TuningSpecification*>& ts,
                            map<TuningParameter*, int>& newMap,
                            string uniqueConfig ) {
    if( index >= size ) {
        Variant* newVariant = new Variant();

        noAttempts++;
        if( noAttempts == maxAttempts ) { // dont entertain much
                                         // abort in between
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: No of attempts at generating population exceeded !!!\n" );
            if( population.size() == 0 ) {
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: Initial population could not be created ! Returning from createScenarios without pushing any scenarios !!!\n" );
            }
            return;
        }

        newVariant->setValue( newMap );

        // check if variant is feasible or not, if not then don't push
        if( !checkFeasible( indexSS, *newVariant ) ) {
            return;
        }

        vector<TuningParameter*> tuningParVec = this->searchSpaces[ indexSS ]->getVariantSpace()->getTuningParameters();

        string                               newString;
        map<TuningParameter*, int>::iterator iter;
        for( iter = newMap.begin(); iter != newMap.end(); ++iter ) {
            stringstream newS;
            newS << iter->second;
            newString.append( newS.str() );
            newString.append( "," );
        }
        newMap.clear();
        uniqueConfig.append( newString );

        TuningSpecification* tuningSpec;
        if(withRtsSupport()) {
            std::list<Rts*>* rtsList;
            rtsList = new list<Rts*>;
            vector<Rts*> rtsVector = searchSpaces[indexSS]->getRts();
            if (rtsVector.size()>0)
          	  rtsList->push_back(rtsVector[0] );

            tuningSpec =  new TuningSpecification( newVariant, rtsList );
        } else {
           std::list<Region*>* regions;
           regions = new list<Region*>;
           regions->push_back( searchSpaces[ indexSS ]->getRegions()[ 0 ] );

           tuningSpec = new TuningSpecification( newVariant, regions );
        }
        ts.push_back( tuningSpec );
        iterateSS( indexSS + 1, ts, newMap, uniqueConfig );
        ts.pop_back();
        return;
    }

    vector<TuningParameter*> tuningParVec = this->searchSpaces[ indexSS ]->getVariantSpace()->getTuningParameters();

  #ifdef BOOSTRANDOM
    double randGen = uniform();
  #else
    double randGen = ( double )rand() / ( double )( RAND_MAX );
  #endif

    Restriction* r     = tuningParVec[ index ]->getRestriction();
    int          value = 0;

    if( r == NULL || r->getType() != 2 ) {
        double valDbl = randGen * ( tuningParVec[ index ]->getRangeTo() - tuningParVec[ index ]->getRangeFrom() );
        valDbl = valDbl / tuningParVec[ index ]->getRangeStep();
        value  = round( valDbl );
        value  = tuningParVec[ index ]->getRangeFrom() + value * tuningParVec[ index ]->getRangeStep();
        if( value > tuningParVec[ index ]->getRangeTo() ) {
            value = tuningParVec[ index ]->getRangeTo();
        }
    }
    else {
        //vector<int> v = reinterpret_cast<VectorRangeRestriction *>(r)->getElements();
        vector<int> v      = reinterpret_cast<Restriction*>( r )->getElements();
        int         vIndex = v.size() * randGen;
        value = v[ vIndex ];
    }

    newMap[ tuningParVec[ index ] ] = value;

  #if DBG > 1
    cout << " SearchSpace: " << indexSS << " TP: " << index << "Value: " << value << endl;
  #endif

    iterateTP( index + 1, size, indexSS, ts, newMap, uniqueConfig );
}

/**
 * @brief Create Scenarios/variants by doing crossover of parents
 * @ingroup GDE3Search
 *
 * For the first generation, generates parents randomly and children by crossover (depending on certain probabilities)
 * From second generation, generates  children by crossover (depending on certain probabilities)
 *
 */
void GDE3Search::createScenarios() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: call to createScenarios()\n" );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAll ), "GDE3 Search: Starting the search\n" );


    if (objectiveFunctions.size()==0){
   	 addObjectiveFunction(new PTF_minObjective(""));
    }


    GenerationNo++;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: Generation No:%d \n", GenerationNo );

    stringstream ss;
    ss << GenerationNo;
    logString.append( "\n\n Generation No: " );
    logString.append( ss.str() );
    logString.append( "\nPopulation: \n { " );

    // initialize no of attempts to create child
    noAttempts = 0;

    memberCreated = 0;
    if( population.empty() ) { // first time in create Scenarios
        while( memberCreated != populationSize ) {
            std::map<TuningParameter*, int> newMap;
            std::list<TuningSpecification*> ts;
            std::string                     uniqueConfig;
            iterateSS( 0, ts, newMap, uniqueConfig );
            if( noAttempts == maxAttempts ) {
                return;
            }
        }

        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: Population Size- %d \n", population.size() );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: Created initial Population !\n" );
    }

    int popSize = population.size();
    if( popSize < 5 ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: Terminating as 3 random additional parents cannot be selected; Population size < 5 \n" );
        return;
    }

    // Generate new population according to GDE3 population generation, even works for first random iteration
    int idx = 0;
    noAttempts = 0;

    while( idx < popSize ) {
    #if DBG > 1
        cout << "Idx: " << idx << endl;
    #endif

        // Generate 3 random members to choose from population
        int random[ 3 ] = { -1, -1, -1 };
        int i           = 0;
        while( i < 3 ) {
      #ifdef BOOSTRANDOM
            int newEntry = uniform() * popSize;
      #else
            int newEntry = rand() % ( popSize );
      #endif

            if( newEntry != random[ ( ( i + 1 ) % 3 ) ] && newEntry != random[ ( ( i - 1 ) % 3 ) ] && newEntry != idx ) { // ensuring no repeat
                random[ i++ ] = newEntry;
            }
        }

    #if DBG > 1
        cout << "Random: " << random[ 0 ] << " " << random[ 1 ] << " " << random[ 2 ] << " " << endl;
    #endif

        // Need to make sure there are no repetitions in the population
        string                     uniqueConfig;
        list<TuningSpecification*> ts;
        bool                       scenarioValid = true;

        //Finding randindex over a flattened search space
        int flattenedSpaceSize = 0;
        for( int i = 0; i < searchSpaces.size(); i++ ) {
            vector<TuningParameter*> tuningParVecCur = this->searchSpaces[ i ]->getVariantSpace()->getTuningParameters();
            flattenedSpaceSize += tuningParVecCur.size();
        }
    #ifdef BOOSTRANDOM
        size_t randIndex = floor( uniform() * flattenedSpaceSize ) + 1;
    #else
        size_t randIndex = floor( rand() % flattenedSpaceSize ) + 1; // Can move outside for flattened space
    #endif

        size_t curIndex = 0;

        // iterate over all search spaces
        for( int i = 0; i < searchSpaces.size(); i++ ) {
      #if DBG > 1
            cout << "SS No: " << i << endl;
      #endif
            // For every member of population
            Variant*                   newChildVariant = new Variant();
            map<TuningParameter*, int> newChildMap;
            vector<TuningParameter*>   tuningParVecCur = this->searchSpaces[ i ]->getVariantSpace()->getTuningParameters();

            // Choose parent
            map<TuningParameter*, int> parentMap;
            map<TuningParameter*, int> rOneMap;
            map<TuningParameter*, int> rTwoMap;
            map<TuningParameter*, int> rThreeMap;

            list<TuningSpecification*>*          tsList       = population[ idx ]->getTuningSpecifications();
            int                                  tuningSpecNo = 0;
            list<TuningSpecification*>::iterator iter;
            for( iter = tsList->begin(); iter != tsList->end(); ++iter, tuningSpecNo++ ) {
                if( tuningSpecNo == i ) {
                    TuningSpecification* tuningSpec;
                    tuningSpec = *iter;
                    VariantContext context = tuningSpec->getVariantContext();
                    Variant*       var     = const_cast<Variant*>( tuningSpec->getVariant() );
                    parentMap = var->getValue();
          #if DBG > 1
                    cout << "Values of parentMap: ";
                    map<TuningParameter*, int>::iterator mapIter;
                    for( mapIter = parentMap.begin(); mapIter != parentMap.end(); ++mapIter ) {
                        cout << mapIter->second << " ";
                    }
                    cout << endl;
          #endif
                    break;
                }
            }

            tsList       = population[ random[ 0 ] ]->getTuningSpecifications();
            tuningSpecNo = 0;
            for( iter = tsList->begin(); iter != tsList->end(); ++iter, tuningSpecNo++ ) {
                if( tuningSpecNo == i ) {
                    TuningSpecification* tuningSpec;
                    tuningSpec = *iter;
                    VariantContext context = tuningSpec->getVariantContext();
                    Variant*       var     = const_cast<Variant*>( tuningSpec->getVariant() );
                    rOneMap = var->getValue();
          #if DBG > 1
                    cout << "Values of rOneMap: ";
                    map<TuningParameter*, int>::iterator mapIter;
                    for( mapIter = rOneMap.begin(); mapIter != rOneMap.end(); ++mapIter ) {
                        cout << mapIter->second << " ";
                    }
                    cout << endl;
          #endif
                    break;
                }
            }

            tsList       = population[ random[ 1 ] ]->getTuningSpecifications();
            tuningSpecNo = 0;
            for( iter = tsList->begin(); iter != tsList->end(); ++iter, tuningSpecNo++ ) {
                if( tuningSpecNo == i ) {
                    TuningSpecification* tuningSpec;
                    tuningSpec = *iter;
                    VariantContext context = tuningSpec->getVariantContext();
                    Variant*       var     = const_cast<Variant*>( tuningSpec->getVariant() );
                    rTwoMap = var->getValue();
          #if DBG > 1
                    cout << "Values of rTwoMap: ";
                    map<TuningParameter*, int>::iterator mapIter;
                    for( mapIter = rTwoMap.begin(); mapIter != rTwoMap.end(); ++mapIter ) {
                        cout << mapIter->second << " ";
                    }
                    cout << endl;
          #endif
                    break;
                }
            }

            tsList       = population[ random[ 2 ] ]->getTuningSpecifications();
            tuningSpecNo = 0;
            for( iter = tsList->begin(); iter != tsList->end(); ++iter, tuningSpecNo++ ) {
                if( tuningSpecNo == i ) {
                    TuningSpecification* tuningSpec;
                    tuningSpec = *iter;
                    VariantContext context = tuningSpec->getVariantContext();
                    Variant*       var     = const_cast<Variant*>( tuningSpec->getVariant() );
                    rThreeMap = var->getValue();
          #if DBG > 1
                    cout << "Values of rThreeMap: ";
                    map<TuningParameter*, int>::iterator mapIter;
                    for( mapIter = rThreeMap.begin(); mapIter != rThreeMap.end(); ++mapIter ) {
                        cout << mapIter->second << " ";
                    }
                    cout << endl;
          #endif
                    break;
                }
            }

            // very important crossover code
            for( std::size_t j = 0; j < tuningParVecCur.size(); j++, curIndex++ ) {
        #ifdef BOOSTRANDOM
                double randGen = uniform();
        #else
                double randGen = ( double )rand() / ( double )( RAND_MAX );
        #endif

                if( randGen < CR || curIndex == randIndex ) {
                    double valDbl = ( double )rThreeMap[ tuningParVecCur[ j ] ] + F * ( rOneMap[ tuningParVecCur[ j ] ] - rTwoMap[ tuningParVecCur[ j ] ] );

                    // rounding off to nearest valid value
                    int from = tuningParVecCur[ j ]->getRangeFrom();
                    int to   = tuningParVecCur[ j ]->getRangeTo();
                    int step = tuningParVecCur[ j ]->getRangeStep();

                    int value = valDbl;

                    // if value is out of range, checkFeasible will take care of that
                    if( valDbl >= ( double )from && valDbl <= ( double )to ) {
                        valDbl = valDbl - from;
                        valDbl = valDbl / ( double )step;
                        value  = round( valDbl );
                        value  = from + value * step;
                    }

                    newChildMap[ tuningParVecCur[ j ] ] = value;
                }
                else {
                    newChildMap[ tuningParVecCur[ j ] ] = parentMap[ tuningParVecCur[ j ] ];
                }
            }
            noAttempts++;

      #if DBG > 1
            cout << "Values of newChildMap: ";
            map<TuningParameter*, int>::iterator mapIter;
            for( mapIter = newChildMap.begin(); mapIter != newChildMap.end(); ++mapIter ) {
                cout << mapIter->second << " ";
            }
            cout << endl;
      #endif

            if( noAttempts == maxAttempts ) { // dont entertain much
                                             // abort in between
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: No of attempts at generating population exceeded !!!\n" );
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: Returning from createScenarios without pushing any scenarios !!!\n" );

                if( idx == 0 &&  GenerationNo != 1 ) { // if no scenarios pushed in this generation
                    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: Pushing 1 scenario from previous generation to avoid seg fault\n" );
                    pool_set->csp->push( population[ 0 ] );
                }
                return;
            }

            newChildVariant->setValue( newChildMap );

            // check if variant is feasible or not, if not then don't push
            if( !checkFeasible( i, *newChildVariant ) ) {
                scenarioValid = false;
                break;
            }

            string                               newString;
            map<TuningParameter*, int>::iterator iterMap;
            for( iterMap = newChildMap.begin(); iterMap != newChildMap.end(); ++iterMap ) {
                stringstream newS;
                newS << iterMap->second;
                newString.append( newS.str() );
                newString.append( "," );
            }
            newChildMap.clear();
            uniqueConfig.append( newString );

            TuningSpecification* tuningSpec;

            if(withRtsSupport()){
               std::list<Rts*>* rtsList;
               rtsList = new list<Rts*>;
               vector<Rts*> rtsVector = searchSpaces[i]->getRts();
               if (rtsVector.size()>0)
             	  rtsList->push_back(rtsVector[0] );
               tuningSpec = new TuningSpecification(newChildVariant, rtsList);
            } else {
               list<Region*>* regions;
               regions = new list<Region*>;
               vector<Region*> rtsVector = searchSpaces[i]->getRegions();
               if (rtsVector.size()>0){
               	regions->push_back(rtsVector[0]);
               }
               tuningSpec = new TuningSpecification( newChildVariant, regions );
            }

            ts.push_back( tuningSpec );
        }

        if( !scenarioValid ) {
            continue;
        }
        if( populElem.count( uniqueConfig ) == 1 ) { // config already present, redo
            continue;
        }

        idx++;
        populElem.insert( uniqueConfig );

        list<TuningSpecification*>*          tsList = new list<TuningSpecification*>();
        list<TuningSpecification*>::iterator iter;
        for( iter = ts.begin(); iter != ts.end(); ++iter ) {
            TuningSpecification* tuningSpec;
            tuningSpec = *iter;
            VariantContext       context = tuningSpec->getVariantContext();
            Variant*             var     = const_cast<Variant*>( tuningSpec->getVariant() );
            TuningSpecification* temp    = new TuningSpecification( var, context.context_union.entity_list );
            tsList->push_back( temp );
        }

        Scenario* scenario;
        scenario = new Scenario( tsList );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: New scenario with value = %s created \n", uniqueConfig.c_str() );

        if( idx == populationSize ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: No of attempts in creating child scenarios - %d \n", noAttempts );
            noAttempts = 0;
        }

        population.push_back( scenario );
        pool_set->csp->push( scenario );
        // Add reference in parent child map for evaluate function
        parentChildMap[ idx - 1 ] = population.size() - 1;
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAll ), "GDE3 Search: Adding scenarios to scenario pool.\n" );
}

/**
 * @brief Terminates the search algorithm.
 * @ingroup GDE3Search
 *
 **/
void GDE3Search::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: call to terminate()\n" );
}

/**
 * @brief Finalizes the search algorithms by calling terminate().
 * @ingroup GDE3Search
 *
 **/
void GDE3Search::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: call to finalize()\n" );
}

/**
 * @brief Returns a vector of optimal scenario ids in case of multiple objective function
 * @ingroup GDE3Search
 *
 **/
vector<int> GDE3Search::getOptima() {
    return optimalIds;
}

/**
 * @brief Compares parent and child for dominance according to objective function
 * @ingroup GDE3Search
 *
 * Long description.
 *
 */
int GDE3Search::compareScenarios( Scenario* parent,
                                  Scenario* child ) {
    // fill parentMap with objective values of parent Scenario
    vector<double> parentMap;
    int            parentScenarioId = parent->getID();

		for (int i = 0; i < objectiveFunctions.size(); i++) {
			double objValue = objectiveFunctions[i]->objective(parentScenarioId, pool_set->srp);
			printf("parentScenarioId %d, objValue %f\n",parentScenarioId,objValue);
			parentMap.push_back( objValue );
		}

    // fill childMap with objective values of child Scenario
    vector<double> childMap;
    int            childScenarioId = child->getID();

		for (int i = 0; i < objectiveFunctions.size(); i++) {
			double objValue = objectiveFunctions[i]->objective(childScenarioId, pool_set->srp);
			printf("childScenarioId %d, objValue %f\n",childScenarioId,objValue);
			childMap.push_back( objValue );
		}


    if( parentMap.size() != childMap.size() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: Fatal Error !\n" );
        return -100;
    }

    // initializing no of objectives
    if( noObjectives == 0 ) {
        noObjectives = parentMap.size();
        if( noObjectives == 1 ) {
            singleObjective =  true;
        }
    }

    if( parentMap.size() == 1 ) { //Single Objective function
                                 // checking if its the optimal scenario
        if( parentMap[ 0 ] < optimalObjVal ) {
            optimalObjVal     = parentMap[ 0 ];
            optimalScenarioId = parent->getID();
        }
        scenarioObjValMap[ parent->getID() ] = parentMap;
        path[ parent->getID() ]              = parentMap[ 0 ];

        if( childMap[ 0 ] < optimalObjVal ) {
            optimalObjVal     = childMap[ 0 ];
            optimalScenarioId = child->getID();
        }
        scenarioObjValMap[ child->getID() ] = childMap;
        path[ child->getID() ]              = childMap[ 0 ];
    }
    else {
        // fill up scenarioObjValMap for crowding distance
        scenarioObjValMap[ parent->getID() ] = parentMap;
        scenarioObjValMap[ child->getID() ]  = childMap;
    }

    // The parent and child scenarios are already checked for feasibility, no need to check again
    // compare for Parent dominance
    bool parentDom = true;
    for( size_t i = 0; i < parentMap.size(); i++ ) {
        if( parentMap[ i ] > childMap[ i ] ) { // 'Minimization' problem
            parentDom = false;
            break;
        }
    }

    if( parentDom ) {
        if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ) ) > 0 ) {
            std::stringstream strStream;
            strStream << "GDE3Search: ParentID: " << parentScenarioId << " ChildID: " << childScenarioId;
            strStream << " Parent Objval: ";
            for( size_t i = 0; i < parentMap.size(); i++ ) {
                strStream << parentMap[ i ] << " ";
            }
            strStream << " Child Objval: ";
            for( size_t i = 0; i < childMap.size(); i++ ) {
                strStream << childMap[ i ] << " ";
            }
            strStream << " [Parent Dominant !!!]" << endl;
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), strStream.str().data() );
        }
        return -1;
    }

    // compare for child dominance
    bool childDom = true;
    for( size_t i = 0; i < parentMap.size(); i++ ) {
        if( parentMap[ i ] < childMap[ i ] ) { // 'Minimization' problem
            childDom = false;
            break;
        }
    }

    if( childDom ) {
        if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ) ) > 0 ) {
            std::stringstream strStream;
            strStream << "GDE3Search: ParentID: " << parentScenarioId << " ChildID: " << childScenarioId;
            strStream << " Parent Objval: ";
            for( size_t i = 0; i < parentMap.size(); i++ ) {
                strStream << parentMap[ i ] << " ";
            }
            strStream << " Child Objval: ";
            for( size_t i = 0; i < childMap.size(); i++ ) {
                strStream << childMap[ i ] << " ";
            }
            strStream << " [ Child Dominant !!!]" << endl;
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), strStream.str().data() );
        }
        return 1;
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "GDE3Search: Both Parent and child non-dominant\n" );
    return 0; // non dominance of both  (does not happen for single objective )
}

/**
 * @brief Removes dominated solutions from the population
 * @ingroup GDE3Search
 *
 */
void GDE3Search::cleanupPopulation() {
    // End of iteration, cleanup mess
    vector<Scenario*> popuNextGen;

    for( std::size_t i = 0; i < population.size(); i++ ) {
        if( tobeDropped.count( i ) != 1 ) { //not found
            popuNextGen.push_back( population[ i ] );
        }
    }

    population.clear();
    population = popuNextGen;
    parentChildMap.clear();
    tobeDropped.clear();
}

/**
 * @brief Returns a pointer to the search object.
 * @ingroup GDE3Search
 *
 **/
ISearchAlgorithm* getSearchAlgorithmInstance( void ) {
    return new GDE3Search();
}

/**
 * @brief Returns the current major version number.
 * @ingroup GDE3Search
 *
 **/
int getVersionMajor( void ) {
    return 1;
}

/**
 * @brief Returns the current minor version number.
 * @ingroup GDE3Search
 *
 **/
int getVersionMinor( void ) {
    return 0;
}

/**
 * @brief Returns the name of the search algorithm.
 * @ingroup GDE3Search
 *
 **/
string getName( void ) {
    return "GDE3 Search";
}

/**
 * @brief Returns a short description.
 * @ingroup GDE3Search
 *
 **/
string getShortSummary( void ) {
    return "Evolutionary algorithm that keeps best scenarios and discards other during course of generations.";
}
