/**
   @file    hagent_accl_handler.h
   @ingroup Communication
   @brief   High-level agent communication layer header
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

#ifndef ACCL_HLAGENT_HANDLER
#define ACCL_HLAGENT_HANDLER

#include "hlagent.h"
#include "accl_handler.h"

class PeriscopeNodeagent;

/**
 * @class ACCL_HLAgent_Handler
 * @ingroup HLAgent
 *
 * @brief Periscope communication agent protocol handler
 *
 * This class extends the ACCL_Handler and implements
 * the communication agents protocol
 */
class ACCL_HLAgent_Handler : public ACCL_Handler {
private:
    PeriscopeHLAgent*                  agent_;
    high_level_agent_accl_statemachine high_level_agent_accl_statemachine_;

public:
    ACCL_HLAgent_Handler( PeriscopeHLAgent* agent,
                          ACE_SOCK_Stream&  peer ) : ACCL_Handler( peer, "Periscope HLAgent" ) {
        agent_ = agent;
        high_level_agent_accl_statemachine_.start();
    }

    void forward_request( std::map< std::string, AgentInfo >* ca );

    int on_heartbeat( heartbeat_req_t&   req,
                      heartbeat_reply_t& reply );

    int on_start( start_req_t&   req,
                  start_reply_t& reply );

    int on_startexperiment( startexperiment_req_t&   req,
                            startexperiment_reply_t& reply );

    int on_quit( quit_req_t&   req,
                 quit_reply_t& reply );

    int on_reinit( reinit_req_t&   req,
                   reinit_reply_t& reply );

    int on_check( check_req_t&   req,
                  check_reply_t& reply );

    int on_foundprop( foundprop_req_t&   req,
                      foundprop_reply_t& reply );

    int on_serializecalltree( calltreeserial_req_t&   req,
                              calltreeserial_reply_t& reply );

    int on_calltree( calltree_req_t&   req,
                     calltree_reply_t& reply );

    int on_calltreesent( calltreesent_req_t&   req,
                         calltreesent_reply_t& reply );

    int on_searchfinished( searchfinished_req_t&   req,
                           searchfinished_reply_t& reply );

    int on_needrestart( needrestart_req_t&   req,
                        needrestart_reply_t& reply );

    int on_terminate( terminate_req_t&   req,
                      terminate_reply_t& reply );

    int on_terminated( terminated_req_t&   req,
                       terminated_reply_t& reply );

    int on_propertiessent( propertiessent_req_t&   req,
                           propertiessent_reply_t& reply );

    int on_reqexperiment( reqexperiment_req_t&   req,
                          reqexperiment_reply_t& reply );
};

#endif // ACCL_HLAGENT_HANDLER
