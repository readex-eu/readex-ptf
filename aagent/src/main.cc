/**
   @file    main.cc
   @ingroup AnalysisAgent
   @brief   Analysis agent main routine
   @author  Edmond Kereku
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

#ifdef __sparc
#include "getopt.h"
#elif __p575
#include "getopt.h"
#else
#include <getopt.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctime>

#include <unistd.h>

#include "global.h"
#include "PerformanceDataBase.h"
#include "Property.h"
#include "regxx.h"
#include "psc_config.h"
#include "psc_errmsg.h"
#include "selective_debug.h"



#include <map>
#include <string>
#include <list>
#include <sstream>

#include "PerformanceDataBase.h"
#include "application.h"
#include "analysisagent.h"
#include "DataProvider.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>


bool                TEST;
extern char*        optarg;
extern int          optind;
struct cmdline_opts opts;

PerformanceDataBase* pdb;
DataProvider*        dp;
Application*         appl;
boost::property_tree::ptree configTree;

AnalysisAgent*       agent;
Prop_List            foundProperties;

bool rts_support=false;

int parse_opts( int                  argc,
                char*                argv[],
                struct cmdline_opts* copts ) {
    static const struct option long_opts[] =
    {
//      { const char* name,         int has_arg,       int* flag, int val }
        { "help",                   no_argument,       0,         'a' },
        { "version",                no_argument,       0,         'b' },
        { "appname",                required_argument, 0,         'c' },
        { "mpinumprocs",            required_argument, 0,         'd' },
        { "ompnumthreads",          required_argument, 0,         'e' },
        { "strategy",               required_argument, 0,         'f' },
        { "tune",                   required_argument, 0,         'g' },
        { "phase",                  required_argument, 0,         'h' },
        { "debug",                  required_argument, 0,         'i' },
        { "selective-debug",        required_argument, 0,         'j' },
        { "registry",               required_argument, 0,         'k' },
        { "port",                   required_argument, 0,         'l' },
        { "parent",                 required_argument, 0,         'm' },
        { "leaf",                   no_argument,       0,         'n' },
        { "timeout",                required_argument, 0,         'o' },
        { "searches",               required_argument, 0,         'p' },
        { "delay",                  required_argument, 0,         'q' },
        { "iterations",             required_argument, 0,         'r' },
        { "dontregister",           no_argument,       0,         's' },
        { "parent-host",            required_argument, 0,         't' },
        { "parent-port",            required_argument, 0,         'u' },
        { "id",                     required_argument, 0,         'v' },
        { "propfile",               required_argument, 0,         'w' },
        { "property",               required_argument, 0,         'x' },
        { "pedantic",               no_argument,       0,         'y' },
//TODO: Revise unused stuff. tag seems to be used only by HL and AA. -RM
        { "tag",                    required_argument, 0,         'z' },
//        { "threads",                required_argument, 0,         'z' },
        { "with-deviation-control", no_argument,       0,         'A' },
        { "inst",                   required_argument, 0,         'B' },
        { "srcrev",                 required_argument, 0,         'C' },
        { "config-file",            required_argument, 0,         'I' },
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
            copts->has_showhelp = true;
            break;

        case 'b':
            fprintf( stdout, "The Periscope Tuning Framework: Version %s\n", stringify( _VERSION ) );
            exit( EXIT_SUCCESS );
            break;

        case 'c':
            if( opt->has_arg == required_argument ) {
                copts->has_appname = true;
                strcpy( copts->appname_string, optarg );
            }
            break;

        case 'd':
            if( opt->has_arg == required_argument ) {
                copts->has_mpinumprocs = 1;
                strcpy( copts->mpinumprocs_string, optarg );
            }
            break;

        case 'e':
            if( opt->has_arg == required_argument ) {
                copts->has_ompnumthreads = 1;
                strcpy( copts->ompnumthreads_string, optarg );
            }
            break;

        case 'f':
            if( opt->has_arg == required_argument ) {
                copts->has_strategy = true;
                strcpy( copts->strategy, optarg );
            }
            break;

        case 'g':
            if( opt->has_arg == required_argument ) {
                copts->has_plugin = 1;
                strcpy( copts->plugin, optarg );
//                char readex_plugin[] = "readex_tuning";
//                rts_support = !strcmp( copts->plugin, readex_plugin );
                std::list<std::string> plugin_name = {"readex_intraphase", "readex_interphase", "readex_configuration"};
                for(auto s : plugin_name){
                    rts_support = !s.compare(copts->plugin);
                    if(rts_support) break;
                }
            }
            break;

        case 'h':               // Delay in number of phase executions
            if( opt->has_arg == required_argument ) {
                copts->has_phase = 1;
                strcpy( copts->phase_string, optarg );
            }
            break;

        case 'i':
            if( opt->has_arg == required_argument ) {
                copts->has_debug = true;
                strcpy( copts->debug_string, optarg );
            }
            break;

        case 'j':
            if( opt->has_arg == required_argument ) {
                copts->has_selectivedebug = 1;
                strcpy( copts->selectivedebug_string, optarg );
                std::string levels = optarg;
                handle_dbgLevelList( levels );
            }
            break;

        case 'k':
            if( opt->has_arg == required_argument ) {
                copts->has_registry = true;
                strcpy( copts->reg_string, optarg );
            }
            break;

        case 'l':
            if( opt->has_arg == required_argument ) {
                copts->has_desired_port = true;
                strcpy( copts->port_string, optarg );
            }
            break;

        case 'm':
            copts->has_parent = true;
            strcpy( copts->parent_string, optarg );
            break;

        case 'n':         // is a leaf node; direct connection to MPI processes
            copts->has_leaf = true;
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ), "agent is a leaf\n" );
            break;

        case 'o':
            if( opt->has_arg == required_argument ) {
                copts->has_timeout = true;
                strcpy( copts->timeout_string, optarg );
            }
            break;

        case 'p':
            if( opt->has_arg == required_argument ) {
                copts->has_searches = true;
                strcpy( copts->searches, optarg );
            }
            break;

        case 'q':         // delay in number of phase executions
            if( opt->has_arg == required_argument ) {
                copts->has_delay = 1;
                strcpy( copts->delay_string, optarg );
            }
            break;

        case 'r':         // phase executions per experiment
            if( opt->has_arg == required_argument ) {
                copts->has_iterations = 1;
                strcpy( copts->iterations_string, optarg );
            }
            break;

        case 's':
            copts->has_dontregister = true;
            break;

        case 't':         // parent host; needed since no registry used
            if( opt->has_arg == required_argument ) {
                copts->has_parent_host = true;
                strcpy( copts->parent_host, optarg );
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ), "agent has parent at host: %s\n", copts->parent_host );
            }
            break;

        case 'u':         // parent port; needed since no registry used
            if( opt->has_arg == required_argument ) {
                copts->has_parent_port = true;
                copts->parent_port     = atoi( optarg );
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ), "agent has parent at port: %d\n", copts->parent_port );
            }
            break;

        case 'v':
            if( opt->has_arg == required_argument ) {
                copts->has_id    = true;
                copts->id_string = ( char* )calloc( strlen( optarg ) + 1, 1 );
                strcpy( copts->id_string, optarg );
            }
            break;

        case 'w':                       // Output file
            if( opt->has_arg == required_argument ) {
                copts->has_propfile = 1;
                std::string tmpStr( optarg );
                if( tmpStr[ 0 ] == '~' ) {
                    char* homeStr = getenv( "HOME" );
                    tmpStr.replace( 0, 1, homeStr );
                }
                strcpy( copts->prop_file, tmpStr.c_str() );
            }
            break;

        case 'x':
            if( opt->has_arg == required_argument ) {
                copts->has_property = true;
                strcpy( copts->property, optarg );
            }
            break;

        case 'y':         // pedantic search
            copts->has_pedantic = 1;
            break;
//TODO: Revise unused stuff. tag seems to be used only by HL and AA. -RM
        case 'z':
            if( opt->has_arg == required_argument ) {
                copts->has_tag = true;
                strcpy( copts->tag_string, optarg );
            }
            break;

//           case 'z':
//               if ( opt->has_arg == required_argument )
//               {
//                   copts->has_threads = true;
//                   strcpy( copts->threads, optarg );
//               }
//               break;

        case 'A':         //enable performance deviation control of multiple experiments

            copts->has_devControl = true;
            break;

        case 'B':
            if( opt->has_arg == required_argument ) {
                copts->has_inst = 1;
                strcpy( copts->inst, optarg );
            }
            break;

        case 'C':         // Source Revision
            if( opt->has_arg == required_argument ) {
                copts->has_srcrev = 1;
                strcpy( copts->srcrev, optarg );
            }
            break;
        case 'I':                // configuration xml file from readex-dyn-detect tool
            if( opt->has_arg == required_argument ){
                copts->has_configurationfile = 1;
                strcpy( copts->configfile_string, optarg );
                read_xml(  opts.configfile_string, configTree );

//                double variation = atof(configTree.get < std::string > ("Configuration.readex-dyn-detect.Inter-phase.variation").c_str());
//                if (variation > 0) {
//                    strcpy( copts->plugin, "interphase_tuning" );
//                    rts_support = true;
//                }

                psc_dbgmsg( 1, "Using configuration xml file \"%s\" \n", opts.configfile_string );
            }
            break;

        default:
            // some unknown option
            return -1;
        }
    }
    return 0;
}


void usage( int   argc,
            char* argv[] ) {
    fprintf( stderr, "Usage: %s <options>\n", argv[ 0 ] );
    fprintf( stderr, "  [--help]                 (Displays this help message)\n" );
    fprintf( stderr, "  [--version]              (Version of Periscope)\n" );
    fprintf( stderr, "  [--appname=name]         (Name of the application)\n" );
    fprintf( stderr, "  [--mpinumprocs=n]        (Number of MPI processes)\n" );
    fprintf( stderr, "  [--ompnumthreads=n]      (Number of OpenMP threads)\n" );
    fprintf( stderr, "  [--strategy=name]        (e.g. EnergyGranularityBF, "
             "Importance, OMP, PerfDyn-OMP, PerfDyn-MPI, PerfDyn-Time, MPI)\n" );
    fprintf( stderr, "  [--phase=name]           (Name of the ScoreP user phase region)\n" );
    fprintf( stderr, "  [--debug=n]              (Max debug level)\n" );
    fprintf( stderr, "  [--selective-debug=list] (Individual debugging level names separated by comma)\n" );
    print_dbgLevelsDefs();
    fprintf( stderr, "  [--registry=host:port]   (Address of the registry service, optional)\n" );
    fprintf( stderr, "  [--port=n]               (Local port number, optional)\n" );
    fprintf( stderr, "  [--parent=parent tag]    (Parent tag)\n" );
    fprintf( stderr, "  [--leaf]                 (Agent directly connects to MPI processes. It is a leaf agent)\n" );
    fprintf( stderr, "  [--timeout=n]            (Timeout for startup of agent)\n" );
    fprintf( stderr, "  [--searches=n]           (Number of searches)\n" );
    fprintf( stderr, "  [--delay=n]              (Search delay in phase executions)\n" );
    fprintf( stderr, "  [--iterations=n]         (Number of phase executions per experiment)\n" );
    fprintf( stderr, "  [--dontregister]         (Don't register with the registry)\n" );
    fprintf( stderr, "  [--parent-host=name]     (Name of the host where HL agent resides)\n" );
    fprintf( stderr, "  [--parent-port=n]        (Port number of the HL agent)\n" );
    fprintf( stderr, "  [--id=list]              (e.g. --id=id1,id2])\n" );
    fprintf( stderr, "  [--propfile=filename]    (File to output the properties)\n" );
    fprintf( stderr, "  [--property=name]        (Name of a property to export)\n" );
    fprintf( stderr, "  [--pedantic]             (Shows all detected properties)\n" );
//TODO: Revise unused stuff. tag seems to be used only by HL and AA. -RM
    fprintf( stderr, "  [--tag=str]              ()\n" );
    fprintf( stderr, "  [--with-deviation-control] (enables performance deviation control on POWER architectures)\n" );
    fprintf( stderr, "  [--inst]                 (Name of the instrumentation strategy, e.g. overhead, analysis)\n" );
    fprintf( stderr, "  [--srcrev]               (Source code revision)\n" );
    fprintf( stderr, "  [--config-file=filename]  (Relative path to the configuration file)\n" );
}

/////////////////////////////////////////
////                                /////
////             MAIN               /////
////                                /////
/////////////////////////////////////////
int main( int   argc,
          char* argv[] ) {
    std::string app_name, metricPlugin_s;
    psc_init_start_time();
    psc_dbgmsg( 1, "Analysis agent started.\n" );
    psc_dbgmsg (2, "AAgent started on node: \n"); system("hostname");


    agent = new AnalysisAgent( ACE_Reactor::instance() );

    if( ( parse_opts( argc, argv, &opts ) == -1 ) || opts.has_showhelp ) {
        usage( argc, argv );
        exit( 1 );
    }
    try {
        metricPlugin_s = configTree.get < std::string > ("Configuration.periscope.metricPlugin.name");
    } catch (exception &e) {
        metricPlugin_s = "hdeem_sync_plugin";
    }

    initEventlist( metricPlugin_s.c_str() );

    if( psc_config_open() ) {
        psc_dbgmsg( 6, "Opened config file\n" );
        if( !opts.has_debug ) {
            sprintf( opts.debug_string, "%d", psc_config_debug() );
            opts.has_debug = true;
            // psc_dbgmsg( 0, "Debug is set from config file\n");
        }

        if( !opts.has_mpinumprocs ) {
            sprintf( opts.mpinumprocs_string, "%d", psc_config_procs() );

            opts.has_mpinumprocs = true;
            //  psc_dbgmsg( 0, "mpinumprocs and mpi per AA is set from config file\n");
        }
        if( !opts.has_ompnumthreads ) {
            sprintf( opts.ompnumthreads_string, "%d", psc_config_ompnumthreads() );
            opts.has_ompnumthreads = true;
        }

        if( !opts.has_parent ) {
            char parentPtr[ 2000 ];
            if( psc_config_aa_parent( parentPtr, 2000 ) ) {
                strcpy( opts.parent_string, parentPtr );
                opts.has_parent = true;
                //  psc_dbgmsg( 0, "Parent is set from config file\n");
            }
        }

        if( !opts.has_tag ) {
            char tagPtr[ 2000 ];
            if( psc_config_aa_tag( tagPtr, 2000 ) ) {
                strcpy( opts.tag_string, tagPtr );
                opts.has_tag = true;
                //  psc_dbgmsg( 0, "Tag is set from config file\n");
            }
        }

        if( !opts.has_appname ) {
            char applnamePtr[ 2000 ];
            if( psc_config_appname( applnamePtr, 2000 ) ) {
                strcpy( opts.appname_string, applnamePtr );
                opts.has_appname = true;
                //  psc_dbgmsg( 0, "Appname is set from config file\n");
            }
        }

        if( !opts.has_strategy ) {
            char strategy_name[ 2000 ];
            if( psc_config_strategy( strategy_name, 2000 ) ) {
                strcpy( opts.strategy, strategy_name );
                opts.has_strategy = true;
                //  psc_dbgmsg( 0, "Strategy is set from config file\n");
            }
        }
        psc_config_close();
    }


    // -----
    // T A G
    // -----

    if( !opts.has_tag ) {
        if( opts.has_parent ) {
            psc_errmsg( "Tag has to be specified\n" );
            usage( argc, argv );
            exit( 1 );
        }
        else {
            strcpy( opts.tag_string, "local" );
        }
    }

#ifdef _BGP_PORT
    // force the tag to be a host name on BG/P
    char host_name[ 256 ];
    gethostname( host_name, 256 );
    strcpy( opts.tag_string, host_name );
    psc_dbgmsg( 8, "Tag is forced to the host name ($s) on BG/P\n", host_name );
#endif

    psc_dbgmsg( 8, "Tag is set to %s\n", opts.tag_string );
    psc_set_msg_prefix( opts.tag_string );
    psc_set_progname( "psc_analysisagent" );
    //End TAG

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
            psc_dbg_level = 0;                          /* assume debug-level 0 */
        }
        else {
            psc_dbg_level = atoi( bufdbg );
        }
    }

