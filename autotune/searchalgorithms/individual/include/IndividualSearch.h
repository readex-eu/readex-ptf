/**
   @file    IndividualSearch.h
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

/**
 * @defgroup IndividualSearch Individual Search Algorithm
 * @ingroup SearchAlgorithms
 */

#ifndef INDIVIDUALSEARCH_H_
#define INDIVIDUALSEARCH_H_

// Autotune includes
#include "AutotuneSearchAlgorithm.h"
#include "search_common.h"

class IndividualSearch : public ISearchAlgorithm {
private:
    ISearchAlgorithm*        searchAlgorithm;
    VariantSpace             variantSpace;
    int                      searchStepCount;
    vector<TuningParameter*> tuningParameters;
    vector<Region*>          regions;
    vector<Rts*>				  rtsVector;
    vector<SearchSpace*>     searchSpaces;
    map<int, int>            selection;
    std::vector<MyPair>      results;
    map<int, double>         individualSearchPath;
    int                      bestScenario; // Currently best scenario
    double                   bestTime;     // Currently best time
    int                      tpEliminated; // TP that only gave worse results
    int                      individual_keep;
    ScenarioPoolSet*         pool_set;
    int                      worstScenario; // Currently worst scenario
    double                   worstTime;     // Currently worst time

public:
    IndividualSearch();

    virtual ~IndividualSearch();

    void initialize( DriverContext*,
                     ScenarioPoolSet* );

    void clear();

    /**
     * @brief Adds a search space with tuning parameters that are investigated individually.
     * @ingroup IndividualSearch
     */
    void addSearchSpace( SearchSpace* );

    void createScenarios();

    void addObjectiveFunction(ObjectiveFunction *obj);

    int getOptimum();

    int getWorst();

    map<int, double>getSearchPath();

    bool searchFinished();

    void terminate();

    void finalize();

    void setScenariosToKeep( int k ) {
        individual_keep = k;
    };
};

#endif
