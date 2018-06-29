/**
   @file    psc_config.c
   @brief   Periscope config file operations
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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "cfgfile.h"
#include "psc_config.h"
#include "psc_errmsg.h"


/*
 * the periscope configuration
 * settings are held in this table
 * currently it's based on a simple
 * configuration file
 */
cfg_table* psc_config;


void psc_create_default_config( char* fname ) {
    FILE* cfgfile = fopen(fname, "w");
    if( cfgfile == NULL )
        return;

    fprintf(cfgfile, "MACHINE = Periscope\nSITE = NONE\nREGSERVICE_HOST = i19r01a03\nREGSERVICE_PORT = 50001\nREGSERVICE_HOST_INIT = localhost\nREGSERVICE_PORT_INIT = 50001\nAPPL_BASEPORT = 51000\nAGENT_BASEPORT = 50002\n");
    fclose( cfgfile );
}


int psc_config_open( void ) {
    char* basedir;
    char  fname[ 100 ];
        #ifndef _BGP_PORT
    if( !( basedir = getenv( "HOME" ) ) ) {
        psc_errmsg( "HOME directory not set.\n" );
        exit( 1 );
    }
        #else
    basedir = ".";
        #endif

    sprintf( fname, "%s/%s", basedir, PSC_CFGFILE );
    return psc_config_open_file( fname );
}


int psc_config_open_file( char* fname ) {
    psc_config = config_open( fname );
    if( !psc_config ) {
        psc_create_default_config( fname );
        psc_config = config_open( fname );
    }

    if( !psc_config ) {
        psc_errmsg( "psc_config_open(): cannot open config file %s\n", fname );
        return 0;
    }
    return 1;
}


void psc_config_close( void ) {
    if( psc_config ) {
        config_close( psc_config );
    }
}

int psc_config_write( void ) {
    char* basedir;
    char  fname[ 100 ];

    if( !psc_config ) {
        return 0;
    }

        #ifndef _BGP_PORT
    if( !( basedir = getenv( "HOME" ) ) ) {
        psc_errmsg( "HOME directory not set.\n" );
        exit( 1 );
    }
        #else
    basedir = ".";
        #endif
    sprintf( fname, "%s/%s", basedir, PSC_CFGFILE );

    return psc_config_write_file( fname );
}

int psc_config_write_file( const char* fname ) {
    if( !psc_config ) {
        return 0;
    }

    config_write( fname, psc_config );
    return 1;
}

int psc_config_machname( char* buf, size_t len ) {
    char*  s;
    size_t slen;

    if( !psc_config ) {
        return 0;
    }

    s = config_query( psc_config, "MACHINE" );
    if( !s ) {
        return 0;
    }

    slen = strlen( s ) + 1;

    strncpy( buf, s, len < slen ? len : slen );
    // psc_dbgmsg( 0, "Read from config file MACHINE=%s\n", s );
    return 1;
}

char* psc_config_update_str( const char* key, const char* newValue ) {
    if( !psc_config ) {
        return 0;
    }

    return config_update( psc_config, key, newValue );
}

int psc_config_update_int( const char* key, int newValue ) {
    char  strNewValue[ 10 ];
    char* oldValue;
    char* endptr;

    if( !psc_config ) {
        return -1;
    }

    sprintf( strNewValue, "%d", newValue );
    oldValue = config_update( psc_config, key, strNewValue );

    return strtol( oldValue, &endptr, 10 );
}

int psc_config_sitename( char* buf, size_t len ) {
    char*  s;
    size_t slen;

    if( !psc_config ) {
        return 0;
    }

    s = config_query( psc_config, "SITE" );
    if( !s ) {
        return 0;
    }

    slen = strlen( s ) + 1;

    strncpy( buf, s, len < slen ? len : slen );
    // psc_dbgmsg( 0, "Read from config file SITE=%s\n", s );
    return 1;
}


int psc_config_reghost( char* buf, size_t len ) {
    char*  s;
    size_t slen;

    if( !psc_config ) {
        return 0;
    }

    s = config_query( psc_config, "REGSERVICE_HOST" );
    if( !s ) {
        return 0;
    }

    slen = strlen( s ) + 1;

    strncpy( buf, s, len < slen ? len : slen );
//  psc_dbgmsg( 0, "Read from config file REGSERVICE_HOST=%s\n", s );
    return 1;
}

int psc_config_reghost_init( char* buf, size_t len ) {
    char*  s;
    size_t slen;

    if( !psc_config ) {
        return 0;
    }

    s = config_query( psc_config, "REGSERVICE_HOST_INIT" );
    if( !s ) {
        return 0;
    }

    slen = strlen( s ) + 1;

    strncpy( buf, s, len < slen ? len : slen );
//  psc_dbgmsg( 0, "Read from config file REGSERVICE_HOST=%s\n", s );
    return 1;
}


int psc_config_regport( void ) {
    char* s;

    if( !psc_config ) {
        printf( "no config file\n" );
        return 0;
    }

    s = config_query( psc_config, "REGSERVICE_PORT" );
    if( !s ) {
        return 0;
    }
    // psc_dbgmsg( 0, "Read from config file REGSERVICE_PORT=%s\n", s );
    return atoi( s );
}

