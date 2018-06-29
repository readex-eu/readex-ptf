/**
   @file    frontend.cc
   @ingroup Frontend
   @brief   Front-end agent
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

#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "config.h"
#include "psc_config.h"

#include "stringutil.h"
#include "timing.h"

#include "frontend.h"

#define PSC_AUTOTUNE_GLOBALS
#include "IPlugin.h"
#include "autotune_services.h"

#include "frontend_accl_handler.h"
#include "frontend_statemachine.h"

#include <ctime>
#include <cmath>
#include <map>
#include "MetaProperty.h"
#include "rts.h"
#include "PropertyID.h"
#include "application.h"
#include "global.h"
#include "selective_debug.h"
#include "TuningParameter.h"
#include "memory"

#include <boost/lexical_cast.hpp>

using namespace std;
using namespace frontend_statemachine;

extern IPlugin* plugin;
extern bool     search_done;
extern bool     calltree_sent;
bool            restart_requested_for_no_tuning;
int             agent_pid = -1;
int             application_pid;
char            user_specified_environment[ 5000 ] = "\0";



PeriscopeFrontend::PeriscopeFrontend( ACE_Reactor* r ) :
    PeriscopeAgent( r ),
    plugin_context( new DriverContext() ),
    fe_context( new DriverContext() ),
    frontend_pool_set( new ScenarioPoolSet() ) {
    automatic_mode      = true;
    need_restart        = false;
    shutdown_allowed    = true;
    interval            = 0;
    quit_fe             = false;
    badRegionsRemoved   = false;
    allRegionsCommented = false;
    ACE_Event_Handler::reactor( r );
    RequiredRegions.erase();
    requiredRegionsListLastRun.clear();

    timer_action            = STARTUP;
    *masteragent_cmd_string = 0;
    *app_name_string        = 0;
    *app_cmd_line_string    = 0;
    *master_host_string     = 0;
    outfilename_string      = " ";

    *psc_reg_host = 0;
    psc_reg_port  = 0;

    ompnumthreads_val       = 1;
    iter_count              = 1; //sss
    ompfinalthreads_val     = 1; //sss
    mpinumprocs_val         = 1;
    maxcluster_val          = 3;
    ranks_started           = 0;
    total_agents_number     = 0;
    started_agents_count    = 0;
    agent_hierarchy_started = false;
    tuning_plugin_executed  = false;
}

void PeriscopeFrontend::terminate_autotune() {
    /*
     * TODO do proper termination here
     *
     * 1. Call plugin terminate
     * 2. Destroy shared pools
     * 3. Destroy other frontend structures
     * 4. Kill agent hierarchy
     * - etc...
     */

    if( plugin ) {
        plugin->terminate();
        delete plugin;
    }
    fe_context->unloadPlugins();
}

// #define HYBRID__MPI_PROC_ON_PROCESSOR

int PeriscopeFrontend::register_self() {
    if( !regsrv_ ) {
        psc_errmsg( "PeriscopeFrontend::register_self(): registry not set\n" );
        return -1;
    }

    int pid, port;
    pid  = getpid();
    port = get_local_port();

    char hostname[ 200 ];
    gethostname( hostname, 200 );

    EntryData data;
    data.id   = -1;
    data.app  = fe->get_appname();
    data.site = sitename();
    data.mach = machinename();
    data.node = hostname;
    data.port = port;
    data.pid  = pid;
    data.comp = "PeriscopeFrontend";
    data.tag  = get_local_tag();

    regid_ = regsrv_->add_entry( data );

    char buf[ 200 ];
    sprintf( buf, "%s[%d]", data.tag.c_str(), regid_ );
    data.tag = buf;

    regsrv_->change_entry( data, regid_ );

    return regid_;
}

void PeriscopeFrontend::register_master_agent( char* ma_cmd_string ) {
    sprintf( masteragent_cmd_string, "%s", ma_cmd_string );
}

void PeriscopeFrontend::register_app_name( char* aname_string ) {
    sprintf( app_name_string, "%s", aname_string );
}

void PeriscopeFrontend::register_app_cmd_line( char* acmdline_string ) {
    sprintf( app_cmd_line_string, "%s", acmdline_string );
}

void PeriscopeFrontend::register_master_host( char* mhost_string ) {
    sprintf( master_host_string, "%s", mhost_string );
}

void PeriscopeFrontend::register_reg_host( char* rhost_string ) {
    sprintf( psc_reg_host, "%s", rhost_string );
}

void PeriscopeFrontend::register_reg_port( int reg_port ) {
    psc_reg_port = reg_port;
}

void PeriscopeFrontend::set_outfilename( const std::string& outfn_string ) {
    outfilename_string = outfn_string;
}

void PeriscopeFrontend::set_ompnumthreads( int ompnt_val ) {
    ompnumthreads_val = ompnt_val;
}

void PeriscopeFrontend::set_ompfinalthreads( int ompfinal_val ) {
    ompfinalthreads_val = ompfinal_val;
}

void PeriscopeFrontend::set_maxiterations( int iter_val ) {
    iter_count = iter_val;
}

void PeriscopeFrontend::set_mpinumprocs( int mpinp_val ) {
    mpinumprocs_val = mpinp_val;
}

void PeriscopeFrontend::set_maxcluster( int mcluster_val ) {
    maxcluster_val = mcluster_val;
}

ACCL_Handler* PeriscopeFrontend::create_protocol_handler( ACE_SOCK_Stream& peer ) {
#ifdef __p575
    sleep( 1 );
#endif
    return new ACCL_Frontend_Handler( this, peer );
}

void PeriscopeFrontend::run() {
    ranks_started        = 0;
    started_agents_count = 0;

    if( opts.has_scalability_OMP ) {
        sleep( 30 );
    }
    int          cores_per_processor = 4;
    char         senvbuf[ 512 ];
    ACE_Reactor* reactor = ACE_Event_Handler::reactor();

    if( !reactor ) {
        psc_errmsg( "Error: Reactor could not start.\n" );
        exit( 0 );
    }

    // We branch here to start the autotune state machine
    if( opts.has_strategy && !strcmp( opts.strategy, "tune" ) ) {
        run_tuning_plugin( reactor );
    }
    else {
        run_analysis( reactor );
    }
}

/**
 * @brief Drives tuning plugins execution
 *
 */
void PeriscopeFrontend::run_tuning_plugin( ACE_Reactor* reactor ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                "Autotuning application with %s processes...\n",
                opts.mpinumprocs_string );

    autotune_statemachine atmsm;
    atmsm.start();
    this->plugin_context->tuning_step      = 0;
    this->plugin_context->search_step      = 0;
    this->plugin_context->experiment_count = 0;
    bool scenarios_executed = true;

    try{
        // starting to pass events to the state machine
        atmsm.process_event( initialize_plugin( this, reactor, starter ) );
        do {
            atmsm.process_event( start_tuning_step() );
            do {
                StrategyRequest* strategy_request = NULL;
                if( plugin->analysisRequired( &strategy_request ) ) {
                    if( strategy_request == NULL ) {
                        psc_abort( "Pre-analysis requested by plugin, with an invalid strategy request\n" );
                    }

                    // TODO find out why the strategy name needs to be set regardless of what is passed to perform analysis
                    strcpy( opts.strategy, strategy_request->getGeneralInfo()->strategy_name.c_str() );
                    atmsm.process_event( perform_analysis( strategy_request, this, reactor, starter,
                                                           this->plugin_context->tuning_step ) );
                    strcpy( opts.strategy, "tune" );
                }
                atmsm.process_event( create_scenarios( this ) );
                while( !frontend_pool_set->csp->empty() ) {
                    atmsm.process_event( prepare_scenarios() );
                    do {
                        atmsm.process_event( define_experiment() );
                        atmsm.process_event( consider_restart( this, reactor, starter,
                                                               this->plugin_context->search_step ) );
                        do {
                            atmsm.process_event( run_phase_experiments( this, reactor, starter,
                                                                        this->plugin_context->search_step,
                                                                        &scenarios_executed,
                                                                        this->plugin_context->experiment_count ) );

                            if( scenarios_executed ) {
                                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                                            "Scenarios executed\n" );
                            }
                            else {
                                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                                            "Scenarios NOT executed\n" );
                            }
                        }
                        while( !scenarios_executed );

                        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                                    "Prepared scenario pool empty = %d\n",
                                    frontend_pool_set->psp->empty() );
                        if( !frontend_pool_set->psp->empty() ) {
                            psc_infomsg( "Prepared scenario pool not empty, still searching...\n" );
                        }
                        this->plugin_context->experiment_count++;
                    }
                    while( !frontend_pool_set->psp->empty() );

                    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                                "Scenario pool empty = %d\n",
                                frontend_pool_set->csp->empty() );
                    if( !frontend_pool_set->csp->empty() ) {
                        psc_infomsg( "Scenario pool not empty, still searching...\n" );
                    }
                }
                this->plugin_context->search_step++;
            }
            while( !plugin->searchFinished() );
            atmsm.process_event( finish_tuning_step() );
            this->plugin_context->tuning_step++;
        }
        while( !plugin->tuningFinished() );
        atmsm.process_event( create_tuning_advice() );
        atmsm.process_event( finalize_autotuning( this ) );
        tuning_plugin_executed = true;
    }
    catch( std::exception& ex ) {
        psc_errmsg( "Exception caught from the Autotune State Machine: %s\n", ex.what() );
        terminate_autotune();
        abort();
    }
}

void PeriscopeFrontend::run_analysis( ACE_Reactor* reactor ) {
    set_shutdown_allowed( false );
    psc_infomsg( "Starting application %s using %s MPI procs and %s OpenMP threads...\n",
                 opts.app_run_string,
                 ( strlen( opts.mpinumprocs_string ) ? opts.mpinumprocs_string : "0" ),
                 ( strlen( opts.ompnumthreads_string ) ? opts.ompnumthreads_string : "0" ) );

    // TODO invert the order here, when in fast mode
    if( get_fastmode() ) {
        psc_infomsg( "Starting agents network...\n" );
        starter->runAgents();
        // TODO need to wait for the agents here first; this prevents connection retries that exhaust file descriptors
        starter->runApplication();
        psc_dbgmsg( 1, "Application started after %5.1f seconds\n", psc_wall_time() );
    }
    else {
        starter->runApplication();
        psc_dbgmsg( 1, "Application started after %5.1f seconds\n", psc_wall_time() );

        psc_infomsg( "Starting agents network...\n" );
        starter->runAgents();
    }

    init_analysis_strategy_requests();
    print_StrategyRequestGeneralInfoQueue();

    if( opts.has_scalability_OMP ) {
        sleep( 30 );               //sss
    }
    reactor->reset_event_loop();
    reactor->register_handler( 0, this, ACE_Event_Handler::READ_MASK );
    reactor->register_handler( SIGINT, this );
    //psc_dbgmsg(1, "STARTUP: run reactor\n");

#ifndef _BGP_PORT_HEARTBEAT_V1
    set_timer( 2, 1, timeout_delta(), PeriscopeFrontend::STARTUP );
#endif
    reactor->run_event_loop();
    int max_runs = 40;

    for( int runs = 1; need_restart && runs < max_runs && !quit_fe && restart_requested_for_no_tuning; runs++ ) {
        fe->increment_global_timeout();
        psc_dbgmsg( FRONTEND_GENERAL_DEBUG_LEVEL, "Restarting application\n" );

        set_shutdown_allowed( runs == max_runs - 1 );               //Needrestart msg will be ignored
        std::map<std::string, AgentInfo>::iterator it;


        for( it = fe->get_child_agents()->begin(); it != fe->get_child_agents()->end(); it++ ) {
            AgentInfo& ag = it->second;
            if( ag.status != AgentInfo::CONNECTED && fe->connect_to_child( &ag ) == -1 ) {
                psc_errmsg( "Error FE not connected to child\n" );
                exit( 1 );
            }
            else {
                ag.appl_terminated = false;
                ag.handler->terminate();
            }
        }
        psc_dbgmsg( FRONTEND_GENERAL_DEBUG_LEVEL, "RESTART_ACTION: waiting for application to terminate\n" );
        reactor->reset_event_loop();
        //evt.reactor->register_handler(0, evt.fe, ACE_Event_Handler::READ_MASK);
        //evt.reactor->register_handler(SIGINT, evt.fe);
        set_timer( 2, 1, timeout_delta(), PeriscopeFrontend::APPLICATION_TERMINATION );
        reactor->run_event_loop();
        psc_dbgmsg( FRONTEND_GENERAL_DEBUG_LEVEL, "RESTART_ACTION: application terminated.\n" );

        starter->rerunApplication();

        ACE_Reactor* reactor = ACE_Event_Handler::reactor();
        if( !reactor ) {
            psc_errmsg( "Error: Reactor could not start.\n" );
            exit( 0 );
        }

        reactor->reset_event_loop();
        //psc_dbgmsg(1, "register reactor\n");
        reactor->register_handler( 0, this, ACE_Event_Handler::READ_MASK );
        reactor->register_handler( SIGINT, this );
        //psc_dbgmsg(1, "REINIT: run reactor\n");
        set_timer( 2, 1, timeout_delta(), PeriscopeFrontend::STARTUP_REINIT );

        reactor->run_event_loop();
        //psc_dbgmsg(1, "finished reactor\n");
    }
}


