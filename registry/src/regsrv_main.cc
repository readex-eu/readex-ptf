/**
   @file    regsrv_main.cc
   @brief   Registry Server-Main Pattern
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

#include <cstdio>
#include <stdlib.h>

#include "ace/INET_Addr.h"
#include "psc_errmsg.h"
#include "regsrv.h"
#include "regsrv_client.h"
#include "psc_config.h"

void usage( int   argc,
            char* argv[] ) {
    fprintf( stderr, "Usage: %s <portnumber>\n", argv[ 0 ] );
}

int main( int   argc,
          char* argv[] ) {
    int  port = 31337;
    char envRegSpec[ 200 ];

    //FILE *myfile = fopen( "regout", "w" );
    //fprintf( myfile, "In registry main:::: arg0 %s, arg1 %s\n", argv[0], argv[1] );
    //fclose( myfile );

    if( !getenv( "PERISCOPE_INFO" ) ) {
        psc_dbg_level = 0;
        /* assume debug-level 0 */
    }
    else {
        strcpy( envRegSpec, getenv( "PERISCOPE_INFO" ) );
        psc_dbg_level = atoi( envRegSpec );
    }

    if( getenv( "PSC_REGISTRY" ) ) {
        char* tmp;
        strcpy( envRegSpec, getenv( "PSC_REGISTRY" ) );

        if( !( tmp = strchr( envRegSpec, ':' ) ) ) {
            fprintf( stderr, "Unrecognized registry setting: %s\n", envRegSpec );
            exit( 1 );
        }
        else {
            ( *tmp ) = 0;
            port     = atoi( tmp + 1 );
        }
    }
    else {
        /*
         * get registry hostname and port from config file
         */
        if( psc_config_open() ) {
            if( !( port = psc_config_regport_init() ) ) {
                fprintf( stderr, "Registry port not set in Periscope configuration file.\n" );
                exit( 1 );
            }
            psc_config_close();
        }
    }


    if( argc > 1 ) {
        port = atoi( argv[ 1 ] );
    }

    RegServ       serv;
    ACE_INET_Addr listen_addr( port );

    serv.reactor( ACE_Reactor::instance() );
    port =  serv.open( listen_addr );
    if( port == -1 ) {
        psc_errmsg( "Error starting server on %d\n", port );
        usage( argc, argv );
        exit( 1 );
    }
    else {
        psc_dbgmsg( 6, "Registry started on %s:%d\n", listen_addr.get_host_name(), listen_addr.get_port_number() );

        if( psc_config_open() ) {
            psc_config_update_str( "REGSERVICE_HOST", listen_addr.get_host_name() );
            psc_config_update_int( "REGSERVICE_PORT", listen_addr.get_port_number() );
            psc_config_write();
            psc_dbgmsg( 6, "Updated psc config file!\n" );
        }
        psc_config_close();
    }
    ACE_Reactor::instance()->run_reactor_event_loop();

    return 0;
}
