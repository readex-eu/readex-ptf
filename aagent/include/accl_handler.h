/**
   @file    accl_handler.h
   @ingroup Communication
   @brief   Generic communication layer header
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

#ifndef ACCL_HANDLER_H_INCLUDED
#define ACCL_HANDLER_H_INCLUDED

#include <string>
#include <ace/CDR_Stream.h>
#include <list>

#include "msghandler.h"
#include "psc_errmsg.h"
#include "TuningParameter.h"
#include "accl_statemachine.h"
using namespace aagent_accl_msm_namespace;

/// Supported handler commands
enum command_t {
    INIT_REQ,              ///< Initialization Request
    INIT_REPLY,            ///< Initialization Reply
    QUIT_REQ,              ///< Quit Request
    QUIT_REPLY,            ///< Quit Reply
    REINIT_REQ,            ///< Reinitialization Request
    REINIT_REPLY,          ///< Reinitialization Reply
    START_REQ,             ///< Start Request
    START_REPLY,           ///< Start Reply
    HEARTBEAT_REQ,         ///< Heartbeat Request
    HEARTBEAT_REPLY,       ///< Heartbeat Reply
    CHECK_REQ,             ///< Propagate the properties request
    CHECK_REPLY,           ///< Propagate the properties reply
    FOUNDPROP_REQ,         ///< Recv prop request???
    FOUNDPROP_REPLY,       ///< Recv prop reply???
    SEARCHFINISHED_REQ,    ///< Search finished Request
    SEARCHFINISHED_REPLY,  ///< Search finished Reply
    NEEDRESTART_REQ,       ///< Restart Request
    NEEDRESTART_REPLY,     ///< Restart Reply
    TERMINATE_REQ,         ///< Terminate application Request
    TERMINATE_REPLY,       ///< Terminate application Reply
    TERMINATED_REQ,        ///< Terminated Request
    TERMINATED_REPLY,      ///< Terminated Reply
    PROPERTIESSENT_REQ,    ///< All properties were sent
    PROPERTIESSENT_REPLY,  ///< All properties were sent
    REQEXPERIMENT_REQ,     ///< Experiment Request
    REQEXPERIMENT_REPLY,   ///< Experiment Reply
    STARTEXPERIMENT_REQ,   ///< Start experiment Request
    STARTEXPERIMENT_REPLY, ///< Start experiment Reply
    SETPARENT_REQ,         ///< Set the agent's parent request
    SETPARENT_REPLY,       ///< Set the agent's parent reply
    CALLTREESERIAL_REQ,    ///< Propagate the serialize call-tree request
    CALLTREESERIAL_REPLY,  ///< Propagate the serialize call-tree reply
    CALLTREE_REQ,          ///< Receive the serialized call-tree
    CALLTREE_REPLY,        ///< Receive the serialized call-tree reply
    CALLTREESENT_REQ,      ///< Call-tree was sent
    CALLTREESENT_REPLY     ///< Call-tree was sent
};

// Preproc macro to convert ENUMs to Strings
#define stringify( name ) # name

//BGP port Heartbeat V1 keys:
#define DISMISS_HEARTBEAT       2
#define FORWARDED_HEARTBEAT     1
#define OWN_HEARTBEAT           0

typedef struct serialized_strategy_request_container_t {
    ACE_CDR::ULong  size;
    ACE_CDR::Octet* payload;
} serialized_strategy_request_container_t;

// RM: Check, maybe to unify it with strategy request container
typedef struct serialized_property_list_container_t {
    ACE_CDR::ULong  size;
    ACE_CDR::Octet* payload;
} serialized_property_list_container_t;

// ENUM value to string conversion table used for
const char* const command_text[] = {
    stringify( INIT_REQ ),
    stringify( INIT_REPLY ),
    stringify( QUIT_REQ ),
    stringify( QUIT_REPLY ),
    stringify( REINIT_REQ ),
    stringify( REINIT_REPLY ),
    stringify( START_REQ ),
    stringify( START_REPLY ),
    stringify( HEARTBEAT_REQ ),
    stringify( HEARTBEAT_REPLY ),
    stringify( CHECK_REQ ),
    stringify( CHECK_REPLY ),
    stringify( FOUNDPROP_REQ ),
    stringify( FOUNDPROP_REPLY ),
    stringify( SEARCHFINISHED_REQ ),
    stringify( SEARCHFINISHED_REPLY ),
    stringify( NEEDRESTART_REQ ),
    stringify( NEEDRESTART_REPLY ),
    stringify( TERMINATE_REQ ),
    stringify( TERMINATE_REPLY ),
    stringify( TERMINATED_REQ ),
    stringify( TERMINATED_REPLY ),
    stringify( PROPERTIESSENT_REQ ),
    stringify( PROPERTIESSENT_REPLY ),
    stringify( REQEXPERIMENT_REQ ),
    stringify( REQEXPERIMENT_REPLY ),
    stringify( STARTEXPERIMENT_REQ ),
    stringify( STARTEXPERIMENT_REPLY ),
    stringify( SETPARENT_REQ ),
    stringify( SETPARENT_REPLY ),
    stringify( CALLTREESERIAL_REQ ),
    stringify( CALLTREESERIAL_REPLY ),
    stringify( CALLTREE_REQ ),
    stringify( CALLTREE_REPLY ),
    stringify( CALLTREESENT_REQ ),
    stringify( CALLTREESENT_REPLY )
};

class ACCL_Handler;

/**
 * @class ACCL_Command_Handler
 * @ingroup AnalysisAgent
 *
 * @brief Command handler
 *
 */
