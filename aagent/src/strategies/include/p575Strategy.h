/**
   @file    p575Strategy.h
   @ingroup p575Strategy
   @brief   Power6 depth-first search strategy header
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
 * @defgroup p575Strategy Power6 Analysis Strategy
 * @ingroup Strategies
 */

#include "global.h"
#include "strategy.h"
#include "application.h"
#include <vector>
#include <bitset>
#include "PropertyID.h"

class p575Strategy : public Strategy {
protected:
    Property*                                                                                    property;
    Prop_List                                                                                    candProperties;
    Prop_List                                                                                    foundPropertiesLastStep;
    Region*                                                                                      phaseRegion;
    int                                                                                          strategyIterations;
    Prop_List                                                                                    propsRefineRegionNesting;
    std::vector<std::map<Region*, std::bitset<( ( int )p575PropEnd - ( int )p575PropBegin )> > > checkedSubs;

public:

    p575Strategy( bool pedantic = false ) : Strategy( pedantic ) {
    }

    ~p575Strategy() {
    }

    int get_strategy_steps() {
        return strategyIterations;
    }
    Prop_List create_initial_candidate_properties_set( Region* initial_region );

    Prop_List create_next_candidate_properties_set( std::list<Property*> ev_set );

    std::string name();

    bool reqAndConfigureFirstExperiment( Region* initial_region ); // TRUE can start; FALSE not ready

    bool evaluateAndReqNextExperiment();                           // TRUE requires next step; FALSE if done

    void configureNextExperiment();
};
