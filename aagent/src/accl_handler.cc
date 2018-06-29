/**
   @file    accl_handler.cc
   @ingroup Communication
   @brief   Generic communication layer implementation
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

#include <ace/CDR_Stream.h>

#include "accl_handler.h"
#include "msghandler.h"
#include "psc_errmsg.h"
#include "TuningParameter.h"
#include "selective_debug.h"

extern bool search_done;

template<class req_t, class reply_t>
int ACCL_Command_Handler<req_t, reply_t>::on_req( ACE_InputCDR& cdr ) {
    analysis_agent_accl_statemachine_.process_event( request_event() );
    req_t   req;
    reply_t reply;
    //psc_dbgmsg( 5, "on_req()\n" );
    // 4 bytes overhead(byte order,command, tag,reserved)
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ),
                "Recvd CDR SIZE req: %ld + 4bytes overhead\n",
                ( cdr.length() > 4 ) ? ( cdr.length() - 4 ) : 0 );

    cdr >> req;
    if( accl_handler_ && req_handler_ ) {
        ( accl_handler_->*req_handler_ )( req, reply );
    }

    psc_dbgmsg( 8, "Processed req: %s(size:%d)\n", command_text[ ( int )req_command_ ], req.size() );

    accl_handler_->last_command_ = req_command_;
    req_data_                    = req;

    // If the Reply is handled, send it
    if( reply_handler_ ) {
        send_reply( reply );
    }

    return 1;
}

template<class req_t, class reply_t>
int ACCL_Command_Handler<req_t, reply_t>::on_reply( ACE_InputCDR& cdr ) {
    reply_t reply;
    cdr >> reply;
    if( accl_handler_ && reply_handler_ ) {
        //psc_dbgmsg(5, "%x %x\n", accl_handler_, reply_handler_);
        ( accl_handler_->*reply_handler_ )( reply );
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Processed reply: %s(size:%d)\n", command_text[ ( int )reply_command_ ], reply.size() );

    accl_handler_->last_command_ = reply_command_;
    reply_data_                  = reply;

    return 1;
}

template<class req_t, class reply_t>
int ACCL_Command_Handler<req_t, reply_t>::send_req( req_t& req ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Sending request: %s (size:%d)\n", command_text[ ( int )req_command_ ], req.size() );

//  const ssize_t size = 10000;
//  ACE_OutputCDR out( size );
//ACE_OutputCDR out( sizeof( ACE_CDR::Long ) + req.size() );
    ACE_OutputCDR out( req.size() );

    out << ( ACE_CDR::Long )req_command_;
    out << req; // Use one of the << operators defined in the header to convert

    accl_handler_->send_msg( ( ACE_Message_Block* )out.begin() );

    return 1;
}

template<class req_t, class reply_t>
int ACCL_Command_Handler<req_t, reply_t>::send_reply( reply_t& reply ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Sending reply: %s (size:%d)\n", command_text[ ( int )reply_command_ ], reply.size() );

//  const ssize_t size = 1000;
//  ACE_OutputCDR out( size );
//ACE_OutputCDR out( sizeof( ACE_CDR::Long ) + reply.size() );
    ACE_OutputCDR out( reply.size() );

    out << ( ACE_CDR::Long )reply_command_;
    out << reply; // Use one of the << operators defined in the header to convert

    accl_handler_->send_msg( ( ACE_Message_Block* )out.begin() );

    return 1;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////


int ACCL_Handler::on_msg( ACE_Message_Block* block ) {
    ACE_InputCDR cdr( block );

    ACE_CDR::Long command;
    cdr >> command;

    last_command_ = ( command_t )command;

    switch( last_command_ ) {
    case QUIT_REQ:
        quit_handler.on_req( cdr );
        break;

    case QUIT_REPLY:
        quit_handler.on_reply( cdr );
        break;

    case REINIT_REQ:
        reinit_handler.on_req( cdr );
        break;

    case REINIT_REPLY:
        reinit_handler.on_reply( cdr );
        break;

    case INIT_REQ:
        init_handler.on_req( cdr );
        break;

    case INIT_REPLY:
        init_handler.on_reply( cdr );
        break;

    case START_REQ:
        start_handler.on_req( cdr );
        break;

    case START_REPLY:
        start_handler.on_reply( cdr );
        break;

    case HEARTBEAT_REQ:
        heartbeat_handler.on_req( cdr );
        break;

    case HEARTBEAT_REPLY:
        heartbeat_handler.on_reply( cdr );
        break;

    case CHECK_REQ:
        check_handler.on_req( cdr );
        break;

    case CHECK_REPLY:
        check_handler.on_reply( cdr );
        break;

    case SETPARENT_REQ:
        setparent_handler.on_req( cdr );
        break;

    case SETPARENT_REPLY:
        setparent_handler.on_reply( cdr );
        break;

    case SEARCHFINISHED_REQ:
        searchfinished_handler.on_req( cdr );
        break;

    case SEARCHFINISHED_REPLY:
        searchfinished_handler.on_reply( cdr );
        break;

    case NEEDRESTART_REQ:
        needrestart_handler.on_req( cdr );
        break;

    case NEEDRESTART_REPLY:
        needrestart_handler.on_reply( cdr );
        break;

    case TERMINATE_REQ:
        terminate_handler.on_req( cdr );
        break;

    case TERMINATE_REPLY:
        terminate_handler.on_reply( cdr );
        break;

    case TERMINATED_REQ:
        terminated_handler.on_req( cdr );
        break;

    case TERMINATED_REPLY:
        terminated_handler.on_reply( cdr );
        break;

    case PROPERTIESSENT_REQ:
        propertiessent_handler.on_req( cdr );
        break;

    case PROPERTIESSENT_REPLY:
        propertiessent_handler.on_reply( cdr );
        break;

    case REQEXPERIMENT_REQ:
        reqexperiment_handler.on_req( cdr );
        break;

    case REQEXPERIMENT_REPLY:
        reqexperiment_handler.on_reply( cdr );
        break;

    case STARTEXPERIMENT_REQ:
        startexperiment_handler.on_req( cdr );
        break;

    case STARTEXPERIMENT_REPLY:
        startexperiment_handler.on_reply( cdr );
        break;

    case FOUNDPROP_REQ:
        foundprop_handler.on_req( cdr );
        break;

    case FOUNDPROP_REPLY:
        foundprop_handler.on_reply( cdr );
        break;

    case CALLTREESERIAL_REQ:
        calltreeserial_handler.on_req( cdr );
        break;

    case CALLTREESERIAL_REPLY:
        calltreeserial_handler.on_reply( cdr );
        break;

    case CALLTREE_REQ:
        calltree_handler.on_req( cdr );
        break;

    case CALLTREE_REPLY:
        calltree_handler.on_reply( cdr );
        break;

    case CALLTREESENT_REQ:
        calltreesent_handler.on_req( cdr );
        break;

    case CALLTREESENT_REPLY:
        calltreesent_handler.on_reply( cdr );
        break;

    }
    ;
    return 1;
    //  psc_dbgmsg( 3, "Lenght of message block: %d\n", block->length() );
}

int ACCL_Handler::send_msg( ACE_Message_Block* block ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Sending message in ACCL_Handler::Length of message block: %d\n", block->length() );
    return MsgHandler::send_msg( MSG_TYPE_ACCL_COMMAND, 0, block );
}

/**
 * @brief Handle a quit request
 *
 * This method is normally overridden by the derived class.
 * It terminates the analysis and the communication.
 */
