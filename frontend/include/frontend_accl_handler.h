/**
   @file    frontend_accl_handler.h
   @ingroup Communication
   @brief   Front-end communication layer header
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

#ifndef ACCL_FRONTEND_HANDLER
#define ACCL_FRONTEND_HANDLER

#include "frontend.h"
#include "accl_handler.h"
#include "frontend_accl_statemachine.h"
#include "config.h"

using namespace femsm;

class PeriscopeNodeagent;

/**
 * @class ACCL_Frontend_Handler
 * @ingroup Frontend
 *
 * @brief Periscope frontend agent protocol handler
 *
 * This class extends the ACCL_Handler and implements
 * the frontend agents protocol
 */
class ACCL_Frontend_Handler : public ACCL_Handler {
private:
    PeriscopeFrontend*    frontend_;
    frontend_statemachine statemachine_;

public:
    ACCL_Frontend_Handler( PeriscopeFrontend* frontend, ACE_SOCK_Stream& peer ) :
        ACCL_Handler( peer, "Periscope Frontend" ) {
        frontend_ = frontend;
        statemachine_.start();
    }

    int on_heartbeat( heartbeat_req_t&   req,
                      heartbeat_reply_t& reply );

    int on_foundprop( foundprop_req_t&   req,
                      foundprop_reply_t& reply );

    int on_calltree( calltree_req_t&   req,
                     calltree_reply_t& reply );

    int on_calltreesent( calltreesent_req_t&   req,
                         calltreesent_reply_t& reply );

    int on_searchfinished( searchfinished_req_t&   req,
                           searchfinished_reply_t& reply );

    int on_needrestart( needrestart_req_t&   req,
                        needrestart_reply_t& reply );

    int on_terminated( terminated_req_t&   req,
                       terminated_reply_t& reply );

    int on_propertiessent( propertiessent_req_t&   req,
                           propertiessent_reply_t& reply );

    int on_reqexperiment( reqexperiment_req_t&   req,
                          reqexperiment_reply_t& reply );

    void decide_continuation( std::map< std::string, AgentInfo >* ca );
};




#endif // ACCL_HLAGENT_HANDLER
