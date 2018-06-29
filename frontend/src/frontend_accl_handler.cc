/**
   @file    frontend_accl_handler.cc
   @ingroup Communication
   @brief   Front-end communication layer
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

#include "frontend_accl_handler.h"

using namespace std;
extern bool search_done;
extern bool restart_requested_for_no_tuning;

int ACCL_Frontend_Handler::on_heartbeat( heartbeat_req_t&   req,
                                         heartbeat_reply_t& reply ) {
#ifdef PSC_FRONTEND_ACCL_STATEMACHINE
    statemachine_.process_event( heartbeat_event() );
#endif

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "on_heartbeat from host=%s, port=%d, tag=%s, forwarded=%d, num_procs=%d\n", req.hostname.c_str(), req.port, req.tag.c_str(), req.heartbeat_type, req.num_procs );

    // currently assumed that the child agent is connected only once
#ifdef _BGP_PORT_HEARTBEAT_V1
    std::map< std::string, AgentInfo >* ca;
    ca = frontend_->get_child_agents();

    switch( req.heartbeat_type ) {
    case OWN_HEARTBEAT:
    case FORWARDED_HEARTBEAT:
        //increment the number of started agents, and issue start if the number of started agents is equal to the precomputed total amount of agents
#ifdef WITH_MRI_OVER_ACE
        //if we have already started dont react on heartbeats
        if( !fe->get_startup_mode() ) {
            return 0;
        }
#endif
        if( req.heartbeat_type == OWN_HEARTBEAT ) {
            //if it is not a forwarded heartbeat accept this agent as a direct child agent
            fe->add_child_agent( req.tag, req.hostname, req.port );
        }

        fe->add_started_agent( req.num_procs );

        break;
    case DISMISS_HEARTBEAT:
        //when the child says it wants to dismiss, then erase that child and check if the number of children is 0, if so dismiss itself
        std::map<std::string, AgentInfo>::iterator it;
        psc_dbgmsg( FRONTEND_GENERAL_DEBUG_LEVEL,
                    "Child %s requests dismiss, removing it then...\n", req.tag.c_str() );
        it = ca->find( req.tag );
        if( it != ca->end() ) {
            ca->erase( it );
        }

        if( ca->size() == 0 ) {
            psc_dbgmsg( FRONTEND_GENERAL_DEBUG_LEVEL, "Ooops, no children are left, no sense to live then, terminating...\n" );

            fe->stop();
        }
        break;
    }
#else
    std::map< std::string, AgentInfo >*          ca;
    std::map< std::string, AgentInfo >::iterator it;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "on_heartbeat\n" );

    ca = frontend_->get_child_agents();
    int total_agents = ca->size();
    int ready_agents = 0;

    bool handle_step_required = false;
    for( it = ca->begin(); it != ca->end(); it++ ) {
        if( it->second.tag == req.tag ) {
            fe->add_started_agent();             // TODO check if this is needed in other locations
            if( it->second.status == AgentInfo::INITIAL ) {
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "status=STARTED\n" );
                it->second.status    = AgentInfo::STARTED;
                handle_step_required = true;
            }
            else {
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "status_reinit=STARTED\n" );
                it->second.status_reinit = AgentInfo::STARTED;
                search_done              = true;
            }

            it->second.hostname        = req.hostname;
            it->second.port            = req.port;
            it->second.properties_sent = false;
            break;
        }
    }

    if( fe->get_fastmode() ) {
        if( handle_step_required ) {
            fe->handle_step( fe->getTimerAction() );
        }
    }

    return 0;
#endif
}

int ACCL_Frontend_Handler::on_foundprop( foundprop_req_t&   req,
                                         foundprop_reply_t& reply ) {
#ifdef PSC_FRONTEND_ACCL_STATEMACHINE
    statemachine_.process_event( found_property_event() );
#endif

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_Frontend_Handler:on_foundprop\n" );

    size_t start, stop;
    if( strstr( req.xmlData.c_str(), "Required regions in the next experiment" ) != NULL ) {
        start = req.xmlData.find( "<addInfo>" );
        stop  = req.xmlData.find( "</addInfo>", start );
        frontend_->set_RequiredRegions( req.xmlData.substr( start + 9, stop - start - 9 ) );

        std::string regions = req.xmlData.substr( start + 9, stop - start - 9 );
        std::string region;
        size_t      start, stop;
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "Property with required regions: ::%s::\n", regions.c_str() );
        start = regions.find( "f: " );
        stop  = start;
        while( start != string::npos && stop != string::npos ) {
            stop   = regions.find( ";", start + 3 );
            region = regions.substr( start, stop - start );
            frontend_->add_requiredRegionsList( region );
            start = regions.find( "f: ", stop );
        }
    }
    else if( strstr( req.xmlData.c_str(), "high Instrumentation overhead" ) != NULL ) {
        std::string reg;
        start = req.xmlData.find( "FileName=" );
        stop  = req.xmlData.find_first_of( 34, start + 10 );
        reg.clear();
        reg.append( "f: " );
        reg.append( req.xmlData.substr( start + 10, stop - start - 10 ) );

        start = req.xmlData.find( "RegionId=", stop );
        start = req.xmlData.find_first_of( '-', start );
        stop  = req.xmlData.find_first_of( "\"", start );
        reg.append( ", r: " );
        reg.append( req.xmlData.substr( start + 1, stop - start - 1 ) );
        frontend_->add_badRegion( reg );
    }
    else {
        frontend_->found_property( req.xmlData );
    }
    return 0;
}


int ACCL_Frontend_Handler::on_calltree( calltree_req_t& req, calltree_reply_t& reply ) {

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_Frontend_Handler:on_calltree\n" );

    frontend_->construct_calltree( req.xmlData );
    return 0;
}


void ACCL_Frontend_Handler::decide_continuation( std::map< std::string, AgentInfo >* ca ) {
#ifdef PSC_FRONTEND_ACCL_STATEMACHINE
    statemachine_.process_event( decide_continuation_event() );
#endif

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_Frontend_Handler::decide_continuation" );
    restart_requested_for_no_tuning = false;


    AgentInfo::SearchStatus                      search_status = AgentInfo::FINISHED;
    std::map< std::string, AgentInfo >::iterator it;
    bool                                         all_properties_sent = true;

    //Implement priorities
    for( it = ca->begin(); it != ca->end(); it++ ) {
        if( it->second.properties_sent == false ) {
            all_properties_sent = false;
        }
        if( it->second.search_status == AgentInfo::REQEXPERIMENT && search_status == AgentInfo::FINISHED ) {
            search_status = AgentInfo::REQEXPERIMENT;
        }
        else if( it->second.search_status == AgentInfo::REQRESTART ) {
            search_status = AgentInfo::REQRESTART;
        }
        else if( it->second.search_status == AgentInfo::UNDEFINED ) {
            //Still information for one child missing
            return;
        }
    }


    /**
     * Before taking any decision ask AAgents to deliver properties and return to event loop.
     * It will come back to this function when propertiessent message will arrive.
     */
    if( all_properties_sent == false ) {
        psc_dbgmsg( FRONTEND_HIGH_DEBUG_LEVEL, "Asking AAgents to deliver properties...\n" );
        for( it = ca->begin(); it != ca->end(); it++ ) {
            AgentInfo& ai = it->second;

            if( ai.status != AgentInfo::CONNECTED ) {
                if( fe->connect_to_child( &ai ) == -1 ) {
                    psc_errmsg( "Error connecting to child at %s:%d\n", ai.hostname.c_str(), ai.port );
                }
                else {
                    ai.handler->check();
                }
            }
        }
        return;
    }

    //Enforce decision
    psc_dbgmsg( FRONTEND_HIGH_DEBUG_LEVEL, "Deciding on continuation...\n" );
    /**
     * Search is finished (searchfinished message arrived). Send check message first. On the second enter, after
     * properties have arrived, call shutdown.
     */
    // Set properties_sent of every agent to FALSE before starting new experiment or terminating
    for( it = ca->begin(); it != ca->end(); it++ ) {
        it->second.properties_sent = false;
    }

    if( search_status == AgentInfo::REQEXPERIMENT ) {
        psc_dbgmsg( FRONTEND_MAX_DEBUG_LEVEL, "Request for Experiment\n" );
        if( fe->checkForReinstrumentation() ) {
            search_status = AgentInfo::REQRESTART;
            psc_dbgmsg( FRONTEND_MAX_DEBUG_LEVEL, "Reinstrumentation required\n" );
        }
        else {
            fe->clear_requiredRegionsList();
            psc_dbgmsg( FRONTEND_MAX_DEBUG_LEVEL, "Reinstrumentation not required\n" );
        }
    }

    if( search_status == AgentInfo::FINISHED ) {
        psc_dbgmsg( FRONTEND_HIGH_DEBUG_LEVEL, "All analysis agents are done\n" );
        if( fe->get_automatic() ) {
            for( it = ca->begin(); it != ca->end(); it++ ) {
                AgentInfo& ai = it->second;

                if( ai.status != AgentInfo::CONNECTED ) {
                    if( fe->connect_to_child( &ai ) == -1 ) {
                        psc_errmsg( "Error connecting to child at %s:%d\n", ai.hostname.c_str(), ai.port );
                    }
                    else {
                        ///< dispatch next analysis scenario from the queue. If the queue is empty the method will call quit() itself
                        fe->start();
                        psc_dbgmsg( FRONTEND_HIGH_DEBUG_LEVEL, "done\n" );
                    }
                }
            }
        }
    }
    else if( search_status == AgentInfo::REQEXPERIMENT ) {
        psc_infomsg( "Additional analysis iteration is required...\n" );

        for( it = ca->begin(); it != ca->end(); it++ ) {
            AgentInfo& ai = it->second;

            if( ai.status != AgentInfo::CONNECTED ) {
                if( fe->connect_to_child( &ai ) == -1 ) {
                    psc_errmsg( "Error connecting to child at %s:%d\n", ai.hostname.c_str(), ai.port );
                }
                else {
                    ai.search_status = AgentInfo::UNDEFINED;
                    ai.handler->startexperiment();
                    search_done = false;
                    return;
                }
            }
        }
    }
    else if( search_status == AgentInfo::REQRESTART ) {
        psc_dbgmsg( FRONTEND_HIGH_DEBUG_LEVEL, "Analysis agent requested restart ...\n" );
        if( fe->get_shutdown_allowed() ) {
            //terminate
            if( fe->get_automatic() ) {
                for( it = ca->begin(); it != ca->end(); it++ ) {
                    AgentInfo& ai = it->second;

                    if( ai.status != AgentInfo::CONNECTED ) {
                        if( fe->connect_to_child( &ai ) == -1 ) {
                            psc_errmsg( "Error connecting to child at %s:%d\n", ai.hostname.c_str(), ai.port );
                        }
                        else {
                            fe->quit();
                        }
                    }
                }
            }
        }
        else {
            //restart application
            restart_requested_for_no_tuning = true;
            for( it = ca->begin(); it != ca->end(); it++ ) {
                AgentInfo& ai = it->second;

                if( ai.status != AgentInfo::CONNECTED ) {
                    if( fe->connect_to_child( &ai ) == -1 ) {
                        psc_errmsg( "Error connecting to child at %s:%d\n", ai.hostname.c_str(), ai.port );
                    }
                    else {
                        fe->set_need_restart( true );
                        ai.appl_terminated = false;
                        ai.search_status   = AgentInfo::UNDEFINED;
                        ai.status_reinit   = AgentInfo::INITIAL;
                        fe->stop();
                    }
                }
            }
        }
    }
    search_done = true;
}