int ACCL_Handler::on_quit( quit_req_t& req, quit_reply_t& reply ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), " ACCL_Handler::on_quit() \n" );

    quit_handler.send_reply( reply );

    peer_.close();

    search_done = true;
    return 1;
}

int ACCL_Handler::quit() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Send quit() \n" );

    // send an empty quit message
    quit_req_t req;
    quit_handler.send_req( req );

// no response is sent on the quit message,
// the connection is simply teared down...
    handle_msg();

//#ifdef __p575
//    peer_.close();
//#else
//    if ( last_command_ != QUIT_REPLY )
//    {
//        psc_errmsg( "quit() failed\n" );
//    }
//    else
//    {
    peer_.close();
//    }
//#endif
    return 1;
}

/**
 * @brief Handle a reinit request
 *
 * This method is normally overridden by the derived class.
 * It provides protocol specific agents reinitialization.
 */
int ACCL_Handler::on_reinit( reinit_req_t& req, reinit_reply_t& reply ) {
    //  psc_dbgmsg( 5, " ACCL_Handler::on_reinit() \n" );

    /*  reinit_handler.send_reply( reply );

       return 1; */
    return 0;
}

int ACCL_Handler::reinit( int map_len, int map_from[ 8192 ], int map_to[ 8192 ] ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), " Send reinit() \n" );

    reinit_req_t req;
    if( map_len < 8192 ) {
        req.maplen = map_len;
    }
    else {
        req.maplen = 8192;
    }

    for( int i = 0; i < req.maplen; i++ ) {
        req.idmap[ i ].idf = map_from[ i ];
        req.idmap[ i ].idt = map_to[ i ];
    }
    reinit_handler.send_req( req );

    /*  psc_dbgmsg( 5, " ACCL_Handler::reinit() done! \n" );

       handle_msg();
       if ( last_command_ != REINIT_REPLY )
       {
           psc_errmsg( "reinit() failed [%i]\n",last_command_ );
       } else
       {

       }*/
    return 1;
}

