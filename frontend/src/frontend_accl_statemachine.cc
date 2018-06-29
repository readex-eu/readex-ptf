/**
   @file    frontend_accl_statemachine.cc
   @ingroup Communication
   @brief   Front-end communication layer state-machine states
   @author  Karl Fuerlinger
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include <queue>
using namespace std;

#include "frontend_accl_statemachine.h"
#include "state_machine_trace.h"

using namespace femsm;

// transition actions
void frontend_msm::handle_heartbeat( heartbeat_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                "---- FRONTEND ACCL STATEMACHINE ---- ; heartbeat;\n" );
}

void frontend_msm::handle_search_finished( search_finished_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                "---- FRONTEND ACCL STATEMACHINE ---- ; searchfinished;\n" );
}
void frontend_msm::handle_found_property( found_property_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                "---- FRONTEND ACCL STATEMACHINE ---- ; foundprop;\n" );
}

void frontend_msm::handle_decide_continuation( decide_continuation_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                "---- FRONTEND ACCL STATEMACHINE ---- ; decide_continuation;\n" );
}

void frontend_msm::handle_properties_sent( properties_sent_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                "---- FRONTEND ACCL STATEMACHINE ---- ; propertiessent;\n" );
}

void frontend_msm::handle_reexperiment( reexperiment_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                "---- FRONTEND ACCL STATEMACHINE ---- ; reqexperiment;\n" );
}

void frontend_msm::handle_need_restart( need_restart_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                "---- FRONTEND ACCL STATEMACHINE ---- ; needrestart;\n" );
}

void frontend_msm::handle_terminated( terminated_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                "---- FRONTEND ACCL STATEMACHINE ---- ; terminated;\n" );
}

void do_finalization( finalize const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                "---- FRONTEND ACCL STATEMACHINE ---- Action: Doing finalization\n" );
}
