/**
   @file    RandomSearch.h
   @ingroup RandomSearch
   @brief   Random sampling search algorithm
   @author  Miklos Homolya
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
 * @defgroup RandomSearch Random Sampling Search
 * @ingroup SearchAlgorithms
 */

#ifndef RANDOMSEARCH_H_
#define RANDOMSEARCH_H_

#include <boost/shared_ptr.hpp>

// Autotune includes
#include "AutotuneSearchAlgorithm.h"
#include "search_common.h"


class ProbabilityModel {
public:
    virtual ~ProbabilityModel() {
    }
    virtual TuningSpecification* sample() const = 0;
};

typedef boost::shared_ptr<ProbabilityModel> ProbabilityModelPtr;

class RandomSearch : public ISearchAlgorithm {
private:
    ScenarioPoolSet* pool_set;
    int              optimumScenario;

    std::vector<ProbabilityModelPtr> models;
    int                              sampleCount;
    int                  optimum;
    double               optimumValue;
    int                  worst;
    double               worstValue;

public:
    RandomSearch();

    virtual ~RandomSearch();

    void initialize( DriverContext*,
                     ScenarioPoolSet* );

    void clear();

    void
    addProbabilityModel( ProbabilityModelPtr );

    void addObjectiveFunction(ObjectiveFunction *obj);

    void addSearchSpace( SearchSpace* );

    void addSearchSpaceWithSignature( SearchSpace*,
                                      ProgramSignature const& );

    void createScenarios();

    int getOptimum();

    int getWorst();

    map<int, double >getSearchPath();

    bool searchFinished();

    void terminate();

    void finalize();

    void setSampleCount( int sc ) {
        sampleCount = sc;
    }
};

#endif /* RANDOMSEARCH_H_ */
