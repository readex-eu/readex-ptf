/**
   @file    StallCycleAnalysisPerSyst.h
   @ingroup StallCycleAnalysisPerSystStrategy
   @brief   Itanium2 stall cycles breadth-first search strategy
   @author  Michael Gerndt
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
 * @defgroup StallCycleAnalysisPerSystStrategy Stall Cycle PerSyst Analysis Strategy
 * @ingroup Strategies
 */

#ifndef StallCycleAnalysisPerSyst_H_
#define StallCycleAnalysisPerSyst_H_
#include "global.h"
#include "application.h"
#include "Metric.h"
#include "strategy.h"
#include "StallCycles.h"

class StallCycleAnalysisPerSyst : public Strategy {
private:
    Region*   phaseRegion;
    Prop_List candProperties;
    Prop_List foundPropertiesLastStep;

    //Only the subregions of one evaluated propertie's context are measured
    //in the next step.  the other evaluated Properties are stocked in this queue
    //or in the call_queue
    Prop_List propsRefineRegionNesting;
    //The list of already checked subroutines
    std::map<Region*, bool> checkedSubs;

    StallCyclesProp* duplicateAndRefine( StallCyclesProp* p,
                                         Metric           m );

public:
    StallCycleAnalysisPerSyst( bool pedantic = false ) : Strategy( pedantic ) {
    }

    ~StallCycleAnalysisPerSyst() {
    }

    std::list<Property*>create_initial_candidate_properties_set( Region* initial_region );

    std::list<Property*>create_next_candidate_properties_set( std::list<Property*> ev_set );

    std::string name();

    bool PerSystList[ 50 ]; //for saving the Persyst properties

    bool reqAndConfigureFirstExperiment( Region* initial_region ); // TRUE can start; FALSE not ready

    bool evaluateAndReqNextExperiment();                           // TRUE requires next step; FALSE if done

    void configureNextExperiment();

    virtual void set_propid_list_persyst( std::list<int> propID_list );           //for saving the Persyst properties
};

#endif /*StallCycleAnalysisPerSyst_H_*/