template<class req_t, class reply_t>
class ACCL_Command_Handler {
private:
    typedef int (ACCL_Handler::* req_handler_t)( req_t&   req,
                                                 reply_t& reply );        ///< ACCL Request Handler Type
    typedef int (ACCL_Handler::* reply_handler_t)( reply_t& reply );      ///< ACCL Reply Handler Type

    req_handler_t                    req_handler_;
    reply_handler_t                  reply_handler_;
    ACCL_Handler*                    accl_handler_;
    command_t                        req_command_;
    command_t                        reply_command_;
    analysis_agent_accl_statemachine analysis_agent_accl_statemachine_;

public:
    req_t   req_data_;
    reply_t reply_data_;

public:
    ACCL_Command_Handler<req_t, reply_t> ( ACCL_Handler * accl_handler, command_t req_command, command_t reply_command,
                                           req_handler_t reqh = 0, reply_handler_t replyh = 0 ) {
        req_command_   = req_command;
        reply_command_ = reply_command;
        accl_handler_  = accl_handler;
        req_handler_   = reqh;
        reply_handler_ = replyh;
        analysis_agent_accl_statemachine_.start();
    }

    int on_req( ACE_InputCDR& cdr );

    int on_reply( ACE_InputCDR& cdr );

    int send_req( req_t& req );

    int send_reply( reply_t& reply );
};

/**
 * @class ACCL_Handler
 * @ingroup AnalysisAgent
 *
 * @brief More general class
 *
 */
class ACCL_Handler : public MsgHandler {
protected:
    // TODO:
    // the msg type used for ACCL command messages
    // results should possibly get an own message type
    // what about streaming data?
    //
    const msg_type_t MSG_TYPE_ACCL_COMMAND;

protected:
    // public:
    typedef struct {
        size_t size() {
            return 0;
        }
    } empty_t;

    /// @brief Identification of the handler
    struct ident_t {
        std::string name; ///< Name of the Handler
        size_t      size() {
            return name.length();
        }
    };

    /// @brief Message Data
    struct heartbeat_t {
        //heartbeat_t() : port(0), heartbeat_type(0),num_procs(0) {}