void PeriscopeFrontend::stop() {
    ACE_Reactor* reactor = ACE_Event_Handler::reactor();

    if( !get_fastmode() ) {
        if( reactor ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ), "Stopping the ACE Reactor (NOT fast mode)\n" );
            reactor->end_event_loop();
        }
    }
#ifdef __p575
    if( !get_fastmode() ) {
        regsrv_->delete_entry( regid_ );
    }
#endif
}

int PeriscopeFrontend::handle_input( ACE_HANDLE hdle ) {
    int       cnt;
    const int bufsize = 200;
    char      buf[ bufsize ];

    buf[ 0 ] = '\0';
    buf[ 1 ] = '\0';
    if( !read_line( hdle, buf, bufsize ) ) {
        return -1;
    }

    handle_command( buf );

    return 0;
}

int PeriscopeFrontend::handle_signal( int signum,
                                      siginfo_t*,
                                      ucontext_t* ) {
    ACE_Reactor* reactor = ACE_Event_Handler::reactor();

    if( reactor ) {
        reactor->end_event_loop();
    }
    quit();

    return 0;
}

void PeriscopeFrontend::handle_command( const std::string& line ) {
    string  cmd;
    ssize_t pos;

    pos = strskip_ws( line, 0 );
    pos = get_token( line, pos, "\t \n", cmd );

    if( cmd.compare( "graph" ) == 0 ) {
        graph();
        prompt();
        return;
    }

    if( cmd.compare( "start" ) == 0 ) {
        start();
        return;
    }

    if( cmd.compare( "check" ) == 0 ) {
        check();
        prompt();
        return;
    }

    if( cmd.compare( "help" ) == 0 ) {
        help();
        return;
    }

    if( cmd.compare( "quit" ) == 0 ) {
        quit();
        return;
    }

    if( cmd.compare( "properties" ) == 0 ) {
        properties();
        return;
    }

    fprintf( stdout, "Unknown command: '%s'\n", cmd.c_str() );
    prompt();
}
// adds a started agent without tracking the processes it handles; there is no need to track the controlled processes here in fast mode, since the
// processes are specified before the launch explicitly
void PeriscopeFrontend::add_started_agent() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                "Entering add_started_agent(); started: %d; total %d; ready: %s;\n",
                started_agents_count, total_agents_number,
                agent_hierarchy_started ? "true" : "false" );

    started_agents_count++;
    if( !agent_hierarchy_started ) {
        if( started_agents_count == total_agents_number ) {
            agent_hierarchy_started = true;
        }
    }
}

// increases the number of started agents and if it equals to the total amount issues start command
// this version is used in the BGP style launchers
void PeriscopeFrontend::add_started_agent( int num_procs ) {
    started_agents_count++;
    ranks_started += num_procs;
    if( num_procs != 0 ) {
        psc_dbgmsg( 0, "Heartbeat received: (%d mpi processes out of %d ready for analysis)\n", ranks_started, get_mpinumprocs() );
    }

    if( ranks_started == get_mpinumprocs() ) {
        psc_dbgmsg( 1, "Agent network UP and RUNNING. Starting search.\n\n" );
        psc_dbgmsg( 1, "Agent network started in %5.1f seconds\n", psc_wall_time() );

        if( !agent_hierarchy_started ) {
            agent_hierarchy_started = true;
            fe->set_startup_time( psc_wall_time() );
        }

        if( automatic_mode ) {
            start();
        }
        else {
            prompt();
        }
    }
}

void PeriscopeFrontend::remove_started_agent() { // increases the number of started agents and if it equals to the total amount issues start command
    started_agents_count--;
    total_agents_number--;
    psc_dbgmsg( FRONTEND_HIGH_DEBUG_LEVEL, "One agent was dismissed\n" );
}

int PeriscopeFrontend::handle_timeout( const ACE_Time_Value& time,
                                       const void*           arg ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ),
                "PeriscopeFrontend::handle_timeout() called\n" );
    return handle_step( timer_action );
}

int PeriscopeFrontend::handle_step( TimerAction timer_action ) {
    std::map<std::string, AgentInfo>::iterator it;
    bool                                       done;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ),
                "PeriscopeFrontend::handle_step()\n" );

    switch( timer_action ) {
    case STARTUP:
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ),
                    "PeriscopeFrontend::handle_step() STARTUP\n" );
        done = true;
        for( it = child_agents_.begin(); it != child_agents_.end(); it++ ) {
            if( it->second.status != AgentInfo::STARTED ) {
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                            "Child agent %s at (%s:%d) not started, still waiting...\n",
                            it->first.c_str(), it->second.hostname.c_str(), it->second.port );
                done = false;
            }
        }

        if( done ) {
            if( !agent_hierarchy_started ) {
                agent_hierarchy_started = true;
                psc_dbgmsg( 1, "Agent network UP and RUNNING. Starting search.\n\n" );
                psc_dbgmsg( 1, "Agent network started in %5.1f seconds\n", psc_wall_time() );
                fe->set_startup_time( psc_wall_time() );
            }

            ACE_Reactor* reactor = ACE_Event_Handler::reactor();
            reactor->cancel_timer( this );

            if( automatic_mode ) {
                // TODO in fast mode, the call to start() needs to occur after the application is launched with the starter
                if( get_fastmode() ) {
                    search_done = true;                         // exit the comm phase in fast mode
                }
                else {
                    start();
                }
            }
            else {
                prompt();
            }
        }
        else {
            if( this->timed_out() ) {
                psc_errmsg( "Timed out waiting for child agent(s)\n" );
                quit();
            }
        }
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ),
                    "PeriscopeFrontend::handle_step() STARTUP done!\n" );
        break;

    case STARTUP_REINIT:
        // TODO check this are for the restart bug
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ),
                    "PeriscopeFrontend::handle_step() STARTUP_REINIT\n" );
        done = true;
        for( it = child_agents_.begin(); it != child_agents_.end(); it++ ) {
            if( it->second.status_reinit != AgentInfo::STARTED ) {
                done = false;
            }
        }

        if( done ) {
            psc_dbgmsg( 0, "Analysis agents connected to new processes and ready for search.\n" );

            ACE_Reactor* reactor = ACE_Event_Handler::reactor();
            reactor->cancel_timer( this );

            for( it = child_agents_.begin(); it != child_agents_.end(); it++ ) {
                AgentInfo& ai = it->second;

                if( fe->connect_to_child( &ai ) == -1 ) {
                    psc_errmsg( "Error connecting to child at %s:%d\n", ai.hostname.c_str(), ai.port );
                }
                else {
                    ai.search_status = AgentInfo::UNDEFINED;
                    ai.handler->startexperiment();
                    ai.properties_sent = false;
                }
            }
        }
        else {                  // Agent network still not running
                                // TODO verify why this is empty -IC
        }
        break;
    case AGENT_STARTUP:
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ),
                    "PeriscopeFrontend::handle_step() AGENT_STARTUP\n" );
        done = true;
        for( it = child_agents_.begin(); it != child_agents_.end(); it++ ) {
            if( it->second.status != AgentInfo::STARTED ) {
                done = false;
            }
        }
        if( done ) {
            //psc_dbgmsg(1,"....Done.....\n\n");
            if( !agent_hierarchy_started ) {
                agent_hierarchy_started = true;
                psc_dbgmsg( 1, "Agent network UP and RUNNING. Starting search.\n\n" );
                psc_dbgmsg( 1, "Agent network started in %5.1f seconds\n", psc_wall_time() );
                fe->set_startup_time( psc_wall_time() );
            }

            ACE_Reactor* reactor = ACE_Event_Handler::reactor();
            reactor->cancel_timer( this );
            stop();
        }
        else {
            if( this->timed_out() ) {
                psc_errmsg( "Timed out waiting for child agent(s)\n" );
                quit();
            }
        }
        break;

    case APPLICATION_TERMINATION:
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ),
                    "PeriscopeFrontend::handle_step() APPLICATION_TERMINATION\n" );
        done = true;
        for( it = child_agents_.begin(); it != child_agents_.end(); it++ ) {
            if( !it->second.appl_terminated ) {
                done = false;
            }
        }
        if( done ) {
            RegistryService*       regsrv = fe->get_registry();
            EntryData              query;
            std::list< EntryData > rresult;
            query.app  = fe->get_appname();
            query.comp = "MRIMONITOR";
            query.tag  = "none";

            if( !fe->get_fastmode() ) {
                rresult.clear();
                if( regsrv->query_entries( rresult, query, false ) == -1 ) {
                    psc_errmsg( "Error querying registry for application\n" );
                    exit( 1 );
                }

                if( rresult.size() > 0 ) {
                    psc_dbgmsg( FRONTEND_GENERAL_DEBUG_LEVEL,
                                "%d processes of application %s still registered\n",
                                rresult.size(), fe->get_appname() );
                }
                else {
                    //psc_dbgmsg(1,"....All application processes terminated.....\n\n");
                    ACE_Reactor* reactor = ACE_Event_Handler::reactor();
                    reactor->cancel_timer( this );
                    stop();
                }
            }
//          force elimination from registry
//            if( !startup_finished ) {
//                std::list<EntryData>::iterator entryit;
//
//                for( entryit = rresult.begin(); entryit != rresult.end(); entryit++) {
//                    regsrv->delete_entry( ( *entryit ).id );
//                }
        }
        else {
            if( this->timed_out() ) {
                psc_errmsg( "Timed out waiting for child agent(s)\n" );
                quit();
            }
        }
        break;
    case CHECK:
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ),
                    "PeriscopeFrontend::handle_step() CHECK\n" );
        this->check();
        break;

    case REQUESTCALLTREE:
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "PeriscopeFrontend::handle_step() REQUESTCALLTREE\n" );

        done = true;
        for( it = child_agents_.begin(); it != child_agents_.end(); it++ ) {
            if( it->second.status != AgentInfo::STARTED ) {
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ), "Child agent %s at (%s:%d) not started, still waiting...\n", it->first.c_str(), it->second.hostname.c_str(), it->second.port );
                done = false;
            }
        }

        if( done ) {
            ACE_Reactor* reactor = ACE_Event_Handler::reactor();
            reactor->cancel_timer( this );

            for( it = child_agents_.begin(); it != child_agents_.end(); it++ ) {
                AgentInfo& ai = it->second;
                ai.calltree_sent = false;
                this->request_calltree();
            }
        }
        else {
            if( this->timed_out() ) {
                psc_errmsg( "Timed out waiting for child agent(s)\n" );
                quit();
            }
        }

        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "PeriscopeFrontend::handle_step() REQUESTCALLTREE done!\n" );
        break;

    case SHUTDOWN:
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ),
                    "PeriscopeFrontend::handle_step() SHUTDOWN\n" );
        psc_dbgmsg( 1, "PERISCOPE shutdown requested [%d], restart needed [%d]\n",
                    shutdown_allowed, need_restart );
        ACE_Reactor* reactor = ACE_Event_Handler::reactor();
        reactor->cancel_timer( this );
        if( shutdown_allowed || !need_restart ) {
            // shutdown the agent hierarchy
            quit();
        }
        else {
            // only exit the reactor event loop
            stop();
        }
        break;
    }
    return 0;
}

