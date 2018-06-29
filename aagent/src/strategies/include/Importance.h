/**
   @file    Importance.h
   @ingroup ImportanceStrategy
   @brief   Importance search strategy header
   @author  Robert Mijacovic
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
 * @defgroup ImportanceStrategy Importance Analysis Strategy
 * @ingroup Strategies
 */

#ifndef Importance_H_
#define Importance_H_
#include "global.h"
#include "application.h"
#include "Metric.h"
#include "strategy.h"
#include <string.h>
#include <vector>

class Importance : public Strategy {
private:
    Region*   phaseRegion;
    Prop_List candProperties;
    Prop_List foundPropertiesLastStep;
    Prop_List propsRefineRegionNesting;

public:
    Importance( bool pedantic = false ) : Strategy( pedantic ) {
    }

    ~Importance() {
    }

    std::list<Property*>create_initial_candidate_properties_set( Region* initial_region );

    std::list<Property*>create_next_candidate_properties_set( std::list<Property*> ev_set );

    std::string name();

    void metric_found_callback( Metric  m,
                                Context ct );

    bool reqAndConfigureFirstExperiment( Region* initial_region ); // TRUE can start; FALSE not ready

    bool evaluateAndReqNextExperiment();                           // TRUE requires next step; FALSE if done

    void configureNextExperiment();
};

#endif /*Importance_H_*/
