/**
   @file    BGPStrategyDF.h
   @ingroup BGPStrategyDF
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
 * @defgroup BGPStrategyDF Blue Gene-P Depth-First Analysis Strategy
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

class BGPStrategyDF : public Strategy {
protected:
    Property*                             property;
    Prop_List                             candProperties;
    Prop_List                             foundPropertiesLastStep;
    Region*                               phaseRegion;
    Context*                              phaseContext;
    int                                   strategyIterations;
    Prop_List                             propsRefineRegionNesting;
    std::vector<std::map<Region*, bool> > checkedSubs;
    //std::map <int, PhaseCYCDeviation*> phaseCycDeviations;
    //bool withDevCheck;
    //bool withMemProp;

public:
    //BGPStrategyDF(bool DevCheck, bool memProp);
    BGPStrategyDF( bool pedantic = false ) : Strategy( pedantic ) {
    }

    ~BGPStrategyDF() {
    }

    Prop_List create_initial_candidate_properties_set( Region* initial_region );
    Prop_List create_next_candidate_properties_set( std::list<Property*> ev_set );
    void createTopProps( Region* initial_region,
                         int     rank );
    std::string name();

    //void restartSearchStep();
    //void initCycDeviation();
    //bool checkCycDeviation();
    //void requestCycDeviationMetrics();
    bool reqAndConfigureFirstExperiment( Region* initial_region ); // TRUE can start; FALSE not ready
    bool evaluateAndReqNextExperiment();                           // TRUE requires next step; FALSE if done
    void configureNextExperiment();
};
