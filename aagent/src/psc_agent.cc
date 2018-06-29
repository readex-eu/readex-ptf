/**
   @file	psc_agent.cc
   @ingroup Communication
   @brief   Communication agent implementation
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

#include "ace/Acceptor.h"
#include "ace/SOCK_Connector.h"
#include "ace/SOCK_Stream.h"

#include "regxx.h"
#include "global.h"
#include "psc_agent.h"
#include "timing.h"
#include "psc_config.h"
#include "config.h"
//#include "analysisagent.h"

#include <iostream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

using std::cerr;
using std::endl;

#include <errno.h>

#ifndef __LINUX_p575
extern int errno;
#endif

int PeriscopeAgent::open( int port ) {
    const int retries = 100;
    int       i       = 0;

    for( int i = 0; i < retries; i++ ) {
        ACE_INET_Addr listen_addr( port );

        errno = 0;
        if( acceptor_.open( listen_addr ) == -1 ) {
            if( errno == EADDRINUSE ) {
                // just retry with next port number
                port = port + 20;
                continue;
            }

            // some other error, bail out...
            ACE_ERROR_RETURN( ( LM_ERROR, "%p\n", "acceptor.open()" ), -1 );
        }
        else {
            own_info_.hostname = listen_addr.get_host_name();
            own_info_.port     = listen_addr.get_port_number();

            return 0;
        }
    }

    ACE_ERROR_RETURN( ( LM_ERROR, "Giving up after %d retries\n", retries ), -1 );
}


int PeriscopeAgent::get_local_port() {
    ACE_INET_Addr addr;

    if( acceptor_.get_local_addr( addr ) == -1 ) {
        return -1;
    }

    return addr.get_port_number();
}


std::string PeriscopeAgent::get_local_hostname() {
    ACE_INET_Addr addr;

    if( acceptor_.get_local_addr( addr ) == -1 ) {
        return "";
    }

    return addr.get_host_name();
}

/**
 * @brief Sets a new RegistryService for the current agent.
 *
 * @param       svc new RegistryService to use
 * @return      the old RegistryService used
 */
RegistryService* PeriscopeAgent::set_registry( RegistryService* svc ) {
    RegistryService* tmp = regsrv_;
    regsrv_ = svc;
    return tmp;
}

int PeriscopeAgent::connect_to_parent() {
    if( parent_handler_ ) {
        psc_dbgmsg( 9, "PeriscopeAgent::connect_to_parent(): Connection already exists!!!\n" );
        return 0;
    }
    ACE_INET_Addr      addr;
    ACE_SOCK_Connector connector;

    std::string host;
    int         port;

    if( fastmode ) {
        host = string( this->parent_host );
        port = this->parent_port;
    }
    else {
        EntryData              query;
        std::list< EntryData > result;

        // specify the component and the node
        // the rest is filled in by our query request
        query.tag = this->parent();
        query.app = appname();

        psc_dbgmsg( 7, "PeriscopeAgent::connect_to_parent() searching registry for parent : |%s| and appl |%s|\n", query.tag.c_str(), query.app.c_str() );
        while( !this->timed_out() ) {
            if( regsrv_->query_entries( result, query, false ) == -1 ) {
                sleep( 5 );
            }
            else {
                break;
            }
        }

        if( result.empty() ) {
            psc_errmsg( "Timeout querying registry for parent |%s| and appl |%s|\n", query.tag.c_str(), query.app.c_str() );
            exit( 1 );
        }

        host = result.begin()->node;
        port = result.begin()->port;
    }

#ifdef _BGP_PORT

    if( psc_config_open() ) {
        char main_host[ 2000 ];
        if( !psc_config_frontend_host( main_host, 2000 ) ) {
            psc_errmsg( "Frontend host suffix is not defined in config file\n", query.tag.c_str() );
            psc_config_close();
            exit( 1 );
        }
        host = host + main_host;
        psc_config_close();
    }
    else {
        psc_errmsg( "Can't open config file\n", query.tag.c_str() );
        exit( 1 );
    }

    if( result.size() > 1 ) {
        std::list <EntryData>::iterator hl;

        srand( gethostid() );

        int iSecret = rand() % result.size();

        psc_dbgmsg( 6, "Multiple parents are possible, randomly choosing parent=%d out of %d parents...\n", iSecret, result.size() );

        int iter = 0;
        for( hl = result.begin(); hl != result.end(); hl++, iter++ ) {
            if( iter == iSecret ) {
                port = hl->port;
            }
        }
    }
    else {
        port = result.begin()->port;
    }


#endif

    psc_dbgmsg( 6, "PeriscopeAgent::connect_to_parent(): connecting to parent at %d %s\n", port, host.c_str() );
    addr.set( port, host.c_str() );

    parent_stream_ = new ACE_SOCK_Stream();


    // TODO check why there is a 3 second sleep here
    // is this where the agent connects to the application?
    while( connector.connect( *parent_stream_, addr ) == -1  &&  !this->timed_out() ) {
        if( fastmode ) {
            sleep( 3 );
        }
    }
    if( this->timed_out() ) {
        psc_errmsg( "Timeout connect_to_parent()\n" );
        return -1;
    }

//debug begin find out address of peer
    ACE_INET_Addr peer_addr, peer_addr2;
    char          peer_name[ MAXHOSTNAMELEN ], peer_name2[ MAXHOSTNAMELEN ];
    ( parent_stream_ )->get_remote_addr( peer_addr );
    peer_addr.addr_to_string( peer_name, MAXHOSTNAMELEN );
    psc_dbgmsg( 7, "PeriscopeAgent::connect_to_parent(): Connected to parent! peer_addr=%s\n", peer_name );
//debug end

    parent_handler_ = create_protocol_handler( *parent_stream_ );


    return 0;
}


int PeriscopeAgent::connect_to_child( AgentInfo* ag ) {
    if( ag->handler ) {
        psc_dbgmsg( 7, "PeriscopeAgent::connect_to_child : Connection already exists!!!\n" );
        return 0;
    }

    ACE_INET_Addr      addr;
    ACE_SOCK_Connector connector;



    addr.set( ag->port, ( ag->hostname ).c_str() );

    ag->stream = new ACE_SOCK_Stream();

    if( connector.connect( *( ag->stream ), addr ) == -1 ) {
        return -1;
    }

//debug begin
    ACE_INET_Addr peer_addr, peer_addr2;
    char          peer_name[ MAXHOSTNAMELEN ], peer_name2[ MAXHOSTNAMELEN ];
    ( ag->stream )->get_remote_addr( peer_addr );
    peer_addr.addr_to_string( peer_name, MAXHOSTNAMELEN );
    psc_dbgmsg( 7, "PeriscopeAgent::connect_to_child: Connected to child! peer_addr=%s\n", peer_name );
//debug end

    ag->handler = create_protocol_handler( *( ag->stream ) );


    return 0;
}


void PeriscopeAgent::add_child_agent( std::string tag,
                                      std::string hostname, int port ) {
    AgentInfo info;
    info.tag      = tag;
    info.hostname = hostname;
    info.port     = port;
    psc_dbgmsg( 4, "Child added: tag=%s,host=%s,port=%d\n", tag.c_str(), hostname.c_str(), port );

#ifdef _BGP_PORT_HEARTBEAT_V1
    info.status = AgentInfo::STARTED;
#endif
    child_agents_[ tag ] = info;
}
