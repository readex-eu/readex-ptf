/**
   @file    hlagent_main.cc
   @ingroup Communication
   @brief   High-level agent main routine
   @author  Karl Fuerlinger
   @verbatim
    Revision:       $Revision$
    Revision date:  Dec 10, 2015
    Committed by:   Robert Mijakovic

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2015, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <utility>
#include <sstream>

#ifdef __sparc
#include "getopt.h"
#elif __p575
#include "getopt.h"
#else
#include <getopt.h>
#endif

#include "hlagent.h"
#include "regxx.h"
#include "psc_errmsg.h"
#include "psc_config.h"
#include "selective_debug.h"

struct cmdline_opts {
    int has_showhelp;
    int has_appname;
    int has_debug;
    int has_selectivedebug;
    int has_registry;
    int has_desired_port;
    int has_parent;
    int has_children;
    int has_timeout;
    int has_gatherprop;
    int has_dontcluster;
    int has_tag;

    char appname_string[ 2000 ];
    char debug_string[ 2000 ];
    char selectivedebug_string[ 4000 ];
    char reg_string[ 2000 ];
    char port_string[ 2000 ];
    char parent_string[ 2000 ];
    char children_string[ 2000 ];
    char timeout_string[ 2000 ];
    char tag_string[ 2000 ];

    cmdline_opts() {
        has_registry       = 0;
        has_desired_port   = 0;
        has_showhelp       = 0;
        has_timeout        = 0;
        has_parent         = 0;
        has_tag            = 0;
        has_debug          = 0;
        has_children       = 0;
        has_dontcluster    = 0;
        has_gatherprop     = 0;
        has_selectivedebug = 0;
    }
};

struct bininfo_rec {
    int num;
    struct cpurec {
        int num;
        int id;
    } map[ 512 ];
    int         nodeagent_cpu;
    std::string host;
    bool        bin_flagged;
};

int parse_opts( int                  argc,
                char*                argv[],
                struct cmdline_opts* copts ) {
    static const struct option long_opts[] =
    {
//      { const char* name,         int has_arg,       int* flag, int val }
        { "help",                   no_argument,       0,         'a' },
        { "version",                no_argument,       0,         'b' },
        { "appname",                required_argument, 0,         'c' },
        { "debug",                  required_argument, 0,         'd' },
        { "selective-debug",        required_argument, 0,         'e' },
        { "registry",               required_argument, 0,         'f' },
        { "port",                   required_argument, 0,         'g' },
        { "parent",                 required_argument, 0,         'h' },
        { "children",               required_argument, 0,         'i' },
        { "timeout",                required_argument, 0,         'j' },
        { "gatherprop",             no_argument,       0,         'k' },
        { "dontcluster",            no_argument,       0,         'l' },
//TODO: Revise unused stuff. tag seems to be used only by HL and AA. -RM
        { "tag",                    required_argument, 0,         'm' },
        0
    };

    while( optind < argc ) {
        int            index = -1;
        struct option* opt   = 0;

        int result = getopt_long( argc, argv, "", long_opts, &index );

        if( result == -1 ) {
            return -1;
        }

        opt = ( struct option* )&( long_opts[ index ] );

        switch( result ) {
        case 'a':
            copts->has_showhelp = 1;
            break;

        case 'b':
            fprintf( stdout, "The Periscope Tuning Framework: Version %s\n", stringify( _VERSION ) );
            exit( 1 );
            break;

        case 'c':
            if( opt->has_arg == required_argument ) {
                copts->has_appname = 1;
                strcpy( copts->appname_string, optarg );
            }
            break;

        case 'd':
            if( opt->has_arg == required_argument ) {
                copts->has_debug = 1;
                strcpy( copts->debug_string, optarg );
            }
            break;

        case 'e':
            if( opt->has_arg == required_argument ) {
                copts->has_selectivedebug = 1;
                strcpy( copts->selectivedebug_string, optarg );
                std::string levels = optarg;
                handle_dbgLevelList( levels );
            }
            break;

        case 'f':
            if( opt->has_arg == required_argument ) {
                copts->has_registry = 1;
                strcpy( copts->reg_string, optarg );
            }
            break;

        case 'g':
            if( opt->has_arg == required_argument ) {
                copts->has_desired_port = 1;
                strcpy( copts->port_string, optarg );
            }
            break;

        case 'h':
            if( opt->has_arg == required_argument ) {
                copts->has_parent = 1;
                strcpy( copts->parent_string, optarg );
            }
            break;

        case 'i':
            if( opt->has_arg == required_argument ) {
                copts->has_children = 1;
                strcpy( copts->children_string, optarg );
            }
            break;

        case 'j':
            if( opt->has_arg == required_argument ) {
                copts->has_timeout = 1;
                strcpy( copts->timeout_string, optarg );
            }
            break;

        case 'k':
            copts->has_gatherprop = 1;
            break;

        case 'l':
            copts->has_dontcluster = 1;
            break;

        case 'm':
            if( opt->has_arg == required_argument ) {
                copts->has_tag = 1;
                strcpy( copts->tag_string, optarg );
            }
            break;

        default:
            // some other / unknown option specified
            return -1;
        }
    }

    return 0;
}


void usage( int   argc,
            char* argv[] ) {
    assert( argc > 0 );

    fprintf( stderr, "Usage: %s <options>\n", argv[ 0 ] );
    fprintf( stderr, "  [--help]                 (Displays this help message)\n" );
    fprintf( stderr, "  [--version]              (Version of Periscope)\n" );
    fprintf( stderr, "  [--registry=host:port]   (Address of the registry service, optional)\n" );
    fprintf( stderr, "  [--port=n]               (Local port number, optional)\n" );
    fprintf( stderr, "  [--timeout=secs]         (Timeout for startup of HL agent)\n" );
    fprintf( stderr, "  [--parent=parent tag]    (Parent tag)\n" );
    fprintf( stderr, "  [--debug=n]              (Max debug level)\n" );
    fprintf( stderr, "  [--selective-debug=list] (Individual debug level names separated by comma)\n" );
    print_dbgLevelsDefs();
    fprintf( stderr, "  [--dontcluster]          (Don't cluster properties)\n" );
    fprintf( stderr, "  [--gatherprop]           (Gather properties)\n" );
    fprintf( stderr, "  [--children=childtag,childtab] \n" );
}



// ***********************************
// ***********************************
//          M  A  I  N
// ***********************************
// ***********************************


int main( int   argc,
          char* argv[] ) {
    struct cmdline_opts opts;
    char*               app_name;
    PeriscopeHLAgent    agent( ACE_Reactor::instance() );


    psc_init_start_time();
    psc_set_msg_prefix( "hlagent" );
    psc_set_progname( "psc_hlagent" );
    if( ( parse_opts( argc, argv, &opts ) == -1 ) || opts.has_showhelp ) {
        usage( argc, argv );
        exit( 1 );
    }



    // -----
    // T A G
    // -----

    if( !opts.has_tag ) {
        psc_errmsg( "Tag has to be specified\n" );
        usage( argc, argv );
        exit( 1 );
    }

    psc_dbgmsg( 5, "Tag is set to %s\n", opts.tag_string );
//  psc_set_msg_prefix(opts.tag_string);
    std::stringstream msg_prefix;
//  msg_prefix << "HLA(" << getpid() << ")";
    msg_prefix << "HLA:" << opts.tag_string;
    psc_set_msg_prefix( msg_prefix.str().c_str() );
    agent.set_own_tag( opts.tag_string );



    // ----------
    // DEBUG
    // ----------
    if( opts.has_debug ) {
        psc_dbg_level = atoi( opts.debug_string );
    }
    else {
        char  buf[ 800 ];
        int   dbg_level;
        char* bufdbg;

        if( ( bufdbg = getenv( "PERISCOPE_INFO" ) ) == 0 ) {
            psc_dbg_level = 0;
            /* assume debug-level 0 */
        }
        else {
            psc_dbg_level = atoi( bufdbg );
        }
    }

    psc_dbgmsg( 1, "High Level Agent started.\n" );


    // -----------------
    // A P P L I C A T I N  N A M E
    // -----------------



    if( opts.has_appname ) {
        app_name = opts.appname_string;
    }
    else {
        if( getenv( "PSC_APPNAME" ) ) {
            app_name = getenv( "PSC_APPNAME" );
        }
        else {
            app_name = strdup( "appl" );
        }
    }

    agent.set_appname( app_name );

    // -----------------
    // R E G I S T R Y
    // -----------------

    char reg_host[ 2000 ];
    int  reg_port = 0;
    char tmp_str[ 2000 ];

    if( opts.has_registry ) {
        strcpy( tmp_str, opts.reg_string ); //opts.reg_string is needed later as well
        char* tmp;

        if( !( tmp = strchr( tmp_str, ':' ) ) ) {
            // registry has to be specified as <hostname>:<port>
            psc_errmsg( "Unrecognized hostname or port for registry\n" );
            usage( argc, argv );
            exit( 1 );
        }
        else {
            // terminate hostname with "\0"
            ( *tmp ) = 0;

            // separate into hostname and port
            strcpy( reg_host, tmp_str );
            reg_port = atoi( tmp + 1 );

            if( reg_port <= 0 ) {
                psc_errmsg( "Unrecognized hostname or port for registry\n" );
                usage( argc, argv );
                exit( 1 );
            }
        }
    }
    else {
        if( psc_config_open() ) {
            psc_config_reghost( reg_host, 2000 );
            reg_port = psc_config_regport();
            psc_config_close();
        }
        else {
            psc_errmsg( "Error opening config file, cannot continue.\n" );
            usage( argc, argv );
            exit( 1 );
        }
    }

    //
    // print the settings that we have...
    //
    psc_dbgmsg( 5, "Using registry at (%s:%d)\n", reg_host, reg_port );



    // ---------------------------------
    // S I T E    A N D    M A C H I N E
    // ---------------------------------

    char psc_site[ 2000 ];
    char psc_machine[ 2000 ];

    if( psc_config_open() ) {
        if( !psc_config_sitename( psc_site, 2000 ) ) {
            strcpy( psc_site, "(site not set)" );
        }

        if( !psc_config_machname( psc_machine, 2000 ) ) {
            strcpy( psc_machine, "(machine not set)" );
        }

        psc_config_close();
    }
    else {
        psc_errmsg( "Error opening config file, site and machine not set.\n" );
    }



    // --------
    // P O R T
    // --------

    int myport = 30000;
    if( opts.has_desired_port ) {
        myport = atoi( opts.port_string );
        if( myport <= 0 ) {
            psc_errmsg( "Bad value for desired port: %s\n", opts.port_string );
            exit( 1 );
        }
    }

    psc_dbgmsg( 2, "Desired port %d\n", myport );



    // -------------
    // T I M E O U T
    // -------------

    int timeout = 120;
    if( opts.has_timeout ) {
        timeout = atoi( opts.timeout_string );
        if( timeout <= 0 ) {
            psc_errmsg( "Bad value for timeout: %s\n", opts.timeout_string );
            exit( 1 );
        }
    }

    psc_dbgmsg( 5, "Using %d seconds timeout\n", timeout );



    // -----------------------
    // C R E A T E   A G E N T
    // -----------------------

    agent.set_machinename( psc_machine );
    agent.set_sitename( psc_site );
    //agent.set_timeout(timeout);
    agent.set_global_timeout( timeout );

    if( agent.open( myport ) == -1 ) {
        psc_errmsg( "Error opening HL agent on port %d\n", myport );
        exit( 1 );
    }

    psc_dbgmsg( 5, "Port set to %d\n", agent.get_local_port() );


    RegistryService regsrv( reg_host, reg_port );

    agent.set_registry( &regsrv );

    if( agent.register_self() == -1 ) {
        psc_errmsg( "Error registering at registry.\n" );
        exit( 1 );
    }

