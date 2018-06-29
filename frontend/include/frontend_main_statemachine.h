#ifndef FRONTEND_MAIN_STATEMACHINE
#define FRONTEND_MAIN_STATEMACHINE

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __sparc
#include "getopt.h"
#elif __p575
#include "getopt.h"
#else
#include <getopt.h>
#endif

#include "frontend.h"
#include "regxx.h"
#include "psc_errmsg.h"
#include "selective_debug.h"
#include "timing.h"
#include "psc_config.h"
#include "application.h"
#include "selective_debug.h"
#include "frontend_accl_statemachine.h"
#include "autotune_services.h"


// maximum number of entries for transition tables
//#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
//#define BOOST_MPL_LIMIT_VECTOR_SIZE 50
//#define BOOST_MPL_LIMIT_MAP_SIZE 50
// maxinum number of states
#define FUSION_MAX_VECTOR_SIZE 15
// back-end
#include <boost/msm/back/state_machine.hpp>
//front-end
#include <boost/msm/front/state_machine_def.hpp>


namespace msm = boost::msm;
namespace mpl = boost::mpl;

using namespace std;

void
usage( int argc,
       char* argv[] );
int
parse_opts( int argc,
            char* argv[],
            struct cmdline_opts* copts );

extern int maxfan_default;

namespace frontend_main_statemachine {
static int   myport;
static int   ompnumthreads_val; // threads
static int   timeout;           // timeouts
static int   mpinumprocs_val;   // processes
static char  app_name[ 2000 ];
static int   error_code = EXIT_SUCCESS;
static int   initial_thread;
static char  reg_host[ 2000 ];
static int   reg_port = 0;
static char  tmp_str[ 2000 ];
static char  envRegSpec[ 200 ];
static char  psc_site[ 2000 ];
static char  psc_machine[ 2000 ];
static char* portstr;
static char  app_run_name[ 8000 ];
static char  strategy_name[ 100 ];

// default values for command-line parameters
static int maxthreads_default = 0;

static RegistryService* regsrv;

// top level events
struct start_run {
    PSC_SM_TRACE_EVENT_DECLARATION( "Start Run" );
    start_run( int    argc_in,
               char** argv_in ) :
        argc( argc_in ),
        argv( argv_in ) {
    }
    int    argc;
    char** argv;
};

struct finalize {
    PSC_SM_TRACE_EVENT_DECLARATION( "Finalize" );
};

// initialization events
struct init_data_structures_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Init Data Structures" );
};

struct parse_parameters_event           // with payload from main call
{
    PSC_SM_TRACE_EVENT_DECLARATION( "Parse Parameters" );
    parse_parameters_event( int                argc_in,
                            char**             argv_in,
                            PeriscopeFrontend* fe_in ) :
        argc( argc_in ),
        argv( argv_in ),
        fe( fe_in ) {
    }
    int                argc;
    char**             argv;
    PeriscopeFrontend* fe;
};

struct setup_debug_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Setup Debug" );
};

struct select_hierarchy_setup_mode_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Select Hierarchy Setup Mode" );
    select_hierarchy_setup_mode_event( int    argc_in,
                                       char** argv_in ) :
        argc( argc_in ),
        argv( argv_in ) {
    }
    int    argc;
    char** argv;
};

struct setup_network_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Setup Network" );
};

struct setup_phases_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Setup Phases" );
};

struct setup_processes_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Setup Processes" );
};

struct setup_threads_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Setup Threads" );
};

struct setup_timeouts_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Setup Timeouts" );
};

struct setup_application_data_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Setup Application Data" );
};

struct setup_agents_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Setup Agents" );
};

struct setup_outputfile_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Setup Output File" );
};

struct connect_to_registry_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Connect to Registry" );
};

struct select_starter_event {
    PSC_SM_TRACE_EVENT_DECLARATION( "Select Starter" );
};

// flags
struct initialization_complete {};

// front-end: define the FSM structure
struct frontend_main_msm : public msm::front::state_machine_def<frontend_main_msm>{
    // used in tracing macros
    PSC_SM_TRACE_MACHINE_DECLARATION( "Frontend MAIN" );

    // The list of FSM states
    // In this case, Initializing is also a state machine (submachine)
    struct Initializing_ : public msm::front::state_machine_def<Initializing_>{
        // sub-states
        struct InitializingDataStructures : public msm::front::state<>{
            PSC_SM_TRACE_SET_STATE_TRACING( "InitializingDataStructures" );
        };

        struct ParsingParameters : public msm::front::state<>{
            PSC_SM_TRACE_SET_STATE_TRACING( "ParsingParameters" );
        };

        struct SettingDebug : public msm::front::state<>{
            PSC_SM_TRACE_SET_STATE_TRACING( "SettingDebug" );
        };

        struct SelectingHierarchySetupMode : public msm::front::state<>{
            PSC_SM_TRACE_SET_STATE_TRACING( "SelectingHierarchySetupMode" );
        };

        struct SettingNetwork : public msm::front::state<>{
            PSC_SM_TRACE_SET_STATE_TRACING( "SettingNetwork" );
        };

        struct SettingPhases : public msm::front::state<>{
            PSC_SM_TRACE_SET_STATE_TRACING( "SettingPhases" );
        };

        struct SettingProcesses : public msm::front::state<>{
            PSC_SM_TRACE_SET_STATE_TRACING( "SettingProcesses" );
        };

        struct SettingThreads : public msm::front::state<>{
            PSC_SM_TRACE_SET_STATE_TRACING( "SettingThreads" );
        };

