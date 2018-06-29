/**
   @file    psc_agent.h
   @ingroup Communication
   @brief   Communication agent header
   @author	Karl Fuerlinger
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

#ifndef PSC_AGENT_H_
#define PSC_AGENT_H_


#include <map>
#include <string>

#include "regxx.h"
#include "peer_acceptor.h"
#include "timing.h"

struct PropertyInfo {
    std::string name;
    std::string context;
    std::string host;
    std::string filename;
    std::string region;
    std::string cluster;
    std::string ID;
    std::string addInfo;
    std::string RegionId;
    std::string Config;
    long double execTime;
    int         fileid;
    double      RFL;
    double      process;
    double      severity;
    double      confidence;
    int         maxThreads;
    int         maxProcs;
    int         purpose;
    long double exec_regions[ 30 ];
    long double devia_regions[ 30 ];
    bool        done;
    int         numthreads;

    PropertyInfo() {
        done       = false;
        numthreads = 1;
    }
};

/// @brief Internal information about the analysis agent
struct AgentInfo {
    /// Possible states of the agent
    enum Status {
        INITIAL,  ///< Initializing
        STARTED,  ///< Running
        CONNECTED ///< Connected to the peer
    };

    /// Possible states of the search
    enum SearchStatus {
        REQRESTART,       ///< Application terminated but additional experiment required
        REQEXPERIMENT,    ///< Application suspended and experiment required
        FINISHED,         ///< Search terminated
        UNDEFINED,        ///< No status information after starting the experiment
    };


    std::string  tag;                 ///< TAG
    std::string  parent;              ///< Tag of parent agent
    std::string  hostname;            ///< Hostname where the agent is running
    int          port;                ///< Port on which it is running
    Status       status;              ///< Current state of the agent
    Status       status_reinit;
    bool         properties_sent;     ///< Indicates that properties were sent
    bool         appl_terminated;     ///< Indicates that the application was terminated by analysis agent
    SearchStatus search_status;       ///< Stores the request for next search step
    bool calltree_sent;               ///< Indicates that the call-tree was sent
    
    ACCL_Handler*    handler;         ///< Communication handler used
    ACE_SOCK_Stream* stream;          ///< Stream used for the communication

    AgentInfo() {
        port          = -1;
        tag           = "";
        parent        = "";
        hostname      = "";
        status        = INITIAL;
        status_reinit = INITIAL;
        handler       = 0;
        stream        = 0;
        search_status = UNDEFINED;
    }
};

/**
 * @class PeriscopeAgent
 * @ingroup AnalysisAgent
 *
 * @brief The base class representing a Periscope agent
 *
 */
class PeriscopeAgent //EK : public Agent
{
protected:
    int            parent_port;
    char           parent_host[ 1000 ];
    bool           fastmode;
    int            timeout_delta_;
    ACE_Time_Value timeout_;
    int            global_timeout_; //Gerndt added to support global timeout via psc_wall_time()


    /// set of child agents for this agent (none for node-agents)
    std::map< std::string, AgentInfo > child_agents_;


    /**
     * @brief Accepts connections from other peers
     *
     * Creates a #PeerConnection objects for each new connection
     */
    PeerAcceptor acceptor_;

    /**
     * @brief Registry service used
     *
     * The agent is using this registry service to register
     * itself and to locate peer agents.
     */
    RegistryService* regsrv_;

    int              regid_;            ///< Agent's own id in the registry
    std::string      machinename_;      ///< Machine name where the agent is running
    std::string      sitename_;         ///< Site name
    std::string      appname_;          ///< Registered name of the analyzed application
    std::list< int > threads_;          ///< List of threads used

    //
    // we make a copy of the reactor
    //
    ACE_Reactor* reactor_;

    /**
     * Record the handles to the dynamic objects so we can close them
     * at the end of the experiment.
     */
    std::list< void* > dynlibs_;

    ACE_SOCK_Stream* parent_stream_;

public:
    // TODO make these protected again and make getters
    AgentInfo     own_info_; ///< information about the current agent
    ACCL_Handler* parent_handler_;

    PeriscopeAgent( ACE_Reactor* r ) {
        acceptor_.reactor( r );
        reactor_ = r;

        acceptor_.set_agent( this );
        regsrv_ = 0;
        regid_  = -1;

        parent_handler_ = 0;
        parent_stream_  = 0;

        appname_     = "(not set)";
        machinename_ = "(not set)";
        sitename_    = "(not set)";

        threads_.clear();

        timeout_delta_ = 20;
        fastmode       = false;
    }

    virtual ~PeriscopeAgent() {
        // TODO: unload shared libraries holding properties

        if( regid_ != -1 ) {
#ifndef __p575
            if( !fastmode ) {
                regsrv_->delete_entry( regid_ );
            }
#endif
        }
    }

    int get_parent_port() {
        return parent_port;
    }
    char* get_parent_host() {
        return parent_host;
    }
    bool get_fastmode() {
        return fastmode;
    }
    void set_fastmode( bool fast ) {
        fastmode = fast;
    }

    void reset_childagentlist() {
        child_agents_.clear();
    }

    void clear_appexit() {
    }