        std::string hostname;       ///< Hostname
        std::string tag;            ///< Tag
        int         port;           ///< Port
        int         heartbeat_type; ///< BGP Port V1 specific. true if it is a forwarded heartbeat, which is counted in FE
        int         num_procs;      ///< number of recently connected application procs

        size_t size() {
            //return (hostname.length() + tag.length() + 3 * sizeof(int));
            return sizeof( char ) * ( hostname.length() + tag.length() + 2 ) + 3 * ( sizeof( ACE_CDR::Long ) );
        }
    };

    // TODO: 8192 elements??
    struct reinit_t {
        int maplen;
        struct idmap_t {
            int idf, idt;
        } idmap[ 8192 ];

        size_t size() {
            return sizeof( int ) + 8192 * sizeof( idmap_t );
        }
    };

    /// @brief Property type used in the data transfer (XML based)
    struct foundprop_t {
        std::string xmlData;    ///< Property in XML format
        size_t      size() {
            return xmlData.length();
        }
    };

    // @brief Call-tree in XML format used during data transfer
    struct calltree_t {
        std::string xmlData;    ///< call-tree in XML format
        size_t      size() {
            return xmlData.length();
        }
    };

public:

    struct start_t {
        serialized_strategy_request_container_t serialized_strategy_request_container;

        size_t size() {
            return sizeof( ACE_CDR::ULong ) + sizeof( ACE_CDR::Octet ) * serialized_strategy_request_container.size;
        }
    };

    struct searchfinished_t {
        std::string tag;
        size_t      size() {
            return tag.length();
        }
    };

    struct needrestart_t {
        std::string tag;
        size_t      size() {
            return tag.length();
        }
    };

    struct terminated_t {
        std::string tag;
        size_t      size() {
            return tag.length();
        }
    };

    struct propertiessent_t {
        std::string tag;
        size_t      size() {
            return tag.length();
        }
    };

    struct calltreesent_t {
        std::string tag;
        size_t      size() {
            return tag.length();
        }
    };


    struct reqexperiment_t {
        std::string tag;
        size_t      size() {
            return tag.length();
        }
    };

    struct startexperiment_t {
        size_t size() {
            return 0;
        }
    };

    struct terminate_t {
        size_t size() {
            return 0;
        }
    };

    struct parent_t {
        std::string hostname;
        int         port;
        size_t      size() {
            return hostname.length() + sizeof( int );
        }
    };

protected:
    //
    // INIT
    //
    typedef ident_t init_req_t;
    typedef ident_t init_reply_t;

    //
    // QUIT messages don't carry data
    //
    typedef empty_t quit_req_t;
    typedef empty_t quit_reply_t;

    //
    // REINIT
    //
    typedef reinit_t reinit_req_t;
    typedef reinit_t reinit_reply_t;

    //
    // START
    //
    typedef start_t start_req_t;
    typedef empty_t start_reply_t;

    //
    // HEARTBEAT
    //
    typedef heartbeat_t heartbeat_req_t;
    typedef heartbeat_t heartbeat_reply_t;

    //
    // FOUNDPROP
    //
    typedef foundprop_t foundprop_req_t;
    typedef empty_t foundprop_reply_t;

    //
    // CHECK
    //
    typedef empty_t check_t;
    typedef empty_t check_req_t;
    typedef empty_t check_reply_t;

    //
    // SEARCHFINISHED
    //
    typedef searchfinished_t searchfinished_req_t;
    typedef empty_t searchfinished_reply_t;

    //
    // NEEDRESTART
    //
    typedef needrestart_t needrestart_req_t;
    typedef empty_t needrestart_reply_t;

    //
    // TERMINATE
    //
    typedef terminate_t terminate_req_t;
    typedef empty_t terminate_reply_t;

    //
    // TERMINATED
    //
    typedef terminated_t terminated_req_t;
    typedef empty_t terminated_reply_t;

