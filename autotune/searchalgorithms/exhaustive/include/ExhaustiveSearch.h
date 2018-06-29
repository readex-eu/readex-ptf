/**
   @file    ExhaustiveSearch.h
   @ingroup ExhaustiveSearch
   @brief   Exhaustive Search
   @author  Houssam Haitof, Umbreen Sabir
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
 * @defgroup ExhaustiveSearch Exhaustive Search Algorithm
 * @ingroup SearchAlgorithms
 */

#ifndef EXHAUSTIVESEARCH_H_
#define EXHAUSTIVESEARCH_H_

// Autotune includes
#include "AutotuneSearchAlgorithm.h"
#include "search_common.h"


class ExhaustiveSearch : public ISearchAlgorithm {
private:
    vector<SearchSpace*> searchSpaces;
    int                  optimum;
    double               optimumValue;
    ScenarioPoolSet*     pool_set;

    //Additions for the comparing static best, worst and READEX dynamic savings
    int                  worst;
    double               worstValue;

public:
    ExhaustiveSearch();

    void initialize( DriverContext*,
                     ScenarioPoolSet* pool_set );

    void addSearchSpace( SearchSpace* );

    void clear();

    void createScenarios();

    void iterate_SS( int,
                     int,
                     list<TuningSpecification*>* );

    void iterate_TP( int,
                     int,
                     vector<TuningParameter*>*,
                     map<TuningParameter*, int>*,
                     list<TuningSpecification*>*,
                     int,
                     int );

    void generatescenario( list<TuningSpecification*>* );

    void addObjectiveFunction(ObjectiveFunction *obj);

    int getOptimum();

    int getWorst();

    map<int, double >getSearchPath();

    bool searchFinished();

    void terminate();

    void finalize();
};

#endif /* EXHAUSTIVESEARCH_H_ */
