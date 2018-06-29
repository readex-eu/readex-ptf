/**
   @file    frontend_main.cc
   @ingroup Frontend
   @brief   Front-end main routine
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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __sparc
#include "getopt.h"
#elif __p575
#include "getopt.h"
#else
#include <getopt.h>
#endif

#include "frontend.h"
#include "regxx.h"
#include "psc_errmsg.h"
#include "selective_debug.h"
#include "timing.h"
#include "psc_config.h"
#include "application.h"
#include "selective_debug.h"
#include "frontend_accl_statemachine.h"
#include "frontend_main_statemachine.h"
#include <boost/property_tree/ptree.hpp>


using namespace std;
using namespace frontend_main_statemachine;

// these are referenced in other files
PeriscopeFrontend*  fe;
ApplicationStarter* starter;
Application*        appl;
boost::property_tree::ptree configTree;
int                 maxfan_default = 4;

struct cmdline_opts opts;

bool rts_support=false;

int parse_opts( int                  argc,
                char*                argv[],
                struct cmdline_opts* copts ) {
    static const struct option long_opts[] = {
//      { const char* name,         int has_arg,       int* flag, int val }
        { "help",                   no_argument,       0,         'a' },
        { "version",                no_argument,       0,         'b' },
        { "appname",                required_argument, 0,         'c' },
        { "apprun",                 required_argument, 0,         'd' },
        { "mpinumprocs",            required_argument, 0,         'e' },
        { "ompnumthreads",          required_argument, 0,         'f' },
        { "strategy",               required_argument, 0,         'g' },
        { "tune",                   required_argument, 0,         'h' },
        { "phase",                  required_argument, 0,         'i' },
        { "uninstrumented",         no_argument,       0,         'j' },
        { "force-localhost",        no_argument,       0,         'l' },
        { "info",                   required_argument, 0,         'm' },
        { "selective-info",         required_argument, 0,         'n' },
        { "statemachine-trace",     no_argument,       0,         'o' },
        { "registry",               required_argument, 0,         'p' },
        { "port",                   required_argument, 0,         'q' },
        { "maxfan",                 required_argument, 0,         'r' },
        { "maxcluster",             required_argument, 0,         's' },
        { "maxthreads",             required_argument, 0,         't' },
        { "timeout",                required_argument, 0,         'u' },
        { "delay",                  required_argument, 0,         'v' },
        { "duration",               required_argument, 0,         'w' },
        { "masterhost",             required_argument, 0,         'x' },
        { "hlagenthosts",           required_argument, 0,         'y' },
        { "nodeagenthosts",         required_argument, 0,         'z' },
        { "agenthostfile",          required_argument, 0,         'A' },
        { "dontcluster",            no_argument,       0,         'B' },
        { "manual",                 no_argument,       0,         'C' },
        { "property",               required_argument, 0,         'D' },
        { "propfile",               required_argument, 0,         'E' },
        { "nrprops",                required_argument, 0,         'F' },
        { "pedantic",               no_argument,       0,         'G' },
        { "iterations",             required_argument, 0,         'H' },
        { "config-file",            required_argument, 0,         'I' },
        { "input-desc",             required_argument, 0,         'J' },
        0
    };

    copts->has_force_localhost = 1;

    while( optind < argc ) {
        int            index = -1;
        struct option* opt   = 0;

        opterr = 0;
        int result = getopt_long( argc, argv, "", long_opts, &index );

        if( result == -1 ) {
            psc_errmsg( "Error parsing command line parameters.\n" );
            return -1;
        }

        opt = ( struct option* )&( long_opts[ index ] );

        switch( result ) {
        case 'a':
            copts->has_showhelp = 1;
            break;

        case 'b':
            fprintf( stdout, "The Periscope Tuning Framework: Version %s\n", xstr( _VERSION ) );
            exit( EXIT_SUCCESS );
            break;

        case 'c':
            if( opt->has_arg == required_argument ) {
                copts->has_appname = 1;
                strcpy( copts->appname_string, optarg );
            }
            break;

        case 'd':
            if( opt->has_arg == required_argument ) {
                copts->has_apprun = 1;
                strcpy( copts->app_run_string, optarg );
            }
            break;

        case 'e':
            if( opt->has_arg == required_argument ) {
                copts->has_mpinumprocs = 1;
                strcpy( copts->mpinumprocs_string, optarg );
            }
            break;

        case 'f':
            if( opt->has_arg == required_argument ) {
                copts->has_ompnumthreads = 1;
                strcpy( copts->ompnumthreads_string, optarg );
            }
            break;

        case 'g':
            if( opt->has_arg == required_argument ) {
                copts->has_strategy = 1;
//TODO: Why do we copy it twice? Compare could be done with strcmp. Do we have these strategies? -RM
                strcpy( copts->strategy, optarg );
                std::string strategy_name = optarg;
                if( strategy_name == "scalability_OMP" ) {
                    copts->has_scalability_OMP = 1;
                }
                else if( !strcmp( optarg, "tune" ) ) {
                    // force no-cluster in autotune mode
                    copts->has_dontcluster = 1;
                }
            }
            break;

        case 'h':
            if( opt->has_arg == required_argument ) {
                copts->has_plugin = 1;
                strcpy( copts->plugin, optarg );
                std::list<std::string> plugin_name = { "readex_intraphase", "readex_interphase", "readex_intraphase_model", "readex_configuration" };
                // checks if readex_tuning or interphase_tuning is requested and if so enables RTS support.
                rts_support = std::find( plugin_name.begin(), plugin_name.end(), copts->plugin ) != plugin_name.end();

                copts->has_strategy = 1;
                strcpy( copts->strategy, "tune" );
                // force no-cluster in autotune mode
                copts->has_dontcluster = 1;
            }
            break;

        case 'i':                 // Delay in number of phase executions
            if( opt->has_arg == required_argument ) {
                copts->has_phase = 1;
                strcpy( copts->phase_string, optarg );
            }
            break;

        case 'j':
            copts->has_uninstrumented = true;
            break;

        case 'l':
            copts->has_force_localhost = 1;
            break;

        case 'm':
            if( opt->has_arg == required_argument ) {
                copts->has_debug = 1;
                strcpy( copts->debug_string, optarg );
            }
            break;

        case 'n':
            if( opt->has_arg == required_argument ) {
                copts->has_selectivedebug = 1;
                strcpy( copts->selectivedebug_string, optarg );
                std::string levels = optarg;
                handle_dbgLevelList( levels );
            }
            break;

        case 'o':
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAll ), "State Machine Tracing enabled...\n" );
            copts->sm_tracing_enabled = 1;
            PSC_SM_TRACE_SINGLETON->enable();
            break;

        case 'p':
            if( opt->has_arg == required_argument ) {
                copts->has_registry = 1;
                strcpy( copts->reg_string, optarg );
            }
            break;

        case 'q':
            if( opt->has_arg == required_argument ) {
                copts->has_desired_port = 1;
                strcpy( copts->port_string, optarg );
            }
            break;

        case 'r':
            if( opt->has_arg == required_argument ) {
                copts->has_maxfan = 1;
                strcpy( copts->maxfan_string, optarg );
            }
            break;

        case 's':
            if( opt->has_arg == required_argument ) {
                copts->has_maxcluster = 1;
                strcpy( copts->maxcluster_string, optarg );
            }
            break;

        case 't':
            if( opt->has_arg == required_argument ) {
                copts->has_maxthreads = 1;
                strcpy( copts->maxthreads_string, optarg );
            }
            break;

        case 'u':
            if( opt->has_arg == required_argument ) {
                copts->has_timeout = 1;
                strcpy( copts->timeout_string, optarg );
            }
            break;

        case 'v':                 // Delay in number of phase executions
            if( opt->has_arg == required_argument ) {
                copts->has_delay = 1;
                strcpy( copts->delay_string, optarg );
            }
            break;

        case 'w':                 // Delay of phase in seconds
            if( opt->has_arg == required_argument ) {
                copts->has_duration = 1;
                strcpy( copts->duration_string, optarg );
            }
            break;

        case 'x':
            if( opt->has_arg == required_argument ) {
                copts->has_masterhost = 1;
                strcpy( copts->masterhost_string, optarg );
            }
            break;

        case 'y':
            if( opt->has_arg == required_argument ) {
                copts->has_hlagenthosts = 1;
                strcpy( copts->hlagenthosts_string, optarg );
            }
            break;

        case 'z':
            if( opt->has_arg == required_argument ) {
                copts->has_nodeagenthosts = 1;
                strcpy( copts->nodeagenthosts_string, optarg );
            }
            break;

        case 'A':
            if( opt->has_arg == required_argument ) {
                copts->has_agenthostfile = 1;
                strcpy( copts->agenthostfile_string, optarg );
            }
            break;

        case 'B':
            copts->has_dontcluster = 1;
            break;

        case 'C':
            copts->has_manual = 1;
            break;

        case 'D':
            if( opt->has_arg == required_argument ) {
                copts->has_property = 1;
                strcpy( copts->property, optarg );
            }
            break;

        case 'E':
            if( opt->has_arg == required_argument ) {
                copts->has_propfile = 1;
                std::string tmpStr( optarg );
                if( tmpStr[ 0 ] == '~' ) {
                    char* homeStr = getenv( "HOME" );
                    tmpStr.replace( 0, 1, homeStr );
                }
                strncpy( copts->prop_file, tmpStr.c_str(), tmpStr.length() );
            }
            break;

        case 'F':
            if( opt->has_arg == required_argument ) {
                copts->has_nrprops = 1;
                copts->nrprops     = atoi( optarg );
            }
            break;

        case 'G':                 // pedantic search
            copts->has_pedantic = true;
            break;

        case 'H':                 // number of phase executions per experiment
            if( opt->has_arg == required_argument ) {
                copts->has_iterations = 1;
                strcpy( copts->iterations_string, optarg );
            }
            break;
        case 'I':                // configuration xml file from readex-dyn-detect tool
            if( opt->has_arg == required_argument ){
                copts->has_configurationfile = 1;
                strcpy( copts->configfile_string, optarg );
                try {
                    read_xml( opts.configfile_string, configTree );
                    psc_dbgmsg( 1, "Using configuration xml file \"%s\" \n", opts.configfile_string );
//                    double variation = atof(configTree.get < std::string > ("Configuration.readex-dyn-detect.Inter-phase.variation").c_str());
//                    if (variation > 0) {
//                        strcpy( copts->plugin, "interphase_tuning" );
//                        rts_support = true;
//                    }
                }
                catch ( std::exception& e )
                {
                    psc_errmsg( "Wrong Configuration file  %s \n", e.what ());
                    return -1;
                }
            }

            break;
        case 'J':
            if( opt->has_arg == required_argument ) {
                 copts->has_input_desc = 1;
                 strcpy( copts->input_desc_string, optarg );
                 psc_dbgmsg( 1, "Parsing input description identifiers: %s\n", opts.input_desc_string );
            }
            break;
//        case 'I':
//            if( opt->has_arg == required_argument ) {
//                copts->has_threads = 1;
//                strcpy( copts->threads, optarg );
//            }
//            break;
//
//        case 'J':                   //BGP mode could be VN/DUAL/SMP
//            if( opt->has_arg == required_argument ) {
//                copts->has_bg_mode = 1;
//                strcpy( copts->bg_mode, optarg );
//            }
//            break;
//
//        case 'K':             //enable performance deviation control of multiple experiments
//            copts->has_devControl = 1;
//            break;
//
//        case 'L':
//            if( opt->has_arg == required_argument ) {
//                copts->has_make = 1;
//                strcpy( copts->make_string, optarg );
//            }
//            break;
//
//        case 'M':
//            if( opt->has_arg == required_argument ) {
//                copts->has_config = 1;
//                strcpy( copts->config_string, optarg );
//            }
//            break;
//
//        case 'N':
//            if( opt->has_arg == required_argument ) {
//                copts->has_inst = 1;
//                strcpy( copts->inst_string, optarg );
//                if( !strcmp( copts->inst_string, "overhead" ) &&
//                    !strcmp( copts->inst_string, "all_overhead" ) &&
//                    !strcmp( copts->inst_string, "analysis" ) ) {
//                    psc_errmsg( "ERROR: invalid instrumentation strategy\n" );
//                    usage( argc, argv );
//                    exit( 1 );
//                }
//            }
//            break;
//
//        case 'O':
//            if( opt->has_arg == required_argument ) {
//                copts->has_instfolder = 1;
//                strcpy( copts->instfolder_string, optarg );
//            }
//            break;
//
//        case 'P':
//            if( opt->has_arg == required_argument ) {
//                copts->has_sourcefolder = 1;
//                strcpy( copts->sourcefolder_string, optarg );
//            }
//            break;
//
//        case 'Q':                   // Source Revision
//            if( opt->has_arg == required_argument ) {
//                copts->has_srcrev = 1;
//                strcpy( copts->srcrev, optarg );
//            }
//            break;

        case '?':
            if( copts->has_plugin == 1 ) { // if --tune=plugin passed, we allow extra parameters
                psc_infomsg( "Non psc_frontend parameter (\"%s\") will be passed to the loaded Autotune plugin's (\"%s\") Context\n",
                             argv[ optind - 1 ], copts->plugin );
                fe->plugin_argv_list.push_back( string( argv[ optind - 1 ] ) );
            }
            else {                    // otherwise we print usage and exit
                psc_errmsg( "Invalid psc_frontend parameter: \"%s\"\n", argv[ optind - 1 ] );
                return -1;
            }
            break;

        default:
            // some other / unknown option specified
            return -1;
        }
    }

    if ( copts->has_phase == 0 ) {
       psc_errmsg( "Mandatory phase region is missing. Please use --phase=<phase name>\n" );
       return -1;
    }

    /*check mandatory --config-file option*/
    if ( copts->has_configurationfile == 0 && rts_support ) {
        psc_errmsg( "Tuning with READEX plugins requires Readex Configuration file which is missing. Please use --config-file=<file_name>\n" );
        return -1;
    }

    return 0;
}