void PeriscopeFrontend::graph() {
    char             command_string[ 200 ];
    RegistryService* regsrv = fe->get_registry();

    EntryData                      query;
    std::list<EntryData>           result;
    std::list<EntryData>::iterator entryit;

    query.app = appname();

    if( regsrv->query_entries( result, query ) == -1 ) {
        psc_errmsg( "Error querying registry for application\n" );
        exit( 1 );
    }

    char  buf[ 200 ];
    char* tmp, * last, * t, * parent;

    FILE* graph;

    graph = fopen( "/tmp/test.dot", "w" );

    fprintf( graph, "digraph test {\n" );

    std::map<std::string, std::list<std::string> > children;
    std::map<std::string, std::string>             label;

    for( entryit = result.begin(); entryit != result.end(); entryit++ ) {
        sprintf( buf, "fe[%d]", regid_ );

        if( ( entryit->tag ).find( buf ) == std::string::npos ) {
            continue;
        }

        if( entryit->comp == "Periscope Frontend" ) {
            sprintf( buf,
                     "Frontend\\n%s:%d", entryit->node.c_str(), entryit->port );
        }

        if( entryit->comp == "Periscope HL Agent" ) {
            sprintf( buf,
                     "HL-Agent\\n%s:%d", entryit->node.c_str(), entryit->port );
        }

        if( entryit->comp == "Periscope Node-Agent" ) {
            sprintf( buf,
                     "Node-Agent\\n%s:%d", entryit->node.c_str(), entryit->port );
        }

        if( entryit->comp == "Monlib" ) {
            sprintf( buf, "Monlib\\n%s %s",
                     entryit->app.c_str(), entryit->node.c_str() );
        }

        label[ entryit->tag ] = buf;

        strcpy( buf, entryit->tag.c_str() );

        tmp  = strtok( buf, ":" );
        last = tmp;

        while( tmp != 0 ) {
            last = tmp;
            tmp  = strtok( 0, ":" );
        }

        t = buf;

        if( buf != last ) {
            while( t != ( last - 1 ) ) {
                if( *t == '\0' ) {
                    *t = ':';
                }
                t++;
            }
            parent = buf;
        }
        else {
            parent = 0;
        }

        if( parent ) {
            children[ parent ].push_back( entryit->tag );
        }
    }

    std::map<std::string, std::list<std::string> >::iterator it1;
    std::list<std::string>::iterator                         it2;
    std::map<std::string, std::string>::iterator             it3;

    for( it1 = children.begin(); it1 != children.end(); it1++ ) {
        for( it2 = it1->second.begin(); it2 != it1->second.end(); it2++ ) {
            fprintf( graph, " \"%s\" -> \"%s\" ;\n", ( *it1 ).first.c_str(),
                     ( *it2 ).c_str() );
        }
    }

    for( it3 = label.begin(); it3 != label.end(); it3++ ) {
        if( ( *it3 ).second.find( "Frontend" ) != string::npos ) {
            fprintf( graph,
                     " \"%s\" [shape=\"ellipse\" style=\"filled\" fillcolor=yellow label =\"%s\"]\n",
                     ( *it3 ).first.c_str(), ( *it3 ).second.c_str() );
        }
        if( ( *it3 ).second.find( "Node-Agent" ) != string::npos ) {
            fprintf( graph,
                     " \"%s\" [shape=\"rect\" style=\"filled\" fillcolor=grey label =\"%s\"]\n",
                     ( *it3 ).first.c_str(), ( *it3 ).second.c_str() );
        }
        if( ( *it3 ).second.find( "HL-Agent" ) != string::npos ) {
            fprintf( graph, " \"%s\" [shape=\"octagon\" label =\"%s\"]\n",
                     ( *it3 ).first.c_str(), ( *it3 ).second.c_str() );
        }
        if( ( *it3 ).second.find( "Monlib" ) != string::npos ) {
            fprintf( graph, " \"%s\" [shape=\"rect\" label =\"%s\"]\n",
                     ( *it3 ).first.c_str(), ( *it3 ).second.c_str() );
        }
    }

    fprintf( graph, "}\n" );
    fclose( graph );

    sprintf( command_string, "%s",
             "dot /tmp/test.dot -Tpng > /tmp/test.png 2> /dev/null ;"
             "qiv /tmp/test.png &" );
    psc_dbgmsg( FRONTEND_MEDIUM_DEBUG_LEVEL, "Going to execute: '%s'\n", command_string );
    int retVal = system( command_string );
}

void PeriscopeFrontend::prompt() {
    // TODO verify what is the purpose of this operation and why it is empty -IC
}

void PeriscopeFrontend::help() {
    fprintf( stdout, " Periscope commands:\n" );
    fprintf( stdout, "  help       -- show this help\n" );
    fprintf( stdout, "  start      -- start target application\n" );
    fprintf( stdout, "  quit       -- quit Periscope\n" );
    fprintf( stdout, "  graph      -- show the agent network graph\n" );
    fprintf( stdout, "  check      -- check properties\n" );
    fflush( stdout );
    prompt();
}

void PeriscopeFrontend::push_StrategyRequestGeneralInfo2Queue( const std::string& strategy_name,
                                                               bool               pedantic,
                                                               int                analysis_duration,
                                                               int                delay_phases,
                                                               int                delay_seconds ) {
    StrategyRequestGeneralInfo* strategyRequestGeneralInfo = new StrategyRequestGeneralInfo;

    strategyRequestGeneralInfo->strategy_name     = strategy_name;
    strategyRequestGeneralInfo->pedantic          = pedantic;
    strategyRequestGeneralInfo->delay_phases      = delay_phases;
    strategyRequestGeneralInfo->delay_seconds     = delay_seconds;
    strategyRequestGeneralInfo->analysis_duration = analysis_duration;

    push_StrategyRequestGeneralInfo2Queue( strategyRequestGeneralInfo );
}

void PeriscopeFrontend::print_StrategyRequestQueue() {
    psc_dbgmsg( 1, "Requested strategy request:\n\n" );
    for( std::list<StrategyRequest*>::iterator sr_it = strategy_request_queue.begin();
         sr_it != strategy_request_queue.end(); sr_it++ ) {
        StrategyRequestGeneralInfo* srgi;
        srgi = ( *sr_it )->getGeneralInfo();
        psc_dbgmsg( 1, "Strategy name: %s, analysis duration %d, analysis delay %d/%d\n (phases/seconds)",
                    srgi->strategy_name.c_str(), srgi->analysis_duration,
                    srgi->delay_phases, srgi->delay_seconds );
        if( ( *sr_it )->getTypeOfConfiguration() == strategy_configuration_type( TUNE ) ) {
            std::list<Scenario*>* sc_list;
            sc_list = ( *sr_it )->getConfiguration().configuration_union.TuneScenario_list;
            for( std::list<Scenario*>::iterator sc_it = sc_list->begin(); sc_it != sc_list->end(); sc_it++ ) {
                ( *sc_it )->print();
            }
        }
        else if( ( *sr_it )->getTypeOfConfiguration() == strategy_configuration_type( PERSYST ) ) {
            std::list<int>* propID_list;
            propID_list = ( *sr_it )->getConfiguration().configuration_union.PersystPropertyID_list;
            psc_dbgmsg( 1, "Initial candidate properties:\n" );
            for( std::list<int>::iterator propID_it = propID_list->begin(); propID_it != propID_list->end(); propID_it++ ) {
                psc_dbgmsg( 1, "\t - %d", ( *propID_it ) );
            }
        }
    }
}

void PeriscopeFrontend::init_analysis_strategy_requests() {
    StrategyRequestGeneralInfo* strategyRequestGeneralInfo = new StrategyRequestGeneralInfo;
    std::string                 sname                      = opts.strategy;

    if( sname.compare( "tune" ) ) {
        std::list<int> empty_list;

        int delay_phases = 0;
        if( opts.has_delay ) {
            delay_phases = atoi( opts.delay_string );
        }
        int duration = 1;
        if( opts.has_duration ) {
            duration = atoi( opts.duration_string );
        }

        strategyRequestGeneralInfo->strategy_name     = opts.strategy;
        strategyRequestGeneralInfo->pedantic          = opts.has_pedantic;
        strategyRequestGeneralInfo->delay_phases      = delay_phases;
        strategyRequestGeneralInfo->delay_seconds     = 0;
        strategyRequestGeneralInfo->analysis_duration = duration;

        StrategyRequest* strategy_request = new StrategyRequest( &empty_list, strategyRequestGeneralInfo );
        serializeStrategyRequests( strategy_request );
    }
}

void PeriscopeFrontend::start() {
    std::vector<char>                          serializedStrategyRequest;
    std::map<std::string, AgentInfo>::iterator it;

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "PeriscopeFrontend::start() called\n" );

    serializedStrategyRequest = get_SerializedStrategyRequestBuffer();

//RM: Should be cleaned up here and moved into run handle for both analysis and tuning.
    if( serializedStrategyRequest.size() == 0 ) {
        psc_dbgmsg( 1, "Search finished.\n" );
        fe->stop();
        return;
    }

    psc_dbgmsg( PSC_AUTOTUNE_ALL_DEBUG, "Start with next strategy request: time: %5.1f seconds\n", psc_wall_time() );

    psc_dbgmsg( FRONTEND_HIGH_DEBUG_LEVEL, "Sending START with strategy request buffer size %d...\n",
                serializedStrategyRequest.size() );

    for( it = child_agents_.begin(); it != child_agents_.end(); it++ ) {
        AgentInfo& ag = it->second;

        if( ag.status != AgentInfo::CONNECTED ) {
            if( connect_to_child( &ag ) == -1 ) {
                psc_errmsg( "Error connecting to child\n" );
                abort();
            }
            else {
                ag.handler->start( serializedStrategyRequest.size(), ( unsigned char* )&serializedStrategyRequest[ 0 ] );
            }

            ag.properties_sent = false;
        }
    }

    strategyRequestBuffer.clear();
}

void PeriscopeFrontend::stop_agents_for_calltree() {
    psc_dbgmsg( 1, "Call-tree sent by one/more agents. Stopping agents.\n" );
    fe->stop();
    return;
}

/**
 * @brief Terminates the frontend and all agents.
 *
 * This method is executed just before the frontend terminates.
 * It stops all child agents, stops the communication routines,
 * and exports the found properties to a file.
 *
 */
void PeriscopeFrontend::quit() {
    std::map<std::string, AgentInfo>::iterator it;
    psc_dbgmsg( 5, "Frontend on quit!\n" );
    quit_fe = true;

    //Do scalability analysis only at the final run and export only the scalability-based properties
    if( opts.has_scalability_OMP && fe->get_ompnumthreads() == fe->get_ompfinalthreads() ) {
        do_pre_analysis();
        export_scalability_properties();
    }
    else if( opts.has_scalability_OMP ) {
        ;
    }
    else {
        export_properties();
        if( psc_get_debug_level() >= 1 ) {
            properties();
        }
    }

    for( it = child_agents_.begin(); it != child_agents_.end(); it++ ) {
        AgentInfo& ag = it->second;

        if( connect_to_child( &ag ) == -1 ) {
            psc_errmsg( "Error connecting to child\n" );
            abort();
        }
        else {
            psc_dbgmsg( 5, "Sending QUIT command...\n" );
            ag.handler->quit();
        }
    }

    if( !get_fastmode() ) {
        RegistryService*                 regsrv = fe->get_registry();
        EntryData                        query;
        std::list< EntryData >           rresult;
        std::list< EntryData >::iterator entry;

        bool   shutdown_finished = false;
        double start_time, current_time;
        PSC_WTIME( start_time );
        while( !shutdown_finished && ( current_time - start_time ) < 60.0 ) {
            sleep( 1 );

            query.app  = fe->get_appname();
            query.comp = "MRIMONITOR";
            query.tag  = "none";
            rresult.clear();
            if( regsrv->query_entries( rresult, query, false ) == -1 ) {
                psc_errmsg( "Error querying registry for application\n" );
                exit( 1 );
            }
            if( rresult.size() > 0 ) {
                psc_dbgmsg( 1, "%d processes of %s still registered\n", rresult.size(), query.app.c_str() );
            }
            shutdown_finished = ( rresult.size() == 0 );

            PSC_WTIME( current_time );
        }


        if( !shutdown_finished ) {
            psc_errmsg( "Not all application processes terminated on time! Cleaning registry.\n" );
            for( entry = rresult.begin(); entry != rresult.end(); entry++ ) {
                regsrv->delete_entry( ( *entry ).id );
            }
        }
        else {
            psc_dbgmsg( 1, "All application processes terminated.\n" );
        }
    }

    psc_dbgmsg( FRONTEND_HIGH_DEBUG_LEVEL, "Sending STOP command...\n" );
    stop();
}