int ACCL_Handler::init() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Send init() \n" );
    init_handler.send_req( my_ident_ );

    // receive the reply
    handle_msg();

    if( last_command_ != INIT_REPLY ) {
        psc_errmsg( "init() failed\n" );
    }
    else {
        psc_errmsg( "init resulted in %s\n", init_handler.reply_data_.name.c_str() );
    }
    return 1;
    // check return type,
}

/**
 * @brief Handle a init request
 *
 * This method is normally overridden by the derived class.
 * It provides protocol specific agents initialization.
 */
int ACCL_Handler::on_init( init_req_t& req, init_reply_t& reply ) {
    peer_ident_ = req;
    reply       = my_ident_;

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "sending ident: %s\n", my_ident_.name.c_str() );
    return 1;
}

/**
 * @brief Handle a start request
 *
 * This method is normally overridden by the derived class.
 * It starts the analysis.
 *
 */
int ACCL_Handler::on_start( start_req_t& req, start_reply_t& reply ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_Handler::on_start()\n" );
    return 1;
}

/**
 * @brief Sends a message with a Strategy configuration information and starts a strategy.
 *
 */
int ACCL_Handler::start( ACE_CDR::ULong container_size, ACE_CDR::Octet* container_payload ) {
    start_req_t req;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Send start(%d) \n", container_size );
    req.serialized_strategy_request_container.size    = container_size;
    req.serialized_strategy_request_container.payload = container_payload;

    start_handler.send_req( req );
    return 1;
}

/**
 * @brief Handle a check request
 *
 * This method is normally overridden by the derived class.
 * It checks for new properties and processes them.
 */
int ACCL_Handler::on_check( check_req_t& req, check_reply_t& reply ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_Handler::on_check\n" );
    return 1;
}

/**
 * @brief Send a message with a check request
 */
int ACCL_Handler::check() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Send check() \n" );

    check_req_t req;
    check_handler.send_req( req );
    return 1;
}

/**
 * @brief Handle a heartbeat request
 *
 * This method is normally overridden by the derived class.
 */
int ACCL_Handler::on_heartbeat( heartbeat_req_t& req, heartbeat_reply_t& reply ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_Handler::on_heartbeat\n" );
    return 1;
}

/**
 * @brief Send a message with a heartbeat request
 */
int ACCL_Handler::heartbeat( std::string hostname, int port, std::string tag, int heartbeat_type, int num_procs ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Send heartbeat() \n" );
    heartbeat_t hp;

    hp.hostname       = hostname;
    hp.port           = port;
    hp.tag            = tag;
    hp.heartbeat_type = heartbeat_type;
    hp.num_procs      = num_procs;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Sending ACCL_Handler::heartbeat: hostname=%s,"
                "port=%d, tag=%s, forwarded=%d, num_procs=%d\n", hostname.c_str(), port, tag.c_str(), heartbeat_type, num_procs );
    heartbeat_handler.send_req( hp );
    return 1;
}

/**
 * @brief Handle a setparent request
 *
 * This method is normally overridden by the derived class.
 * It provides protocol specific processing of the request.
 */
int ACCL_Handler::on_setparent( setparent_req_t& req, setparent_reply_t& reply ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_Handler::on_setparent\n" );
    return 1;
}

