/**
   @file    InstrumentationStrategy.h
   @ingroup InstrumentationStrategy
   @brief   Instrumentation search strategy header
   @author  Yury Oleynik
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
 * @defgroup InstrumentationStrategy Instrumentation Analysis Strategy
 * @ingroup Strategies
 */

#ifndef STRATEGY_INSTRUMENTATION_H
#define STRATEGY_INSTRUMENTATION_H

#include <vector>
#include <bitset>
#include <string>
#include <map>
#include "global.h"
#include "strategy.h"
#include "application.h"
#include "OverheadProp.h"

//#define InstrumentationOverhead 406400
//#define InstrumentationOverhead 329000
//#define InstrumentationOverhead 140000
#define InstrumentationOverhead _OVERHEAD  //Overhead for start_region and stop_region together that is not canceled out by stopping the counters
//in cycles
//defined in makefile

class InstrumentationStrategy : public Strategy {
public:
    enum InstStrategy {
        NONE, OVERHEAD_BASED, ANALYSIS_BASED, ALL_OVERHEAD_BASED
    };

protected:

    Region* phaseRegion;
    bool    profiling_done;

    std::list<Region*>      leafRegions;
    std::map<long, Region*> parent;
    int                     region_cnt;
    InstStrategy            inst_type;

public:
    InstrumentationStrategy( std::string strategy,
                             bool        pedantic = false );

    ~InstrumentationStrategy();

    //void estimateOverhead(std::list<INT64*> time);
    void setProfiling_done( bool value );

    bool requireProfilingRun();

    std::string name();

    void requiredRegions();

    void createTopProps( Region* region,
                         int     rank );

    void createOverheadProperties( Region*,
                                   int,
                                   int );

    bool reqAndConfigureFirstExperiment( Region* ); // TRUE can start; FALSE not ready

    bool evaluateAndReqNextExperiment();            // TRUE requires next step; FALSE if done

    void configureNextExperiment();
};

#endif