        struct SettingTimeouts : public msm::front::state<>{
            PSC_SM_TRACE_SET_STATE_TRACING( "SettingTimeouts" );
        };

        struct SettingApplicationData : public msm::front::state<>{
            PSC_SM_TRACE_SET_STATE_TRACING( "SettingApplicationData" );
        };

        struct SettingAgents : public msm::front::state<>{
            PSC_SM_TRACE_SET_STATE_TRACING( "SettingAgents" );
        };

        struct SettingOutputfile : public msm::front::state<>{
            PSC_SM_TRACE_SET_STATE_TRACING( "SettingOutputfile" );
        };

        struct ConnectingToRegistry : public msm::front::state<>{
            PSC_SM_TRACE_SET_STATE_TRACING( "ConnectingToRegistry" );
        };

        struct SelectingStarter : public msm::front::state<>{
            PSC_SM_TRACE_SET_STATE_TRACING( "SelectingStarter" );
        };

        struct InitializationComplete : public msm::front::state<>{
            PSC_SM_TRACE_SET_STATE_TRACING( "InitializationComplete" );
            typedef mpl::vector1<initialization_complete> flag_list;
        };

        typedef InitializingDataStructures initial_state;


        void init_data_structures( init_data_structures_event const& evt );

        void parse_parameters( parse_parameters_event const& evt );

        void setup_debug( setup_debug_event const& evt );

        void select_hierarchy_setup_mode( select_hierarchy_setup_mode_event const& evt );

        void setup_network( setup_network_event const& evt );

        void setup_phases( setup_phases_event const& evt );

        void setup_processes( setup_processes_event const& evt );

        void setup_threads( setup_threads_event const& evt );

        void setup_timeouts( setup_timeouts_event const& evt );

        void setup_application_data( setup_application_data_event const& evt );

        void setup_agents( setup_agents_event const& evt );

        void setup_outputfile( setup_outputfile_event const& evt );

        void connect_to_registry( connect_to_registry_event const& evt );

        void select_starter( select_starter_event const& evt );

        typedef Initializing_ i_msm;  // makes transition table cleaner
        struct transition_table : mpl::vector<
                //      Start State                  Command                            Next State                   Action                               Guard
                a_row < InitializingDataStructures,  init_data_structures_event,        ParsingParameters,           & i_msm::init_data_structures        >,
                a_row < ParsingParameters,           parse_parameters_event,            SettingDebug,                & i_msm::parse_parameters            >,
                a_row < SettingDebug,                setup_debug_event,                 SelectingHierarchySetupMode, & i_msm::setup_debug                 >,
                a_row < SelectingHierarchySetupMode, select_hierarchy_setup_mode_event, SettingNetwork,              & i_msm::select_hierarchy_setup_mode >,
                a_row < SettingNetwork,              setup_network_event,               SettingApplicationData,      & i_msm::setup_network               >,
                a_row < SettingApplicationData,      setup_application_data_event,      SettingPhases,               & i_msm::setup_application_data      >,
                a_row < SettingPhases,               setup_phases_event,                SettingProcesses,            & i_msm::setup_phases                >,
                a_row < SettingProcesses,            setup_processes_event,             SettingThreads,              & i_msm::setup_processes             >,
                a_row < SettingThreads,              setup_threads_event,               SettingTimeouts,             & i_msm::setup_threads               >,
                a_row < SettingTimeouts,             setup_timeouts_event,              SettingAgents,               & i_msm::setup_timeouts              >,
                a_row < SettingAgents,               setup_agents_event,                SettingOutputfile,           & i_msm::setup_agents                >,
                a_row < SettingOutputfile,           setup_outputfile_event,            ConnectingToRegistry,        & i_msm::setup_outputfile            >,
                a_row < ConnectingToRegistry,        connect_to_registry_event,         SelectingStarter,            & i_msm::connect_to_registry         >,
                a_row < SelectingStarter,            select_starter_event,              InitializationComplete,      & i_msm::select_starter              >
                > {};
    };

    // Pick a back-end
    typedef msm::back::state_machine<Initializing_> Initializing;

    struct Running : public msm::front::state<>{
        PSC_SM_TRACE_SET_STATE_TRACING( "Running" );
    };

    struct Finalizing : public msm::front::state<>{
        PSC_SM_TRACE_SET_STATE_TRACING( "Finalizing" );
    };


    // the initial state of the frontend SM. Must be defined.
    typedef Initializing initial_state;


    void start_experiments( start_run const& evt );

    void do_finalization( finalize const& evt );

    // executed on invalid transitions
    template <class Fsm, class Event>
    void no_transition( Event const& e, Fsm&, int state ) {
        // TODO attach a name field to every state and event class
        printf( "---- FRONTEND MAIN STATEMACHINE ---- ; "
                "ERROR: Invalid event \"%s\" received in state: %s\n",
                e.toString(), PSC_SM_TRACE_SINGLETON->get_current_state() );
        PSC_SM_TRACE_SINGLETON->print();
    }

    typedef frontend_main_msm fm_msm; // makes transition table cleaner

    // Transition table for main_frontend_statemachine
    struct transition_table : mpl::vector<
            //      Start State              Command       Next State               Action                             Guard
            a_row < Initializing,            start_run,    Running,                 & fm_msm::start_experiments        >,
            a_row < Running,                 finalize,     Finalizing,              & fm_msm::do_finalization          >
            > {};
};
// Pick a back-end
typedef msm::back::state_machine<frontend_main_msm> main_frontend_statemachine;
}

#endif
