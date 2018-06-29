/**
   @file    msghandler.h
   @ingroup Communication
   @brief   Low-level communication routines header
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

#ifndef MSGHANDLER_H_
#define MSGHANDLER_H_

#include <string>
#include <iostream>

#include <ace/SOCK_Stream.h>
#include <ace/Message_Block.h>
#include <ace/Message_Queue.h>
#include <ace/CDR_Stream.h>
#include <ace/Synch.h>

#include "psc_errmsg.h"
/**
 * @class MsgHandler
 * @ingroup AnalysisAgent
 * @brief Base class for the message handlers
 *
 * Implements core framing protocol (i.e, implicitly defines the message header)
 */
class MsgHandler {
public:
    typedef ACE_CDR::Octet msg_type_t;     ///< Message Type
    typedef ACE_CDR::Octet msg_tag_t;      ///< Message Tag
    typedef ACE_CDR::Octet err_type_t;     ///< Error Type

protected:
    ACE_SOCK_Stream& peer_;     ///< Peer to get the data

//    const ssize_t MAX_MSG_LEN; ///< Maximum message length

    /**
     * Whether to send all data immediately or to put it in a
     * message queue and send it incrementally upon calls to
     * send_outstanding()
     */
    bool                              send_immediately_;
    ACE_Message_Queue<ACE_NULL_SYNCH> send_queue_;     ///< ACE Send Queue

public:
    /// Create a message handler for a specified peer.
    MsgHandler( ACE_SOCK_Stream& peer, int immediate = 1 ) :
        peer_( peer ), send_immediately_( immediate ) {
//    ,MAX_MSG_LEN(1 000 000)
    }

    virtual ~MsgHandler() {
    }

    /**
     * @brief Handle a single message from the peer.
     *
     * Handle a single message from the peer i.e., receive the header,
     * call want_to_handle() to see if the derived class wants to
     * handle the message and if yes read the message body into a
     * ACE_Message_Block and call on_msg()
     *
     * @return -1  Abnormal termination of the connection (connection lost, ...)
     * @return  0  OK, no error
     * @return >0  Protocol specific error code
     */
    int handle_msg();

protected:
    /**
     * @brief Check whether the derived class can be handled.
     *
     * This method is called by the base-class handler to
     * see whether the derived class is willing to handle
     * the message of the specified type. #msg_type_t
     *
     * @param  t  Message type ( #msg_type_t )
     * @return 1  Type can be handled
     * @return 0  Not handled
     */
    virtual int want_to_handle( msg_type_t t ) = 0;

    /**
     * @brief Request the derived class to handle the message.
     *
     * Called by the base-class handler to request the
     * derived class to handle the message.
     *
     * @param block Message block
     * @return
     */
    virtual int on_msg( ACE_Message_Block* block ) = 0;

    /**
     * @brief Send the specified message to the peer.
     *
     * Sends the specified message to the peer and
     * appends the required header.
     *
     * @param type   Message type ( #msg_type_t )
     * @param tag    Message tag ( #msg_tag_t )
     * @param block  Message block
     * @return
     */
    int send_msg( msg_type_t         type,
                  msg_tag_t          tag,
                  ACE_Message_Block* block );

    //
    // try to send outstanding data from the send queue
    //
    // int send_outstanding();
};

#endif /* MSGHANDLER_H_ */