void PeriscopeFrontend::terminate_agent_hierarchy() {
    std::map<std::string, AgentInfo>::iterator it;

    for( it = child_agents_.begin(); it != child_agents_.end(); it++ ) {
        AgentInfo& ag = it->second;

        if( connect_to_child( &ag ) == -1 ) {
            psc_errmsg( "Error connecting to child\n" );
            abort();
        }
        else {
            psc_dbgmsg( 5, "Sending QUIT command...\n" );
            ag.handler->quit();
        }
    }

    double start_time, current_time;
    int    status;
    PSC_WTIME( start_time );
    psc_dbgmsg( 2, "size of child agents: %d\n", child_agents_.size() );
    child_agents_.erase( child_agents_.begin(), child_agents_.end() );

    if( get_fastmode() ) { // check for starters that work in fast mode
                           // TODO need to terminate the application and agents in fast mode -IC
        // need a way to verify that the agents and MPI application terminated, before continuing
        // instead of waiting, we can simply use new ports and execute asynchronously

        if( agent_pid > 0 ) { // in interactive runs, this variable holds the PID of the current forked AA -IC
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ), "This is an interactive run.\n" );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ), "Waiting for id of the Agent: %d\n\n", agent_pid );
            waitpid( agent_pid, &status, 0 );
        }
        else {           // agents in a distributed memory run
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ), "Running on a distributed system.\n" );
            psc_errmsg( "Waiting for agents on distributed systems not yet implemented.  "
                        "Throwing an exception at PeriscopeFrontend::terminate_agent_hierarchy()" );
            throw 0;
            // TODO need to verify the termination of agents in distributed systems
        }
    }
    else {
        RegistryService*                 regsrv = fe->get_registry();
        EntryData                        query;
        std::list< EntryData >           rresult;
        std::list< EntryData >::iterator entry;
        bool                             shutdown_finished = false;

        while( !shutdown_finished && ( current_time - start_time ) < 60.0 ) {
            query.app  = fe->get_appname();
            query.comp = "MRIMONITOR";
            query.tag  = "none";

            rresult.clear();
            if( regsrv->query_entries( rresult, query, false ) == -1 ) {
                psc_errmsg( "Error querying registry for application\n" );
                exit( 1 );
            }

            if( rresult.size() > 0 ) {
                psc_dbgmsg( 1, "%d processes of %s still registered\n", rresult.size(), query.app.c_str() );
            }
            shutdown_finished = ( rresult.size() == 0 );

            PSC_WTIME( current_time );
        }

        if( !shutdown_finished ) {
            psc_errmsg( "Not all application processes terminated on time! Cleaning registry.\n" );
            for( entry = rresult.begin(); entry != rresult.end(); entry++ ) {
                regsrv->delete_entry( ( *entry ).id );
            }
            //exit(1);
        }
        else {
            psc_dbgmsg( 1, "All application processes terminated.\n" );
        }
    }

    // the frontend will hold the PID of the MPI application (more accurately, the mpiexec/launcher's PID)
    // the reason is that we fork a child that becomes an mpiexec process; this process terminates with the parallel
    // job, regardless of whether it is running locally or on a distributed system -IC
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ), "Waiting for id of the MPI application: %d\n\n", application_pid );
    waitpid( application_pid, &status, 0 );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ), "Termination of the application and agents done (fast mode).\n" );

    psc_dbgmsg( FRONTEND_HIGH_DEBUG_LEVEL, "Sending STOP command...\n" );
    psc_dbgmsg( 2, "stopping the agents...\n" );
    stop();
    fe->set_agent_hierarchy_started( false );
}

/**
 * @brief Pre-analysis for scalability tests in OpenMP codes
 *
 * Does speedup analysis, scalability tests, and inserts the appropriate properties
 *
 */
void PeriscopeFrontend::do_pre_analysis() {
    //initializations
    properties_.clear();
    property.clear();
    property_threads.clear();
    p_threads.clear();
    PhaseTime_user.clear();
    std::list<PropertyInfo>::iterator prop_it;

    //Get the properties information
    //for( int i = 0; std::list<MetaProperty>::iterator it = metaproperties_.begin(); it != metaproperties_.end(); it++ ){
    for( int i = 0; i < metaproperties_.size(); i++ ) {
        get_prop_info( metaproperties_[ i ] );
    }

    //Copy list of properties starting from thread no. 2
    copy_properties_for_analysis();
    //This function finds the existing properties for all runs
    find_existing_properties();

    //Determine if the user_region scales or not / Meantime find the deviation time, execution time for user region
    bool user_flag = false;
    for( prop_it = properties_.begin(); prop_it != properties_.end(); ++prop_it ) {
        long double seq_user_time = 0.0;
        if( ( *prop_it ).maxThreads == 1 ) {
            if( ( *prop_it ).region == "USER_REGION" ) {
                string phase_user_Region = "USER_REGION";
                seq_user_time = find_sequential_time( *prop_it );
                do_speedup_analysis( *prop_it, seq_user_time, phase_user_Region );
                user_flag = true;
            }
            else {
                ;
            }
        }
        else if( ( *prop_it ).maxThreads > 1 ) {
            break;
        }
        else {
            ;
        }
    }

    //If there is no user_region, then depend on main region, so..
    if( !user_flag ) {
        for( prop_it = properties_.begin(); prop_it != properties_.end(); ++prop_it ) {
            long double seq_main_time = 0.0;
            if( ( *prop_it ).maxThreads == 1 ) {
                if( ( *prop_it ).region == "MAIN_REGION" ) {
                    string phase_main_Region = "MAIN_REGION";
                    seq_main_time = find_sequential_time( *prop_it );
                    do_speedup_analysis( *prop_it, seq_main_time, phase_main_Region );
                }
                else {
                    ;
                }
            }
            else if( ( *prop_it ).maxThreads > 1 ) {
                break;
            }
            else {
                ;
            }
        }
    }

    //Find speedup for OpenMP regions
    std::list<long double> RFL_history;
    RFL_history.clear();
    for( prop_it = properties_.begin(); prop_it != properties_.end(); ++prop_it ) {
        long double seq_time  = 0.0;
        bool        RFL_check = false;
        RFL_history.push_back( ( *prop_it ).RFL );
        if( ( *prop_it ).maxThreads == 1 ) {
            if( ( int )count( RFL_history.begin(), RFL_history.end(), ( *prop_it ).RFL ) > 1 ) {
                RFL_check = true;
            }

            if( ( ( *prop_it ).region == "PARALLEL_REGION" || ( *prop_it ).region == "WORKSHARE_DO" ||
                  ( *prop_it ).region == "DO_REGION" || ( *prop_it ).region == "CALL_REGION"
                  || ( *prop_it ).region == "SUB_REGION" ) && !RFL_check ) {
                string phaseRegion = ( *prop_it ).region;
                seq_time = find_sequential_time( *prop_it );
                do_speedup_analysis( *prop_it, seq_time, phaseRegion );
            }
            else {
                ;
            }
        }
        else if( ( *prop_it ).maxThreads > 1 ) {
            break;
        }
        else {
            ;
        }
    }

    //This function calculates the total deviation among the regions;
    //it finds the maximum among them; it shows the region with its severity value.
    for( int i = 1; i <= fe->get_ompfinalthreads(); i += i ) {
        Check_severity_among_parallel_regions( i );
    }
}

/**
 * @brief Export scalability based found properties to a file
 *
 * Exports scalability based found properties to a file in XML format.
 *
 */
