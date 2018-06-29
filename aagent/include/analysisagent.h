#ifndef MRI_ANALYSISAGENT_H_INCLUDED
#define MRI_ANALYSISAGENT_H_INCLUDED

#include "config.h"
#include "global.h"
#include "Property.h"
#include "strategy.h"
#include "experiment.h"
#include "accl_mrinodeagent_handler.h"
#include "psc_agent.h"
#include "DataProvider.h"
#include "StrategyRequest.h"

#include <ace/Reactor.h>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <iostream>
#include <map>
#include <utility>




/// Default file to dump the properties
#define PROP_FILE "properties.psc"

/// Command line options
struct cmdline_opts {
    bool has_showhelp;
    bool has_appname;
    bool has_mpinumprocs;
    bool has_ompnumthreads;
    bool has_strategy;
    bool has_plugin;
    bool has_phase;
    bool has_debug;
    bool has_selectivedebug;
    bool has_registry;
    bool has_desired_port;
    bool has_parent;
    bool has_leaf;
    bool has_timeout;
    bool has_searches;
    bool has_delay;
    bool has_iterations;
    bool has_fastmode;
    bool has_dontregister;
    bool has_parent_host;
    bool has_parent_port;
    bool has_id;
    bool has_propfile;
    bool has_property;
    bool has_pedantic;

//TODO: Revise unused stuff. tag seems to be used only by HL and AA. -RM
    bool has_tag;
    bool has_devControl;
    bool has_inst;
    bool has_srcrev;        ///< Source revision is specified
    bool has_configurationfile; // configuration xml file




    char  appname_string[ 2000 ];
    char  mpinumprocs_string[ 20 ];
    char  ompnumthreads_string[ 20 ];
    char  strategy[ 2000 ];
    char  phase_string[ 2000 ];
    char  debug_string[ 2000 ];
    char  selectivedebug_string[ 4000 ];
    char  reg_string[ 2000 ];
    char  port_string[ 2000 ];
    char  parent_string[ 2000 ];
    char  timeout_string[ 2000 ];
    char  searches[ 2000 ];
    char  delay_string[ 2000 ];
    char  iterations_string[ 2000 ];
    char  parent_host[ 4000 ];
    int   parent_port;
    char* id_string;
    char  prop_file[ 2000 ];
    char  property[ 2000 ];
//TODO: Revise unused stuff. tag seems to be used only by HL and AA. -RM
    char tag_string[ 2000 ];
//  char threads[2000];
    char srcrev[ 20 ];                  ///< Source revision
    char inst[ 100 ];
    char configfile_string[ 2000 ]; // name of the configuration xml file
    char plugin[ 2000 ];

    cmdline_opts() {
        has_showhelp       = false;
        has_appname        = false;
        has_mpinumprocs    = false;
        has_ompnumthreads  = false;
        has_strategy       = false;
        has_plugin         = false;
        has_phase          = false;
        has_debug          = false;
        has_selectivedebug = false;
        has_registry       = false;
        has_desired_port   = false;
        has_parent         = false;
        has_leaf           = false;
        has_timeout        = false;
        has_searches       = false;
        has_delay          = false;
        has_iterations     = false;
        has_fastmode       = false;
        has_dontregister   = false;
        has_parent_host    = false;
        has_parent_port    = false;
        has_id             = false;
        has_propfile       = false;
        has_property       = false;
        has_pedantic       = false;
//TODO: Revise unused stuff. tag seems to be used only by HL and AA. -RM
        has_tag        = false;
        has_devControl = false;
//    has_threads = false;
        has_inst   = false;
        has_srcrev = false;
        has_configurationfile=false; // configuration xml file

    }
};

extern struct cmdline_opts opts;

typedef int req_id_t;

typedef double severity_t; ///< Property Severity Type


/**
 * @class AnalysisAgent
 * @ingroup AnalysisAgent
 *
 * @brief The main class representing analysis agents
 *
 * @sa PeriscopeAgent
 *
 */
class AnalysisAgent : public PeriscopeAgent {
private:
    Prop_List candidate_properties_set;
    Prop_List evaluated_properties_set;
    Prop_List complete_list;

