/**
   @file    peer_connection.cc
   @ingroup Communication
   @brief   Connection abstraction header
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

#ifndef PEER_CONNECTION_H
#define PEER_CONNECTION_H

#include "accl_handler.h"

#include "ace/Reactor.h"
#include "ace/SOCK_Stream.h"

/**
 * @class PeerConnection
 * @ingroup AnalysisAgent
 *
 * @brief Class representing the connection between peers
 *
 */
class PeerConnection : public ACE_Event_Handler {
private:
    ACCL_Handler* proto_handler_;

public:

    virtual ACE_HANDLE get_handle() const {
        return stream_.get_handle();
    }

    /// Called when input is available from the client
    virtual int handle_input( ACE_HANDLE hdle = ACE_INVALID_HANDLE );

    /// Called when output is possible
    virtual int handle_output( ACE_HANDLE hdle = ACE_INVALID_HANDLE );

    /// Called when handler is removed from reactor
    virtual int handle_close( ACE_HANDLE       hdle,
                              ACE_Reactor_Mask close_mask );

    /// Specify the protocol handler
    void set_protocol_handler( ACCL_Handler* handler ) {
        proto_handler_ = handler;
    }

    /// @return the stream used
    ACE_SOCK_Stream& peer() {
        return stream_;
    }

    /// Create the connection
    int open();

protected:
    ACE_SOCK_Stream stream_;
};


#endif // PEER_CONNECTION_H