void usage( int   argc,
            char* argv[] ) {
    assert( argc > 0 );

    fprintf( stderr, "Usage: %s <options>\n", argv[ 0 ] );
    fprintf( stderr, "  [--help]                 (Displays this help message)\n" );
    fprintf( stderr, "  [--version]              (Version of Periscope)\n" );
    fprintf( stderr, "  [--appname=name]         (Name of the application)\n" );
    fprintf( stderr, "  [--apprun=commandline]   (Command used to run the application)\n" );
    fprintf( stderr, "  [--mpinumprocs=n]        (Number of MPI processes)\n" );
    fprintf( stderr, "  [--ompnumthreads=n]      (Number of OpenMP threads)\n" );
//    fprintf( stderr, "  [--strategy=name]        (e.g. All, Benchmarking, "
//             "BGP_Cache, ConfigAnalysis, EnergyGranularity, "
//             "EnergyGranularityBF, EnergyStrategy, GPU, Importance, "
//             "Instrumentation, MPI, OCL, OpenCL, OMP, p575Strategy, "
//             "p575StrategyBF, PerfDyn-OMP, PerfDyn-MPI, PerfDyn-Time, "
//             "Pipeline, RegionNestingStrategy (RN), SCPS_BF, SC, SC_BF, "
//             "SCPerSyst, WestmereBF, "WestmereDF)\n" );
    fprintf( stderr, "  [--strategy=name]        (e.g. EnergyGranularityBF, "
             "Importance, OMP, PerfDyn-OMP, PerfDyn-MPI, PerfDyn-Time, MPI)\n" );
    fprintf( stderr, "  [--tune=name]            (e.g. adaptivesequence, compilerflags,"
             " dvfs, fixedsequence, mpicap, mpiparameters, pcap, pcap-speedup, energy)\n");
    fprintf( stderr, "  [--phase=name]           (Name of the ScoreP user phase region)\n" );
    fprintf( stderr, "  [--uninstrumented]       (Application was not instrumented)\n" );
//    fprintf( stderr, "  [--starter=name]         (e.g. FastInteractive/Interactive/SLURM)\n" );
    fprintf( stderr, "  [--force-localhost]      (Do not use SSH to start the agents)\n" );
    fprintf( stderr, "  [--info=n]               (Max info level)\n" );
    fprintf( stderr, "  [--selective-info=list]  (Individual information level names separated by comma)\n" );
    print_dbgLevelsDefs();
    fprintf( stderr, "  [--statemachine-trace    (Collects and prints state-machine transitions)\n" );
    fprintf( stderr, "  [--registry=host:port]   (Address of the registry service, optional)\n" );
    fprintf( stderr, "  [--port=n]               (Local port number, optional)\n" );
    fprintf( stderr, "  [--maxfan=n]             (Max. number of child agents, default=%d)\n", maxfan_default );
    fprintf( stderr, "  [--maxcluster=n]         (Max. number of processes controlled by an agent, default=%d)\n", 4 );
    fprintf( stderr, "  [--maxthreads=n]         (Max. number of threads assigned to a node agent)\n");
    fprintf( stderr, "  [--timeout=n]            (Timeout for startup of agent hierarchy)\n" );
    fprintf( stderr, "  [--delay=n]              (Search delay in phase executions)\n" );
    fprintf( stderr, "  [--iterations=n]         (Number of phase executions per experiment)\n" );
    fprintf( stderr, "  [--duration=n]           (Search delay in seconds of phase)\n" );
    fprintf( stderr, "  [--masterhost=hostname]  (Name of the host where the frontend starts)\n" );
    fprintf( stderr, "  [--hlagenthosts=list]    (List with host names of HL agents, e.g. --hlagenthost=h1,h2,...)\n" );
    fprintf( stderr, "  [--nodeagenthosts=list]  (List with host names of analysis agents, e.g. --nodeagenthosts=h1,h2,...)\n" );
    fprintf( stderr, "  [--agenthostfile=name]   (File containing host configuration)\n" );
    fprintf( stderr, "  [--dontcluster]          (Do not try to cluster the properties in the HL Agents)\n" );
    fprintf( stderr, "  [--manual]               (Run Periscope in manual mode)\n" );
    fprintf( stderr, "  [--property=name]        (Name of a property to export)\n" );
    fprintf( stderr, "  [--propfile=filename]    (File to output the properties)\n" );
    fprintf( stderr, "  [--nrprops=n]            (Limit the number of found properties to be printed)\n" );
    fprintf( stderr, "  [--pedantic]             (Shows all detected properties)\n" );
    fprintf( stderr, "  [--config-file=filename] (Relative path to the configuration file)\n" );
    fprintf( stderr, "  [--input-desc=filename]  (Relative path to the input identifier configuration file)\n" );
// Options that do not work for now.
//    fprintf( stderr, "  [--threads=n]            (Number of threads)\n");
//    fprintf( stderr, "  [--with-deviation-control] (enables performance deviation control on POWER architectures)\n" );
//    fprintf( stderr, "  [--bg-mode=mode] (when running on BlueGene/P set the mode of nodes: SMP(default)/DUAL/VN)\n" );

// Reinstrumentation strategy specific options
//    fprintf( stderr, "  [--make=make command]    (Command to be issued in order to automatically recompile application)\n" );
//    fprintf( stderr, "  [--psc-inst-config=path] (Relative path of psc_inst_config)\n" );
//    fprintf( stderr, "  [--inst=name]            (Name of the instrumentation strategy, e.g. overhead, analysis)\n" );
//    fprintf( stderr, "  [--inst-folder=path]     (Relative path to the inst folder)\n" );
//    fprintf( stderr, "  [--src-folder=path]      (Relative path to the source folder)\n" );
//    fprintf( stderr, "  [--srcrev=n]             (Source code revision)\n");

// Plugin specific options
    fprintf( stderr, "  [--cfs-config=filename]  (Relative path to the CFS plugin configuration file)\n" );

}