#ifdef _BGP_PORT
    psc_dbgmsg( 1, "Analysis agent started on %s.\n", host_name );
#else
    psc_dbgmsg( 1, "Analysis agent started [%d]\n", getpid() );
#endif

    RegistryService* regsrv;

    if( opts.has_fastmode ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ), "operating in fast mode; no registry...\n" );
        agent->set_fastmode( true );
        agent->set_parent_host( opts.parent_host );
        agent->set_parent_port( opts.parent_port );
    }
    else {
        int  reg_port = 0;
        char tmp_str[ 2000 ];
        char reg_host[ 2000 ];
        if( opts.has_registry ) {
            strcpy( tmp_str, opts.reg_string );            //opts.reg_string is needed later as well
            char* tmp;

            if( !( tmp = strchr( tmp_str, ':' ) ) ) {
                // string has to specify hostname:port
                psc_errmsg( "Unrecognized hostname or port for registry\n" );
                usage( argc, argv );
                exit( 1 );
            }
            else {
                // terminate the hostname part with "\0";
                ( *tmp ) = 0;
                // determine hostname and port
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
                if( !opts.has_dontregister ) {
                    psc_errmsg( "Error opening config file, cannot continue\n" );
                    usage( argc, argv );
                    exit( 1 );
                }
                else {
                    // need to specify reghost and port later...
                    strcpy( reg_host, "(reg_host not specified)" );
                    reg_port = -1;
                }
            }
        }
        psc_dbgmsg( 1, "Using reg_host=%s reg_port=%d\n", reg_host, reg_port );
        regsrv = new RegistryService( reg_host, reg_port );
    }

    if( strcmp( opts.strategy, "SCOREP_MPI" ) == 0 ) {
        psc_dbgmsg( 5, "Analysis using Score-P is requested.\n" );
        appl = &Application::instance();
    }


    /*Instantiating an application class object.*/
    appl = &Application::instance();


    if( opts.has_appname ) {
        app_name = opts.appname_string;
    }
    else {
        if( getenv( "PSC_APPNAME" ) ) {
            app_name = getenv( "PSC_APPNAME" );
        }
        else {
            app_name = "appl";
        }
    }

    if( opts.has_searches ) {
        //str_searches = opts.searches;
    }

    if( !opts.has_propfile ) {
        strcpy( opts.prop_file, PROP_FILE );
    }
    if( !opts.has_parent ) {
        psc_dbgmsg( 1, "Using output file: %s\n", opts.prop_file );
    }

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

    int myport = 30000;
    if( opts.has_desired_port ) {
        myport = atoi( opts.port_string );
        if( myport <= 0 ) {
            psc_errmsg( "Bad port number: %s\n", opts.port_string );
            usage( argc, argv );
            exit( 1 );
        }
    }
    else {
        char* portstr;
        if( ( portstr = getenv( "PSC_AGENT_BASEPORT" ) ) != 0 ) {
            myport = atoi( portstr );
        }
        else {
            if( psc_config_open() ) {
                int configPort = psc_config_agent_baseport();
                if( configPort > 0 ) {
                    myport = configPort;
                }
                psc_config_close();
            }
        }
    }
    psc_dbgmsg( 5, "Desired port %d\n", myport );
    if( myport < 20000 ) {
        psc_dbgmsg( 5, "Desired port reset to 52100%d\n" );
        myport = 52100;
    }

    // -------------
    // T I M E O U T
    // -------------
    int timeout = 120;
    if( opts.has_timeout ) {
        timeout = atoi( opts.timeout_string );
        if( timeout <= 0 ) {
            psc_errmsg( "Bad value for timeout: %s\n",
                        opts.timeout_string );
            exit( 1 );
        }
    }
    psc_dbgmsg( 5, "Using %d seconds timeout\n", timeout );

    // ------------------
    // D E L A Y
    // ------------------
    if( opts.has_delay ) {
        int delay = atoi( opts.delay_string );
        if( delay < 0 ) {
            psc_errmsg( "invalid delay %i specified, using 0 (disabled)\n", delay );
            opts.has_delay = 0;
        }
        else {
            agent->set_delay( delay );
        }
    }

    // ------------------------
    // I T E R A T I O N S
    // ------------------------
    if( opts.has_iterations ) {
        int iterations = atoi( opts.iterations_string );
        if( iterations < 1 ) {
            psc_errmsg( "invalid number of iterations %i specified, using default value\n", iterations );
            opts.has_delay = 0;
        }
        else {
            agent->set_num_iterations( iterations );
        }
    }

    if( opts.has_mpinumprocs ) {
        appl->setMpiProcs( atoi( opts.mpinumprocs_string ) );
    }
    else {
        appl->setMpiProcs( 1 );
    }

    if( opts.has_ompnumthreads ) {
        appl->setOmpThreads( atoi( opts.ompnumthreads_string ) );
    }
    else {
        appl->setOmpThreads( 1 );
    }

    if( !app_name.empty() ) {
        appl->set_app_name( app_name );
        agent->set_appname( app_name );
    }
    else {
        agent->set_appname( "(unknown)" );
    }

    psc_dbgmsg( 1, "Appname is set to: %s\n", appl->get_app_name().c_str() );

    std::list<int> ids;

