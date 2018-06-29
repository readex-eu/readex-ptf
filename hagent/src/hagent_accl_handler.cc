/**
   @file    hagent_accl_handler.cc
   @ingroup Communication
   @brief   High-level agent communication layer
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
#include "hagent_accl_handler.h"
#include "selective_debug.h"

int ACCL_HLAgent_Handler::on_start( start_req_t&   req,
                                    start_reply_t& reply ) {
    std::map<std::string, AgentInfo>*          ca;
    std::map<std::string, AgentInfo>::iterator it;

    high_level_agent_accl_statemachine_.process_event( start_event() );

    //5
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_HLAgent_Handler::on_start\n" );

    ca = agent_->get_child_agents();
    psc_dbgmsg( 1, "Final number of connected children is %d\n", ca->size() );

// if the number of children is 0, then dismiss itself and quit
#ifdef _BGP_PORT_HEARTBEAT_V1
    if( ca->size() == 0 ) {
        psc_dbgmsg( 1, "Ooops, no children are accepted, no sense to live then, terminating...\n" );
        if( agent_->get_parent_handler() ) {
            agent_->get_parent_handler()->heartbeat( agent_->get_own_info().hostname,
                                                     agent_->get_own_info().port,
                                                     agent_->get_own_info().tag,
                                                     DISMISS_HEARTBEAT, 0 );
        }
        else {
            psc_errmsg( " parent_handler_ not set\n" );
            agent_->stop();
        }
        agent_->stop();
    }
#endif
//forward start to children
    for( it = ca->begin(); it != ca->end(); it++ ) {
        AgentInfo& ai = it->second;

        if( ai.status != AgentInfo::CONNECTED ) {
            if( agent_->connect_to_child( &ai ) == -1 ) {
                psc_errmsg( "Error connecting to child at %s:%d\n",
                            ai.hostname.c_str(), ai.port );
            }
            else {
                ai.handler->start( req.serialized_strategy_request_container.size,
                                   req.serialized_strategy_request_container.payload );
            }
        }
    }

    return 0;
}

int ACCL_HLAgent_Handler::on_startexperiment( startexperiment_req_t&   req,
                                              startexperiment_reply_t& reply ) {
    std::map<std::string, AgentInfo>*          ca;
    std::map<std::string, AgentInfo>::iterator it;

//  psc_dbgmsg(5, "on_startexperiment\n");
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_HLAgent_Handler::on_startexperiment\n" );


    ca = agent_->get_child_agents();

    for( it = ca->begin(); it != ca->end(); it++ ) {
        AgentInfo& ai = it->second;

        if( ai.status != AgentInfo::CONNECTED ) {
            if( agent_->connect_to_child( &ai ) == -1 ) {
                psc_errmsg( "Error connecting to child at %s:%d\n",
                            ai.hostname.c_str(), ai.port );
            }
            else {
                ai.handler->startexperiment();
            }
        }
    }

    return 0;
}

int ACCL_HLAgent_Handler::on_check( check_req_t&   req,
                                    check_reply_t& reply ) {
    std::map<std::string, AgentInfo>*          ca;
    std::map<std::string, AgentInfo>::iterator it;

// psc_dbgmsg(5, "on_check()\n");
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_HLAgent_Handler::on_check\n" );

    ca = agent_->get_child_agents();

    for( it = ca->begin(); it != ca->end(); it++ ) {
        AgentInfo& ai = it->second;

        if( ai.status != AgentInfo::CONNECTED ) {
            if( agent_->connect_to_child( &ai ) == -1 ) {
                psc_errmsg( "Error connecting to child at %s:%d\n",
                            ai.hostname.c_str(), ai.port );
            }
            else {
                ai.properties_sent = false;
                ai.handler->check();
            }
        }
    }

    return 0;
}

int ACCL_HLAgent_Handler::on_quit( quit_req_t&   req,
                                   quit_reply_t& reply ) {
    std::map<std::string, AgentInfo>*          ca;
    std::map<std::string, AgentInfo>::iterator it;

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_HLAgent_Handler::on_quit\n" );

    ca = agent_->get_child_agents();

    for( it = ca->begin(); it != ca->end(); it++ ) {
        AgentInfo& ai = it->second;

        if( ai.status != AgentInfo::CONNECTED ) {
            if( agent_->connect_to_child( &ai ) == -1 ) {
                psc_errmsg( "Error connecting to child at %s:%d\n",
                            ai.hostname.c_str(), ai.port );
            }
            else {
                ai.handler->quit();
            }
        }
    }
    agent_->stop();
//#ifdef __p575
    return ACCL_Handler::on_quit( req, reply );
//#else
// return 0;
//#endif
}

int ACCL_HLAgent_Handler::on_reinit( reinit_req_t&   req,
                                     reinit_reply_t& reply ) {
    std::map<std::string, AgentInfo>*          ca;
    std::map<std::string, AgentInfo>::iterator it;

//  psc_dbgmsg(5, "ACCL_HLAgent_Handler::on_reinit\n");
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_HLAgent_Handler::on_reinit\n" );

    //  agent_->clear_appexit();

    ca = agent_->get_child_agents();

    for( it = ca->begin(); it != ca->end(); it++ ) {
        AgentInfo& ai = it->second;

        if( ai.status != AgentInfo::CONNECTED ) {
            if( agent_->connect_to_child( &ai ) == -1 ) {
                psc_errmsg( "Error connecting to child at %s:%d\n",
                            ai.hostname.c_str(), ai.port );
            }
            else {
                int mapfrom[ 8192 ];
                int mapto[ 8192 ];
                for( int i = 0; i < req.maplen; i++ ) {
                    mapfrom[ i ] = req.idmap[ i ].idf;
                    mapto[ i ]   = req.idmap[ i ].idt;
                }
                ai.handler->reinit( req.maplen, mapfrom, mapto );
            }
        }
        ai.status_reinit = AgentInfo::INITIAL;
        // psc_dbgmsg(1,"testing child appexit: %d\n",ai.appexit);
    }
    psc_init_start_time();
    agent_->set_reinit_startup_timer();

    return 0;
}

int ACCL_HLAgent_Handler::on_heartbeat( heartbeat_req_t&   req,
                                        heartbeat_reply_t& reply ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "on_heartbeat from host = %s, port = %d, tag = %s, forwarded = %d, num_procs = %d\n",
                req.hostname.c_str(), req.port, req.tag.c_str(), req.heartbeat_type, req.num_procs );

#ifdef _BGP_PORT_HEARTBEAT_V1
    std::map<std::string, AgentInfo>* ca;
    ca = agent_->get_child_agents();
    switch( req.heartbeat_type ) {
    case OWN_HEARTBEAT:
    case FORWARDED_HEARTBEAT:
//forward the heartbeat to the parent, who will forward it up to FE
#ifdef WITH_MRI_OVER_ACE
        //if the network is already started do nothing here
        if( !agent_->get_startup_mode() ) {
            psc_errmsg( "Received heartbeat while startup mode is off!!!" );
            return 0;
        }
#endif
        if( req.heartbeat_type == OWN_HEARTBEAT ) {
            //if it is not a forwarded heartbeat accept this agent as a direct child agent
            agent_->add_child_agent( req.tag, req.hostname, req.port );

            /*  if (it->second.status==AgentInfo::INITIAL) { TODO Handle reinit cases properly here. Before accepting the child check whether it is
                                                             already there and check the status according to this code block

               psc_dbgmsg(3, "status = STARTED\n");
               it->second.status=AgentInfo::STARTED;
               } else {
               psc_dbgmsg(3, "status_reinit = STARTED\n");
               it->second.status_reinit=AgentInfo::STARTED;
               } */
        }

        if( agent_->get_parent_handler() ) {
            agent_->get_parent_handler()->heartbeat( req.hostname, req.port, req.tag,
                                                     FORWARDED_HEARTBEAT, req.num_procs );
        }
        else {
            psc_errmsg( " parent_handler_ not set\n" );
            agent_->stop();
        }
        psc_dbgmsg( 3, "Agent %s status=STARTED\n", req.tag.c_str() );
        break;
    case DISMISS_HEARTBEAT:
