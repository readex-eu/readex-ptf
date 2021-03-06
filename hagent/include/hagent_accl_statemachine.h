#ifndef HAGENT_ACCL_STATEMACHINE
#define HAGENT_ACCL_STATEMACHINE


// maximum number of entries for transition tables
//#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
//#define BOOST_MPL_LIMIT_VECTOR_SIZE 50
//#define BOOST_MPL_LIMIT_MAP_SIZE 50
// maxinum number of states

#define FUSION_MAX_VECTOR_SIZE 15
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>

#include <stdio.h>
#include "state_machine_trace.h"
#include "selective_debug.h"

namespace hagent_accl_msm_namespace {
// top level events
struct request_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Request Event" );
};

struct reply_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Reply Event" );
};

struct send_request_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Send Request Event" );
};

struct send_reply_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Send Reply Event" );
};

struct start_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Start Event" );
};

struct hagent_accl_msm : public boost::msm::front::state_machine_def<hagent_accl_msm>{
    // used in tracing macros
    PSC_SM_TRACE_MACHINE_DECLARATION( "High Level Agent ACCL" );

    // top level states
    struct InitialState : public boost::msm::front::state<>{ PSC_SM_TRACE_SET_STATE_TRACING( "Initializing" ); };

    // the initial state of the frontend SM. Must be defined.
    typedef InitialState initial_state;

    // transition actions
    void handle_request( request_event const& evt );

    void handle_reply( reply_event const& evt );

    void handle_send_request( send_request_event const& evt );

    void handle_send_reply( send_reply_event const& evt );

    void handle_start( start_event const& evt );

    // executed on invalid transitions
    template <class Fsm, class Event>
    void no_transition( Event const& e, Fsm&, int state ) {
        //printf( "ERROR: Invalid event \"%s\" received in state: %d\n",
        //e.toString(), PSC_SM_TRACE_SINGLETON->get_current_state());
        PSC_SM_TRACE_SINGLETON->print();
    }

    typedef hagent_accl_msm hamsm; // makes transition table cleaner

    // Transition table for main_frontend_statemachine
    struct transition_table : boost::mpl::vector<
            //      Start State   Event               Next State    Action                       Guard
            a_row < InitialState, request_event,      InitialState, & hamsm::handle_request      >,
            a_row < InitialState, reply_event,        InitialState, & hamsm::handle_reply        >,
            a_row < InitialState, send_request_event, InitialState, & hamsm::handle_send_request >,
            a_row < InitialState, start_event,        InitialState, & hamsm::handle_start        >,
            a_row < InitialState, send_reply_event,   InitialState, & hamsm::handle_send_reply   >
            > {};
};
typedef boost::msm::back::state_machine<hagent_accl_msm> high_level_agent_accl_statemachine;
}

#endif
