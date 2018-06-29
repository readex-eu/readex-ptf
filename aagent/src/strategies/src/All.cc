/**
   @file    All.cc
   @ingroup AllStrategy
   @brief   Generic search strategy that combines stall cycles and MPI analysis
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

#include <iostream>
#include "psc_errmsg.h"
#include "global.h"
#include "All.h"
#include "MPIStrategy.h"
#include "StallCycleAnalysis.h"

AllStrategy::AllStrategy( bool pedantic ) : Strategy( pedantic ) {
    mpiStrategy       = new MPIStrategy( pedantic );
    stallStrategy     = new StallCycleAnalysis( pedantic );
    stallStrategyDone = false;
}

AllStrategy::~AllStrategy() {
    //mpiStrategy->~Strategy();
    //stallStrategy->~Strategy();
}

bool AllStrategy::reqAndConfigureFirstExperiment( Region* r ) {
    mpiStrategy->set_max_strategy_steps( 5 );
    stallStrategy->set_max_strategy_steps( max_strategy_steps - 5 );

    phase = r;
    strategy_steps++;

    return mpiStrategy->reqAndConfigureFirstExperiment( r );
}

void AllStrategy::configureNextExperiment() {
    if( !stallStrategyDone ) {
        mpiStrategy->configureNextExperiment();
    }
    else {
        stallStrategy->configureNextExperiment();
    }
}

bool AllStrategy::evaluateAndReqNextExperiment() {
    // TRUE requires next step; FALSE if done
    bool                nextExperiment = false;
    Prop_List::iterator prop_it;
    Prop_List           foundProps;

    if( !stallStrategyDone ) {
        if( mpiStrategy->evaluateAndReqNextExperiment() ) {
            nextExperiment = true;
        }
        else {
            stallStrategyDone = true;
            stallStrategy->set_strategy_steps( mpiStrategy->get_strategy_steps() );
            if( stallStrategy->reqAndConfigureFirstExperiment( phase ) ) {
                nextExperiment = true;
            }
        }
    }
    else {
        if( stallStrategy->evaluateAndReqNextExperiment() ) {
            nextExperiment = true;
        }
    }

    foundProperties = mpiStrategy->get_found_properties();
    mpiStrategy->clear_found_properties();
    foundProps = stallStrategy->get_found_properties();
    stallStrategy->clear_found_properties();
    for( prop_it = foundProps.begin(); prop_it != foundProps.end(); prop_it++ ) {
        foundProperties.push_back( *prop_it );
    }

    return nextExperiment;
}

std::list<Property*> AllStrategy::create_initial_candidate_properties_set( Region* initial_region ) {
}

std::list<Property*> AllStrategy::create_next_candidate_properties_set( std::list<Property*> ev_set ) {
}

std::string AllStrategy::name() {
    return "AllStrategy";
}