/// Send a message with a setparent request
int ACCL_Handler::setparent( std::string hostname, int port ) {
    parent_t sp;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Send setparent() \n" );

    sp.hostname = hostname;
    sp.port     = port;

    setparent_handler.send_req( sp );
    return 1;
}

/**
 * @brief Handle a searchfinished request
 *
 * This method is normally overridden by the derived class.
 * It provides protocol specific processing of the request.
 */
int ACCL_Handler::on_searchfinished( searchfinished_req_t& req, searchfinished_reply_t& reply ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_Handler::on_searchfinished\n" );
    return 1;
}

/**
 * @brief Send a message with an searchfinished request
 */
int ACCL_Handler::searchfinished( std::string tag ) {
    searchfinished_t req;
    req.tag = tag;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Send searchfinished() \n" );

    searchfinished_handler.send_req( req );
    return 1;
}

/**
 * @brief Handle a needrestart request
 *
 * This method is normally overridden by the derived class.
 * It provides protocol specific processing of the request.
 */
int ACCL_Handler::on_needrestart( needrestart_req_t& req, needrestart_reply_t& reply ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_Handler::on_needrestart\n" );
    return 1;
}

/**
 * @brief Send a message with an needrestart request
 */
int ACCL_Handler::needrestart( std::string tag ) {
    needrestart_t req;
    req.tag = tag;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Send needrestart() \n" );

    needrestart_handler.send_req( req );
    return 1;
}


/**
 * @brief Handle a terminate request
 *
 * This method is normally overridden by the derived class.
 * It provides protocol specific processing of the request.
 */
int ACCL_Handler::on_terminate( terminate_req_t& req, terminate_reply_t& reply ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_Handler::on_terminate\n" );
    return 1;
}

/*
 * @brief Send a message with an terminate request
 */
int ACCL_Handler::terminate() {
    terminate_t req;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Send terminate() \n" );

    terminate_handler.send_req( req );
    return 1;
}

/**
 * @brief Handle a terminated request
 *
 * This method is normally overridden by the derived class.
 * It provides protocol specific processing of the request.
 */
int ACCL_Handler::on_terminated( terminated_req_t& req, terminated_reply_t& reply ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_Handler::on_terminated\n" );
    return 1;
}

/**
 * @brief Send a message with an terminated request
 */
int ACCL_Handler::terminated( std::string tag ) {
    terminated_t req;
    req.tag = tag;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Send terminated() \n" );

    terminated_handler.send_req( req );
    return 1;
}

/**
 * @brief Handle a propertiessent request
 *
 * This method is normally overridden by the derived class.
 * It provides protocol specific processing of the request.
 */
int ACCL_Handler::on_propertiessent( propertiessent_req_t& req, propertiessent_reply_t& reply ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_Handler::on_propertiessent\n" );
    return 1;
}

/**
 * @brief Send a message with a propertiessent request
 */
int ACCL_Handler::propertiessent( std::string tag ) {
    propertiessent_t req;
    req.tag = tag;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Send propertiessent() \n" );

    propertiessent_handler.send_req( req );
    return 1;
}

/**
 * @brief Handle a reqexperiment request
 *
 * This method is normally overridden by the derived class.
 * It provides protocol specific processing of the request.
 */
int ACCL_Handler::on_reqexperiment( reqexperiment_req_t& req, reqexperiment_reply_t& reply ) {
    return 0;
}

/**
 * @brief Send a message with an experiment request
 */
int ACCL_Handler::reqexperiment( std::string tag ) {
    reqexperiment_t req;
    req.tag = tag;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Send reqexperiment() \n" );

    reqexperiment_handler.send_req( req );
    return 0;
}

/**
 * @brief Handle a startexperiment request
 *
 * This method is normally overridden by the derived class.
 * It provides protocol specific processing of the request.
 */
int ACCL_Handler::on_startexperiment( startexperiment_req_t& req, startexperiment_reply_t& reply ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_Handler::on_startexperiment\n" );
    return 1;
}

/**
 * @brief Send a message with a startexperiment request
 */
int ACCL_Handler::startexperiment() {
    startexperiment_t req;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Send startexperiment() \n" );
    startexperiment_handler.send_req( req );
    return 1;
}