int ACCL_Frontend_Handler::on_searchfinished( searchfinished_req_t&   req,
                                              searchfinished_reply_t& reply ) {
#ifdef PSC_FRONTEND_ACCL_STATEMACHINE
    statemachine_.process_event( search_finished_event() );
#endif
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "on_searchfinished\n" );

    std::map< std::string, AgentInfo >*          ca;
    std::map< std::string, AgentInfo >::iterator it;

    ca = fe->get_child_agents();

    for( it = ca->begin(); it != ca->end(); it++ ) {
        if( it->second.tag == req.tag ) {
            it->second.search_status = AgentInfo::FINISHED;
            break;
        }
    }

    decide_continuation( ca );

    return 0;
}

int ACCL_Frontend_Handler::on_reqexperiment( reqexperiment_req_t&   req,
                                             reqexperiment_reply_t& reply ) {
#ifdef PSC_FRONTEND_ACCL_STATEMACHINE
    statemachine_.process_event( reexperiment_event() );
#endif

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "on_reqexperiment\n" );

    std::map< std::string, AgentInfo >*          ca;
    std::map< std::string, AgentInfo >::iterator it;

    ca = fe->get_child_agents();

    for( it = ca->begin(); it != ca->end(); it++ ) {
        if( it->second.tag == req.tag ) {
            it->second.search_status = AgentInfo::REQEXPERIMENT;
            break;
        }
    }
    decide_continuation( ca );

    return 0;
}

