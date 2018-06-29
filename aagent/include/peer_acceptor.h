/**
   @file    peer_acceptor.cc
   @ingroup Communication
   @brief   Acceptor pattern header
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

#ifndef PEER_ACCEPTOR_H
#define PEER_ACCEPTOR_H


#include "accl_handler.h"

#include "ace/INET_Addr.h"
#include "ace/Reactor.h"
#include "ace/SOCK_Acceptor.h"

class PeriscopeAgent;

/**
 * @class PeerAcceptor
 * @ingroup AnalysisAgent
 *
 * @brief Accepts connection and handles them
 */
class PeerAcceptor : public ACE_Event_Handler {
private:
    ACE_SOCK_Acceptor acceptor_;
    PeriscopeAgent*   agent_;

public:
    PeerAcceptor() {
        psc_dbgmsg( 7, "PeerAcceptor created\n" );
        agent_ = 0;
    }

    void set_agent( PeriscopeAgent* agent ) {
        agent_ = agent;
    }


    virtual ~PeerAcceptor();

    /// get the I/O handle
    virtual ACE_HANDLE get_handle() const {
        return acceptor_.get_handle();
    }

    /// called upon a new connection attempt
    virtual int handle_input( ACE_HANDLE hdle = ACE_INVALID_HANDLE );

    /// called when the handle is removed from the reactor
    virtual int handle_close( ACE_HANDLE       hdle,
                              ACE_Reactor_Mask close_mask );


    /**
     * Prepare to accept connections on the specified port
     * and add ourself to the reactors event loop.
     *
     * @param listen_addr  Address to listen to
     * @return
     */
    int open( ACE_INET_Addr& listen_addr );

    /// get the local addr our acceptor_ is bound to
    int get_local_addr( ACE_INET_Addr& addr );
};

#endif // PEER_ACCEPTOR_H
