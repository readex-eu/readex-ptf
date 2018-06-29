/**
   @file    OCLStrategy.h
   @ingroup OCLStrategy
   @brief   OpenCL search strategy based on global request header
   @author  Robert Mijakovic
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2015, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

/**
 * @defgroup OCLStrategy OpenCL Analysis Strategy
 * @ingroup Strategies
 */

#ifndef STRATEGY_OCL_H
#define STRATEGY_OCL_H

#include "global.h"
#include "strategy.h"
#include "application.h"

class OCLStrategy : public Strategy {
protected:
    Property* property;
    Prop_List candProperties;
    Prop_List foundPropertiesLastStep;
    Region*   phaseRegion;

public:
    OCLStrategy( bool pedantic = false ) :
        Strategy( pedantic ) {
    }

    ~OCLStrategy();
// Strategy class virtual functions
    Prop_List create_initial_candidate_properties_set( Region* initial_region );
    Prop_List create_next_candidate_properties_set( std::list<Property*> ev_set );
    std::string name();

    bool reqAndConfigureFirstExperiment( Region* initial_region ); // TRUE can start; FALSE not ready
    bool evaluateAndReqNextExperiment();                           // TRUE requires next step; FALSE if done
    void configureNextExperiment();

// OCL Strategy specific functions
    void createCandidateProperties();
};

#endif /* STRATEGY_OCL_H */
