/**
   @file    frontend.h
   @ingroup Frontend
   @brief   Front-end agent header
   @author  Karl Fuerlinger
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

#ifndef FRONTEND_H_INCLUDED
#define FRONTEND_H_INCLUDED

#include "psc_agent.h"
#include "PropertyID.h"
#include "MetaProperty.h"
#include "selective_debug.h"
#include "ApplicationStarter.h"
#include "Scenario.h"
#include "StrategyRequest.h"
#include "ScenarioPoolSet.h"
#include "DriverContext.h"
#include "rts.h"
#include <ace/SOCK_Stream.h>
#include <ace/Reactor.h>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <map>
#include <list>
#include <queue>

#define stringify( name ) # name
#define xstr( s ) stringify( s )

//TODO: Revise and go with selective debug levels and debug levels -RM
#define FRONTEND_MSM_DEBUG_LEVEL PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines )
#define FRONTEND_GENERAL_DEBUG_LEVEL 2
#define FRONTEND_STARTER_DEBUG_LEVEL 3
#define FRONTEND_MEDIUM_DEBUG_LEVEL 4
#define FRONTEND_HIGH_DEBUG_LEVEL 5
#define FRONTEND_MAX_DEBUG_LEVEL 7

// Create an high level agents group
/// @defgroup Frontend Frontend

#define PROP_FILE       "properties.psc"

/// Command line options
struct cmdline_opts {
    int has_showhelp;
    int has_appname;
    int has_apprun;
    int has_mpinumprocs;
    int has_ompnumthreads;
    int has_strategy;
//RM: Not sure does it work or do we need it?
    int has_scalability_OMP;

    int  has_plugin;
    int  has_phase;
    bool has_uninstrumented;
//RM: Not sure can we delete it?
    bool has_fastmode;

    int has_force_localhost;
    int has_debug;
    int has_selectivedebug;
    int sm_tracing_enabled;
    int has_registry;
    int has_desired_port;
    int has_maxfan;
    int has_maxcluster;
    int has_maxthreads;

    int has_timeout;
    int has_delay;
    int has_iterations;
    int has_duration;
    int has_masterhost;
    int has_hlagenthosts;
    int has_nodeagenthosts;
    int has_agenthostfile;

    int  has_dontcluster;
    int  has_manual;
    int  has_property;
    int  has_propfile;
    int  has_nrprops;
    bool has_pedantic;

//TODO: Should be cleaned up. -RM
//    int  has_threads;
    int has_bg_mode;
    int has_devControl;
    int has_make;
    int has_config;
    int has_inst;
    int has_instfolder;
    int has_sourcefolder;
    int has_srcrev;             ///< Source revision is specified
////////////////////////////////////////

    int has_cfs_config;
    int has_configurationfile; // configuration xml file
    int has_input_desc;

    char appname_string[ 2000 ];
    char app_run_string[ 2000 ];
    char mpinumprocs_string[ 2000 ];
    char ompnumthreads_string[ 2000 ];
    char strategy[ 2000 ];
    char plugin[ 2000 ];
    char phase_string[ 2000 ];
    char debug_string[ 2000 ];
    char selectivedebug_string[ 4000 ];
    char reg_string[ 2000 ];
    char port_string[ 2000 ];
    char maxfan_string[ 2000 ];
    char maxcluster_string[ 2000 ];
    char maxthreads_string[ 2000 ];
    char timeout_string[ 2000 ];
    char delay_string[ 2000 ];
    char iterations_string[ 2000 ];
    char duration_string[ 2000 ];
    char masterhost_string[ 2000 ];
    char hlagenthosts_string[ 2000 ];
    char nodeagenthosts_string[ 2000 ];
    char agenthostfile_string[ 2000 ];
    char property[ 2000 ];
    char prop_file[ 2000 ];
    int  nrprops;

//TODO: Should be cleaned up. -RM
//    char threads[ 2000 ];
    char bg_mode[ 5 ];
    char make_string[ 2000 ];
    char config_string[ 2000 ];
    char inst_string[ 100 ];
    char instfolder_string[ 2000 ];
    char sourcefolder_string[ 2000 ];
    char srcrev[ 20 ];                   ///< Source revision
////////////////////////////////////////
    char cfs_config_file_string[ 2000 ];
    char configfile_string[ 2000 ]; // name of the configuration xml file
    char input_desc_string[ 2000 ]; // name of the input identifier specification file

    cmdline_opts() {
        has_showhelp          = 0;
        has_appname           = 0;
        has_apprun            = 0;
        has_mpinumprocs       = 0;
        has_ompnumthreads     = 0;
        has_strategy          = 0;
        has_scalability_OMP   = 0;
        has_plugin            = 0;
        has_phase             = 0;
        has_uninstrumented    = false;
        has_fastmode          = false;

        has_force_localhost   = 1;
        has_debug             = 0;
        has_selectivedebug    = 0;
        sm_tracing_enabled    = 0;
        has_registry          = 0;
        has_desired_port      = 0;
        has_maxfan            = 0;
        has_maxcluster        = 0;
        has_maxthreads        = 0;

        has_timeout           = 0;
        has_delay             = 0;
        has_iterations        = 0;
        has_masterhost        = 0;
        has_hlagenthosts      = 0;
        has_nodeagenthosts    = 0;
        has_agenthostfile     = 0;

        has_dontcluster       = 0;
        has_manual            = 0;
        has_property          = 0;
        has_propfile          = 0;
        has_pedantic          = false;

//TODO: Should be cleaned up. -RM
        has_bg_mode           = 0;
        has_devControl        = 0;
        has_make              = 0;
        has_config            = 0;
        has_inst              = 0;
        has_instfolder        = 0;
        has_sourcefolder      = 0;
        has_srcrev            = 0;
////////////////////////////////////////
        has_cfs_config        = 0;
        has_configurationfile = 0;
        has_input_desc        = 0;
    }
};

extern struct cmdline_opts opts;
extern ApplicationStarter* starter;


/**
 * @class PeriscopeFrontend
 * @ingroup Frontend
 *
 * @brief Periscope frontend
 *
 * This class extends the ACE_Event_Handler and implements
 * the frontend agent
 *
 * @sa PeriscopeAgent
 */