//when the child says it wants to dismiss, then erase that child and check if the number of children is 0, if so dismiss itself
        std::map<std::string, AgentInfo>::iterator it;
        psc_dbgmsg( 1, "Child %s requests dismiss, removing it then...\n", req.tag.c_str() );

        /*for (it = ca->begin(); it != ca->end(); it++) {
           if (it->second.tag == req.tag) {
            ca->erase(it);
            break;
           }
           }*/

        it = ca->find( req.tag );
        if( it != ca->end() ) {
            ca->erase( it );
        }

        if( agent_->get_parent_handler() ) {
            agent_->get_parent_handler()->heartbeat( req.hostname, req.port, req.tag,
                                                     DISMISS_HEARTBEAT, 0 );
        }
        else {
            psc_errmsg( " parent_handler_ not set\n" );
            agent_->stop();
        }

        if( ca->size() == 0 ) {
            psc_dbgmsg( 1, "Ooops, no children are left, no sense to live then, terminating...\n" );
            if( agent_->get_parent_handler() ) {
                agent_->get_parent_handler()->heartbeat( agent_->get_own_info().hostname,
                                                         agent_->get_own_info().port,
                                                         agent_->get_own_info().tag,
                                                         DISMISS_HEARTBEAT, 0 );
            }
            else {
                psc_errmsg( " parent_handler_ not set\n" );
                agent_->stop();
            }
            agent_->stop();
        }
        break;
    }

