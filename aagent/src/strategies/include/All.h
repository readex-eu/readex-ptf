/**
   @file    All.h
   @ingroup AllStrategy
   @brief   Generic search strategy header
   @author  Michael Ott
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2013, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

/**
 * @defgroup AllStrategy Meta Analysis Strategy
 * @ingroup Strategies
 */


#ifndef STRATEGY_All_H
#define STRATEGY_All_H

#include "global.h"
#include "application.h"
#include "strategy.h"
#include "MPIStrategy.h"
#include "StallCycleAnalysis.h"

class AllStrategy : public Strategy {
protected:
    Property*           property;
    Region*             phase;
    MPIStrategy*        mpiStrategy;
    StallCycleAnalysis* stallStrategy;
    bool                stallStrategyDone;

public:
    AllStrategy( bool pedantic = false );
    ~AllStrategy();

    Prop_List create_initial_candidate_properties_set( Region* initial_region );
    Prop_List create_next_candidate_properties_set( std::list<Property*> ev_set );
    std::string name();

    bool reqAndConfigureFirstExperiment( Region* initial_region ); // TRUE can start; FALSE not ready
    bool evaluateAndReqNextExperiment();                           // TRUE requires next step; FALSE if done
    void configureNextExperiment();
};

#endif