class PeriscopeFrontend : public PeriscopeAgent, public ACE_Event_Handler {
public:
    enum TimerAction {
        STARTUP, STARTUP_REINIT, CHECK, SHUTDOWN, AGENT_STARTUP, APPLICATION_TERMINATION, REQUESTCALLTREE
    };


    const std::unique_ptr<DriverContext>   plugin_context;
    const std::unique_ptr<DriverContext>   fe_context;
    const std::unique_ptr<ScenarioPoolSet> frontend_pool_set;

    int          initial_argc;
    char**       initial_argv;
    list<string> plugin_argv_list;

    char psc_reg_host[ 2000 ];
    int  psc_reg_port;

    int  psc_port;
    char psc_host[ 2000 ];

private:
    void run_analysis( ACE_Reactor* reactor );

    void run_tuning_plugin( ACE_Reactor* reactor );

    bool        automatic_mode;
    bool        need_restart;
    bool        shutdown_allowed;
    int         interval;
    char        masteragent_cmd_string[ 65536 ];
    char        app_name_string[ 2000 ];
    char        app_cmd_line_string[ 8192 ];
    char        master_host_string[ 8192 ];
    std::string outfilename_string;
    int         phaseFileId, phaseRFL;

    int         ompfinalthreads_val; //sss
    int         iter_count;          //sss
    int         ompnumthreads_val;
    int         mpinumprocs_val;
    int         maxcluster_val;
    int         maxfan;
    bool        quit_fe;
    double      startup_time;
    bool        badRegionsRemoved;
    bool        allRegionsCommented;
    TimerAction timer_action;
    bool        tuning_plugin_executed;

    std::list<std::string>                 badRegions;
    std::list<std::string>                 requiredRegionsList;
    std::string                            RequiredRegions;
    std::list<std::string>                 requiredRegionsListLastRun;
    std::list<StrategyRequestGeneralInfo*> strategy_request_general_info_queue;     ///< list of analysis strategies to be executed one after another
    std::list<StrategyRequest*>            strategy_request_queue;


    //BGP Port V1 specific variables
    int  total_agents_number;           ///< total number of agents in hierarchy
    int  started_agents_count;          ///< number of already started agents
    int  ranks_started;                 ///< number of connected application processes
    bool agent_hierarchy_started;

    // RM: Serialization
    serialized_strategy_request_container_t strategyRequestContainer;
    std::vector<char>                       strategyRequestBuffer;

public:
    // list of discovered properties
    std::list<PropertyInfo> properties_;
    std::list<PropertyInfo> property_threads;
    std::list<PropertyInfo> p_threads;
    std::list<PropertyInfo> existing_properties;
    std::list<PropertyInfo> property;

    std::vector<MetaProperty> metaproperties_;
    std::vector<Rts*>         rts_list_;