    //
    // PROPERTIESSENT
    //
    typedef propertiessent_t propertiessent_req_t;
    typedef empty_t propertiessent_reply_t;

    //
    // REQEXPERIMENT
    //
    typedef reqexperiment_t reqexperiment_req_t;
    typedef empty_t reqexperiment_reply_t;

    //
    // STARTEXPERIMENT
    //
    typedef startexperiment_t startexperiment_req_t;
    typedef empty_t startexperiment_reply_t;

    typedef parent_t setparent_req_t;
    typedef empty_t setparent_reply_t;

    //
    //CALLTREESERIAL
    //
    typedef empty_t calltreeserial_req_t;
    typedef empty_t calltreeserial_reply_t;

    //
    // CALLTREEREQUEST
    //
    typedef calltree_t calltree_req_t;
    typedef empty_t calltree_reply_t;

    //CALLTREESENT
    typedef calltreesent_t calltreesent_req_t;
    typedef empty_t calltreesent_reply_t;


protected:
    ident_t my_ident_;
    ident_t peer_ident_;

    command_t last_command_;

    // ?? void     *last_data_;

protected:
    int send_msg( ACE_Message_Block* block );

public:
    virtual int on_quit( quit_req_t&   req,
                         quit_reply_t& reply );

    virtual int quit();

    virtual int on_reinit( reinit_req_t&   req,
                           reinit_reply_t& reply );

    virtual int reinit( int map_len, int map_from[ 8192 ], int map_to[ 8192 ] );

    virtual int on_init( init_req_t&   req,
                         init_reply_t& reply );

    virtual int init();

    virtual int on_start( start_req_t&   req,
                          start_reply_t& reply );

    virtual int start( ACE_CDR::ULong  container_size,
                       ACE_CDR::Octet* container_payload );

    virtual int on_check( check_req_t&   req,
                          check_reply_t& reply );

    virtual int check();

    virtual int on_heartbeat( heartbeat_req_t&   req,
                              heartbeat_reply_t& reply );

    virtual int heartbeat( std::string hostname,
                           int         port,
                           std::string tag,
                           int         heartbeat_type,
                           int         num_procs );

    virtual int on_setparent( setparent_req_t&   req,
                              setparent_reply_t& reply );

    virtual int setparent( std::string hostname,
                           int         port );

    virtual int on_searchfinished( searchfinished_req_t&   req,
                                   searchfinished_reply_t& reply );

    virtual int searchfinished( std::string tag );

    virtual int on_needrestart( needrestart_req_t&   req,
                                needrestart_reply_t& reply );

    virtual int needrestart( std::string tag );

    virtual int on_terminate( terminate_req_t&   req,
                              terminate_reply_t& reply );

    virtual int terminate();

    virtual int on_terminated( terminated_req_t&   req,
                               terminated_reply_t& reply );

    virtual int terminated( std::string tag );

    virtual int on_propertiessent( propertiessent_req_t&   req,
                                   propertiessent_reply_t& reply );

    virtual int propertiessent( std::string tag );

    virtual int on_reqexperiment( reqexperiment_req_t&   req,
                                  reqexperiment_reply_t& reply );

    virtual int reqexperiment( std::string tag );

    virtual int on_startexperiment( startexperiment_req_t&   req,
                                    startexperiment_reply_t& reply );

    virtual int startexperiment();

    virtual int on_foundprop( foundprop_req_t&   req,
                              foundprop_reply_t& reply );

    virtual int foundprop( std::string hostname,
                           std::string name,
                           double      severity,
                           double      confidence,
                           int         numthreads,
                           std::string context );

    virtual int foundprop( std::string& propData );

    virtual int foundprop( foundprop_t& fp );

    virtual int serializecalltree();  //Send a message with a serialize call-tree request

    //Calls save_calltree() in dataprovider when the serialize calltree request is received
    virtual int on_serializecalltree( calltreeserial_req_t&   req,
                                      calltreeserial_reply_t& reply );