// cleaned up main routine with the use of Boost: Meta State Machine
int main( int   argc,
          char* argv[] ) {
    // state machine creation and startup
    main_frontend_statemachine fmsm;
    fmsm.start();     // initial state: Initializing
    fmsm.process_event( init_data_structures_event() );
    fmsm.process_event( parse_parameters_event( argc, argv, fe ) );
    fmsm.process_event( setup_debug_event() );
    psc_dbgmsg (2, "psc_frontend started on node: \n"); system("hostname");

    // version defined by configure script in config.h
    fprintf( stdout, "Periscope Performance Analysis Tool (ver. %s)\n", VERSION );
    psc_infomsg( "Preparing to start the performance analysis...\n" );

    // assert that initialization is not done yet
    psc_dbgmsg( FRONTEND_MSM_DEBUG_LEVEL,
                "---- FRONTEND MAIN SM ---- Initializing state; flag set: %d\n",
                fmsm.is_flag_active<initialization_complete>() );
    assert( !( fmsm.is_flag_active<initialization_complete>() ) );

    // still on: Initializing submachine
    fmsm.process_event( select_hierarchy_setup_mode_event( argc, argv ) );
    fmsm.process_event( setup_network_event() );
    fmsm.process_event( setup_application_data_event() );
    fmsm.process_event( setup_phases_event() );
    fmsm.process_event( setup_processes_event() );
    fmsm.process_event( setup_threads_event() );
    fmsm.process_event( setup_timeouts_event() );
    fmsm.process_event( setup_agents_event() );
    fmsm.process_event( setup_outputfile_event() );
    fmsm.process_event( connect_to_registry_event() );
    fmsm.process_event( select_starter_event() );

    // assert that last initialization step was reached
    psc_dbgmsg( FRONTEND_MSM_DEBUG_LEVEL,
                "---- FRONTEND MAIN SM ---- Initializing state; flag set: %d\n",
                fmsm.is_flag_active<initialization_complete>() );
    assert( fmsm.is_flag_active<initialization_complete>() );

    // state changes in main machine
    fmsm.process_event( start_run( argc, argv ) );


    fmsm.process_event( finalize() );

    PSC_SM_TRACE_SINGLETON->print();

    return frontend_main_statemachine::error_code ;
}
