/**
   @file    IndividualATPSearch.h
   @ingroup IndividualATPSearch
   @brief   Individual search algorithm for ATP
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

/**
 * @defgroup IndividualSearch Individual Search Algorithm
 * @ingroup SearchAlgorithms
 */

#ifndef INDIVIDUALATPSEARCH_H_
#define INDIVIDUALATPSEARCH_H_

// Autotune includes
#include "AutotuneSearchAlgorithm.h"
#include "search_common.h"
#include "ATPService.h"

class IndividualATPSearch : public ISearchAlgorithm {
private:
    ISearchAlgorithm*        searchAlgorithm;
    VariantSpace             variantSpaceDomain;
    VariantSpace             variantSpace;
    int                      searchStepCount;
    vector<TuningParameter*> tuningParameters;
    vector<TuningParameter*> exploredTuningParameters;
    vector<Region*>          regions;
    vector<Rts*>			 rtsVector;
    vector<SearchSpace*>     searchSpaces;
    map<int, vector<int>>    selection;
    std::vector<MyPair>      results;
    map<int, double>         individualSearchPath;
    int                      bestScenario; // Currently best scenario
    double                   bestEnergy;     // Currently best Energy
    int                      tpEliminated; // TP that only gave worse results
    int                      individual_keep;
    ScenarioPoolSet*         pool_set;
    int                      worstScenario; // Currently worst scenario
    double                   worstEnergy;     // Currently worst Energy
    atpService*              atpServc;

public:
    IndividualATPSearch();

    virtual ~IndividualATPSearch();

    void initialize( DriverContext*,
                     ScenarioPoolSet* );

    void clear();

    /**
     * @brief Adds a search space with tuning parameters that are investigated individually.
     * @ingroup IndividualATPSearch
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

    void setATPService( atpService* atp_srvc ){
        atpServc = atp_srvc;
    };
};
int getTPIndex( string , vector<TuningParameter*> );
#endif