int psc_config_regport_init( void ) {
    char* s;

    if( !psc_config ) {
        printf( "no config file\n" );
        return 0;
    }

    s = config_query( psc_config, "REGSERVICE_PORT_INIT" );
    if( !s ) {
        return 0;
    }
    // psc_dbgmsg( 0, "Read from config file REGSERVICE_PORT_INIT=%s\n", s );
    return atoi( s );
}

int psc_config_appl_baseport( void ) {
    char* s;

    if( !psc_config ) {
        return 0;
    }

    s = config_query( psc_config, "APPL_BASEPORT" );
    if( !s ) {
        return 0;
    }
    //psc_dbgmsg( 0, "Read from config file APPL_BASEPORT=%s\n", s );
    return atoi( s );
}

int psc_config_agent_baseport( void ) {
    char* s;

    if( !psc_config ) {
        return 0;
    }

    s = config_query( psc_config, "AGENT_BASEPORT" );
    if( !s ) {
        return 0;
    }
    // psc_dbgmsg( 0, "Read from config file AGENT_BASEPORT=%s\n", s );
    return atoi( s );
}

int psc_config_appname( char* buf, size_t len ) {
    char*  s;
    size_t slen;

    s = getenv( "PERISCOPE_APP" );
    if( !s ) {
        if( !psc_config ) {
            return 0;
        }
        s = config_query( psc_config, "APPNAME" );
        if( !s ) {
            return 0;
        }
    }

    slen = strlen( s ) + 1;

    strncpy( buf, s, len < slen ? len : slen );
    return 1;
}

int psc_config_frontend_host( char* buf, size_t len ) {
    char*  s;
    size_t slen;

    if( !psc_config ) {
        return 0;
    }

    s = config_query( psc_config, "FRONTEND_HOST" );
    if( !s ) {
        return 0;
    }

    slen = strlen( s ) + 1;

    strncpy( buf, s, len < slen ? len : slen );
    // psc_dbgmsg( 0, "Read from config file FRONTEND_HOST=%s\n", s );
    return 1;
}

int psc_config_strategy( char* buf, size_t len ) {
    char*  s;
    size_t slen;

    if( !psc_config ) {
        return 0;
    }

    s = config_query( psc_config, "STRATEGY" );
    if( !s ) {
        return 0;
    }

    slen = strlen( s ) + 1;

    strncpy( buf, s, len < slen ? len : slen );
    //psc_dbgmsg( 0, "Read from config file STRATEGY=%s\n", s );
    return 1;
}

int psc_config_aa_parent( char* buf, size_t len ) {
    char*  s;
    size_t slen;

    if( !psc_config ) {
        return 0;
    }

    s = config_query( psc_config, "AA_PARENT" );
    if( !s ) {
        return 0;
    }

    slen = strlen( s ) + 1;

    strncpy( buf, s, len < slen ? len : slen );
    //psc_dbgmsg( 0, "Read from config file AA_PARENT=%s\n", s );
    return 1;
}

int psc_config_aa_tag( char* buf, size_t len ) {
    char*  s;
    size_t slen;

    if( !psc_config ) {
        return 0;
    }

    s = config_query( psc_config, "AA_TAG" );
    if( !s ) {
        return 0;
    }

    slen = strlen( s ) + 1;

    strncpy( buf, s, len < slen ? len : slen );
    //psc_dbgmsg( 0, "Read from config file AA_TAG=%s\n", s );
    return 1;
}

// If no configuration available, run with 1 proc/AA
int psc_config_procs_per_aa( void ) {
    char* s;

    if( !psc_config ) {
        return 1;
    }

    s = config_query( psc_config, "PROCS_PER_AA" );
    if( !s ) {
        return 1;
    }

    //psc_dbgmsg( 0, "Read from config file PROCS_PER_AA=%s\n", s );
    return atoi( s );
}

// If no configuration available, run with 1 thread
int psc_config_ompnumthreads( void ) {
    char* s;

    if( !psc_config ) {
        return 1;
    }

    s = config_query( psc_config, "OMPNUMTHREADS" );
    if( !s ) {
        return 1;
    }

    //psc_dbgmsg( 0, "Read from config file PROCS_PER_AA=%s\n", s );
    return atoi( s );
}

int psc_config_mode( void ) {
    char* s;

    if( !psc_config ) {
        return 0;
    }

    s = config_query( psc_config, "BG_MODE" );
    if( !s ) {
        return 0;
    }

    //psc_dbgmsg( 0, "Read from config file PROCS_PER_AA=%s\n", s );
    return atoi( s );
}

// If no configuration available, run with 1 process
int psc_config_procs( void ) {
    char* s;

    if( !psc_config ) {
        return 1;
    }

    s = config_query( psc_config, "PROCS" );
    if( !s ) {
        return 1;
    }

// psc_dbgmsg( 0, "Read from config file PROCS=%s\n", s );
    return atoi( s );
}

int psc_config_debug( void ) {
    char* s;

    if( !psc_config ) {
        return 0;
    }

    s = config_query( psc_config, "DEBUG" );
    if( !s ) {
        return 0;
    }

    // psc_dbgmsg( 0, "Read from config file DEBUG=%s\n", s );
    return atoi( s );
}

int psc_config_uninstrumented( void ) {
    char* s;

    if( !psc_config ) {
        return 0;
    }
    s = config_query( psc_config, "UNINSTRUMENTED" );
    if( !s ) {
        return 0;
    }

    // psc_dbgmsg( 0, "Read from config file DEBUG=%s\n", s );
    return 1;
}
