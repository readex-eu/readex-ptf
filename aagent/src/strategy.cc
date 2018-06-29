/**
   @file	strategy.cc
   @ingroup AnalysisAgent
   @brief   Performance analysis strategy abstraction
   @author	Edmond Kereku
   @verbatim
        Revision:       $Revision$
        Revision date:  $Date$
        Committed by:   $Author$

        This file is part of the Periscope performance measurement tool.
        See http://www.lrr.in.tum.de/periscope for details.

        Copyright (c) 2005-2011, Technische Universitaet Muenchen, Germany
        See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include "global.h"
#include "Property.h"
#include "application.h"
#include "strategy.h"
#include "psc_errmsg.h"

#include <iostream>

//
//Implementation of class NodeAgent
//

Strategy::~Strategy() {
}

void Strategy::set_strategy_steps( int steps ) {
    strategy_steps = steps;
}

int Strategy::get_strategy_steps() {
    return strategy_steps;
}


void Strategy::set_max_strategy_steps( int steps ) {
    max_strategy_steps = steps;
}

int Strategy::get_max_strategy_steps() {
    return max_strategy_steps;
}

void Strategy::set_require_restart( bool set ) {
    require_restart = set;
}

bool Strategy::get_require_restart() {
    return require_restart;
}


void clear_found_properties() {
    for( auto& prop : foundProperties ) {
        delete prop;
    }
    foundProperties.clear();
}

void add_to_found_properties( Property* prop ) {
    foundProperties.push_back( prop );
}

void Strategy::metric_found_callback( Metric m, Context ct ) {
    //empty method
    return;
}
void Strategy::region_definition_received_callback( Region* reg ) {
    //empty method
    return;
}