void PeriscopeFrontend::export_scalability_properties() {
    //initialization
    std::list<PropertyInfo>::iterator prop_it, prop_itr, s_it, e_it, sc_it, sp_it, d_it, p_it;
    double                            threshold_for_foundproperties = 0.0; //threshold above which the properties are displayed

    std::string   propfilename = fe->get_outfilename();
    std::ofstream xmlfile( propfilename.c_str() );
    if( ( !xmlfile ) && opts.has_propfile ) {
        std::cout << "Cannot open xmlfile. \n";
        exit( 1 );
    }


    if( xmlfile && fe->get_ompnumthreads() == fe->get_ompfinalthreads() ) {
        psc_infomsg( "Exporting results to %s\n", propfilename.c_str() );

        time_t rawtime;
        tm*    timeinfo;
        time( &rawtime );
        timeinfo = localtime( &rawtime );

        xmlfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
        xmlfile << "<Experiment xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
            "xmlns=\"http://www.lrr.in.tum.de/Periscope\" "
            "xsi:schemaLocation=\"http://www.lrr.in.tum.de/Periscope psc_properties.xsd \">"
                << std::endl << std::endl;

        xmlfile << std::setfill( '0' );
        xmlfile << "  <date>" << std::setw( 4 ) << ( timeinfo->tm_year + 1900 )
                << "-" << std::setw( 2 ) << timeinfo->tm_mon + 1 << "-" << std::setw( 2 )
                << timeinfo->tm_mday << "</date>" << std::endl;
        xmlfile << "  <time>" << std::setw( 2 ) << timeinfo->tm_hour << ":"
                << std::setw( 2 ) << timeinfo->tm_min << ":" << std::setw( 2 )
                << timeinfo->tm_sec << "</time>" << std::endl;

        // Wording directory for the experiment: can be used to find the data file(s)
        char* cwd = getenv( "PWD" );
        xmlfile << "  <dir>" << cwd << "</dir>" << std::endl;
        // Source revision
        if( opts.has_srcrev ) {
            // Source revision
            xmlfile << "  <rev>" << opts.srcrev << "</rev>" << std::endl;
        }
        // Empty line before the properties
        xmlfile << std::endl;
        /*
           //The following lines should be removed sss
           std::list<std::string>::iterator it;
           //sss here we have to process the required properties.
           for( it = xmlproperties_.begin(); it != xmlproperties_.end(); it++ ){
           xmlfile << *it;
           }
         */
    }

    if( xmlfile && fe->get_ompnumthreads() == fe->get_ompfinalthreads() ) {
//        std::cout << std::endl;
        //Export all properties other than exectime::::: properties to .psc file
        for( p_it = properties_.begin(); p_it != properties_.end(); ++p_it ) {
            string::size_type p_loc = ( ( *p_it ).name ).find( ":::::", 0 );
            if( false && p_loc != string::npos ) {
                ;
            }
            else {
                if( ( *p_it ).severity > threshold_for_foundproperties ) {
                    xmlfile << "<property cluster=\"" << ( *p_it ).cluster
                            << "\" ID=\"" << ( *p_it ).ID << "\" >\n  "
                            << "\t<name>" << ( *p_it ).name << "</name>\n  "
                            << "\t<context FileID=\"" << ( *p_it ).fileid
                            << "\" FileName=\"" << ( *p_it ).filename
                            << "\" RFL=\"" << ( *p_it ).RFL << "\" Config=\""
                            << ( *p_it ).maxProcs << "x" << ( *p_it ).maxThreads
                            << "\" Region=\"" << ( *p_it ).region
                            << "\" RegionId=\"" << ( *p_it ).RegionId
                            << "\">\n  " << "\t\t<execObj process=\""
                            << ( *p_it ).process << "\" thread=\""
                            << ( *p_it ).numthreads << "\"/>\n  "
                            << "\t</context>\n  \t<severity>"
                            << double( ( *p_it ).severity ) << "</severity>\n  "
                            << "\t<confidence>" << double( ( *p_it ).confidence )
                            << "</confidence>\n  \t<addInfo>\n  " << "\t\t";
                    xmlfile << ( *p_it ).addInfo;
                    string::size_type p_loc = ( ( *p_it ).name ).find( ":::::", 0 );

                    if( p_loc != string::npos ) {
                        xmlfile << "\t<exectime>" << ( p_it->execTime / NANOSEC_PER_SEC_DOUBLE ) << "s </exectime> ";
                    }

                    xmlfile << "\n\t</addInfo>\n  </property>\n";
                }
            }
        }

//        std::cout << std::endl;
        //Export speedup based newly developed property to properties.psc file
        for( sp_it = speedup_based_properties.begin(); sp_it != speedup_based_properties.end(); ++sp_it ) {
//            std::cout << " speedup_based_properties " << " size = "
//                      << speedup_based_properties.size() << " config= "
//                      << ( *sp_it ).maxThreads << " RFL = " << ( *sp_it ).RFL
//                      << " " << ( *sp_it ).filename << " region = "
//                      << ( *sp_it ).region << " process = "
//                      << ( *sp_it ).process << " severity = "
//                      << double( ( *sp_it ).severity ) << " cluster = "
//                      << ( *sp_it ).cluster << " ID = " << ( *sp_it ).ID << " "
//                      << ( *sp_it ).name << " fileid = " << ( *sp_it ).fileid
//                      << std::endl;

            if( ( *sp_it ).severity > threshold_for_foundproperties ) {
                xmlfile << "<property cluster=\"" << ( *sp_it ).cluster
                        << "\" ID=\""<< ( *sp_it ).ID << "\" >\n  "
                        << "\t<name>" << ( *sp_it ).name << "</name>\n  "
                        << "\t<context FileID=\"" << ( *sp_it ).fileid
                        << "\" FileName=\"" << ( *sp_it ).filename
                        << "\" RFL=\"" << ( *sp_it ).RFL << "\" Config=\""
                        << ( *sp_it ).maxProcs << "x" << ( *sp_it ).maxThreads
                        << "\" Region=\"" << ( *sp_it ).region
                        << "\" RegionId=\"" << ( *sp_it ).RegionId << "\">\n  "
                        << "\t\t<execObj process=\"" << ( *sp_it ).process
                        << "\" thread=\"" << ( *sp_it ).numthreads << "\"/>\n  "
                        << "\t</context>\n  \t<severity>"
                        << double( ( *sp_it ).severity ) << "</severity>\n  "
                        << "\t<confidence>" << double( ( *sp_it ).confidence )
                        << "</confidence>\n  \t<addInfo>\n  " << "\t\t";
                if( ( *sp_it ).addInfo == "" ) {
                    ;
                }
                else {
                    xmlfile << ( *sp_it ).addInfo;
                }

                xmlfile << "\n\t</addInfo>\n  </property>\n";
            }
            else {
                ;
            }
        }

//        std::cout << std::endl;
        //Export PropertyInfo from existing property list to properties.psc file
        //Different approach for printing properties
        for( prop_it = existing_properties.begin(); prop_it != existing_properties.end(); ++prop_it ) {
            string::size_type e_loc = ( ( *prop_it ).name ).find( ":::::", 0 );
            if( e_loc != string::npos ) {
                ;
            }
            else {
//                std::cout << " existing_properties " << " size = "
//                          << existing_properties.size() << " exec = "
//                          << ( *prop_it ).execTime << " config= "
//                          << ( *prop_it ).maxThreads << " RFL = "
//                          << ( *prop_it ).RFL << " " << ( *prop_it ).filename
//                          << " region = " << ( *prop_it ).region
//                          << " process = " << ( *prop_it ).process
//                          << " severity = " << double( ( *prop_it ).severity )
//                          << " cluster = " << ( *prop_it ).cluster << " ID = "
//                          << ( *prop_it ).ID << " " << ( *prop_it ).name
//                          << " fileid = " << ( *prop_it ).fileid << std::endl;

                string addinfo_name = ( *prop_it ).name;               //get the property name which stays for all the configurations here
                int    id           = Property_occurring_in_all_configurations_id();
                string id_s         = convertInt( id );

                if( ( *prop_it ).severity > threshold_for_foundproperties ) {
                    xmlfile << "<property cluster=\"" << ( *prop_it ).cluster << "\" ID=\"" << id_s << "\" >\n  "
                            << "\t<name>" << " SCA -- Property occurring in all configurations " << "</name>\n  "
                            << "\t<context FileID=\"" << ( *prop_it ).fileid << "\" FileName=\"" << ( *prop_it ).filename
                            << "\" RFL=\"" << ( *prop_it ).RFL << "\" Config=\"" << ( *prop_it ).maxProcs << "x" << ( *prop_it ).maxThreads
                            << "\" Region=\"" << ( *prop_it ).region << "\" RegionId=\"" << ( *prop_it ).RegionId << "\">\n  "
                            << "\t\t<execObj process=\"" << ( *prop_it ).process << "\" thread=\"" << ( *prop_it ).numthreads
                            << "\"/>\n  " << "\t</context>\n  \t<severity>" << double( ( *prop_it ).severity ) << "</severity>\n  "
                            << "\t<confidence>" << double( ( *prop_it ).confidence ) << "</confidence>\n  \t<addInfo>\n  "
                            << "\t\t";
                    if( addinfo_name == "" ) {
                        ;
                    }
                    else {
                        xmlfile << "<Property_Name>" << addinfo_name << "</Property_Name>";
                    }

                    xmlfile << "\n\t</addInfo>\n  </property>\n";
                }
                else {
                    ;
                }
            }
        }

//        std::cout << std::endl;
        //Export severity based newly developed property to properties.psc file
        for( s_it = severity_based_properties.begin(); s_it != severity_based_properties.end(); ++s_it ) {
//            std::cout << " severity_based_properties " << " size = "
//                      << severity_based_properties.size() << " exec = "
//                      << ( *s_it ).execTime << " config= "
//                      << ( *s_it ).maxThreads << " RFL = " << ( *s_it ).RFL
//                      << " " << ( *s_it ).filename << " region = "
//                      << ( *s_it ).region << " process = "
//                      << ( *s_it ).process << " severity = "
//                      << double( ( *s_it ).severity ) << " cluster = "
//                      << ( *s_it ).cluster << " ID = " << ( *s_it ).ID << " "
//                      << ( *s_it ).name << " fileid = " << ( *s_it ).fileid
//                      << std::endl;


            if( ( *s_it ).severity > threshold_for_foundproperties ) {
                xmlfile << "<property cluster=\"" << ( *s_it ).cluster << "\" ID=\"" << ( *s_it ).ID << "\" >\n  "
                        << "\t<name>" << ( *s_it ).name << "</name>\n  "
                        << "\t<context FileID=\"" << ( *s_it ).fileid << "\" FileName=\"" << ( *s_it ).filename
                        << "\" RFL=\"" << ( *s_it ).RFL << "\" Config=\"" << ( *s_it ).maxProcs << "x" << ( *s_it ).maxThreads
                        << "\" Region=\"" << ( *s_it ).region << "\" RegionId=\"" << ( *s_it ).RegionId << "\">\n  "
                        << "\t\t<execObj process=\"" << ( *s_it ).process << "\" thread=\"" << ( *s_it ).numthreads
                        << "\"/>\n  " << "\t</context>\n  \t<severity>" << double( ( *s_it ).severity ) << "</severity>\n  "
                        << "\t<confidence>" << double( ( *s_it ).confidence ) << "</confidence>\n  \t<addInfo>\n  "
                        << "\t\t";
                if( ( *s_it ).addInfo == "" ) {
                    ;
                }
                else {
                    xmlfile << ( *s_it ).addInfo;
                }

                xmlfile << "\n\t</addInfo>\n  </property>\n";
            }
            else {
                ;
            }
        }

//        std::cout << std::endl;
        for( d_it = DeviationProperties.begin(); d_it != DeviationProperties.end(); ++d_it ) {
//            std::cout << " DeviationProperties " << " size = "
//                      << DeviationProperties.size() << " config= "
//                      << ( *d_it ).maxThreads << " RFL = "
//                      << ( *d_it ).RFL << " " << ( *d_it ).filename
//                      << " region = " << ( *d_it ).region
//                      << " process = " << ( *d_it ).process << " severity = "
//                      << double( ( *d_it ).severity ) << " cluster = "
//                      << ( *d_it ).cluster << " ID = " << ( *d_it ).ID << " "
//                      << ( *d_it ).name << " fileid = " << ( *d_it ).fileid
//                      << std::endl;

            if( ( *d_it ).severity > threshold_for_foundproperties ) {
                xmlfile << "<property cluster=\"" << ( *d_it ).cluster << "\" ID=\"" << ( *d_it ).ID << "\" >\n  "
                        << "\t<name>" << ( *d_it ).name << "</name>\n  "
                        << "\t<context FileID=\"" << ( *d_it ).fileid << "\" FileName=\"" << ( *d_it ).filename
                        << "\" RFL=\"" << ( *d_it ).RFL << "\" Config=\"" << ( *d_it ).maxProcs << "x" << ( *d_it ).maxThreads
                        << "\" Region=\"" << ( *d_it ).region << "\" RegionId=\"" << ( *d_it ).RegionId << "\">\n  "
                        << "\t\t<execObj process=\"" << ( *d_it ).process << "\" thread=\"" << ( *d_it ).numthreads
                        << "\"/>\n  " << "\t</context>\n  \t<severity>" << double( ( *d_it ).severity ) << "</severity>\n  "
                        << "\t<confidence>" << double( ( *d_it ).confidence ) << "</confidence>\n  \t<addInfo>\n  "
                        << "\t\t";
                if( ( *d_it ).addInfo == "" ) {
                    ;
                }
                else {
                    xmlfile << ( *d_it ).addInfo;
                }

                xmlfile << "\n\t</addInfo>\n  </property>\n";
            }
            else {
                ;
            }
        }



        xmlfile << "</Experiment>" << std::endl;
    }

    xmlfile.close();
}

