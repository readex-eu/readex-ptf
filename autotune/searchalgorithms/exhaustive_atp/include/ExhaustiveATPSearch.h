/**
   @file    ExhaustiveATPSearch.h
   @ingroup ExhaustiveATPSearch
   @brief   Exhaustive Search
   @author  Anamika Chowdhury
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2017 Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

/**
 * @defgroup ExhaustiveSearch Exhaustive Search Algorithm
 * @ingroup SearchAlgorithms
 */

#ifndef EXHAUSTIVEATPSEARCH_H_
#define EXHAUSTIVEATPSEARCH_H_

// Autotune includes
#include "AutotuneSearchAlgorithm.h"
#include "search_common.h"
#include "ATPService.h"


class ExhaustiveATPSearch : public ISearchAlgorithm {
private:
    vector<SearchSpace*> searchSpaces;
    int                  optimum;
    double               optimumValue;
    ScenarioPoolSet*     pool_set;

    //Additions for the comparing static best, worst and READEX dynamic savings
    int                  worst;
    double               worstValue;
    atpService*          atpServc;

public:
    ExhaustiveATPSearch();

    void initialize( DriverContext*,
                     ScenarioPoolSet* pool_set );

    void addSearchSpace( SearchSpace* );

    void clear();

    void createScenarios();

    void iterate_SS( int,
                     int,
                     list<TuningSpecification*>* );

    void iterate_valid_combination(  int,
                                     int,
                                     std::unordered_map<int, std::vector<int32_t>>& ,
                                     vector<TuningParameter*>* tuningParameters,
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

    void setATPService( atpService* atp_srvc ){
        atpServc = atp_srvc;
    }

};

#endif /* EXHAUSTIVEATPSEARCH_H_ */
