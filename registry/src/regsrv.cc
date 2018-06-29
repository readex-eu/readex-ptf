/**
   @file	regsrv.cc
   @brief   Registry server implementation
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

#include <stdio.h>
#include <stdlib.h>

#include "ace/INET_Addr.h"
#include "regsrv.h"
#include "regsrv_client.h"
#include "psc_errmsg.h"


/**
 * @brief Open a port in listening mode for incoming connections
 * @param addr	Port number to use (defined as ACE_INET_Addr)
 * @return -1: no port could be opened
 */
int RegServ::open( ACE_INET_Addr& addr ) {
    unsigned int numRetries = 10;

    u_short selectedPort = addr.get_port_number();

    /* initialize random seed: */
    srand( time( NULL ) );

    while( ( this->acceptor_.open( addr, 1 ) == -1 ) && ( numRetries-- > 0 ) ) {
        selectedPort += rand() % 500;
        addr.set_port_number( selectedPort );
    }

    this->hostname_ = addr.get_host_name();
    this->port_     = selectedPort;

    if( ( numRetries == 0 ) || ( this->reactor()->register_handler( this, ACE_Event_Handler::ACCEPT_MASK ) ) == -1 ) {
        return -1;
    }
    else {
        return selectedPort;
    }
}


int RegServ::handle_close( ACE_HANDLE       h,
                           ACE_Reactor_Mask m ) {
    if( acceptor_.get_handle() != ACE_INVALID_HANDLE ) {
        ACE_Reactor_Mask m =
            ACE_Event_Handler::ACCEPT_MASK |
            ACE_Event_Handler::DONT_CALL;

        reactor()->remove_handler( this, m );
        acceptor_.close();
    }
    return 0;
}

int RegServ::handle_input( ACE_HANDLE h ) {
    RegServClient* client = new RegServClient( this );

    if( acceptor_.accept( client->peer() ) == -1  ) {
        psc_errmsg( "Failed to accept connection\n" );
        delete client;
        return -1;
    }

    client->reactor( this->reactor() );

    if( client->open() == -1 ) {
        client->handle_close( ACE_INVALID_HANDLE, 0 );
    }
    return 0;
}
