/**
   @file    LoadSearch.h
   @ingroup LoadSearch
   @brief   Load search algorithm header
   @author  Robert Mijakovic
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

/**
 * @defgroup LoadSearch Load Search Algorithm
 * @ingroup SearchAlgorithms
 */

#ifndef LOADSEARCH_H_
#define LOADSEARCH_H_

// Autotune includes
#include "AutotuneSearchAlgorithm.h"
#include "search_common.h"

class LoadSearch : public ISearchAlgorithm {
private:
    vector<SearchSpace*>     searchSpaces;
    vector<TuningParameter*> tuningParameters;
    vector<unsigned int>     scenarioMap;
    int                      optimum;
    double                   optimumValue;
    ScenarioPoolSet*         pool_set;
public:
    LoadSearch();

    virtual ~LoadSearch();

    void initialize( DriverContext*,
                     ScenarioPoolSet* );

    void clear();

    void addSearchSpace( SearchSpace* );

    void addObjectiveFunction(ObjectiveFunction *obj);

    void createScenarios();

    int getOptimum();

    int getWorst();

    map<int, double >getSearchPath();

    bool searchFinished();

    void terminate();

    void finalize();
};

int parseLoadConfig( const char*           filename,
                     vector<unsigned int>& sIds );

#endif /* LOADSEARCH_H_ */