    virtual int sendcalltree( std::string& calltreeData );

    //Deserializes the received call-tree in frontend
    virtual int on_calltree( calltree_req_t&   req,
                             calltree_reply_t& reply );

    virtual int on_calltreesent( calltreesent_req_t&   req,
                                 calltreesent_reply_t& reply );

    virtual int calltreesent( std::string tag ); //Sends that the call tree is sent



protected:

    ACCL_Command_Handler<quit_req_t, quit_reply_t>                       quit_handler;
    ACCL_Command_Handler<reinit_req_t, reinit_reply_t>                   reinit_handler;
    ACCL_Command_Handler<init_req_t, init_reply_t>                       init_handler;
    ACCL_Command_Handler<start_req_t, start_reply_t>                     start_handler;
    ACCL_Command_Handler<heartbeat_req_t, heartbeat_reply_t>             heartbeat_handler;
    ACCL_Command_Handler<check_req_t, check_reply_t>                     check_handler;
    ACCL_Command_Handler<setparent_req_t, setparent_reply_t>             setparent_handler;
    ACCL_Command_Handler<foundprop_req_t, foundprop_reply_t>             foundprop_handler;
    ACCL_Command_Handler<searchfinished_req_t, searchfinished_reply_t>   searchfinished_handler;
    ACCL_Command_Handler<needrestart_req_t, needrestart_reply_t>         needrestart_handler;
    ACCL_Command_Handler<terminate_req_t, terminate_reply_t>             terminate_handler;
    ACCL_Command_Handler<terminated_req_t, terminated_reply_t>           terminated_handler;
    ACCL_Command_Handler<propertiessent_req_t, propertiessent_reply_t>   propertiessent_handler;
    ACCL_Command_Handler<reqexperiment_req_t, reqexperiment_reply_t>     reqexperiment_handler;
    ACCL_Command_Handler<startexperiment_req_t, startexperiment_reply_t> startexperiment_handler;
    ACCL_Command_Handler<calltreeserial_req_t, calltreeserial_reply_t>   calltreeserial_handler;
    ACCL_Command_Handler<calltree_req_t, calltree_reply_t>               calltree_handler;
    ACCL_Command_Handler<calltreesent_req_t, calltreesent_reply_t>       calltreesent_handler;


public:
    ACCL_Handler( ACE_SOCK_Stream& peer, std::string ident ) :
        MsgHandler( peer ),
        MSG_TYPE_ACCL_COMMAND( 'c' ),
        quit_handler( this, QUIT_REQ, QUIT_REPLY, &ACCL_Handler::on_quit, 0 ),
        reinit_handler( this, REINIT_REQ, REINIT_REPLY, &ACCL_Handler::on_reinit, 0 ),
        init_handler( this, INIT_REQ, INIT_REPLY, &ACCL_Handler::on_init, 0 ),
        start_handler( this, START_REQ, START_REPLY, &ACCL_Handler::on_start, 0 ),
        heartbeat_handler( this, HEARTBEAT_REQ, HEARTBEAT_REPLY, &ACCL_Handler::on_heartbeat, 0 ),
        check_handler( this, CHECK_REQ, CHECK_REPLY, &ACCL_Handler::on_check, 0 ),
        setparent_handler( this, SETPARENT_REQ, SETPARENT_REPLY, &ACCL_Handler::on_setparent, 0 ),
        foundprop_handler( this, FOUNDPROP_REQ, FOUNDPROP_REPLY, &ACCL_Handler::on_foundprop, 0 ),
        searchfinished_handler( this, SEARCHFINISHED_REQ, SEARCHFINISHED_REPLY, &ACCL_Handler::on_searchfinished, 0 ),
        needrestart_handler( this, NEEDRESTART_REQ, NEEDRESTART_REPLY, &ACCL_Handler::on_needrestart, 0 ),
        terminate_handler( this, TERMINATE_REQ, TERMINATE_REPLY, &ACCL_Handler::on_terminate, 0 ),
        terminated_handler( this, TERMINATED_REQ, TERMINATED_REPLY, &ACCL_Handler::on_terminated, 0 ),
        propertiessent_handler( this, PROPERTIESSENT_REQ, PROPERTIESSENT_REPLY, &ACCL_Handler::on_propertiessent, 0 ),
        reqexperiment_handler( this, REQEXPERIMENT_REQ, REQEXPERIMENT_REPLY, &ACCL_Handler::on_reqexperiment, 0 ),
        startexperiment_handler( this, STARTEXPERIMENT_REQ, STARTEXPERIMENT_REPLY, &ACCL_Handler::on_startexperiment, 0 ),
        calltreeserial_handler( this, CALLTREESERIAL_REQ, CALLTREESERIAL_REPLY, &ACCL_Handler::on_serializecalltree, 0 ),
        calltree_handler( this, CALLTREE_REQ, CALLTREE_REPLY, &ACCL_Handler::on_calltree, 0 ),
        calltreesent_handler( this, CALLTREESENT_REQ, CALLTREESENT_REPLY, &ACCL_Handler::on_calltreesent, 0 ) {
        my_ident_.name = ident;
    }


