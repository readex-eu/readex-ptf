/**
   @file    ScorepMPIStrategy.h
   @ingroup ScorepMPIStrategy
   @brief   ScoreP MPI search strategy header
   @author  Yury Oleynik
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2011, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

/**
 * @defgroup ScorepMPIStrategy ScoreP MPI Analysis Strategy
 * @ingroup Strategies
 */

#ifndef STRATEGY_SCOREPMPISTRATEGY_H
#define STRATEGY_SCOREPMPISTRATEGY_H

#include <stdint.h>

#include "global.h"
#include "strategy.h"
#include "application.h"
#include "Metric.h"


class ScorepMPIStrategy : public Strategy {
protected:

    Prop_List candProperties;
    Region*   phaseRegion;

    Prop_List create_initial_candidate_properties_set( Region* initial_region );
    Prop_List create_next_candidate_properties_set( std::list< Property* > ev_set );
    Metric interpret_MPI_LATE_SEND_OfRegion( std::string region_name );

public:
    ScorepMPIStrategy( bool pedantic = false )
        : Strategy( pedantic ), phaseRegion( NULL ) {
    }

    ~ScorepMPIStrategy();

    void metric_found_callback( Metric  m,
                                Context ct );

    std::string name();

    bool reqAndConfigureFirstExperiment( Region* initial_region );                                   // TRUE can start; FALSE not ready

    bool evaluateAndReqNextExperiment();                                                             // TRUE requires next step; FALSE if done

    void configureNextExperiment();
};


#endif