int ACCL_Frontend_Handler::on_needrestart( needrestart_req_t&   req,
                                           needrestart_reply_t& reply ) {
#ifdef PSC_FRONTEND_ACCL_STATEMACHINE
    statemachine_.process_event( need_restart_event() );
#endif

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "on_needrestart\n" );

    std::map< std::string, AgentInfo >*          ca;
    std::map< std::string, AgentInfo >::iterator it;

    ca = fe->get_child_agents();

    for( it = ca->begin(); it != ca->end(); it++ ) {
        if( it->second.tag == req.tag ) {
            it->second.search_status = AgentInfo::REQRESTART;
            break;
        }
    }

    decide_continuation( ca );

    return 0;
}

int ACCL_Frontend_Handler::on_terminated( terminated_req_t&   req,
                                          terminated_reply_t& reply ) {
#ifdef PSC_FRONTEND_ACCL_STATEMACHINE
    statemachine_.process_event( terminated_event() );
#endif

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "on_terminated\n" );

    std::map< std::string, AgentInfo >*          ca;
    std::map< std::string, AgentInfo >::iterator it;
    bool                                         done = true;

    ca = fe->get_child_agents();

    for( it = ca->begin(); it != ca->end(); it++ ) {
        if( it->second.tag == req.tag ) {
            it->second.appl_terminated = true;
        }
        if( !it->second.appl_terminated ) {
            done = false;
        }
    }

    if( fe->get_fastmode() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ), "Skipping registry calls in the Frontend ACCL handler...\n" );
    }
    else {
        if( done ) {
            RegistryService*       regsrv = fe->get_registry();
            EntryData              query;
            std::list< EntryData > rresult;
            bool                   startup_finished = false;

            int i = 1;
            while( !startup_finished && i > 0 ) {
                sleep( 5 );
                query.app  = fe->get_appname();
                query.comp = "MRIMONITOR";
                query.tag  = "none";

                rresult.clear();
                if( regsrv->query_entries( rresult, query, false ) == -1 ) {
                    psc_errmsg( "Error querying registry for application\n" );
                    exit( 1 );
                }

                if( rresult.size() > 0 ) {
                    psc_dbgmsg( FRONTEND_GENERAL_DEBUG_LEVEL, "%d processes of application %s still registered\n", rresult.size(), fe->get_appname() );
                }
                startup_finished = ( rresult.size() == 0 );
                i--;
            }             //waiting for all processes to unregister
        }
    }

    search_done = true;
    return 0;
}