#ifdef _BGP_PORT
// On the
    sprintf( opts.tag_string, "%s:%d", opts.tag_string, agent.get_local_port() );

    psc_dbgmsg( 5, "Tag will be reset to %s\n", opts.tag_string );

    msg_prefix << "HLA:" << opts.tag_string;
    psc_set_msg_prefix( msg_prefix.str().c_str() );
    agent.set_own_tag( opts.tag_string );
#endif



    // ----------------------------
    // P A R E N T
    // ----------------------------

    if( opts.has_parent ) {
        std::string parentTag;

        parentTag = opts.parent_string;
        agent.set_parent( parentTag );
        psc_dbgmsg( 2, "Using parent %s\n", agent.parent() );
        if( agent.connect_to_parent() == -1 ) {
            psc_errmsg( "Error in analysisagent connecting to parent at %s\n", agent.parent() );
            exit( 1 );
        }
    }
    else {
        psc_errmsg( "Parent has to be specified\n" );
        usage( argc, argv );
        exit( 1 );
    }


    if( opts.has_dontcluster ) {
        agent.dontcluster();
    }


#ifndef _BGP_PORT_HEARTBEAT_V1 // in this configuration we don't care who are the children, we accept all who come and forward the heartbeats to FE who counts them
    if( opts.has_children ) {
        char* tmp;
        int   id;

        // tokenize the string and put the IDs in the list
        tmp = strtok( opts.children_string, " ,\n" );
        while( tmp != 0 ) {
            agent.add_child_agent( tmp, "none", -1 );
            tmp = strtok( 0, " ,\n" );
        }
    }
    else {
        psc_errmsg( "Children specification missing.\n" );
        usage( argc, argv );
        exit( 1 );
    }
#endif

    agent.run();
    ACE_Reactor::instance()->close();
}
