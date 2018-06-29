/**
   @file    ActiveHarmony.h
   @ingroup ActiveHarmony
   @brief   Interface to a search algorithms of Active Harmony header
   @author  Isaias Compres
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

/**
 * @defgroup ActiveHarmony Active Harmony Search Algorithms Interface
 * @ingroup SearchAlgorithms
 */

#ifndef ACTIVEHARMONY_H_
#define ACTIVEHARMONY_H_

#include "AutotuneSearchAlgorithm.h"

class ActiveHarmony : public ISearchAlgorithm {
private:
    vector<int>          objectives;
    vector<SearchSpace*> searchSpaces;
    int                  optimum;
    map<int, double>     results;
    void defaultObjectiveFunction( int*,
                                   double* );

    int objective;

public:
    ActiveHarmony();

    virtual ~ActiveHarmony();

    void initialize( DriverContext*,
                     ScenarioPoolSet* );

    void addSearchSpace( SearchSpace* );

    void createScenarios();

    int getOptimum();

    int getWorst();

    map<int, double >getSearchPath();

    bool searchFinished( void );

    void terminate();

    void finalize();

    void setObjective( int obj );

    int getObjective();
};

#endif