int ACCL_Frontend_Handler::on_propertiessent( propertiessent_req_t&   req,
                                              propertiessent_reply_t& reply ) {
#ifdef PSC_FRONTEND_ACCL_STATEMACHINE
    statemachine_.process_event( properties_sent_event() );
#endif

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_Frontend_Handler::on_propertiessent from %s\n", ( ( std::string )req.tag ).c_str() );

    std::map< std::string, AgentInfo >*          ca;
    std::map< std::string, AgentInfo >::iterator it;
    bool                                         done = true;


    ca = fe->get_child_agents();

    for( it = ca->begin(); it != ca->end(); it++ ) {
        if( it->second.tag == req.tag ) {
            it->second.properties_sent = true;
        }
        if( !it->second.properties_sent ) {
            done = false;
        }
    }

    if( done ) {
        decide_continuation( ca );
    }

    return 0;
}

int ACCL_Frontend_Handler::on_calltreesent( calltreesent_req_t&   req,
                                            calltreesent_reply_t& reply ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_Frontend_Handler::on_calltreesent from %s\n", ( ( std::string )req.tag ).c_str() );

    std::map< std::string, AgentInfo >*          ca;
    std::map< std::string, AgentInfo >::iterator it;
    bool                                         done = true;

    ca = fe->get_child_agents();

    for( it = ca->begin(); it != ca->end(); it++ ) {
        if( it->second.tag == req.tag ) {
            it->second.calltree_sent = true;
            break;
        }
        if( !it->second.calltree_sent ) {
            done = false;
        }
    }

    if( done ) {  //One of the aagents sent its call-trees, so we can return from this function
        for( it = ca->begin(); it != ca->end(); it++ ) {
            it->second.calltree_sent = false;
            if( fe->get_automatic() ) {
                AgentInfo& ai = it->second;
                if( ai.status != AgentInfo::CONNECTED ) {
                    if( fe->connect_to_child( &ai ) == -1 ) {
                        psc_errmsg( "Error connecting to child at %s:%d\n", ai.hostname.c_str(), ai.port );
                    }
                    else {
                        fe->stop_agents_for_calltree();
                        psc_dbgmsg( FRONTEND_HIGH_DEBUG_LEVEL, "Stopped ACE Reactor in frontend_accl_handler\n" );
                    }
                }
            }
        }
    }
    return 0;
}