//This function calculates the total deviation among the regions;
//it finds the maximum among them; it shows the region with its severity value.
void PeriscopeFrontend::Check_severity_among_parallel_regions( int config ) {
    std::list<PropertyInfo>::iterator prop_it, ex_it, p_it;
    std::list<PropertyInfo>           prop_threadbased;
    std::list<long double>            RFL_history;

    long double total_deviation_time = 0.0;
    long double phaseTime            = 0.0;
    long double parallelTime         = 0.0;
    bool        severity_check       = false;
    string      userregion_filename;
    int         userregion_fileid;
    string      userregion_cluster;
    string      userregion_ID;
    string      userregion_context;
    string      userregion_region;
    string      userregion_RegionId;
    double      userregion_RFL        = 0.0;
    int         userregion_maxThreads = 0;
    int         userregion_maxProcs   = 0;
    int         userregion_numthreads = 0;
    int         userregion_process    = 0;
    double      userregion_confidence = 0.0;
    prop_threadbased.clear();
    RFL_history.clear();

    //Finds the right config based properties and add them in a list;
    //calculates the sequential time for parallel regions in the program
    for( p_it = properties_.begin(); p_it != properties_.end(); ++p_it ) {
        //it should be restricted to only
        if( ( *p_it ).maxThreads == config ) {
            prop_threadbased.push_back( *p_it );
        }
        else {
            ;
        }

        if( ( ( *p_it ).maxThreads == 1 ) && ( ( *p_it ).region == "PARALLEL_REGION" ||
                                               ( *p_it ).region == "DO_REGION" ||
                                               ( *p_it ).region == "WORKSHARE_REGION" ) ) {
            parallelTime += ( *p_it ).execTime;
        }
        else {
            ;
        }
    }
    int config_thread_count = prop_threadbased.size();

    //Finds the total deviation time among all the regions
    for( ex_it = ExecAndDeviaProperties.begin(); ex_it != ExecAndDeviaProperties.end(); ++ex_it ) {
        //convert config to equivalent array.
        int config_equiv = 0;
        int count        = 0;
        for( int i = 1; i <= fe->get_ompfinalthreads(); i += i ) {
            if( i == config ) {
                config_equiv = count;
            }
            count++;
        }
        //Get the phasetime for this configuration
        if( ( *ex_it ).region == "USER_REGION" ) {
            phaseTime             = ( *ex_it ).exec_regions[ config_equiv ];
            userregion_filename   = ( *ex_it ).filename;
            userregion_region     = ( *ex_it ).region;
            userregion_fileid     = ( *ex_it ).fileid;
            userregion_cluster    = ( *ex_it ).cluster;
            userregion_RFL        = ( *ex_it ).RFL;
            userregion_ID         = ( *ex_it ).ID;
            userregion_context    = ( *ex_it ).context;
            userregion_maxThreads = ( *ex_it ).maxThreads;
            userregion_maxProcs   = ( *ex_it ).maxProcs;
            userregion_numthreads = ( *ex_it ).numthreads;
            userregion_process    = ( *ex_it ).process;
            userregion_confidence = ( *ex_it ).confidence;
            userregion_RegionId   = ( *ex_it ).RegionId;
        }
        else if( ( *ex_it ).region == "MAIN_REGION" ) {
            phaseTime             = ( *ex_it ).exec_regions[ config_equiv ];
            userregion_region     = ( *ex_it ).region;
            userregion_filename   = ( *ex_it ).filename;
            userregion_fileid     = ( *ex_it ).fileid;
            userregion_cluster    = ( *ex_it ).cluster;
            userregion_RFL        = ( *ex_it ).RFL;
            userregion_ID         = ( *ex_it ).ID;
            userregion_context    = ( *ex_it ).context;
            userregion_maxThreads = ( *ex_it ).maxThreads;
            userregion_maxProcs   = ( *ex_it ).maxProcs;
            userregion_numthreads = ( *ex_it ).numthreads;
            userregion_process    = ( *ex_it ).process;
            userregion_confidence = ( *ex_it ).confidence;
            userregion_RegionId   = ( *ex_it ).RegionId;
        }
        else {
            ;
        }

        total_deviation_time += ( *ex_it ).devia_regions[ config_equiv ];
    }

    long double  max_deviation_time = 0.0;
    PropertyInfo max_info;
    for( prop_it = prop_threadbased.begin(); prop_it != prop_threadbased.end(); ++prop_it ) {
        //First find for main_region / user_region
        config_thread_count--;
        bool RFL_check = false;

        //Ensure that the RFL is unique
        RFL_history.push_back( ( *prop_it ).RFL );

        if( ( int )count( RFL_history.begin(), RFL_history.end(), ( *prop_it ).RFL ) > 1 ) {
            RFL_check = true;
        }


        //if((*prop_it).region == "USER_REGION" || (*prop_it).region == "MAIN_REGION" ||
        if( ( ( *prop_it ).region == "PARALLEL_REGION" ||
              ( *prop_it ).region == "WORKSHARE_DO" ||
              ( *prop_it ).region == "DO_REGION" ||
              ( *prop_it ).region == "CALL_REGION" ||
              ( *prop_it ).region == "SUB_REGION" ) && !RFL_check ) {
            //check if it matches with devia_properties
            string temp_region     = ( *prop_it ).region;
            double temp_process    = ( *prop_it ).process;
            int    temp_maxThreads = ( *prop_it ).maxThreads;
            double temp_RFL        = ( *prop_it ).RFL;
            int    temp_fileid     = ( *prop_it ).fileid;

            for( ex_it = ExecAndDeviaProperties.begin(); ex_it != ExecAndDeviaProperties.end(); ++ex_it ) {
                if( temp_process == ( *ex_it ).process && temp_RFL == ( *ex_it ).RFL
                    && ( *ex_it ).fileid == temp_fileid && ( *ex_it ).region == temp_region ) {
                    //here we have to find the maximal deviation based region
                    int         count             = 0;
                    int         config_equiv      = 0;
                    long double devia_time_region = 0.0;
                    for( int i = 1; i <= fe->get_ompfinalthreads(); i += i ) {
                        if( i == config ) {
                            config_equiv = count;
                        }
                        count++;
                    }
                    devia_time_region = ( *ex_it ).devia_regions[ config_equiv ];


                    if( devia_time_region > max_deviation_time ) {
                        max_deviation_time = devia_time_region;

                        //store the maximal region and its context
                        max_info.filename = ( *prop_it ).filename;
                        max_info.context  = ( *prop_it ).context;
                        max_info.region   = ( *prop_it ).region;
                        max_info.cluster  = ( *prop_it ).cluster;
                        int id = Code_region_with_the_lowest_speedup_in_the_run_id();
                        max_info.ID         = convertInt( id );
                        max_info.fileid     = ( *prop_it ).fileid;
                        max_info.RFL        = ( *prop_it ).RFL;
                        max_info.confidence = ( double )( *prop_it ).confidence;
                        max_info.maxThreads = config;                         //config val
                        max_info.process    = ( *prop_it ).process;
                        max_info.numthreads = ( *prop_it ).numthreads;
                        max_info.maxProcs   = ( *prop_it ).maxProcs;
                        max_info.RegionId   = ( *prop_it ).RegionId;
                        //here include the property saying that the region had maximal deviation with
                        //the severity calculated
                        double severity  = 0.0;
                        double condition = 0.0;
                        if( config == 1 ) {
                            max_info.severity = 0.0;                             //because the deviation is always zero for config 1
                        }
                        else {
                            condition         = ( devia_time_region / total_deviation_time ) * 100;
                            severity          = ( devia_time_region / phaseTime ) * 100;
                            max_info.severity = severity;
                        }
                        max_info.name = " SCA -- Code region with the lowest speedup in the run ";
                        if( condition > 5 ) {
                            severity_check = true;
                        }
                        else {
                            severity_check = false;
                        }
                    }
                    else {
                        ;
                    }

                    break;
                }
                else {
                    ;
                }
            }
        }
        else {
            ;          //for region checks
        }
        //We need to add in the list which has the maximal deviation
        if( severity_check && config_thread_count == 0 ) {     //ensure that we are in the final region check process
            DeviationProperties.push_back( max_info );
        }
    }

    //Add another property for finding the percentage of the parallelized code
    if( config == 1 ) {
        PropertyInfo s_info;
        s_info.filename = userregion_filename;
        s_info.context  = userregion_context;
        s_info.region   = userregion_region;
        s_info.cluster  = userregion_cluster;

//        char buf[ 5 ];
//        itoa( Sequential_Computation_id(), buf, 10 );

        int id = Sequential_Computation_id();
        s_info.ID = convertInt( id );
//        std::cout << " SEQUENTIALCOMPUTATION " << id << " s_info " << s_info.ID << std::endl;
        s_info.fileid     = userregion_fileid;
        s_info.RFL        = userregion_RFL;
        s_info.confidence = userregion_confidence;
        s_info.process    = userregion_process;
        s_info.numthreads = userregion_numthreads;
        s_info.maxThreads = userregion_maxThreads;
        s_info.maxProcs   = userregion_maxProcs;
        s_info.severity   = ( ( phaseTime - parallelTime ) / phaseTime ) * 100;
        s_info.RegionId   = userregion_RegionId;
        s_info.name       = " SCA -- Sequential Computation ";

        DeviationProperties.push_back( s_info );
    }
    else {
        ;
    }
}

//This function find the sequential time for the forwarded existing property
long double PeriscopeFrontend::find_sequential_time( PropertyInfo prop_info ) {
    std::list<PropertyInfo>::iterator seq_it;

    string region_t  = prop_info.region;
    string ID_t      = prop_info.ID;
    string name_t    = prop_info.name;
    double process_t = prop_info.process;
    double RFL_t     = prop_info.RFL;
    int    fileid_t  = prop_info.fileid;


    //Check all the properties and if it identifies the same property, return its execution time.
    for( seq_it = properties_.begin(); seq_it != properties_.end(); ++seq_it ) {
        if( region_t == ( *seq_it ).region && process_t == ( *seq_it ).process
            && RFL_t == ( *seq_it ).RFL && fileid_t == ( *seq_it ).fileid
            && ID_t == ( *seq_it ).ID && name_t == ( *seq_it ).name ) {
            return ( *seq_it ).execTime;
        }
        else {
            ;
        }
    }
    return 0.0;
}

//This function copies properties from thread 2
void PeriscopeFrontend::copy_properties_for_analysis() {
    std::list<PropertyInfo>::iterator copy_it;

    //Check all the properties and add in the list if the threads are above 2
    //Required because the OpenMP properties show up only after thread no. 2
    for( copy_it = properties_.begin(); copy_it != properties_.end(); ++copy_it ) {
        if( ( *copy_it ).maxThreads > 1 ) {
            property_threads.push_back( *copy_it );
            p_threads.push_back( *copy_it );
        }
        else {
            ;
        }
    }
}

//This function identifies the existing_properties among threads
void PeriscopeFrontend::find_existing_properties() {
    std::list<PropertyInfo>::iterator prop_it, prop_itr;
    existing_properties.clear();
    int initial_thread = 2;

    std::cout << std::endl;
    long int count = 0;
    for( prop_it = property_threads.begin(); prop_it != property_threads.end(); ++prop_it ) {
        //surf within property infos...
        //fixing on one propertyInfo, check if we have to include in the list of properties or not.
        count++;
        if( ( *prop_it ).maxThreads == initial_thread ) {
            string temp_region     = ( *prop_it ).region;
            string temp_name       = ( *prop_it ).name;
            double temp_severity   = double( ( *prop_it ).severity );
            double temp_process    = ( *prop_it ).process;
            int    temp_maxThreads = ( *prop_it ).maxThreads;
            double temp_RFL        = ( *prop_it ).RFL;
            int    temp_fileid     = ( *prop_it ).fileid;

            long int count_scaled = 0;
            for( int th_number = initial_thread; th_number <= fe->get_ompfinalthreads(); th_number += th_number ) {
                long int count_in = 0;
                for( prop_itr = p_threads.begin(); prop_itr != p_threads.end(); ++prop_itr ) {
                    //after storing details, go through all the properties from the point of comparison to compare
                    //and determine whether that property should be in the list of scalable properties
                    count_in++;                                 //
                    if( temp_name == ( *prop_itr ).name && temp_process == ( *prop_itr ).process
                        && temp_RFL == ( *prop_itr ).RFL && count_in >= count && temp_fileid == ( *prop_itr ).fileid &&
                        ( *prop_itr ).maxThreads == th_number && ( *prop_itr ).maxThreads != 1 ) {
                        //For existing_properties checks process here
                        count_scaled++;
                        if( ( count_scaled + 1 ) >= fe->get_maxiterations() ) {
                            string::size_type loc = ( ( *prop_it ).name ).find( ":::::", 0 );
                            if( loc != string::npos ) {
                                ;
                            }
                            else {
                                existing_properties.push_back( *prop_it );
                            }
                        }
                        break;                         //need not check for other properties in that thread number
                    }
                    else {
                        ;
                    }
                }
            }
        }
        else {
            break;
        }                     //not from initial thread
    }
}

////Function that converts integer to string
//std::string PeriscopeFrontend::convertInt( int number )
//{
//    if( number == 0 ) {
//        return "0";
//    }
//    string temp = "";
//    string returnvalue = "";
//    while(number > 0) {
//        temp   += number % 10 + 48;
//        number /= 10;
//    }
//    for( int i = 0; i < temp.length(); i++ ) {
//        returnvalue += temp[ temp.length() - i - 1 ];
//    }
//    return returnvalue;
//}

//Function that converts integer to string
std::string PeriscopeFrontend::convertInt( int number ) {
    stringstream ss; //create a stringstream
    ss << number;    //add number to the stream
    return ss.str(); //return a string with the contents of the stream
}