#ifndef _BGP_PORT
    // TODO check the if condition; changed
    if( opts.has_id || agent->get_fastmode() ) {    // ID is mandatory in fast mode
        char* tmp;
        int   id;

        // tokenize the string and put the IDs in the list
        tmp = strtok( opts.id_string, " ,\n" );
        while( tmp != 0 ) {
            id = atoi( tmp );
            if( id < 0 ) {
                psc_errmsg( "Malformed Id: %s\n", tmp );
                usage( argc, argv );
                exit( 1 );
            }
            ids.push_back( id );

            tmp = strtok( 0, " ,\n" );
        }
        free( opts.id_string );
    }
    else {
        //ids are not given searching registry
        char hostname[ 2000 ];
        gethostname( hostname, 2000 );

        psc_dbgmsg( 1, "id==local => querying registry for processes of application %s.\n", appl->get_app_name().c_str() );

        EntryData             query;
        std::list<EntryData > result;

        // specify the component and the node
        // the rest is filled in by our query request
        query.app =  appl->get_app_name().c_str();

        if( regsrv->query_entries( result, query, true ) == -1 ) {
            psc_errmsg( "Error querying registry for processes of application %s\n", query.app.c_str() );
            exit( 1 );
        }

        psc_dbgmsg( 3, "Identified %d matching processes of application %s\n", result.size(), query.app.c_str() );

        std::list< EntryData >::iterator it;
        for( it = result.begin(); it != result.end(); it++ ) {
            psc_dbgmsg( 3, " id=%d app=%s site=%s mach=%s node=%s port=%d pid=%d comp=%s, %d strings\n",
                        it->id, it->app.c_str(), it->site.c_str(), it->mach.c_str(),
                        it->node.c_str(), it->port, it->pid, it->comp.c_str(),
                        it->strings.size() );

            ids.push_back( it->id );
        }
    }      //has_id