    std::list<PropertyInfo> severity_based_properties;
    std::list<PropertyInfo> execTime_based_properties;
    std::list<PropertyInfo> speedup_based_properties;
    std::list<PropertyInfo> ExecAndDeviaProperties;
    std::list<PropertyInfo> DeviationProperties;
    std::list<long double>  PhaseTime_user;

public:
    PeriscopeFrontend( ACE_Reactor* r );

    void terminate_autotune();

    void terminate_agent_hierarchy();

    bool get_agent_hierarchy_started() {
        return agent_hierarchy_started;
    }

    void set_agent_hierarchy_started( bool started ) {
        agent_hierarchy_started = started;
    }

    TimerAction getTimerAction() {
        return timer_action;
    }

    double calculate_severity( double* severity_threads,
                               int     thread_number );

    void push_StrategyRequestGeneralInfo2Queue( const std::string& strategy_name,
                                                bool               pedantic,
                                                int                analysis_duration,
                                                int                delay_phases,
                                                int                delay_seconds );

    void push_StrategyRequestGeneralInfo2Queue( StrategyRequestGeneralInfo* strategyRequestGeneralInfo ) {
        strategy_request_general_info_queue.push_back( strategyRequestGeneralInfo );
    }

    StrategyRequestGeneralInfo* pop_StrategyRequestGeneralInfoFromQueue() {
        StrategyRequestGeneralInfo* strategyRequestGeneralInfo = strategy_request_general_info_queue.front();
        strategy_request_general_info_queue.pop_front();
        return strategyRequestGeneralInfo;
    }

    int size_StrategyRequestGeneralInfoQueue() {
        return strategy_request_general_info_queue.size();
    }

    void print_StrategyRequestGeneralInfoQueue() {
        if( psc_get_debug_level() < 6 ) {
            return;
        }
        psc_dbgmsg( 1, "####################################\n" );
        psc_dbgmsg( 1, "Requested analysis scenarios:\n\n" );
        for( std::list<StrategyRequestGeneralInfo*>::iterator it = strategy_request_general_info_queue.begin(); it != strategy_request_general_info_queue.end(); it++ ) {
            psc_dbgmsg( 1, "Strategy name: %s, analysis duration %d(phases), analysis delay %d(phases):%d(seconds)\n",
                        ( *it )->strategy_name.c_str(),
                        ( *it )->analysis_duration,
                        ( *it )->delay_phases,
                        ( *it )->delay_seconds );
        }
        psc_dbgmsg( 1, "####################################\n" );
    }

    void push_StrategyRequest2Queue( StrategyRequest* strategyRequest ) {
        strategy_request_queue.push_back( strategyRequest );
    }

    StrategyRequest* pop_StrategyRequestFromQueue() {
        StrategyRequest* strategyRequest = strategy_request_queue.front();
        strategy_request_queue.pop_front();
        return strategyRequest;
    }

    void print_StrategyRequestQueue();

    void serializeStrategyRequests( StrategyRequest* strategyRequest ) {
        namespace io = boost::iostreams;

        io::stream<io::back_insert_device<std::vector<char> > > serializedStrategyRequestStream( strategyRequestBuffer );
        boost::archive::binary_oarchive                         oaStrategyRequest( serializedStrategyRequestStream );

        oaStrategyRequest << strategyRequest;
        serializedStrategyRequestStream.flush();
    }

    std::vector<char> get_SerializedStrategyRequestBuffer() {
        return strategyRequestBuffer;
    }

    void init_analysis_strategy_requests();

    void set_badRegionsRemoved( bool set ) {
        badRegionsRemoved = set;
    }

    bool get_badRegionsRemoved() {
        return badRegionsRemoved;
    }

    void set_allRegionsCommented( bool set ) {
        allRegionsCommented = set;
    }

    bool get_allRegionsCommented() {
        return allRegionsCommented;
    }

    void add_badRegion( const std::string& region ) {
        badRegions.push_back( region );
    }

    std::list<std::string> get_badRegions() {
        return badRegions;
    }

    void print_badRegions() {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "List of too fine granular regions\n" );