#else

    std::map<std::string, AgentInfo>*          ca;
    std::map<std::string, AgentInfo>::iterator it;

    ca = agent_->get_child_agents();
    for( it = ca->begin(); it != ca->end(); it++ ) {
        if( it->second.tag == req.tag ) {
            if( it->second.status == AgentInfo::INITIAL ) {
                it->second.status = AgentInfo::STARTED;
            }
            else {
                it->second.status_reinit = AgentInfo::STARTED;
            }
            it->second.hostname = req.hostname;
            it->second.port     = req.port;
            break;
        }
    }

    return 0;

#endif
}

int ACCL_HLAgent_Handler::on_foundprop( foundprop_req_t&   req,
                                        foundprop_reply_t& reply ) {
    // pass on to parent

    //psc_infomsg("HLAgent: on_foundprop %s %s %f %f %i %s\n",
    //            req.hostname.c_str(),
    //            req.prop_name.c_str(),
    //            req.severity, req.confidence, req.numthreads,
    //            req.context.c_str());
    // agent_->found_property(req.hostname,
    //                        req.prop_name,
    //                        req.severity,
    //                        req.confidence,
    //                        req.numthreads,
    //                        req.context);

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "Property in hlagent: ::%s::\n", ( ( std::string )req.xmlData ).c_str() );

    agent_->found_property( req.xmlData );

    return 0;
}


int ACCL_HLAgent_Handler::on_serializecalltree(calltreeserial_req_t& req, calltreeserial_reply_t& reply) {
    std::map<std::string, AgentInfo>*          ca;
    std::map<std::string, AgentInfo>::iterator it;

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_HLAgent_Handler::on_serializecalltree\n" );

    ca = agent_->get_child_agents();

    for( it = ca->begin(); it != ca->end(); it++ ) {
        AgentInfo& ai = it->second;

        if( ai.status != AgentInfo::CONNECTED ) {
            if( agent_->connect_to_child( &ai ) == -1 ) {
                psc_errmsg( "Error connecting to child at %s:%d\n",
                            ai.hostname.c_str(), ai.port );
            }
            else {
                ai.calltree_sent = false;
                ai.handler->serializecalltree();
            }
        }
    }

    return 0;
}


