/**
   @file    experiment.cc
   @ingroup AnalysisAgent
   @brief   Performance experiment abstraction
   @author  Edmond Kereku
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2012, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include "global.h"
#include "Property.h"
#include "experiment.h"
#include <iostream>
#include "psc_errmsg.h"
//
//Implementation of class Experiment
//

/*****************************************************************************************/
//
//Implementation of class Experiment
//

/**
 * @brief Experiment constructor
 *
 * Experiment constructor. Here is specified what to measure (property set) and
 * the application is released to apply the measures.
 *
 * @param app Pointer to the application object
 */
Experiment::Experiment( Application* app ) {
    started     = false;
    application = app;
//	psc_dbgmsg(1,"Experiment initialized\n");
}

/**
 * @brief Experiment destructor
 */
Experiment::~Experiment() {
}

/**
 * @brief Application setter
 *
 * @param app Pointer to Application object
 */
void Experiment::set_application( Application* app ) {
    application = app;
}

/**
 * @brief begin experiment
 *
 * Begin of experiment. It first checks if phase region or user region should be used. Then calls
 * through get_required_info() the data provider to request for information. Sends requests to processes
 * (through a start command to agents) to get measurements and to stop on end region.
 *
 */
void Experiment::begin() {
    if( appl->get_phase_region() ) {
        psc_dbgmsg( 3, "Starting default experiment, for phase region (%d,%d)\n", appl->get_phase_region()->get_ident().file_id, appl->get_phase_region()->get_ident().rfl );
    }
    else {
        psc_dbgmsg( 3, "Starting default experiment, phase not specified\n" );
    }
    //Start the MRI and let it run through the phase region
    dp->stop_on_end_region( appl->get_phase_region() );
    started = true;
}

/**
 * @brief End the experiment
 *
 * End the experiment. Checks if it is indeed running or started and if so, it first checks if phase region or user region is used.
 * It gets the results from data provider and stops at the end of the region. If all information has
 * been gathered, properties are evaluated and added to the evaluated property list. If no phase was
 * specified the application is restarted
 *
 * @return true if experiment was running false if it wasn't started or is no longer executed
 */
bool Experiment::end() {
    dp->wait();

    return true;
}

/**
 * @brief Run the experiment
 *
 */
void Experiment::run() {
    begin();
    end();
}


/**
 * @brief not yet implemented
 */
double Experiment::get_duration() {
    return 1.0;
}

/**
 * @brief not yet implemented
 */
int Experiment::get_threads() {
    return 8;
}
