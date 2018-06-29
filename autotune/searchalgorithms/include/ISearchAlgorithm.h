/**
   @file    ISearchAlgorithm.h
   @ingroup SearchAlgorithms
   @brief   Search Algorithm interface
   @author  Houssam Haitof
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
 * @defgroup SearchAlgorithms Search Algorithms
 * @ingroup Autotune
 */

#ifndef ISEARCHALGORITHM_H_
#define ISEARCHALGORITHM_H_

#include "selective_debug.h"
#include "psc_errmsg.h"
#include "SearchSpace.h"
#include "DriverContext.h"
#include "ScenarioPoolSet.h"
#include "search_common.h"


class DriverContext;

// STL includes
#include <map>
#include <queue>

class ProgramSignature;


enum PTF_SEARCH_ERROR {
    PROGRAM_NOT_FOUND,
    INITIALIZATION_ERROR,
    CONNECTION_ERROR,
    INVALID_SEARCH_SPACE
};


class ISearchAlgorithm {
protected:
    int                   search_steps;
    std::map<int, double> path;
    std::queue<int>       scenarioIds;
    vector<ObjectiveFunction*> objectiveFunctions;

public:
    ISearchAlgorithm();

    virtual ~ISearchAlgorithm();

    virtual void initialize( DriverContext*,
                             ScenarioPoolSet* ) = 0;

    virtual void clear() = 0;

    virtual void addSearchSpace( SearchSpace* ) = 0;

    virtual void addObjectiveFunction(ObjectiveFunction *obj) = 0;

    virtual void addSearchSpaceWithSignature( SearchSpace* ss, ProgramSignature const& ignored ) {
        addSearchSpace( ss );
    }

    virtual void createScenarios() = 0;

    virtual int getOptimum() = 0;

    virtual int getWorst() = 0;

    virtual std::map<int, double>getSearchPath() = 0;

    virtual bool searchFinished() = 0;

    virtual void terminate() = 0;

    virtual void finalize() = 0;
};

#endif /* ISEARCHALGORITHM_H_ */
