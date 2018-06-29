/**
   @file    EnergyGranularity.h
   @ingroup EnergyGranularityStrategy
   @brief   Energy granularity depth-first search strategy header
   @author  Robert Mijakovic
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
 * @defgroup EnergyGranularityStrategy Energy Granularity Analysis Strategy
 * @ingroup Strategies
 */

#ifndef EnergyGranularity_H_
#define EnergyGranularity_H_
#include "global.h"
#include "application.h"
#include "Metric.h"
#include "strategy.h"
#include "TuningSpecification.h"
#include <vector>

using namespace std;

class EnergyGranularity : public Strategy {
private:
    Region*                           phaseRegion;
    Prop_List                         candProperties;
    Prop_List                         foundPropertiesLastStep;
    const list<TuningSpecification*>* preconfigureTSs;


    //Only the subregions of one evaluated propertie's context are measured
    //in the next step. The other evaluated Properties are stocked in this queue
    //or in the call_queue
    Prop_List propsRefineRegionNesting;
    //The list of already checked subroutines
    vector<map <Region*, bool> > checkedSubs;


public:
    EnergyGranularity( bool                              pedantic = false,
                       const list<TuningSpecification*>* tuningSpecs = NULL );
    ~EnergyGranularity() {
    }

    list <Property*>create_initial_candidate_properties_set( Region* initial_region );

    list <Property*>create_next_candidate_properties_set( list< Property* > ev_set );
    string name();
    void preconfigureAnalysis();
    bool reqAndConfigureFirstExperiment( Region* initial_region ); // TRUE can start; FALSE not ready
    bool evaluateAndReqNextExperiment();                           // TRUE requires next step; FALSE if done
    void configureNextExperiment();
};

#endif /*EnergyGranularity_H_*/