        for( std::list<std::string>::iterator it = badRegions.begin(); it != badRegions.end(); it++ ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "%s\n", ( *it ).c_str() );
        }
    }


    bool is_in_badRegions( const std::string& region ) {
        for( std::list<std::string>::iterator it = badRegions.begin(); it != badRegions.end(); it++ ) {
            //printf("Bad regions: %s?%s\n",region.c_str(), (*it).c_str());
            if( ( *it ).compare( region ) == 0 ) {
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "Region %s found in bad regions list\n", region.c_str() );
                return true;
            }
            if( ( *it ).compare( region ) > 0 ) {
                return false;
            }
        }
        return false;
    }

    void set_RequiredRegions( const std::string& str ) {
        RequiredRegions.clear();
        RequiredRegions = str;
    }

    const std::string& get_RequiredRegions() {
        return RequiredRegions;
    }

    void add_requiredRegionsList( const std::string& region ) {
        requiredRegionsList.push_back( region );
    }

    void rm_duplicates_requiredRegionsList() {
        requiredRegionsList.sort();
        requiredRegionsList.unique();
    }

    const std::list<std::string>& get_requiredRegionsList() {
        return requiredRegionsList;
    }

    bool checkForReinstrumentation() {
        if( !opts.has_inst ) {
            return false;
        }

        if( get_badRegions().size() > 0 && !get_badRegionsRemoved() && ( strcmp( opts.inst_string, "overhead" ) == 0 || strcmp( opts.inst_string, "all_overhead" ) == 0 ) ) {
            return true;
        }

        if( strcmp( opts.inst_string, "analyse" ) == 0 && !get_allRegionsCommented() ) {
            return true;
        }

        bool newInstrumentationRequired = false;

        for( std::list<std::string>::iterator it = requiredRegionsList.begin(); it != requiredRegionsList.end(); it++ ) {
            std::string region = *it;
            std::string filename, tmp, rfl;

            if( is_in_badRegions( region ) ) {
                continue;                 //This region would have too much overhead.
            }
            if( is_in_requiredRegionsListLastRun( region ) ) {
                continue;                 //This region is still instrumented from last experiment
            }
            newInstrumentationRequired = true;
            break;
        }
        return newInstrumentationRequired;
    }

    void clear_requiredRegionsList() {
        requiredRegionsList.clear();
    }

    void print_requiredRegionsList() {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "List of regions required in next experiment\n" );

        for( std::list<std::string>::iterator it = requiredRegionsList.begin(); it != requiredRegionsList.end(); it++ ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "%s\n", ( *it ).c_str() );
        }
    }

    void print_requiredRegionsListLastRun() {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "List of regions required in last experiment\n" );

        for( std::list<std::string>::iterator it = requiredRegionsListLastRun.begin(); it != requiredRegionsListLastRun.end(); it++ ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "%s\n", ( *it ).c_str() );
        }
    }


    void copy_requiredRegionsToLastRun() {
        requiredRegionsListLastRun.clear();
        for( std::list<std::string>::iterator it = requiredRegionsList.begin(); it != requiredRegionsList.end(); it++ ) {
            requiredRegionsListLastRun.push_back( *it );
        }
    }

    std::list <std::string> get_requiredRegionsListLastRun() {
        return requiredRegionsListLastRun;
    }

    bool is_in_requiredRegionsListLastRun( const std::string& region ) {
        for( std::list<std::string>::iterator it = requiredRegionsListLastRun.begin(); it != requiredRegionsListLastRun.end(); it++ ) {
            if( ( *it ).compare( region ) == 0 ) {
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "Region %s found in list of instrumented regions in last run\n", region.c_str() );
                return true;
            }
            if( ( *it ).compare( region ) > 0 ) {
                return false;
            }
        }
        return false;
    }



    void set_startup_time( double val ) {
        startup_time = val;
    }

    double get_startup_time() {
        return startup_time;
    }

    void set_automatic( bool val ) {
        automatic_mode = val;
    }

    bool get_automatic() {
        return automatic_mode;
    }

    void set_need_restart( bool val ) {
        need_restart = val;
    }

    bool get_need_restart() {
        return need_restart;
    }

    void set_shutdown_allowed( bool sdallowed ) {
        shutdown_allowed = sdallowed;
    }

    bool get_shutdown_allowed() {
        return shutdown_allowed;
    }

    void set_maxfan( int val ) {
        maxfan = val;
    }

    int get_maxfan() {
        return maxfan;
    }

    int get_ompnumthreads() {
        return ompnumthreads_val;
    }

    const std::string& get_outfilename() {
        return outfilename_string;
    }

    int get_maxiterations() {
        return iter_count;
    }

    int get_ompfinalthreads() {
        return ompfinalthreads_val;
    }

    int get_mpinumprocs() {
        return mpinumprocs_val;
    }

    int get_maxcluster() {
        return maxcluster_val;
    }

    char* get_appname() {
        return app_name_string;
    }

    void set_appname( char* n ) {
        strcpy( app_name_string, n );
    }

    void add_started_agent();

    // BGP Port Heartbeat V1 implementation specific methods used to count started agents in the hierarchy.
    // In this implementation AAgents randomly choose their parent agents, therefore exact hierarchy computation
    // by starter is not possible. But nevertheless the total amount of agents is known to frontend. Therefore the heartbeat of
    // each agent is forwarded up to frontend which count them. When frontend receives all the heartbeats it issues start command.

    void add_started_agent( int num_procs ); // increases the number of connected appl procs and if it equals to the total amount issues start command

    void remove_started_agent();             // decrease the number of started agents

    int get_agents_number() {   // returns the expected amount of agents in hierarchy
        return total_agents_number;
    }

    void set_agents_number( int value ) {       // sets the expected amount of agents in hierarchy
        total_agents_number = value;
        psc_dbgmsg( 5, "Total amount of agents is set to %d\n", total_agents_number );
    }
    // end of BGP Port Heartbeat V1 implementation specific methods

    // register with registry service
    int register_self();

    void reinit_providers( int maplen,
                           int idmap_from[ 8192 ],
                           int idmap_to[ 8192 ] ) {
    };

    void reinit( int map_len,
                 int map_from[ 8192 ],
                 int map_to[ 8192 ] );

    void register_master_agent( char* ma_cmd_string );

    void register_app_name( char* aname_string );

    void register_app_cmd_line( char* acmdline_string );

    void register_master_host( char* mhost_string );

    void register_reg_host( char* rhost_string );

    void register_reg_port( int reg_port );

    void set_outfilename( const std::string& outfn_string );          ///< Print the results to this file

    void set_ompnumthreads( int ompnt_val );

    void set_ompfinalthreads( int ompfinal_val );  //sss

    void set_maxiterations( int iter_val );        //sss

    void set_mpinumprocs( int mpinp_val );

    void set_maxcluster( int mcluster_val );

    // create a suitable handler for the ACCL protocol
    ACCL_Handler* create_protocol_handler( ACE_SOCK_Stream& peer );

    void run();

    void stop();

    int handle_input( ACE_HANDLE hdle );

    int handle_signal( int signum,
                       siginfo_t*,
                       ucontext_t* );

    void handle_command( const std::string& line );

    int handle_timeout( const ACE_Time_Value& curr_time,
                        const void*           arg );

    int handle_step( TimerAction );

    void set_timer( int         initial,
                    int         interval,
                    int         max,
                    TimerAction ta );

    int read_line( ACE_HANDLE hdle,
                   char*      str,
                   int        maxlen );

    void prompt();

    //
    // interactive commands
    //
    void graph();

    /** Sends all strategy requests that have been pushed so far to the analysis agents. */
    void start();

    void help();

    void quit();

    void properties();

    void check();

    void export_properties();                ///< Write the results to a file

    void export_scalability_properties();

    void get_prop_info( MetaProperty& prop );

    void do_speedup_analysis( PropertyInfo       prop,
                              long double        seq_time,
                              const std::string& phase_region );

    void find_existing_properties();

    void copy_properties_for_analysis();

    long double find_sequential_time( PropertyInfo prop );

    void do_pre_analysis();

    void Check_severity_among_parallel_regions( int config );

    std::string convertInt( int number );

    PropertyID Property_with_increasing_severity_across_configurations_id();

    PropertyID SuperLinear_Speedup_for_all_configurations_id();

    PropertyID Linear_Speedup_for_all_configurations_id();

    PropertyID Linear_speedup_failed_for_the_first_time_id();

    PropertyID Low_Speedup_id();

    PropertyID Speedup_Decreasing_id();

    PropertyID Sequential_Computation_id();

    PropertyID Code_region_with_the_lowest_speedup_in_the_run_id();

    PropertyID Property_occurring_in_all_configurations_id();

    //
    // called when a property was found
    //
    void found_property( PropertyInfo& info );

    void found_property( const std::string& propData );


    //Checks all child agents for call-tree
    void request_calltree();

    void stop_agents_for_calltree();

    //Deserialization
    void construct_calltree( std::string& calltreeData );

    //Set to true after the tuning plugin finishes execution, so that the tuning model can be generated
    void plugin_executed(bool plugin_executed) {
        tuning_plugin_executed = plugin_executed;
    }

    bool executed_tuning_plugin() {
        return tuning_plugin_executed;
    }

};

std::ostream& operator<<( std::ostream&       os,
                          const PropertyInfo& info );

std::string region2string( int fid,
                           int rfl );

bool applUninstrumented();

extern PeriscopeFrontend* fe;

#endif /* FRONTEND_H_INCLUDED */
