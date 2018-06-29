/**
   @file	mri_acceptor.cc
   @ingroup Communication
   @brief   Acceptor pattern implementation for accepting mri's
   @author	Yury Oleynik
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

#include <assert.h>


#include "ace/INET_Addr.h"
#include "ace/Reactor.h"
#include "ace/Log_Msg.h"
#include "psc_errmsg.h"
#include "mri_acceptor.h"
#include "DataProvider.h"

mriAcceptor::~mriAcceptor() {
    this->handle_close( ACE_INVALID_HANDLE, 0 );
}

void mriAcceptor::set_dp( DataProvider* dp ) {
    dp_ = dp;
}


int mriAcceptor::handle_close( ACE_HANDLE       hdle,
                               ACE_Reactor_Mask maske ) {
    psc_dbgmsg( 7, "mriAcceptor::handle_close\n" );
    if( acceptor_.get_handle() != ACE_INVALID_HANDLE ) {
        ACE_Reactor_Mask m =
            ACE_Event_Handler::ACCEPT_MASK |
            ACE_Event_Handler::DONT_CALL;

        assert( reactor() );
        reactor()->remove_handler( this, m );
        acceptor_.close();
    }
    return 0;
}


int mriAcceptor::handle_input( ACE_HANDLE hdle ) {
    ACE_SOCK_Stream stream;

    if( acceptor_.accept( stream ) == -1 ) {
        ACE_ERROR_RETURN( ( LM_ERROR, "Failed to accept()" ),
                          -1 );
    }

    dp_->add_new_appl_proc( stream );

    return 0;
}


int mriAcceptor::open( ACE_INET_Addr& listen_addr ) {
    if( acceptor_.open( listen_addr, 1 ) == -1 ) {
        return -1;
    }

    assert( reactor() );

    return reactor()->register_handler( this, ACE_Event_Handler::ACCEPT_MASK );
}


int mriAcceptor::get_local_addr( ACE_INET_Addr& addr ) {
    psc_dbgmsg( 9, "mriAcceptor::get_local_addr\n" );
    return acceptor_.get_local_addr( addr );
}