    /**
     * @brief Start the analysis agent on a specified port.
     *
     * Start the analysis agent on a specified port and
     * start accepting connections from other agents.
     *
     * Default port: 30000
     *
     * @param port Which port to listen to
     * @return
     */
    int open( int port = 30000 );

    /**
     * Returns the port number used by the analysis agent
     */
    int get_local_port();

    /**
     * Returns the name of the host where the analysis agent runs
     */
    std::string get_local_hostname();

    /**
     * Returns the agent's tag
     */
    std::string get_local_tag() {
        return own_info_.tag;
    }

    /**
     * Set a registry service to use
     */
    RegistryService* set_registry( RegistryService* svc );

    /**
     * Returns the used registry service
     */
    RegistryService* get_registry() {
        return regsrv_;
    };

    /**
     * Returns a handle to the ACE_Reactor used
     */
    ACE_Reactor* get_reactor() {
        return reactor_;
    }

    //void add_provider( DataProvider *prov )
    //{
    //    providers_.push_back( prov );
    //}

    virtual void
    reinit_providers( int maplen,
                      int idmap_from[ 8192 ],
                      int idmap_to[ 8192 ] ) = 0;

    /**
     * Register the agent with the Registry service
     */
    virtual int register_self() = 0;

    /**
     * @brief Create a new protocol handler for the ACCL protocol.
     *
     * Factory method to create a new protocol handler for the ACCL protocol
     * classes derived from PeriscopeAgent must overload this method as to
     * provide a suitable handler for the ACCL protocol.
     *
     * @param peer
     * @return Created handler
     */
    virtual ACCL_Handler* create_protocol_handler( ACE_SOCK_Stream& peer ) = 0;

    /// @todo: Load the properties
    void load_properties() {
    };

    /**
     * Connects to the parent process
     */
    int connect_to_parent();

    /**
     * Connects to a child process
     */
    int connect_to_child( AgentInfo* ag );

    /**
     * Adds a child agent with its tag
     */
    void add_child_agent( std::string tag,
                          std::string hostname,
                          int         port );

    /**
     * Returns a handler to the parent process
     */
    ACCL_Handler* get_parent_handler() {
        return parent_handler_;
    }

    void set_timeout_delta( int t ) {
        timeout_delta_ = t;
    }

    void set_global_timeout( int t ) {
        global_timeout_ = t;
    }

    void increment_global_timeout() {
        global_timeout_ += timeout_delta_;
    }

    double get_global_timeout() {
        return ( double )global_timeout_;
    }

    void set_parent( std::string parentTag ) {
        own_info_.parent = parentTag;
    }

    void set_timeout( ACE_Time_Value& tv ) {
        timeout_ = tv;
    }

    int timeout_delta() {
        return timeout_delta_;
    }

    bool timed_out() {
        return psc_wall_time() > ( double )global_timeout_;
    }

    /**
     * Returns a mapping of the child processes
     */
    std::map< std::string, AgentInfo >* get_child_agents() {
        return &child_agents_;
    }

    /**
     * Set agent's own tag
     */
    void set_own_tag( std::string tag ) {
        own_info_.tag = tag;
    }

    /**
     * Set the name of the machine
     */
    void set_machinename( std::string s ) {
        machinename_ = s;
    }

    /**
     * Set the Site name
     */
    void set_sitename( std::string s ) {
        sitename_ = s;
    }

    /**
     * Set the name of the analyzed application
     */
    void set_appname( std::string s ) {
        appname_ = s;
    }

    void set_threads( std::string s ) {
        char* s_cs = ( char* )s.c_str();
        // tokenize the string and put the IDs in the list
//        psc_dbgmsg( 1, "tokenizing >%s<\n", s_cs );
        char* tmp = strtok( s_cs, " ,\n" );
        while( tmp != 0 ) {
            int th = atoi( tmp );
            if( th < 0 ) {
                psc_errmsg( "Malformed Threadnumber: %s\n", tmp );
            }
            else {
//                psc_dbgmsg( 1,"adding >%d<\n",th );
                threads_.push_back( th );
            }

            tmp = strtok( 0, " ,\n" );
        }
/*        psc_dbgmsg( 1,"looping %d entries\n",threads_.size() );
          for ( int ct = 0; ct < threads_.size(); ct++ )
                  psc_dbgmsg( 1, "thread %i\n", ct );
                psc_dbgmsg( 1, "end tokenizing\n" ); */
    }

    /**
     * Returns the name of the machine
     */
    const char* machinename() {
        return machinename_.c_str();
    }

    /**
     * Returns the name of the Site
     */
    const char* sitename() {
        return sitename_.c_str();
    }

    /**
     * Returns the parent tag
     */
    const char* parent() {
        return own_info_.parent.c_str();
    }

    /**
     * Returns the name of the analyzed application
     */
    const char* appname() {
        return appname_.c_str();
    }

    /**
     * Returns a list of working threads
     */
    const std::list< int >threads() {
        return threads_;
    }

    /**
     * Returns the agent registry ID
     */
    int own_id() {
        return regid_;
    }

    /**
     * allow ID's to be set outside of registry communication, for fast starts
     */
    void set_id( int id ) {
        regid_ = id;
    }

    /**
     * Returns the internal agent's information
     */
    AgentInfo get_own_info() {
        return own_info_;
    }
};

#endif // PSC_AGENT_H_