/**
 * @brief Handle a foundprop request
 *
 * This method is normally overridden by the derived class.
 * It processes a received property.
 */
int ACCL_Handler::on_foundprop( foundprop_req_t& req, foundprop_reply_t& reply ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_Handler::on_foundprop\n" );
    return 1;
}

/**
 * @deprecated Properties are transmitted/processed using XML
 *
 * @brief Generates a found property request using specified arguments
 *
 * This method is generates a found property request from the specified options.
 * It creates a string-based property structure for transferring the property
 * and sends it to the peer agent.
 */
int ACCL_Handler::foundprop( std::string hostname, std::string name, double severity, double confidence, int numthreads, std::string context ) {
    foundprop_t fp;
    //psc_dbgmsg(5, "Send foundprop() \n");

    fp.xmlData = "ERROR! WRONG Fn!";
//  fp.prop_name = name;
//  fp.severity = severity;
//  fp.confidence = confidence;
//  fp.context = context;
//  fp.numthreads = numthreads;

    foundprop_handler.send_req( fp );
    return 1;
}

/**
 * @brief Generates a found property request from a string
 *
 * This method is generates a found property request from a string for
 * transferring the property and sends it to the peer agent.
 */
int ACCL_Handler::foundprop( std::string& propData ) {
    foundprop_t fp;
    fp.xmlData = propData;
    //psc_dbgmsg( 5, "Send foundprop() \n" );

    foundprop_handler.send_req( fp );
    return 1;
}

/**
 * @brief Generates a found property request
 *
 * This method is generates a foundprop request from string-based
 * property structure for transferring the property and sends it to
 * the peer agent.
 */
int ACCL_Handler::foundprop( foundprop_t& fp ) {
    //psc_dbgmsg( 5, "Send foundprop() \n" );
    foundprop_handler.send_req( fp );
    return 1;
}


/**
 * @brief Send a message with a serialize call-tree request
 */
int ACCL_Handler::serializecalltree() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Send serializecalltree() \n" );

    calltreeserial_req_t req;
    calltreeserial_handler.send_req( req );
    return 1;
}


/**
 * @brief Handle a serialize call-tree request
 *
 * This method is normally overridden by the derived class.
 * It checks for new call-tree nodes and processes them.
 */
int ACCL_Handler::on_serializecalltree( calltreeserial_req_t& req, calltreeserial_reply_t& reply ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_Handler::on_serializecalltree\n" );
    return 1;
}


/**
 * @brief Generates an call-tree request from a string
 *
 * This method generates a call-tree request from a string for
 * transferring the call-tree and sends it to the peer agent/ HLagent
 */
int ACCL_Handler::sendcalltree( std::string& calltreeData ) {
    calltree_t ct;

    ct.xmlData = calltreeData;
    psc_dbgmsg( 5, "Send sendcalltree() \n" );
    calltree_handler.send_req( ct );
    return 1;
}

/**
 * @brief Handle a received call-tree node
 *
 * This method is normally overridden by the derived class.
 * It processes the received call-tree node.
 */
int ACCL_Handler::on_calltree( calltree_req_t& req, calltree_reply_t& reply ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_Handler::on_calltree\n" );
    return 1;
}


/**
 * @brief Sends a message that the serialized calltree was sent from one/more aagent
 */

int ACCL_Handler::calltreesent( std::string tag ) {
    calltreesent_t req;
    req.tag = tag;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Send calltreesent() \n" );

    calltreesent_handler.send_req( req );
    return 1;
}


/**
 * @brief Handle a calltreesent message
 *
 * This method is normally overridden by the derived class.
 */
int ACCL_Handler::on_calltreesent( calltreesent_req_t& req, calltreesent_reply_t& reply ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_Handler::on_calltreesent\n" );
    return 1;
}


//
// helper for C++ strings
//
int operator<<( ACE_OutputCDR& cdr, const std::string& str ) {
    size_t len = str.length();

    cdr << ACE_CDR::ULong( len );
    cdr.write_char_array( str.c_str(), len );

    return cdr.good_bit();
}

int operator>>( ACE_InputCDR& cdr, std::string& str ) {
    ACE_CDR::ULong len;
    cdr >> len;
    char* buf = new char[ len + 1 ];

    cdr.read_char_array( buf, len );
    buf[ len ] = '\0';
    str        = buf;

    delete[] buf;
    return cdr.good_bit();
}

