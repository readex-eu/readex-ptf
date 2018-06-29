/**
   @file    MPIStrategy.h
   @ingroup MPIStrategy
   @brief   MPI search strategy header
   @author  Michael Ott
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
 * @defgroup MPIStrategy MPI Analysis Strategy
 * @ingroup Strategies
 */

#ifndef STRATEGY_MPI_H
#define STRATEGY_MPI_H

#include "global.h"
#include "strategy.h"
#include "application.h"

class MPIStrategy : public Strategy {
protected:
    Property* property;
    Prop_List candProperties;
    Prop_List foundPropertiesLastStep;
    Region*   phaseRegion;

    void requestMPI();

public:
    MPIStrategy( bool pedantic = false ) :
        Strategy( pedantic ) {
    }

    ~MPIStrategy();

    Prop_List create_initial_candidate_properties_set( Region* initial_region );

    Prop_List create_next_candidate_properties_set( std::list<Property*> ev_set );

    std::string name();

    void createCandidateProperties();

    bool reqAndConfigureFirstExperiment( Region* initial_region ); // TRUE can start; FALSE not ready

    bool evaluateAndReqNextExperiment();                           // TRUE requires next step; FALSE if done

    void configureNextExperiment();
};

#endif
