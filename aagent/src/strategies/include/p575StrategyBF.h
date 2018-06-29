/**
   @file    p575StrategyBF.h
   @ingroup p575StrategyBF
   @brief   Power6 breadth-first search strategy header
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
 * @defgroup p575StrategyBF Power6 Breadth-First Analysis Strategy
 * @ingroup Strategies
 */

#include "global.h"
#include "strategy.h"
#include "application.h"
#include <vector>
#include <bitset>
#include "PropertyID.h"

typedef struct PphaseCYCDeviation {
    Context*  phaseContext;
    INT64     baseCyc;
    long long deviation;
} PhaseCYCDeviation;

class p575StrategyBF : public Strategy {
protected:
    Property*                             property;
    Prop_List                             candProperties;
    Prop_List                             foundPropertiesLastStep;
    Region*                               phaseRegion;
    Context*                              phaseContext;
    Context*                              phase_0_0_context;
    int                                   strategyIterations;
    Prop_List                             propsRefineRegionNesting;
    std::vector<std::map<Region*, bool> > checkedSubs;
    std::map<int, PhaseCYCDeviation*>     phaseCycDeviations;
    bool                                  withDevCheck;
    bool                                  withMemProp;
    bool                                  withLeafNodes;

public:
    p575StrategyBF( bool pedantic = false ) : Strategy( pedantic ), withDevCheck( false ) {
    }

    p575StrategyBF( bool DevCheck,
                    bool memProp,
                    bool leafNodes,
                    bool pedantic = false );

    ~p575StrategyBF() {
    }

    Prop_List create_initial_candidate_properties_set( Region* initial_region );

    Prop_List create_next_candidate_properties_set( std::list<Property*> ev_set );

    void createTopProps( Region* initial_region,
                         int     rank );

    void createTopProps( Context* ct,
                         Context* phaseCt );

    std::string name();

    void restartSearchStep();

    void initCycDeviation();

    bool checkCycDeviation();

    void requestCycDeviationMetrics();

    bool reqAndConfigureFirstExperiment( Region* initial_region ); // TRUE can start; FALSE not ready

    bool evaluateAndReqNextExperiment();                           // TRUE requires next step; FALSE if done

    void configureNextExperiment();
};