//
// reinit_t
//
int operator<<( ACE_OutputCDR& cdr, const ACCL_Handler::reinit_t& hp ) {
    cdr << hp.maplen;
    for( int i = 0; i < 8192; i++ ) {
        cdr << hp.idmap[ i ].idf;
        cdr << hp.idmap[ i ].idt;
    }

    return cdr.good_bit();
}

int operator>>( ACE_InputCDR& cdr, ACCL_Handler::reinit_t& hp ) {
    cdr >> hp.maplen;
    for( int i = 0; i < 8192; i++ ) {
        cdr >> hp.idmap[ i ].idf;
        cdr >> hp.idmap[ i ].idt;
    }

    return cdr.good_bit();
}

//
// start_t
//
int operator<<( ACE_OutputCDR& cdr, ACCL_Handler::start_t& hp ) {
    cdr << hp.serialized_strategy_request_container.size;
    cdr.write_octet_array( ( ACE_CDR::Octet* )hp.serialized_strategy_request_container.payload, hp.serialized_strategy_request_container.size );

    return cdr.good_bit();
}

int operator>>( ACE_InputCDR& cdr, ACCL_Handler::start_t& hp ) {
    cdr >> hp.serialized_strategy_request_container.size;
    // TODO: RM: Here is a memory leak. Should free memory afterwards.
    ACE_CDR::Octet* pbuffer;
    pbuffer = ( ACE_CDR::Octet* )malloc( hp.serialized_strategy_request_container.size );
    // TODO: Put it into the C++ helper overloaded operators
    cdr.read_octet_array( pbuffer, hp.serialized_strategy_request_container.size );
    hp.serialized_strategy_request_container.payload = pbuffer;
    return cdr.good_bit();
}

//
// ident_t
//
int operator<<( ACE_OutputCDR& cdr, const ACCL_Handler::ident_t& id ) {
    cdr << id.name;

    // todo: implement me
    return cdr.good_bit();
}

int operator>>( ACE_InputCDR& cdr, ACCL_Handler::ident_t& id ) {
    cdr >> id.name;
    // todo: implement me
    return cdr.good_bit();
}

//
// empty_t
//
int operator<<( ACE_OutputCDR& cdr, const ACCL_Handler::empty_t& id ) {
    // nothing to do...
    return cdr.good_bit();
}

int operator>>( ACE_InputCDR& cdr, ACCL_Handler::empty_t& id ) {
    // nothing to do...
    return cdr.good_bit();
}

//
// heartbeat_t
//
int operator<<( ACE_OutputCDR& cdr, const ACCL_Handler::heartbeat_t& hp ) {
    ACE_CDR::Long port, h_type, procs;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_Handler:: heartbeat << %s %s %d %d %d\n",
                hp.hostname.c_str(), hp.tag.c_str(), hp.port, hp.heartbeat_type, hp.num_procs );
    cdr << hp.hostname;
    cdr << hp.tag;
    port = ( ACE_CDR::Long )hp.port;
    cdr << port;
    h_type = ( ACE_CDR::Long )hp.heartbeat_type;
    cdr << h_type;
    procs = ( ACE_CDR::Long )hp.num_procs;
    cdr << procs;
    return cdr.good_bit();
}

int operator>>( ACE_InputCDR& cdr, ACCL_Handler::heartbeat_t& hp ) {
    ACE_CDR::Long port, h_type, procs;
    cdr >> hp.hostname;
    cdr >> hp.tag;
    cdr >> port;
    hp.port = ( int )port;
    cdr >> h_type;
    hp.heartbeat_type = ( int )h_type;
    cdr >> procs;
    hp.num_procs = ( int )procs;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_Handler:: heartbeat >> %s %s %d %d %d\n",
                hp.hostname.c_str(), hp.tag.c_str(), hp.port, hp.heartbeat_type, hp.num_procs );
    return cdr.good_bit();
}

//
// parent_t
//
int operator<<( ACE_OutputCDR& cdr, const ACCL_Handler::parent_t& hp ) {
    cdr << hp.hostname;
    cdr << hp.port;

    return cdr.good_bit();
}