int ACCL_HLAgent_Handler::on_calltree( calltree_req_t&   req,
                                       calltree_reply_t& reply ) {

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "Calltree in hlagent: ::%s::\n" );

    agent_->send_calltree( req.xmlData );

    return 0;
}


/**
 * \brief Processes the calltreesent message from the aagent.
 * Propagates the calltreesent message to the frontend as soon as one child
 * agent sends its call-tree
 */
int ACCL_HLAgent_Handler::on_calltreesent( calltreesent_req_t&   req,
                                           calltreesent_reply_t& reply ) {
    std::map<std::string, AgentInfo>*          ca;
    std::map<std::string, AgentInfo>::iterator it;
    bool                                       done = true;

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_HLAgent_Handler::on_calltreesent: Msg from %s\n", ( ( std::string )req.tag ).c_str() );

    ca = agent_->get_child_agents();

    // Iterate the Child agents mapping
    for( it = ca->begin(); it != ca->end(); it++ ) {
        if( it->second.tag == req.tag ) {
            it->second.calltree_sent = true;
            break;
        }
        if( !it->second.calltree_sent ) {
            done = false;
        }
    }

    // If a child agent has already sent its call_tree
    if( done ) {

        agent_->get_parent_handler()->calltreesent( agent_->get_local_tag() );
    }

    return 0;
}



void ACCL_HLAgent_Handler::forward_request( std::map<std::string, AgentInfo>* ca ) {
    AgentInfo::SearchStatus                    search_status = AgentInfo::FINISHED;
    std::map<std::string, AgentInfo>::iterator it;

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_HLAgent_Handler::forward_request\n" );

    //Implement priorities
    for( it = ca->begin(); it != ca->end(); it++ ) {
        if( it->second.search_status == AgentInfo::REQEXPERIMENT && search_status == AgentInfo::FINISHED ) {
            search_status = AgentInfo::REQEXPERIMENT;
        }
        else if( it->second.search_status == AgentInfo::REQRESTART ) {
            search_status = AgentInfo::REQRESTART;
        }
        else if( it->second.search_status == AgentInfo::UNDEFINED ) {
            //psc_dbgmsg(1,"forward_request: information for one agent is missing (%s).\n",it->second.tag.c_str());
            //Still information for one child missing
            return;
        }
    }

    //Propagate information to parent
    if( search_status == AgentInfo::FINISHED ) {
        agent_->get_parent_handler()->searchfinished( agent_->get_local_tag() );
    }
    else if( search_status == AgentInfo::REQEXPERIMENT ) {
        agent_->get_parent_handler()->reqexperiment( agent_->get_local_tag() );
    }
    else if( search_status == AgentInfo::REQRESTART ) {
        agent_->get_parent_handler()->needrestart( agent_->get_local_tag() );
    }

    //Reset status to collect status after next experiment
    for( it = ca->begin(); it != ca->end(); it++ ) {
        it->second.search_status = AgentInfo::UNDEFINED;
    }
}

int ACCL_HLAgent_Handler::on_searchfinished( searchfinished_req_t&   req,
                                             searchfinished_reply_t& reply ) {
    std::map<std::string, AgentInfo>*          ca;
    std::map<std::string, AgentInfo>::iterator it;

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_HLAgent_Handler: Searchfinished from %s\n", req.tag.c_str() );

    ca = agent_->get_child_agents();

    for( it = ca->begin(); it != ca->end(); it++ ) {
        if( it->second.tag == req.tag ) {
            it->second.search_status = AgentInfo::FINISHED;
            break;
        }
    }

    ACCL_HLAgent_Handler::forward_request( ca );

    return 0;
}

