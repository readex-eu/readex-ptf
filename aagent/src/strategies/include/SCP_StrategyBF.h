/**
   @file    SCP_StrategyBF.h
   @ingroup SCPStrategyBF
   @brief   Generic search strategy header
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
 * @defgroup SCPStrategyBF SCP Breadth-First Analysis Strategy
 * @ingroup Strategies
 */

#include "global.h"
#include "strategy.h"
#include "application.h"
#include <vector>
#include <bitset>
#include "PropertyID.h"

class SCP_StrategyBF : public Strategy {
protected:
    Property*                             property;
    Prop_List                             candProperties;
    Prop_List                             foundPropertiesLastStep;
    Region*                               phaseRegion;
    int                                   strategyIterations;
    Prop_List                             propsRefineRegionNesting;
    std::vector<std::map<Region*, bool> > checkedSubs;
    std::map<PropertyID, bool>            availProps;

public:
    SCP_StrategyBF( bool pedantic = false ) : Strategy( pedantic ) {
    }

    ~SCP_StrategyBF() {
    }

    Prop_List create_initial_candidate_properties_set( Region* initial_region );

    Prop_List create_next_candidate_properties_set( std::list<Property*> ev_set );

    void createTopProps( Region* initial_region,
                         int     rank );

    std::string name();

    bool reqAndConfigureFirstExperiment( Region* initial_region ); // TRUE can start; FALSE not ready

    bool evaluateAndReqNextExperiment();                           // TRUE requires next step; FALSE if done

    void configureNextExperiment();
};