//This function does speedup analysis for every parallel regions
void PeriscopeFrontend::do_speedup_analysis( PropertyInfo       prop,
                                             long double        seq_time,
                                             const std::string& phase_region ) {
    std::list<PropertyInfo>::iterator prop_itr;
    std::list<long double>::iterator  rt_it;
    std::list<long double>            temp_PhaseTime;
    temp_PhaseTime.clear();

    string      temp_region                = prop.region;
    string      temp_name                  = prop.name;
    double      temp_severity              = double( prop.severity );
    double      temp_process               = prop.process;
    int         temp_maxThreads            = prop.maxThreads;
    double      temp_RFL                   = prop.RFL;
    double      severity_for_initialthread = 0.0;
    long double PhaseTime                  = 0.0;
    int         temp_fileid                = prop.fileid;
    string      temp_ID                    = prop.ID;

    int count_scaled   = 0;
    int count_execTime = 0;
    int count_speedup  = 0;
    int count_severity = 0;

    bool stop_speedup_check  = false;
    bool stop_tspeedup_check = false;
    bool stop_severity_check = false;
    bool need_not_list       = false;

    double      t_execTime       = 0.0;
    double      t_severity       = 0.0;
    int         initial_thread   = 2;
    long double initial_execTime = seq_time;
    double      speedup_prop     = 0.0;
    double      t_speedup        = 0.0;
    std::string string_name      = " ";

    //First initialization
    int          Max              = 10000;
    long double* execTime_regions = new long double[ Max ];
    long double* deviationTime    = new long double[ Max ];
    execTime_regions[ 1 ] = initial_execTime;
    deviationTime[ 1 ]    = 0.0;
    if( phase_region == "MAIN_REGION" || phase_region == "USER_REGION" ) {
        PhaseTime_user.push_back( initial_execTime );
    }
    //make a temp list of phaseTime and remove the first one
    else if( ( phase_region != "MAIN_REGION" || phase_region != "USER_REGION" )
             && PhaseTime_user.size() > 0 ) {
        temp_PhaseTime.assign( PhaseTime_user.begin(), PhaseTime_user.end() );
        temp_PhaseTime.pop_front();
    }
    else {
        ;
    }

    for( int th_number = initial_thread; th_number <= fe->get_ompfinalthreads(); th_number += th_number ) {
        execTime_regions[ th_number ] = 0.0;
        deviationTime[ th_number ]    = 0.0;

        //For each thread number, we have to calculate the phaseTime
        PhaseTime = 0.0;
        if( temp_PhaseTime.size() > 0 && PhaseTime_user.size() > 0 ) {
            PhaseTime = temp_PhaseTime.front();
            temp_PhaseTime.pop_front();
        }
        else {
            ;
        }

        for( prop_itr = property_threads.begin(); prop_itr != property_threads.end(); ++prop_itr ) {
            need_not_list = false;
            if( temp_process == ( *prop_itr ).process && temp_fileid == ( *prop_itr ).fileid
                && temp_RFL == ( *prop_itr ).RFL && ( *prop_itr ).maxThreads == th_number
                && ( *prop_itr ).maxThreads != 1 && temp_name == ( *prop_itr ).name
                && ( *prop_itr ).ID == temp_ID ) {
                //We don't need to add the property relating to execTime
                string            p_name = ( *prop_itr ).name;
                string::size_type loc    = p_name.find( ":::::", 0 );
                if( loc != string::npos ) {
                    need_not_list = true;
                }
                else {
                    ;
                }

                //we need to get the severity for initial run
                if( th_number == initial_thread ) {
                    severity_for_initialthread = ( *prop_itr ).severity;
                }
                else {
                    ;
                }


                count_scaled++;

                //the following property should be checked again
                //here we have to check if the severity is increasing or not.
                if( ( *prop_itr ).severity >= t_severity && stop_severity_check == false ) {
                    count_severity++;
                    if( ( count_severity + 1 ) >= fe->get_maxiterations() ) {
                        //if the severity has continuously increased
                        PropertyInfo s_info;
                        s_info.name     = " SCA -- Property with increasing severity across configurations ";
                        s_info.filename = prop.filename;
                        s_info.context  = prop.context;
                        s_info.region   = prop.region;
                        s_info.cluster  = prop.cluster;
                        int id = Property_with_increasing_severity_across_configurations_id();
                        s_info.ID         = convertInt( id );
                        s_info.fileid     = prop.fileid;
                        s_info.numthreads = prop.numthreads;
                        s_info.maxProcs   = prop.maxProcs;
                        s_info.RegionId   = prop.RegionId;

                        //make a definition here
                        //severity is defined based on how much it increased for this particular property for various threads
                        s_info.severity   = ( prop.severity - severity_for_initialthread );
                        s_info.RFL        = prop.RFL;
                        s_info.confidence = ( double )prop.confidence;
                        s_info.maxThreads = th_number;                         //the property increased till this thread number
                        string_name       = "<prop_name>";
                        string string_name_end = "</prop_name>";
                        s_info.addInfo = string_name + prop.name + string_name_end;
                        s_info.process = prop.process;

                        if( !need_not_list ) {
                            severity_based_properties.push_back( s_info );
                        }
                        else {
                            ;
                        }
                    }
                    t_severity = ( *prop_itr ).severity;
                }
                else {
                    ;
                }


                //Speedup property requirements
                speedup_prop = ( long double )initial_execTime / ( double )( *prop_itr ).execTime;

                int    rounded_speedup = speedup_prop;
                double decimal_val     = ( double )speedup_prop - rounded_speedup;

                if( decimal_val > 0.5 ) {
                    rounded_speedup++;
                }
                else {
                    ;
                }

                //Store the execution time & deviation time for regions
                execTime_regions[ th_number ] = ( *prop_itr ).execTime;
                if( phase_region == "MAIN_REGION" || phase_region == "USER_REGION" ) {
                    PhaseTime_user.push_back( ( *prop_itr ).execTime );
                }
                else {
                    ;
                }
                if( ( ( *prop_itr ).execTime ) - ( initial_execTime / th_number ) > 0 ) {
                    deviationTime[ th_number ] = ( ( *prop_itr ).execTime ) - ( initial_execTime / th_number );
                }
                else {
                    deviationTime[ th_number ] = 0.0;
                }

                //Converting double to string
                std::ostringstream sstream;
                sstream << speedup_prop;
                std::string speedup_str = sstream.str();

                speedup_str += "";
                string speedup_start = "<speedup>";
                string speedup_end   = "</speedup>";

                //Check if this satisfies the linear speedup/superlinear speedup condition (RARELY OCCURS)
                if( rounded_speedup == th_number ||
                    rounded_speedup > th_number &&
                    stop_speedup_check == false ) {
                    count_speedup++;
                    PropertyInfo s_info;

                    s_info.filename   = prop.filename;
                    s_info.context    = prop.context;
                    s_info.region     = prop.region;
                    s_info.cluster    = prop.cluster;
                    s_info.fileid     = prop.fileid;
                    s_info.severity   = 0.0;                   //severity should be 0.0
                    s_info.RFL        = prop.RFL;
                    s_info.confidence = ( double )prop.confidence;
                    s_info.maxThreads = th_number;
                    s_info.addInfo    = speedup_start + speedup_str + speedup_end;
                    s_info.process    = prop.process;
                    s_info.numthreads = prop.numthreads;
                    s_info.maxProcs   = prop.maxProcs;
                    s_info.RegionId   = prop.RegionId;

                    if( rounded_speedup > th_number ) {
                        s_info.name = " SCA -- SuperLinear Speedup for all configurations ";
                        int id = SuperLinear_Speedup_for_all_configurations_id();
                        s_info.ID = convertInt( id );
                        speedup_based_properties.push_back( s_info );
                    }
                    else {
                        ;
                    }
                    if( ( count_speedup + 1 ) >= fe->get_maxiterations() ) {
                        //now we shall include a new property saying that the speedup satisfied for all threads
                        // in the particular RFL
                        s_info.name = " SCA -- Linear Speedup for all configurations ";
                        int id = Linear_Speedup_for_all_configurations_id();
                        s_info.ID = convertInt( id );
                        speedup_based_properties.push_back( s_info );
                    }
                }

                else if( !stop_speedup_check ) {
                    //speedup fails in linear speedup condition once from the previous runs
                    stop_speedup_check = true;
                    //now we shall include a new property saying that the speedup is not satisfied due to these properties
                    // in the particular RFL
                    PropertyInfo s_info;

                    s_info.name     = " SCA -- Linear speedup failed for the first time ";
                    s_info.filename = prop.filename;
                    s_info.context  = prop.context;
                    s_info.region   = prop.region;
                    s_info.cluster  = prop.cluster;
                    int id = Linear_speedup_failed_for_the_first_time_id();
                    s_info.ID     = convertInt( id );
                    s_info.fileid = prop.fileid;
                    if( s_info.region == "MAIN_REGION" || s_info.region == "USER_REGION" ) {
                        s_info.severity = ( deviationTime[ th_number ] / ( *prop_itr ).execTime ) * 100;
                    }
                    else {
                        s_info.severity = ( deviationTime[ th_number ] / PhaseTime ) * 100;
                    }
                    s_info.RFL        = prop.RFL;
                    s_info.confidence = ( double )prop.confidence;
                    s_info.maxThreads = th_number;
                    s_info.addInfo    = speedup_start + speedup_str + speedup_end;
                    s_info.process    = prop.process;
                    s_info.numthreads = prop.numthreads;
                    s_info.maxProcs   = prop.maxProcs;
                    s_info.RegionId   = prop.RegionId;

                    speedup_based_properties.push_back( s_info );
                }
                else {
                    ;
                }

                //We shall include a property that reports the low speedup considering every regions
                PropertyInfo l_info;
                long double  l_condition = 0.0;
                l_condition = ( deviationTime[ th_number ] / PhaseTime ) * 100;

                if( l_condition > 0 ) {              //Threshold set to zero (Increase later)
                    l_info.name     = " SCA -- Low Speedup ";
                    l_info.filename = prop.filename;
                    l_info.context  = prop.context;
                    l_info.region   = prop.region;
                    l_info.cluster  = prop.cluster;
                    int id = Low_Speedup_id();
                    l_info.ID     = convertInt( id );
                    l_info.fileid = prop.fileid;
                    if( l_info.region == "MAIN_REGION" || l_info.region == "USER_REGION" ) {
                        l_info.severity = ( deviationTime[ th_number ] / ( *prop_itr ).execTime ) * 100;
                    }
                    else {
                        l_info.severity = ( deviationTime[ th_number ] / PhaseTime ) * 100;
                    }
                    l_info.RFL        = prop.RFL;
                    l_info.confidence = ( double )prop.confidence;
                    l_info.maxThreads = th_number;
                    l_info.addInfo    = speedup_start + speedup_str + speedup_end;
                    l_info.process    = prop.process;
                    l_info.numthreads = prop.numthreads;
                    l_info.maxProcs   = prop.maxProcs;
                    l_info.RegionId   = prop.RegionId;

                    speedup_based_properties.push_back( l_info );
                }
                else {
                    ;
                }


                //Find the thread when the speedup decreased
                //if(rounded_speedup < t_speedup && th_number != initial_thread && !stop_tspeedup_check){
                if( rounded_speedup < t_speedup && !stop_tspeedup_check ) {
                    //now we shall include a new property saying that the highest speedup reached until this threadno.
                    // in the particular RFL
                    stop_tspeedup_check = true;
                    PropertyInfo s_info;

                    s_info.name     = " SCA -- Speedup Decreasing ";
                    s_info.filename = prop.filename;
                    s_info.context  = prop.context;
                    s_info.region   = prop.region;
                    s_info.cluster  = prop.cluster;
                    int id = Speedup_Decreasing_id();
                    s_info.ID     = convertInt( id );
                    s_info.fileid = prop.fileid;
                    if( s_info.region == "MAIN_REGION" || s_info.region == "USER_REGION" ) {
                        s_info.severity = 0.0;
                    }
                    else {
                        s_info.severity = ( deviationTime[ th_number ] / PhaseTime ) * 100;
                    }
                    s_info.RFL        = prop.RFL;
                    s_info.confidence = ( double )prop.confidence;
                    s_info.maxThreads = th_number;
                    s_info.addInfo    = speedup_start + speedup_str + speedup_end;
                    s_info.process    = prop.process;
                    s_info.numthreads = prop.numthreads;
                    s_info.maxProcs   = prop.maxProcs;
                    s_info.RegionId   = prop.RegionId;

                    speedup_based_properties.push_back( s_info );
                }
                else {
                    t_speedup = rounded_speedup;
                }



                break;    //need not check for other properties in that thread number
            }             //if(th_number==)
            else {
                ;
            }
        } //for all properties_
    }     //for all threads

    PropertyInfo d_info;
    d_info.name       = " ";
    d_info.filename   = prop.filename;
    d_info.context    = prop.context;
    d_info.region     = prop.region;
    d_info.cluster    = prop.cluster;
    d_info.ID         = prop.ID;
    d_info.fileid     = prop.fileid;
    d_info.severity   = 0.0;
    d_info.RFL        = prop.RFL;
    d_info.confidence = ( double )prop.confidence;
    d_info.maxThreads = prop.maxThreads;
    d_info.addInfo    = " ";
    d_info.execTime   = prop.execTime;
    d_info.process    = prop.process;
    d_info.numthreads = prop.numthreads;
    d_info.maxProcs   = prop.maxProcs;
    d_info.RegionId   = prop.RegionId;

    int         count_exec = 0;
    long double temp_execTime[ 30 ];
    long double temp_deviaTime[ 30 ];
    for( int i = 1; i <= fe->get_ompfinalthreads(); i += i ) {
        temp_execTime[ count_exec ]  = execTime_regions[ i ];
        temp_deviaTime[ count_exec ] = deviationTime[ i ];
        count_exec++;
    }
    for( int i = 0; i < count_exec; i++ ) {
        d_info.exec_regions[ i ]  = temp_execTime[ i ];
        d_info.devia_regions[ i ] = temp_deviaTime[ i ];
    }

    ExecAndDeviaProperties.push_back( d_info );

    delete[] execTime_regions;
    delete[] deviationTime;
}

PropertyID PeriscopeFrontend::Property_with_increasing_severity_across_configurations_id() {
    return PROPERTYWITHINCREASINGSEVERITYACROSSCONFIGURATIONS;
}

PropertyID PeriscopeFrontend::SuperLinear_Speedup_for_all_configurations_id() {
    return SUPERLINEARSPEEDUPFORALLCONFIGURATIONS;
}