int operator>>( ACE_InputCDR& cdr, ACCL_Handler::parent_t& hp ) {
    cdr >> hp.hostname;
    cdr >> hp.port;

    return cdr.good_bit();
}

//
// foundprop_t
//
int operator<<( ACE_OutputCDR& cdr, const ACCL_Handler::foundprop_t& fp ) {
//  cdr << fp.hostname;
//  cdr << fp.prop_name;
//  cdr << fp.severity;
//  cdr << fp.confidence;
//  cdr << fp.numthreads;
//  cdr << fp.context;

    cdr << fp.xmlData;

    return cdr.good_bit();
}

int operator>>( ACE_InputCDR& cdr, ACCL_Handler::foundprop_t& fp ) {
//  cdr >> fp.hostname;
//  cdr >> fp.prop_name;
//  cdr >> fp.severity;
//  cdr >> fp.confidence;
//  cdr >> fp.numthreads;
//  cdr >> fp.context;
    cdr >> fp.xmlData;

    return cdr.good_bit();
}

//
// searchfinished_t
//
int operator<<( ACE_OutputCDR& cdr, const ACCL_Handler::searchfinished_t& ae ) {
    cdr << ae.tag;

    return cdr.good_bit();
}

int operator>>( ACE_InputCDR& cdr, ACCL_Handler::searchfinished_t& ae ) {
    cdr >> ae.tag;

    return cdr.good_bit();
}

//
// reqexperiment_t
//
int operator<<( ACE_OutputCDR& cdr, const ACCL_Handler::reqexperiment_t& ae ) {
    cdr << ae.tag;

    return cdr.good_bit();
}

int operator>>( ACE_InputCDR& cdr, ACCL_Handler::reqexperiment_t& ae ) {
    cdr >> ae.tag;

    return cdr.good_bit();
}

//
// needrestart_t
//
int operator<<( ACE_OutputCDR& cdr, const ACCL_Handler::needrestart_t& ae ) {
    cdr << ae.tag;

    return cdr.good_bit();
}

int operator>>( ACE_InputCDR& cdr, ACCL_Handler::needrestart_t& ae ) {
    cdr >> ae.tag;

    return cdr.good_bit();
}

//
// terminate_t
//
int operator<<( ACE_OutputCDR& cdr, const ACCL_Handler::terminate_t& ae ) {
    return cdr.good_bit();
}

int operator>>( ACE_InputCDR& cdr, ACCL_Handler::terminate_t& ae ) {
    return cdr.good_bit();
}

//
// terminated_t
//
int operator<<( ACE_OutputCDR& cdr, const ACCL_Handler::terminated_t& ae ) {
    cdr << ae.tag;

    return cdr.good_bit();
}

int operator>>( ACE_InputCDR& cdr, ACCL_Handler::terminated_t& ae ) {
    cdr >> ae.tag;

    return cdr.good_bit();
}

//
// propertiessent_t
//
int operator<<( ACE_OutputCDR& cdr, const ACCL_Handler::propertiessent_t& ae ) {
    cdr << ae.tag;

    return cdr.good_bit();
}

int operator>>( ACE_InputCDR& cdr, ACCL_Handler::propertiessent_t& ae ) {
    cdr >> ae.tag;

    return cdr.good_bit();
}

//
// startexperiment_t
//
int operator<<( ACE_OutputCDR& cdr, const ACCL_Handler::startexperiment_t& ae ) {
    return cdr.good_bit();
}

int operator>>( ACE_InputCDR& cdr, ACCL_Handler::startexperiment_t& ae ) {
    return cdr.good_bit();
}

//
//calltree_t
//
int operator<<( ACE_OutputCDR& cdr, const ACCL_Handler::calltree_t& ct ) {

    cdr << ct.xmlData;
    return cdr.good_bit();
}

int operator>>( ACE_InputCDR& cdr, ACCL_Handler::calltree_t& ct ) {

    cdr >> ct.xmlData;
    return cdr.good_bit();
}

//
// calltreesent_t
//
int operator<<( ACE_OutputCDR& cdr, const ACCL_Handler::calltreesent_t& cts ) {

    cdr << cts.tag;
    return cdr.good_bit();
}

int operator>>( ACE_InputCDR& cdr, ACCL_Handler::calltreesent_t& cts ) {

    cdr >> cts.tag;
    return cdr.good_bit();
}