    Strategy* strategy;

    Experiment* current_experiment;

    bool doneappexit;
    bool started;
    bool leader;
    int  delay;
    int  iterations_per_experiment;

    // RM: Serialization
    serialized_property_list_container_t propertyListContainer;
    std::vector<char>                    propertyListBuffer;

    void appropriate_request();

public:
    AnalysisAgent( ACE_Reactor* r );

    AnalysisAgent( Strategy*    st,
                   ACE_Reactor* r );

    ~AnalysisAgent();


    void set_parent_host( char* host ) {
        strcpy( parent_host, host );
    }

    void set_parent_port( int port ) {
        parent_port = port;
    }

    void run();

    std::list< Property* >get_results();

    std::list< Property* >get_candidate_set() {
        return candidate_properties_set;
    }

    void terminate_analysis() {
        parent_handler_->searchfinished( this->get_local_tag() );
    }

    void set_candidate_properties_set( Prop_List list ) {
        candidate_properties_set = list;
    }

    Strategy* get_strategy() {
        return strategy;
    }

    void set_strategy( Strategy* s ) {
        strategy = s;
    }

    void set_strategy( StrategyRequest* strategyRequest );

    RegistryService* get_registry();

    DataProvider* get_provider();

    void stop() {
        reactor_->end_reactor_event_loop();
#ifdef __p575
        regsrv_->delete_entry( regid_ );
        //psc_dbgmsg(5, "###################stop(): after reactor_->end_reactor_event_loop();\n");
#endif
    }

    /**
     * Add our own data to the registry
     */
    int register_self();

    /**
     * set the 'ready' flag on all RIPs
     */
    void set_ready();

    bool get_started() const {
        return started;
    }

    void set_leader() {
        leader = true;
    }

    bool get_leader() const {
        return leader;
    }

    void set_delay( int d ) {
        delay = d;
    }

    int get_delay() const {
        return delay;
    }

    /** Sets the number of iterations to be performed for each experiment. */
    void set_num_iterations( int iterations );

    /**
     * Instantiate the appropriate protocol handler
     */
    ACCL_Handler* create_protocol_handler( ACE_SOCK_Stream& peer );

    void on_search_finished();

    void stand_alone_search();

    void start_experiment();

    bool evaluate_experiment();

    void reinit_providers( int maplen, int idmap_from[ 8192 ], int idmap_to[ 8192 ] );

    /**
     * Export the set of found(evaluated) properties to a file
     */
    void export_property_set( char* fileName );

    /**
     * Print a single property in formated way to stdout
     */
    void print_property( Property* p,
                         bool      with_severity,
                         bool      with_add_info );

    /**
     * Print a property set
     */
    void print_property_set( std::list <Property*> property_set,
                             const char*           str,
                             bool                  with_severity,
                             bool                  with_add_info );

    /**
     *  Compare the results of 2 runs and print the MISSING and ADDITIONAL properties
     */
    void compareResults( Prop_List newResults,
                         Prop_List prevResults,
                         int       run );

    int handle_step();
};

/**
 * @class EventHandling
 * @ingroup AnalysisAgent
 *
 * @brief Communication event handler
 *
 */
class EventHandling : public ACE_Event_Handler {
private:
    AnalysisAgent* nagent_;
public:
    EventHandling( AnalysisAgent* agent ) {
        nagent_ = agent;
        reactor( nagent_->get_reactor() );

        if( !agent->get_fastmode() ) {      // timer based ACE
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ), "setting handler WITH timers\n" );
            ACE_Time_Value initial( 5 );
            ACE_Time_Value interval( 2 );
            reactor()->schedule_timer( this, 0, initial, interval );
        }
        else {           // fast timer-less ACE
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ), "setting handler without timers\n" );
            reactor()->register_handler( this, ACE_Event_Handler::READ_MASK );
        }
    }

    void evh_cleanup() {
        reactor()->cancel_timer( this );
    }

    int handle_timeout( const ACE_Time_Value& curr_time,
                        const void*           arg );
};


extern AnalysisAgent* agent;

#endif // MRI_ANALYSISAGENT_H_INCLUDED