#endif

    // -----------------------------
    // I N S T R U M E N T A T I O N
    // -----------------------------

    if( opts.has_inst ) {
        psc_errmsg( "Instrumentation strategy support was deprecated\n" );
    }


    // ------------------
    // P H A S E
    // ------------------


    if( !opts.has_phase ) {
        psc_errmsg( "The name of the phase region was not provided! \n" );
        exit( 1 );
    }

    /////////////////////////////////////////////////////////////////////////
    //                  Initialize the Nodeagent                           //
    /////////////////////////////////////////////////////////////////////////
    //Initialize the context Provider

    agent->set_own_tag( opts.tag_string );
    agent->set_machinename( psc_machine );
    agent->set_sitename( psc_site );
    agent->set_global_timeout( timeout );
    agent->set_parent_port( opts.parent_port );
    agent->set_parent_host( opts.parent_host );

    if( agent->open( myport ) == -1 ) {
        psc_errmsg( "Error open()ing nodeagent on port %d. Terminating.\n", myport );
        exit( 1 );
    }

    // register with registry unless told not to do so
    psc_dbgmsg( 5, "Before register with registry\n" );
    if( !opts.has_dontregister && !agent->get_fastmode() ) {
        agent->set_registry( regsrv );

        if( agent->register_self() == -1 ) {
            psc_errmsg( "Error registering at registry...\n" );
            exit( 1 );
        }
    }

    dp  = new DataProvider( appl, opts.phase_string, agent->get_registry() );
    pdb = new PerformanceDataBase( dp );
    pdb->setDataProvider( dp );

    if( agent->get_fastmode() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ), "fast mode setup is set in the AA\n" );
        if( opts.has_parent ) {
            std::string parentTag;
            parentTag = opts.parent_string;
            agent->set_parent( parentTag );
            psc_dbgmsg( 2, "Using parent %s\n",  agent->parent() );
            if( agent->connect_to_parent() == -1 ) {
                psc_errmsg( "Error in analysisagent connecting to parent at %s\n", agent->parent() );
                exit( 1 );
            }
        }
        else {
            psc_errmsg( "Parent not specified, printing to stdout\n" );
        }

        dp->init( agent->get_registry(), ids );
    }
    else {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ), "fast mode setup is NOT set in the AA\n" );
        dp->init( agent->get_registry(), ids );
        if( opts.has_parent ) {
            std::string parentTag;
            parentTag = opts.parent_string;
            agent->set_parent( parentTag );
            psc_dbgmsg( 2, "Using parent %s\n",  agent->parent() );
            if( agent->connect_to_parent() == -1 ) {
                psc_errmsg( "Error in analysisagent connecting to parent at %s\n", agent->parent() );
                exit( 1 );
            }
        }
        else {
            psc_errmsg( "Parent not specified, printing to stdout\n" );
        }
        // we're possibly listening on a port other our desired one, if it was already in use
        psc_dbgmsg( 5, "Port set to %d\n", agent->get_local_port() );
    }



    // temporary solution to get the data to the frontend until it accepts commands
    if( opts.has_parent ) {
        EventHandling evh( agent );
        agent->run();
        evh.evh_cleanup();
    }
    else {
        StrategyRequestGeneralInfo* strategyRequestGeneralInfo = new StrategyRequestGeneralInfo;
        // set the analysis strategy
        strategyRequestGeneralInfo->strategy_name     = opts.strategy;
        strategyRequestGeneralInfo->delay_phases      = 0;
        strategyRequestGeneralInfo->delay_seconds     = 0;
        strategyRequestGeneralInfo->analysis_duration = 1;
        StrategyRequest* strategyRequest = new StrategyRequest( strategyRequestGeneralInfo );

        agent->set_strategy( strategyRequest );

        // OR do the automatic analysis
        // as a standalone tool
        // check_properties sets the initial properties and starts the experiment
        agent->stand_alone_search();
        delete strategyRequest;
        Prop_List results = agent->get_results();
        Prop_List newResults;

        // Print the found properties to the stdout
        agent->print_property_set( results, " SET OF FOUND PROPERTIES Search 1", true, true );

        agent->export_property_set( opts.prop_file );

        int numberSearches = 1;
        if( opts.has_searches ) {
            numberSearches = atoi( opts.searches );
        }
        for( int run = 2; run <= numberSearches; run++ ) {
            pdb->clean();
            std::cout << std::endl << "Starting search " << run << std::endl;
            std::cout << "=================" << std::endl;
            agent->stand_alone_search();
            std::list <Property*> newResults = agent->get_results();
            if( psc_get_debug_level() >= 1 ) {
                agent->print_property_set( newResults, " SET OF FOUND PROPERTIES", true, true );
            }
            agent->compareResults( newResults, results, run );
            results = newResults;
        }
    }    //if(opts.has_parent) */

    //free resources
    dp->app_finish();

    delete agent;
    delete pdb;
    if( !opts.has_fastmode ) {
        delete regsrv;
    }
    finalizeEventlist();

    return 0;
}