int ACCL_HLAgent_Handler::on_reqexperiment( reqexperiment_req_t&   req,
                                            reqexperiment_reply_t& reply ) {
    std::map<std::string, AgentInfo>*          ca;
    std::map<std::string, AgentInfo>::iterator it;

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_HLAgent_Handler: Reqexperiment from %s\n", req.tag.c_str() );

    ca = agent_->get_child_agents();

    for( it = ca->begin(); it != ca->end(); it++ ) {
        if( it->second.tag == req.tag ) {
            it->second.search_status = AgentInfo::REQEXPERIMENT;
            break;
        }
    }

    forward_request( ca );

    return 0;
}

int ACCL_HLAgent_Handler::on_needrestart( needrestart_req_t&   req,
                                          needrestart_reply_t& reply ) {
    std::map<std::string, AgentInfo>*          ca;
    std::map<std::string, AgentInfo>::iterator it;

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_HLAgent_Handler::on_needrestart\n" );

    ca = agent_->get_child_agents();

    for( it = ca->begin(); it != ca->end(); it++ ) {
        if( it->second.tag == req.tag ) {
            it->second.search_status = AgentInfo::REQRESTART;
            break;
        }
    }

    forward_request( ca );

    return 0;
}

int ACCL_HLAgent_Handler::on_terminated( terminated_req_t&   req,
                                         terminated_reply_t& reply ) {
    std::map<std::string, AgentInfo>*          ca;
    std::map<std::string, AgentInfo>::iterator it;
    bool                                       done = true;

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_HLAgent_Handler::on_terminated\n" );

    ca = agent_->get_child_agents();

    for( it = ca->begin(); it != ca->end(); it++ ) {
        if( it->second.tag == req.tag ) {
            it->second.appl_terminated = true;
        }
        if( !it->second.appl_terminated ) {
            done = false;
        }
    }

    if( done ) {
        agent_->get_parent_handler()->terminated( agent_->get_local_tag() );
    }

    return 0;
}

int ACCL_HLAgent_Handler::on_terminate( terminate_req_t&   req,
                                        terminate_reply_t& reply ) {
    std::map<std::string, AgentInfo>*          ca;
    std::map<std::string, AgentInfo>::iterator it;

// psc_dbgmsg(5, "on_terminate\n");
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_HLAgent_Handler::on_terminate\n" );

    ca = agent_->get_child_agents();

    for( it = ca->begin(); it != ca->end(); it++ ) {
        AgentInfo& ai = it->second;
        ai.appl_terminated = false;

        if( ai.status != AgentInfo::CONNECTED ) {
            if( agent_->connect_to_child( &ai ) == -1 ) {
                psc_errmsg( "Error connecting to child at %s:%d\n",
                            ai.hostname.c_str(), ai.port );
            }
            else {
                ai.handler->terminate();
            }
        }
    }

    return 0;
}

/**
 * \brief Process incoming Propertiessend request
 */
int ACCL_HLAgent_Handler::on_propertiessent( propertiessent_req_t&   req,
                                             propertiessent_reply_t& reply ) {
    std::map<std::string, AgentInfo>*          ca;
    std::map<std::string, AgentInfo>::iterator it;
    bool                                       done = true;

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_HLAgent_Handler::on_propertiessent: Msg from %s\n", ( ( std::string )req.tag ).c_str() );

    ca = agent_->get_child_agents();

    // Iterate the Child agents mapping
    for( it = ca->begin(); it != ca->end(); it++ ) {
        if( it->second.tag == req.tag ) {
            it->second.properties_sent = true;
        }
        if( !it->second.properties_sent ) {
            done = false;
        }
    }

    // If all child agents have already sent their properties
    if( done ) {
        if( agent_->nocluster ) {
            agent_->get_parent_handler()->propertiessent( agent_->get_local_tag() );
        }
        else {
            agent_->clusterProps();
            agent_->addHotRegionProps();
            agent_->get_parent_handler()->propertiessent( agent_->get_local_tag() );
        }
    }

    return 0;
}
