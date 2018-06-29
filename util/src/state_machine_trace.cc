#include "state_machine_trace.h"

StateMachineTrace*  StateMachineTrace::instance_                         = NULL;
queue<trace_entry>* StateMachineTrace::frontend_statemachine_event_trace = NULL;
int                 StateMachineTrace::global_sequence                   = 0;
string              StateMachineTrace::current_machine_name              = "";
string              StateMachineTrace::current_source_state              = "";
string              StateMachineTrace::current_target_state              = "";
string              StateMachineTrace::current_event_name                = "";
string              StateMachineTrace::current_state                     = "";
bool                StateMachineTrace::enabled                           = false;

StateMachineTrace::StateMachineTrace( void ) {
    frontend_statemachine_event_trace = new queue<trace_entry>();
}

void StateMachineTrace::push_current( void ) {
    // after updating the values, the current state is stored
    frontend_statemachine_event_trace->push(
        trace_entry(
            global_sequence++,
            current_machine_name,
            current_source_state,
            current_target_state,
            current_event_name ) );
    // target state becomes the current state in the state machine
    current_state = current_target_state;
}

void StateMachineTrace::set_machine_name( string machine_name ) {
    current_machine_name = machine_name;
}

void StateMachineTrace::set_target_state( string state_name ) {
    current_target_state = state_name;
}

void StateMachineTrace::set_source_state( string state_name ) {
    current_source_state = state_name;
}

void StateMachineTrace::set_event_name( string event_name ) {
    current_event_name = event_name;
}

void StateMachineTrace::enable( void ) {
    enabled = true;
}

bool StateMachineTrace::is_enabled( void ) {
    return enabled;
}

const char* StateMachineTrace::get_current_state( void ) {
    return current_state.c_str();
}

trace_entry StateMachineTrace::pop( void ) {
    trace_entry temp = frontend_statemachine_event_trace->front();
    frontend_statemachine_event_trace->pop();
    return temp;
}

void StateMachineTrace::print( void ) {
    if( enabled ) {
        printf( "\nPrinting the event trace:\n" );
        printf( "#    -             State Machine -                      Source State -"
                "                         Event -                      Target State\n" );
        while( !empty() ) {
            trace_entry te = pop();
            printf( "%4d - %25s - %33s - %29s - %33s\n",
                    te.sequence,
                    te.machine_name.c_str(),
                    te.source_state_name.c_str(),
                    te.event_name.c_str(),
                    te.target_state_name.c_str() );
        }
        printf( "\n" );
    }
}

bool StateMachineTrace::empty( void ) {
    return frontend_statemachine_event_trace->empty();
}
