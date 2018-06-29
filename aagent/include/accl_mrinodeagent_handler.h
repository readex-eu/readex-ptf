/**
   @file    accl_mrinodeagent_handler.h
   @ingroup Communication
   @brief   Analysis agent communication layer header
   @author  Karl Fuerlinger
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2013, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#ifndef ACCL_MRINODEAGENT_HANDLER_H_
#define ACCL_MRINODEAGENT_HANDLER_H_

#include "analysisagent.h"
#include "accl_handler.h"
#include "Scenario.h"
#include "rts.h"

class AnalysisAgent;
extern Rts* root;

/**
 * @class ACCL_MRINodeagent_Handler
 * @ingroup AnalysisAgent
 *
 * @brief Communication with the MRINode agents
 *
 * This class extends the ACCL_Handler and implements
 * the communication interface with the MRI Node agents
 */
class ACCL_MRINodeagent_Handler : public ACCL_Handler {
private:
    AnalysisAgent*       agent_;
    std::list<Scenario*> scenarioList;
public:
    /// Create a new Handler for the MRI Protocol
    ACCL_MRINodeagent_Handler( AnalysisAgent* agent, ACE_SOCK_Stream& peer ) : ACCL_Handler( peer, "MRI Nodeagent" ) {
        agent_ = agent;
    }

    /**
     * Start the strategy
     */
    int on_start( start_req_t&   req,
                  start_reply_t& reply );

    /**
     * Start the experiment for next strategy step
     */
    int on_startexperiment( startexperiment_req_t&   req,
                            startexperiment_reply_t& reply );

    /**
     * Stop the experiment
     */
    int on_quit( quit_req_t&   req,
                 quit_reply_t& reply );

    /**
     * Terminate application processes
     */
    int on_terminate( terminate_req_t&   req,
                      terminate_reply_t& reply );

    /**
     * Restart
     */
    int on_reinit( reinit_req_t&   req,
                   reinit_reply_t& reply );

    /**
     * Transfer the properties as STRINGS to a higher level agent
     */
    int on_check( check_req_t&   req,
                  check_reply_t& reply );

    /**
     * Specify the parent to receive the data
     */
    int on_setparent( setparent_req_t&   req,
                      setparent_reply_t& reply );

    /**
     * Handle a serialize call-tree request. Transfers the serialized call-tree to a higher-level agent
     */
    int on_serializecalltree( calltreeserial_req_t&   req,
                              calltreeserial_reply_t& reply );
};

#endif // ACCL_MRINODEAGENT_HANDLER_H_
