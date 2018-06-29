/**
   @file    frontend_accl_statemachine.cc
   @ingroup Communication
   @brief   Front-end communication layer state-machine states header
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

#ifndef FRONTEND_ACCL_STATEMACHINE
#define FRONTEND_ACCL_STATEMACHINE

// maximum number of entries for transition tables
//#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
//#define BOOST_MPL_LIMIT_VECTOR_SIZE 50
//#define BOOST_MPL_LIMIT_MAP_SIZE 50
// maxinum number of states

#define FUSION_MAX_VECTOR_SIZE 15
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>

#include <stdio.h>
#include <string>
#include "frontend.h"
#include "selective_debug.h"
#include "state_machine_trace.h"

namespace msm = boost::msm;
namespace mpl = boost::mpl;

namespace femsm {
// top level events
struct heartbeat_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Heart-beat" );
};

struct search_finished_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Search Finished" );
};

struct decide_continuation_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Decide Continuation" );
};

struct found_property_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Found Property" );
};

struct properties_sent_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Properties Sent" );
};

struct reexperiment_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Re-experiment" );
};

struct need_restart_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Need Restart" );
};

struct terminated_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Terminated" );
};

struct finalize {
    PSC_SM_TRACE_EVENT_DECLARATION( "Finalize" );
};

struct frontend_msm : public msm::front::state_machine_def<frontend_msm>{
    // used in tracing macros
    PSC_SM_TRACE_MACHINE_DECLARATION( "Frontend ACCL" );

    // top level states
    struct WaitingForHeartBeat : public msm::front::state<>{
        PSC_SM_TRACE_SET_STATE_TRACING( "WaitingForHeartBeat" );
    };

    struct SearchingForProperties : public msm::front::state<>{
        PSC_SM_TRACE_SET_STATE_TRACING( "SearchingForProperties" );
    };

    struct WaitingForProperties : public msm::front::state<>{
        PSC_SM_TRACE_SET_STATE_TRACING( "WaitingForProperties" );
    };

    struct DecidingContinuation : public msm::front::state<>{
        PSC_SM_TRACE_SET_STATE_TRACING( "DecidingContinuation" );
    };

    struct Finalizing : public msm::front::state<>{
        PSC_SM_TRACE_SET_STATE_TRACING( "Finalizing" );
    };

    // the initial state of the frontend SM. Must be defined.
    typedef WaitingForHeartBeat initial_state;

    // transition actions
    void handle_heartbeat( heartbeat_event const& evt );

    void handle_search_finished( search_finished_event const& evt );

    void handle_found_property( found_property_event const& evt );

    void handle_decide_continuation( decide_continuation_event const& evt );

    void handle_properties_sent( properties_sent_event const& evt );

    void handle_reexperiment( reexperiment_event const& evt );

    void handle_need_restart( need_restart_event const& evt );

    void handle_terminated( terminated_event const& evt );

    void do_finalization( finalize const& );

    // executed on invalid transitions
    template <class Fsm, class Event>
    void no_transition( Event const& e,
                        Fsm&,
                        int          state ) {
        printf( "---- FRONTEND ACCL STATEMACHINE ---- ; "
                "ERROR: Invalid event \"%s\" received in state: %d\n",
                e.toString(), PSC_SM_TRACE_SINGLETON->get_current_state() );
        PSC_SM_TRACE_SINGLETON->print();
    }

    typedef frontend_msm f_msm; // makes transition table cleaner

    // Transition table for main_frontend_statemachine
    struct transition_table : mpl::vector<
            //      Start State             Event                      Next State              Action                              Guard
            a_row < WaitingForHeartBeat,    heartbeat_event,           SearchingForProperties, & f_msm::handle_heartbeat           >,
            a_row < SearchingForProperties, search_finished_event,     DecidingContinuation,   & f_msm::handle_search_finished     >,
            a_row < SearchingForProperties, need_restart_event,        DecidingContinuation,   & f_msm::handle_need_restart        >,
            a_row < DecidingContinuation,   decide_continuation_event, WaitingForProperties,   & f_msm::handle_decide_continuation >,
            a_row < WaitingForProperties,   search_finished_event,     DecidingContinuation,   & f_msm::handle_search_finished     >,
            a_row < WaitingForProperties,   found_property_event,      WaitingForProperties,   & f_msm::handle_found_property      >,
            a_row < WaitingForProperties,   properties_sent_event,     DecidingContinuation,   & f_msm::handle_properties_sent     >,
            a_row < WaitingForProperties,   heartbeat_event,           SearchingForProperties, & f_msm::handle_heartbeat           >,
            a_row < WaitingForProperties,   terminated_event,          WaitingForHeartBeat,    & f_msm::handle_terminated          >
            > {};
};

// Pick a back-end
typedef msm::back::state_machine<frontend_msm> frontend_statemachine;
}


#endif
