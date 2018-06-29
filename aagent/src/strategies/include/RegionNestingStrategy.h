/**
   @file    RegionNestingStrategy.h
   @ingroup RegionNestingStrategy
   @ingroup AnalysisAgent
   @brief   Nesting of regions search strategy header
   @author  Edmond Kereku
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
 * @defgroup RegionNestingStrategy Region Nesting Analysis Strategy
 * @ingroup Strategies
 */

#ifndef AMC_STRATEGY_REFINE_H
#define AMC_STRATEGY_REFINE_H

#include "global.h"
#include "strategy.h"
#include "application.h"

class RegionNestingStrategy : public Strategy {
protected:
    Property* property;

    //Only the subregions of one evaluated propertie's context are measured
    //in the next step.  the other evaluated Properties are stocked in this queue
    //or in the call_queue
    Prop_List evaluated_queue;
    //this is a queue of call regions.
    Prop_List call_queue;

    //The list of already checked subroutines
    std::list<Region*> checked_subroutines;
    //each valid found subroutine from a call region is checked
    //against this list to prevent double measurements

    //If there are more Requests for the subregions as available nr. of
    //hardware counters, save the rest of subregions in a list for measurements
    //in the next loop
    std::list<Region*> rest_subregions;
    //return true if the subroutine is on the checked list,
    //otherwise includes the region in the list and return false
    bool subroutine_already_checked( Region* reg );

public:
    RegionNestingStrategy( Property* prop );

    ~RegionNestingStrategy();

    Prop_List create_initial_candidate_properties_set( Region* initial_region );

    Prop_List create_next_candidate_properties_set( std::list<Property*> ev_set );

    std::string name();
};

#endif
