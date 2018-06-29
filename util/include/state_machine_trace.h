#ifndef PSC_STATEMACHINE_TRACING_
#define PSC_STATEMACHINE_TRACING_

#include <stdio.h>
#include <queue>
#include <string>
using namespace std;

#define PSC_SM_TRACE_SINGLETON StateMachineTrace::getInstance()

#define PSC_SM_TRACE_SET_EVENT_NAME                               \
    if( PSC_SM_TRACE_SINGLETON->is_enabled() ) {                 \
    PSC_SM_TRACE_SINGLETON->set_event_name( evt.toString() ); \
    }

#define PSC_SM_TRACE_SET_STATE_TRACING( _state_name_ )                 \
    template <class Event, class Fsm>                                  \
    void on_entry( Event const & evt, Fsm & fsm ) {                     \
        if( PSC_SM_TRACE_SINGLETON->is_enabled() ) {                  \
            PSC_SM_TRACE_SINGLETON->set_machine_name( machineName() ); \
            PSC_SM_TRACE_SINGLETON->set_target_state( toString() );    \
            PSC_SM_TRACE_SINGLETON->push_current();                    \
        }                                                              \
    }                                                                  \
    template <class Event, class Fsm>                                  \
    void on_exit( Event const & evt, Fsm & fsm ) {                      \
        if( PSC_SM_TRACE_SINGLETON->is_enabled() ) {                  \
            PSC_SM_TRACE_SINGLETON->set_source_state( toString() );    \
        }                                                              \
    }                                                                  \
    static char* toString() { return ( char* )_state_name_; }

#define PSC_SM_TRACE_EVENT_DECLARATION( _event_name_ )        \
    static char* toString() { return ( char* )_event_name_; }

#define PSC_SM_TRACE_MACHINE_DECLARATION( _machine_name_ )         \
    static char* machineName() { return ( char* )_machine_name_; }

struct trace_entry {
    int    sequence;
    string machine_name;
    string event_name;
    string source_state_name;
    string target_state_name;

    trace_entry( int    new_sequence,
                 string new_machine_name,
                 string new_source_state_name,
                 string new_target_state_name,
                 string new_event_name ) :
        sequence( new_sequence ),
        machine_name( new_machine_name ),
        source_state_name( new_source_state_name ),
        target_state_name( new_target_state_name ),
        event_name( new_event_name ) {
    }
};

class StateMachineTrace {
public:
    // the state machine trace is a singleton
    static StateMachineTrace* getInstance() {
        if( !instance_ ) {
            instance_ = new StateMachineTrace();
        }
        return instance_;
    }

    void push_current( void );

    void set_machine_name( string machine_name );

    void set_target_state( string state_name );

    void set_source_state( string state_name );

    void set_event_name( string event_name );

    const char* get_current_state( void );

    void enable( void );

    bool is_enabled( void );

    trace_entry pop( void );

    void print( void );

    bool empty( void );

private:
    static int                 global_sequence;
    static queue<trace_entry>* frontend_statemachine_event_trace;
    static StateMachineTrace*  instance_;
    static string              current_machine_name;
    static string              current_source_state;
    static string              current_target_state;
    static string              current_event_name;
    static string              current_state;
    static bool                enabled;

    StateMachineTrace( void );
};

#endif // STATEMACHINE_TRACING_
