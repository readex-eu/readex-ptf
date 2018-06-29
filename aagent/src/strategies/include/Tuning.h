/**
   @file    Tuning.h
   @ingroup TuningStrategy
   @brief   Tuning strategy header
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
 * @defgroup TuningStrategy Tuning Strategy
 * @ingroup Strategies
 */

#ifndef STRATEGY_TUNINGSTRATEGY_H
#define STRATEGY_TUNINGSTRATEGY_H

#include "global.h"
#include "strategy.h"
#include "application.h"
#include "DataProvider.h"
#include "analysisagent.h"
#include <list>

class TuningStrategy : public Strategy {
private:
    Prop_List                   candProperties;
    Region*                     phaseRegion;
    const std::list<Scenario*>* scenarioList;
    Strategy*                   analysisStrategy;
    bool                        rtsBased;

    std::map<int, std::list<Scenario*> > TS_related_scenarios_per_rank;
    std::map<int, std::list<Scenario*> > PR_related_scenarios_per_rank;

    void requestTuningActions();

    void map_scenarios_to_ranks();

public:
    TuningStrategy( const std::list<Scenario*>* scenarioList,
                    StrategyRequest*            analysisStrategy,
                    bool                        rts_based = false,
                    bool                        pedantic = false);

    ~TuningStrategy();

    std::string name();

    bool reqAndConfigureFirstExperiment( Region* initial_region ); // TRUE can start; FALSE not ready

    bool evaluateAndReqNextExperiment();                           // TRUE requires next step; FALSE if done

    void configureNextExperiment();

    void createCandidatePropertyList( Region*          region,
                                      int              scenarioId,
                                      int              processRank,
                                      PropertyRequest* propertyRequest );

    void createCandidatePropertyList( std::string      entityID,
                                      int              scenarioId,
                                      int              processRank,
                                      PropertyRequest* propertyRequest );
};

#endif