PropertyID PeriscopeFrontend::Linear_Speedup_for_all_configurations_id() {
    return LINEARSPEEDUPFORALLCONFIGURATIONS;
}

PropertyID PeriscopeFrontend::Linear_speedup_failed_for_the_first_time_id() {
    return LINEARSPEEDUPFAILEDFORTHEFIRSTTIME;
}

PropertyID PeriscopeFrontend::Low_Speedup_id() {
    return LOWSPEEDUP;
}

PropertyID PeriscopeFrontend::Speedup_Decreasing_id() {
    return SPEEDUPDECREASING;
}

PropertyID PeriscopeFrontend::Sequential_Computation_id() {
    return SEQUENTIALCOMPUTATION;
}

PropertyID PeriscopeFrontend::Code_region_with_the_lowest_speedup_in_the_run_id() {
    return CODEREGIONWITHTHELOWESTSPEEDUPINTHERUN;
}

PropertyID PeriscopeFrontend::Property_occurring_in_all_configurations_id() {
    return PROPERTYOCCURRINGINALLCONFIGURATIONS;
}
/*
 * This function returns the property info as a list using MetaProperty.cc
 */
void PeriscopeFrontend::get_prop_info( MetaProperty& prop ) {
    PropertyInfo info;
    info.filename = prop.getFileName();
    info.name     = prop.getName();
    info.region   = prop.getRegionType();
    if( prop.getCluster() ) {
        info.cluster = "true";
    }
    else {
        info.cluster = "false";
    }
    info.ID         = prop.getId();
    info.RegionId   = prop.getRegionId();
    info.Config     = prop.getConfiguration();
    info.fileid     = prop.getFileId();
    info.RFL        = prop.getStartPosition();
    info.process    = prop.getProcess();
    info.numthreads = prop.getThread();
    info.severity   = prop.getSeverity();
    info.confidence = prop.getConfidence();
    info.maxThreads = prop.getMaxThreads();
    info.maxProcs   = prop.getMaxProcs();
    info.purpose    = prop.getPurpose();

    //To find info.execTime
    addInfoType                 addInfo          = prop.getExtraInfo();
    addInfoType::const_iterator it               = addInfo.find( "ExecTime" );
    string                      addInfo_ExecTime = it->second;
    info.execTime = atof( addInfo_ExecTime.c_str() );

    //To find info.addInfo
    string addInfo_str;
    for( addInfoType::const_iterator it = addInfo.begin(); it != addInfo.end(); ++it ) {
        addInfo_str  = "\t\t<";
        addInfo_str += it->first;
        addInfo_str += ">";
        addInfo_str += it->second;
        addInfo_str += "</";
        addInfo_str += it->first;
        addInfo_str += ">\n";
    }

    //Add in the list of properties
    if( info.execTime == 0 && ( info.region == "SUB_REGION" || info.region == "CALL_REGION" ) ) {
        ;
    }
    else {
        property.push_back( info );
        properties_.push_back( info );
    }
}


void PeriscopeFrontend::reinit( int map_len,
                                int map_from[ 8192 ],
                                int map_to[ 8192 ] ) {
    std::map<std::string, AgentInfo>::iterator it;

    for( it = child_agents_.begin(); it != child_agents_.end(); it++ ) {
        AgentInfo& ag = it->second;

        if( ag.status != AgentInfo::CONNECTED ) {
            if( connect_to_child( &ag ) == -1 ) {
                psc_errmsg( "Error connecting to child\n" );
                abort();
            }
            else {
                psc_dbgmsg( 5, "Sending REINIT command...\n" );
                ag.handler->reinit( map_len, map_from, map_to );
                psc_dbgmsg( 5, "Sending REINIT command...OK\n" );
            }
        }
    }
}

std::ostream& operator<<( std::ostream&       os,
                          const PropertyInfo& info ) {
    os << endl;
    os << "Property '" << info.name << "'" << endl;
    os << "     holds for '" << info.context << "'" << endl;
    os << "            on '" << info.host << "'" << endl;
    os << "      severity " << setprecision( 5 ) << info.severity << ", confidence " << info.confidence;
    return os;
}

bool sevComparator( MetaProperty p1,
                    MetaProperty p2 ) {
    return p1.getSeverity() > p2.getSeverity();
}

/**
 * @brief Prints all found properties
 *
 */
void PeriscopeFrontend::properties() {
    int limitProps = 50;
    if( opts.has_nrprops ) {
        limitProps = opts.nrprops;
    }

    if( limitProps > 0 ) {
        std::cout << std::endl << "ALL FOUND PROPERTIES" << std::endl;
        std::cout << "-------------------------------------------------------------------------------\n";
//        std::cout << std::setw(7) << "Procs\t" << std::setw(7) << "Threads\t"
//                  << std::setw(13) << "Region\t" << std::setw(12) << "Location\t"
//                  << std::setw(10) << "Severity\t" << "Description" << std::endl;
        std::cout << std::setw(7) << "Procs\t" << std::setw(13) << "Region\t"
                  << std::setw(12) << "Location\t" << std::setw(10)
                  << "Severity\t" << "Description" << std::endl;
        std::cout << "-------------------------------------------------------------------------------\n";

        //metaproperties_.sort(sevComparator);
        std::sort( metaproperties_.begin(), metaproperties_.end(), sevComparator );

        //for(std::list<MetaProperty>::iterator it = metaproperties_.begin(); ( it != metaproperties_.end() && limitProps > 0) ; it++, limitProps-- ){
        for( int i = 0; i < metaproperties_.size() && limitProps > 0; i++, limitProps-- ) {
            string            prop_string = ( metaproperties_[ i ] ).toString();
            string::size_type p_loc       = prop_string.find( ":::::", 0 );
            // Exclude internal properties used to calculate the execution time for the scalability analysis
            if( p_loc != string::npos ) {
                ;
            }
            else {
                std::cout << prop_string << std::endl;
            }
        }
        //prompt();
    }
}

/**
 * @brief Export all found properties to a file
 *
 * Exports all found properties to a file in XML format.
 * The filename can be specified using the --propfile command argument.
 */
void PeriscopeFrontend::export_properties() {
    std::string   propfilename = fe->get_outfilename();
    std::ofstream xmlfile( propfilename.c_str() );

    if( !xmlfile && opts.has_propfile ) {
        std::cout << "Cannot open xmlfile. \n";
        exit( 1 );
    }

    if( xmlfile ) {
        psc_infomsg( "Exporting results to %s\n", propfilename.c_str() );

        time_t rawtime;
        tm*    timeinfo;
        time( &rawtime );
        timeinfo = localtime( &rawtime );

        xmlfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
        xmlfile << "<Experiment xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
            "xmlns=\"http://www.lrr.in.tum.de/Periscope\" "
            "xsi:schemaLocation=\"http://www.lrr.in.tum.de/Periscope psc_properties.xsd \">"
                << std::endl << std::endl;

        xmlfile << std::setfill( '0' );
        xmlfile << "  <date>" << std::setw( 4 ) << ( timeinfo->tm_year + 1900 )
                << "-" << std::setw( 2 ) << timeinfo->tm_mon + 1 << "-" << std::setw( 2 )
                << timeinfo->tm_mday << "</date>" << std::endl;
        xmlfile << "  <time>" << std::setw( 2 ) << timeinfo->tm_hour << ":"
                << std::setw( 2 ) << timeinfo->tm_min << ":" << std::setw( 2 )
                << timeinfo->tm_sec << "</time>" << std::endl;

        xmlfile << "  <numProcs>" << opts.mpinumprocs_string << "</numProcs>" << std::endl;
        xmlfile << "  <numThreads>" << opts.ompnumthreads_string << "</numThreads>" << std::endl;

        // Wording directory for the experiment: can be used to find the data file(s)
        char* cwd = getenv( "PWD" );
        xmlfile << "  <dir>" << cwd << "</dir>" << std::endl;
        // Source revision
        if( opts.has_srcrev ) {
            // Source revision
            xmlfile << "  <rev>" << opts.srcrev << "</rev>" << std::endl;
        }
        // Empty line before the properties
        xmlfile << std::endl;

        for( int i = 0; i < metaproperties_.size(); i++ ) {
            string            prop_string = ( metaproperties_[ i ] ).toString();
            string::size_type p_loc       = prop_string.find( ":::::", 0 );
            // Exclude internal properties used to calculate the execution time for the scalability analysis
            if( p_loc != string::npos ) {
                ;
            }
            else {
                xmlfile << ( metaproperties_[ i ] ).toXML();
            }
        }
        xmlfile << "</Experiment>" << std::endl;
    }
    xmlfile.close();
}

/**
 * @brief Checks all child agents for new properties
 *
 * Triggers connection to all lower level agents and
 * checks if new properties are available.
 */
void PeriscopeFrontend::check() {
    std::map<std::string, AgentInfo>::iterator it;

    for( it = child_agents_.begin(); it != child_agents_.end(); it++ ) {
        AgentInfo& ag = it->second;

        if( ag.status != AgentInfo::CONNECTED ) {
            if( connect_to_child( &ag ) == -1 ) {
                psc_errmsg( "Error connecting to child\n" );
                abort();
            }
            else {
                psc_dbgmsg( 1, "Checking properties...\n" );
                ag.handler->check();
            }
        }
    }
}

/**
 * @brief Checks all child agents for call-tree
 *
 * Triggers connection to all lower level agents and
 * checks if call-tree is available from any of the agents
 */

void PeriscopeFrontend::request_calltree() {
    std::map<std::string, AgentInfo>::iterator it;
    for( it = child_agents_.begin(); it != child_agents_.end(); it++ ) {
        AgentInfo& ag = it->second;
        psc_dbgmsg( 1, "Checking for call-tree nodes...\n" );
        ag.handler->serializecalltree();
    }
}


int PeriscopeFrontend::read_line( ACE_HANDLE hdle,
                                  char*      buf,
                                  int        len ) {
    int eoln = 0;

    while( !eoln && ( len > 0 ) && ( ACE::recv( hdle, buf, 1 ) == 1 ) ) {
        eoln = ( *buf == '\n' );
        buf++;
        len--;
    }

    return eoln;
}

/**
 * @deprecated Properties are transmitted/processed using XML
 * @brief Prints the found performance property
 *
 * This method is called whenever a new performance property is found
 *
 * @param prop    performance property to print
 */
void PeriscopeFrontend::found_property( PropertyInfo& prop ) {
    std::cout << prop.name << "\t" << prop.context << "\t" << prop.host << "\t"
              << prop.severity << std::endl;
    prompt();
}

/**
 * @brief Processes the found performance property
 *
 * This method is called whenever a new performance property is found.
 * This new property is added to a list containing all properties.
 *
 * @param propData    performance property (as string)
 */
void PeriscopeFrontend::found_property( const std::string& propData ) {
    metaproperties_.push_back( MetaProperty::fromXMLDeserialize( propData ) );
}

/**
 * @brief Processes the received call-tree node
 *
 * This method is called whenever a new call-tree node is sent from the aagent
 * The node is sent as an argument for constructing the call-tree in the frontend
 *
 * @param calltreeData    Serialized call-tree node (as XML string)
 */
void PeriscopeFrontend::construct_calltree( std::string& calltreeData ) {
    //Deserialization of the call-tree node
    Rts* current( Rts::fromXMLdata( calltreeData ) );
    appl->construct_frontend_calltree( current );
}

void PeriscopeFrontend::set_timer( int         init,
                                   int         inter,
                                   int         max,
                                   TimerAction ta ) {
    ACE_Reactor*   reactor = ACE_Event_Handler::reactor();
    ACE_Time_Value initial( init );
    ACE_Time_Value interval( inter );

    reactor->schedule_timer( this, 0, initial, interval );

    ACE_Time_Value timeout = ACE_OS::gettimeofday();
    timeout += max;
    set_timeout( timeout );

    timer_action = ta;
}

std::string region2string( int fid, int rfl ) {
    std::stringstream info;
    std::string       str;

    info << ":" << fid << ":" << rfl << ":";
    str = info.str();
    return str;
}

bool applUninstrumented() {
    if( opts.has_uninstrumented ) {
        return true;
    }
    if( psc_config_open() ) {
        if( psc_config_uninstrumented() ) {
            return true;
        }
        psc_config_close();
    }
    if( getenv( "PSC_UNINSTRUMENTED" ) ) {
        return true;
    }
    return false;
}