    ~ACCL_Handler() {
    }

    int want_to_handle( msg_type_t t ) {
        if( t == MSG_TYPE_ACCL_COMMAND ) {
            return 1;
        }

        return 0;
    }

    int on_msg( ACE_Message_Block* block );

    //
    // make the individual command handlers friends
    //
    template<class req_t, class reply_t>
    friend class ACCL_Command_Handler;

    //
    // declare all the stream I/O operators as friends
    //
    friend int operator<<( ACE_OutputCDR&               cdr,
                           const ACCL_Handler::ident_t& id );

    friend int operator>>( ACE_InputCDR&          cdr,
                           ACCL_Handler::ident_t& id );

    friend int operator<<( ACE_OutputCDR&                cdr,
                           const ACCL_Handler::reinit_t& hp );

    friend int operator>>( ACE_InputCDR&           cdr,
                           ACCL_Handler::reinit_t& hp );

    friend int operator<<( ACE_OutputCDR&         cdr,
                           ACCL_Handler::start_t& fp );

    friend int operator>>( ACE_InputCDR&          cdr,
                           ACCL_Handler::start_t& fp );

    friend int operator<<( ACE_OutputCDR&               cdr,
                           const ACCL_Handler::empty_t& id );

    friend int operator>>( ACE_InputCDR&          cdr,
                           ACCL_Handler::empty_t& id );

    friend int operator<<( ACE_OutputCDR&                   cdr,
                           const ACCL_Handler::heartbeat_t& hp );

    friend int operator>>( ACE_InputCDR&              cdr,
                           ACCL_Handler::heartbeat_t& hp );

    friend int operator<<( ACE_OutputCDR&                cdr,
                           const ACCL_Handler::parent_t& hp );

    friend int operator>>( ACE_InputCDR&           cdr,
                           ACCL_Handler::parent_t& hp );

    friend int operator<<( ACE_OutputCDR&                   cdr,
                           const ACCL_Handler::foundprop_t& fp );

    friend int operator>>( ACE_InputCDR&              cdr,
                           ACCL_Handler::foundprop_t& fp );

    friend int operator<<( ACE_OutputCDR&                        cdr,
                           const ACCL_Handler::searchfinished_t& ae );

    friend int operator>>( ACE_InputCDR&                         cdr,
                           const ACCL_Handler::searchfinished_t& ae );

    friend int operator<<( ACE_OutputCDR&                     cdr,
                           const ACCL_Handler::needrestart_t& ae );

    friend int operator>>( ACE_InputCDR&                      cdr,
                           const ACCL_Handler::needrestart_t& ae );

    friend int operator<<( ACE_OutputCDR&                   cdr,
                           const ACCL_Handler::terminate_t& ae );

