/**
   @file    peer_connection.cc
   @ingroup Communication
   @brief   Connection abstraction
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

#ifndef __LINUX_p575
#define MAXHOSTNAMELEN 1000
#endif

#include "peer_connection.h"



int PeerConnection::handle_input( ACE_HANDLE hdle ) {
//  psc_dbgmsg(7,"PeerConnection::handle_input\n");
    char    buffer[ 1000 ];
    ssize_t recv_c, send_c;

    if( proto_handler_ ) {
        if( proto_handler_->handle_msg() == -1 ) {
            stream_.close();
        }
    }

    else {
        if( ( recv_c = stream_.recv( buffer, sizeof( buffer ) ) ) <= 0 ) {
            //ACE_DEBUG( (LM_DEBUG, "Closed connection\n") );
            return -1;
        }

        stream_.send( buffer, recv_c );
    }
    return 0;
}

int PeerConnection::handle_output( ACE_HANDLE hdle ) {
    psc_dbgmsg( 7, "PeerConnection::handle_output\n" );
    return 0;
}

int PeerConnection::handle_close( ACE_HANDLE       hdle,
                                  ACE_Reactor_Mask mask ) {
    psc_dbgmsg( 7, "PeerConnection::handle_close\n" );
    if( mask == ACE_Event_Handler::WRITE_MASK ) {
        return 0;
    }

    mask = ACE_Event_Handler::ALL_EVENTS_MASK |
           ACE_Event_Handler::DONT_CALL;

    reactor()->remove_handler( this, mask );
    stream_.close();
    delete proto_handler_;
    return 0;
}

int PeerConnection::open() {
    psc_dbgmsg( 7, "PeerConnection::open\n" );
    ACE_INET_Addr peer_addr;
    char          peer_name[ MAXHOSTNAMELEN ];


    if( stream_.get_remote_addr( peer_addr ) == 0 &&
        peer_addr.addr_to_string( peer_name, MAXHOSTNAMELEN ) == 0 ) {
        //ACE_DEBUG( (LM_DEBUG, "Connection from %s\n",
        //peer_name ) );
    }

    return reactor()->register_handler( this,
                                        ACE_Event_Handler::READ_MASK );
}
