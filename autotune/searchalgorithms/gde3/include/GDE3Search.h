/**
   @file    GDE3Search.h
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

/**
 * @defgroup GDE3Search GDE 3 Search Algorithm
 * @ingroup SearchAlgorithms
 */

#ifndef GDE3SEARCH_H_
#define GDE3SEARCH_H_

// Autotune includes
#include "AutotuneSearchAlgorithm.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <string>
#include <set>
#include <algorithm>

#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <ctime>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/lexical_cast.hpp>

bool timerExceeded = false;

void timerFunction( double curTime );

class GDE3Search : public ISearchAlgorithm {
    double               F, CR;
    vector<SearchSpace*> searchSpaces;
    ScenarioPoolSet*     pool_set;

    int GenerationNo;
    int noAttempts; // attempts to create a child variant
    int samePopulationVector;
    int memberCreated;

    std::size_t populationSize;
    int         maxNoGenerations;
    int         maxAttempts;
    double      timer;

    vector<Scenario*>         population;        // population of scenarios
    vector<Scenario*>         recentPopulation;  // keeps track of last population
    set<std::string>          populElem;         // makes sure the variants are unique
    map<int, vector<double> > scenarioObjValMap; // for crowding distance

    double      optimalObjVal;                   // temporary for minimizing optimal value
    int         optimalScenarioId;
    vector<int> optimalIds;
    bool        singleObjective;
    int         noObjectives;           // keeping track, used in crowding distance

    map<int, int> parentChildMap;
    set<int>      tobeDropped;

    string logString; // keeps track of all activities, helps to debug

    int compareScenarios( Scenario* parent,
                          Scenario* child );

    void cleanupPopulation();

    bool checkFeasible( int      indexSS,
                        Variant& variant );

    void removeOptimalId( int scenarioId );

    void addOptimalId( int scenarioId );

public:
    GDE3Search();

    virtual ~GDE3Search();

    void initialize( DriverContext*   context,
                     ScenarioPoolSet* pool_set );

    void clear();

    void addSearchSpace( SearchSpace* );

    void createScenarios();

    void addObjectiveFunction(ObjectiveFunction *obj);

    int getOptimum();

    vector<int>getOptima();

    int getWorst();

    map<int, double >getSearchPath();

    bool searchFinished();

    void terminate();

    void finalize();

    void setPopulationSize( int size ) {
        populationSize = size;
    }

    void setMaxGenerations( int maxGenerations ) {
        maxNoGenerations = maxGenerations;
    }

    void setTimer( double seconds ) {
        timer = seconds;
    }

    int getPopulationSize() {
        return populationSize;
    }

    int getMaxGenerations() {
        return maxNoGenerations;
    }

    double getTimer() {
        return timer;
    }

    void iterateTP( int index,
                    int size,
                    int indexSS,
                    list<TuningSpecification*>& ts,
                    map<TuningParameter*, int>& newMap,
                    string uniqueConfig );

    void iterateSS( int index,
                    list<TuningSpecification*>& ts,
                    map<TuningParameter*, int>& newMap,
                    string uniqueConfig );
};

#endif