    friend int operator>>( ACE_InputCDR&                    cdr,
                           const ACCL_Handler::terminate_t& ae );

    friend int operator<<( ACE_OutputCDR&                    cdr,
                           const ACCL_Handler::terminated_t& ae );

    friend int operator>>( ACE_InputCDR&                     cdr,
                           const ACCL_Handler::terminated_t& ae );

    friend int operator<<( ACE_OutputCDR&                        cdr,
                           const ACCL_Handler::propertiessent_t& ae );

    friend int operator>>( ACE_InputCDR&                         cdr,
                           const ACCL_Handler::propertiessent_t& ae );

    friend int operator<<( ACE_OutputCDR&                  cdr,
                           const ACCL_Handler::calltree_t& ct );

    friend int operator>>( ACE_InputCDR&             cdr,
                           ACCL_Handler::calltree_t& ct );

    friend int operator<<( ACE_OutputCDR&                      cdr,
                           const ACCL_Handler::calltreesent_t& cts );

    friend int operator>>( ACE_InputCDR&                 cdr,
                           ACCL_Handler::calltreesent_t& cts );

};

//
// ident_t
//
int operator<<( ACE_OutputCDR&               cdr,
                const ACCL_Handler::ident_t& id );

int operator>>( ACE_InputCDR&          cdr,
                ACCL_Handler::ident_t& id );

//
// start_t
//
int operator<<( ACE_OutputCDR&               cdr,
                const ACCL_Handler::start_t& ae );

int operator>>( ACE_InputCDR&          cdr,
                ACCL_Handler::start_t& ae );

//
// empty_t
//
int operator<<( ACE_OutputCDR&               cdr,
                const ACCL_Handler::empty_t& id );

int operator>>( ACE_InputCDR&          cdr,
                ACCL_Handler::empty_t& id );

//
// heartbeat_t
//
int operator<<( ACE_OutputCDR&                   cdr,
                const ACCL_Handler::heartbeat_t& hp );

int operator>>( ACE_InputCDR&              cdr,
                ACCL_Handler::heartbeat_t& hp );

//
// searchfinished_t
//
int operator<<( ACE_OutputCDR&                        cdr,
                const ACCL_Handler::searchfinished_t& ae );

int operator>>( ACE_InputCDR&                   cdr,
                ACCL_Handler::searchfinished_t& ae );

//
// needrestart_t
//
int operator<<( ACE_OutputCDR&                     cdr,
                const ACCL_Handler::needrestart_t& ae );

int operator>>( ACE_InputCDR&                cdr,
                ACCL_Handler::needrestart_t& ae );

//
// terminate_t
//
int operator<<( ACE_OutputCDR&                   cdr,
                const ACCL_Handler::terminate_t& ae );

int operator>>( ACE_InputCDR&              cdr,
                ACCL_Handler::terminate_t& ae );

//
// terminate_t
//
int operator<<( ACE_OutputCDR&                    cdr,
                const ACCL_Handler::terminated_t& ae );

int operator>>( ACE_InputCDR&               cdr,
                ACCL_Handler::terminated_t& ae );

//
// propertiessent_t
//
int operator<<( ACE_OutputCDR&                        cdr,
                const ACCL_Handler::propertiessent_t& ae );

int operator>>( ACE_InputCDR&                   cdr,
                ACCL_Handler::propertiessent_t& ae );


//
// calltreesent_t
//
int operator<<( ACE_OutputCDR&                      cdr,
                const ACCL_Handler::calltreesent_t& cts );

int operator>>( ACE_InputCDR&                 cdr,
                ACCL_Handler::calltreesent_t& cts );


//
// helper for C++ strings
//
int operator<<( ACE_OutputCDR&     cdr,
                const std::string& str );

int operator>>( ACE_OutputCDR& cdr,
                std::string&   str );

#endif //  ACCL_HANDLER_H_INCLUDED
