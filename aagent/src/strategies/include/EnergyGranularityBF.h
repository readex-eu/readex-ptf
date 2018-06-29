/**
   @file    EnergyGranularityBF.h
   @ingroup EnergyGranularityBFStrategy
   @brief   Energy granularity breadth-first search strategy header
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
 * @defgroup EnergyGranularityBFStrategy Energy Granularity Breadth-First Analysis Strategy
 * @ingroup Strategies
 */

#ifndef EnergyGranularityBF_H_
#define EnergyGranularityBF_H_
#include "global.h"
#include "application.h"
#include "Metric.h"
#include "strategy.h"
#include "TuningSpecification.h"
#include <string.h>
#include <vector>


class EnergyGranularityBF : public Strategy {
public:
    EnergyGranularityBF( TuningSpecification* tuningSpec,
                         bool                 pedantic = false );

    std::list<Property*>create_initial_candidate_properties_set( Region* initial_region );

    std::list<Property*>create_next_candidate_properties_set( std::list<Property*> ev_set );
    std::string name();
    void preconfigureAnalysis();
    bool reqAndConfigureFirstExperiment( Region* initial_region ); // TRUE can start; FALSE not ready
    bool evaluateAndReqNextExperiment();                           // TRUE requires next step; FALSE if done
    void configureNextExperiment();

private:
    Region*                    phaseRegion;
    Prop_List                  candProperties;
    Prop_List                  foundPropertiesLastStep;
    Prop_List                  propsRefineRegionNesting;
    TuningSpecification* const spec;
};

#endif
