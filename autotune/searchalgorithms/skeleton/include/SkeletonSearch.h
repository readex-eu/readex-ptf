/**
   @file    SkeletonSearch.h
   @ingroup SkeletonSearch
   @brief   Skeleton of a search algorithm header
   @author  Author's name
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
 * @defgroup SkeletonSearch Skeleton Search Algorithm
 * @ingroup SearchAlgorithms
 */

#ifndef SKELETONSEARCH_H_
#define SKELETONSEARCH_H_

// Autotune includes
#include "AutotuneSearchAlgorithm.h"

class SkeletonSearch : public ISearchAlgorithm {
public:
    SkeletonSearch();
    virtual ~SkeletonSearch();

    void initialize( DriverContext*,
                     ScenarioPoolSet* );
    void clear();
    void addSearchSpace( SearchSpace* );
    void createScenarios();
    int getOptimum();
    int getWorst();

    map<int, double >getSearchPath();
    bool searchFinished();
    void terminate();
    void finalize();
};

#endif
