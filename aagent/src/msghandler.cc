/**
   @file    msghandler.cc
   @ingroup Communication
   @brief   Low-level communication routines
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

#include "msghandler.h"
#include <ace/Message_Block.h>
#include "psc_errmsg.h"


/* #include <ace/Log_Msg.h> */

#include <string>
#include <iostream>


using std::cin;
using std::cout;
using std::cerr;
using std::endl;


int MsgHandler::handle_msg() {
    ACE_Message_Block* msg = new ACE_Message_Block( ACE_DEFAULT_CDR_BUFSIZE );

    ACE_CDR::mb_align( msg );

    // TODO: possibly use a timeout here, and check return value?
    if( ( peer_.recv_n( msg->wr_ptr(), 8 ) ) != 8 ) {
        msg->release();
        msg = 0;


        ACE_INET_Addr peer_addr;
        char          peer_name[ MAXHOSTNAMELEN ];

        // find out address of peer
        peer_.get_remote_addr( peer_addr );
        peer_addr.addr_to_string( peer_name, MAXHOSTNAMELEN );

        // TODO check why this occurs after rebuilding the agent hierarchy on restart
        //psc_errmsg("Could not receive complete header from %s\n", peer_name);

        // REMOVE ME
        //exit(1);
        delete msg;

        return -1;
    }

    msg->wr_ptr( 8 );
    ACE_InputCDR cdr( msg );

    // determine byte-order...
    ACE_CDR::Octet oct;
    cdr.read_octet( oct );

    if( oct != ( ACE_CDR::Octet )1 && oct != ( ACE_CDR::Octet )0 ) {
        psc_errmsg( "Warning: bad byteorder: %d\n", oct );
    }

    ACE_CDR::Boolean byteorder = ( ACE_CDR::Boolean )oct;
    cdr.reset_byte_order( byteorder );

    ACE_CDR::Octet msgtype, tag, reserved;
    ACE_CDR::ULong length;

    // read the rest of the header...
    cdr.read_octet( msgtype );
    cdr.read_octet( tag );
    cdr.read_octet( reserved );
    cdr.read_ulong( length );
//debug output begin
    ACE_INET_Addr peer_addr;
    char          peer_name[ MAXHOSTNAMELEN ];
    peer_.get_remote_addr( peer_addr );
    peer_addr.addr_to_string( peer_name, MAXHOSTNAMELEN );
    psc_dbgmsg( 9, "MsgHandler::handle_msg(): Received message from %s\n", peer_name );
//debug output end
    if( !want_to_handle( ( msg_type_t )msgtype ) ) {
        psc_errmsg( "Don't know how to handle message of type %d\n", msgtype );
        delete msg;

        return 1;
    }

//  if (length>MAX_MSG_LEN) {
//    length=MAX_MSG_LEN;
//    psc_errmsg("Message too long, setting length to %d\n", length);
//  }


    ACE_Message_Block* payload = new ACE_Message_Block( length + ACE_CDR::MAX_ALIGNMENT );

    ssize_t ret;
    // TODO: handle return of -1!!!
    if( ( ret = peer_.recv_n( payload->wr_ptr(), length ) ) != length ) {
        if( ret == -1 || ret == 0 ) {
            payload->release();
            delete msg;

            psc_errmsg( "Broken connection\n" );

            return -1;
        }
    }

    payload->wr_ptr( ret );

    psc_dbgmsg( 8, "MsgHandler::handle_msg(): >>>> message size is %d\n", length );
    //
    // call the message handler
    //
    on_msg( payload );

    // TODO: memory leak???
    payload->release();

    delete msg;
    return 0;
}

int MsgHandler::send_msg( msg_type_t type, msg_tag_t tag, ACE_Message_Block* block ) {
    //psc_dbgmsg( 5, "Sending message in Msg_Handler\n");
    ACE_OutputCDR  cdr( 8 + ACE_CDR::MAX_ALIGNMENT );
    ACE_CDR::ULong length = block->length();

    cdr.write_octet( ( ACE_CDR::Octet )ACE_CDR_BYTE_ORDER );
    //  ACE_DEBUG((LM_DEBUG, "%d\n", cdr.good_bit()));

    cdr.write_octet( ( ACE_CDR::Octet )type );
    //  ACE_DEBUG( (LM_DEBUG, "%d\n", cdr.good_bit()) );

    cdr.write_octet( ( ACE_CDR::Octet )tag );
    //  ACE_DEBUG((LM_DEBUG, "%d\n", cdr.good_bit()));

    cdr.write_octet( ( ACE_CDR::Octet )0 ); // reserved
    //  ACE_DEBUG( (LM_DEBUG, "%d\n", cdr.good_bit()) );

    psc_dbgmsg( 8, "MsgHandler::handle_msg(): <<<< message size is %d\n", length );

    cdr.write_ulong( length );
    //ACE_DEBUG( (LM_DEBUG, "%d\n", cdr.good_bit()) );

    iovec iov[ 2 ];
    iov[ 0 ].iov_base = cdr.begin()->rd_ptr();
    iov[ 0 ].iov_len  = 8;
    iov[ 1 ].iov_base = block->rd_ptr();
    iov[ 1 ].iov_len  = length;

    int sent;
// debug output begin
    ACE_INET_Addr peer_addr;
    char          peer_name[ MAXHOSTNAMELEN ];
    peer_.get_remote_addr( peer_addr );
    peer_addr.addr_to_string( peer_name, MAXHOSTNAMELEN );
    psc_dbgmsg( 9, "MsgHandler::send_msg: Sending message to %s\n", peer_name );
// debug output end

    if( send_immediately_ ) {
        sent = peer_.sendv_n( iov, 2 );
    }
    else {
        sent = peer_.sendv( iov, 2 );

        if( sent < length + 8 ) {
            ACE_DEBUG( ( LM_DEBUG, "Short write: %d instead of %d\n", sent, length + 8 ) );

            if( sent < 8 ) {
                ACE_Message_Block* hdr = new ACE_Message_Block( 8 - sent );
                memcpy( hdr->wr_ptr(), cdr.begin()->rd_ptr() + sent, 8 - sent );

                send_queue_.enqueue_tail( hdr );
                send_queue_.enqueue_tail( block );
            }
            else {
                block->rd_ptr( sent - 8 );

                send_queue_.enqueue_tail( block );
            }
        }
    }

    return sent;
}
