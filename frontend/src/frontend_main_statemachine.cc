#include "frontend_main_statemachine.h"


namespace frontend_main_statemachine {
void frontend_main_msm::Initializing_::init_data_structures( init_data_structures_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    initial_thread = 1;
    psc_set_msg_prefix( "fe" );
    psc_set_progname( "psc_frontend" );
    psc_init_start_time();
    fe = new PeriscopeFrontend( ACE_Reactor::instance() );
    fe->set_own_tag( "fe" );
    appl = &Application::instance();
}

void frontend_main_msm::Initializing_::parse_parameters( parse_parameters_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    // store all options passed by the user for reference or reuse later
    evt.fe->initial_argc = evt.argc;
    evt.fe->initial_argv = ( char** )malloc( ( evt.fe->initial_argc ) * sizeof *evt.fe->initial_argv );
    for( int i = 0; i < ( evt.fe->initial_argc ); i++ ) {
        int size = strlen( evt.argv[ i ] ) + 1;
        evt.fe->initial_argv[ i ] = ( char* )malloc( size * sizeof( char ) );
        strcpy( evt.fe->initial_argv[ i ], evt.argv[ i ] );
    }

    // parse them for the frontend
    if( ( parse_opts( evt.argc, evt.argv, &opts ) == -1 )
        || opts.has_showhelp || evt.argc <= 2 ) {
        usage( evt.argc, evt.argv );
        exit( 1 );
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                "Setting registry mode by default.\n" );
    evt.fe->set_fastmode( false );

    // prepare plugin options
    generate_context_argc_argv( evt.fe->plugin_context.get(),
                                evt.fe->plugin_argv_list,
                                evt.argv[ 0 ] );
}

void frontend_main_msm::Initializing_::setup_debug( setup_debug_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    psc_dbgmsg( FRONTEND_MSM_DEBUG_LEVEL,
                "---- FRONTEND MAIN SM ---- SUBMACHINE: Action: -- Setting up debug\n" );
    if( opts.has_debug ) {
        psc_dbg_level = atoi( opts.debug_string );
    }
    else {
        int   dbg_level;
        char* bufdbg;

        if( ( bufdbg = getenv( "PERISCOPE_INFO" ) ) == 0 ) {
            psc_dbg_level = 0;
        }
        else {                  /* assume debug-level 0 */
            psc_dbg_level = atoi( bufdbg );
        }
    }
}

void frontend_main_msm::Initializing_::select_hierarchy_setup_mode( select_hierarchy_setup_mode_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    psc_dbgmsg( FRONTEND_MSM_DEBUG_LEVEL,
                "---- FRONTEND MAIN SM ---- SUBMACHINE: Action: -- Setting up the registry\n" );
    if( fe->get_fastmode() ) {          // check for starters that work in fast mode
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                    "Selecting fast setup (registry-less, fast starters and fast ACE...\n" );
        fe->set_id( 1 );
        fe->set_fastmode( true );
    }
    else {               // registry setup, previous slower method
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                    "Selecting registry based hierarchy setup... \n" );
        if( opts.has_registry ) {
            strcpy( tmp_str, opts.reg_string );                    //opts.reg_string is needed later as well
            char* tmp;

            if( !( tmp = strchr( tmp_str, ':' ) ) ) {                // registry has to be specified as <hostname>:<port>
                psc_errmsg( "Unrecognized hostname or port for registry\n" );
                usage( evt.argc, evt.argv );
                exit( 1 );
            }
            else {                     // terminate hostname with "\0"
                ( *tmp ) = 0;
                strcpy( reg_host, tmp_str );                         // separate into hostname and port
                reg_port = atoi( tmp + 1 );

                if( reg_port <= 0 ) {
                    psc_errmsg( "Unrecognized hostname or port for registry\n" );
                    usage( evt.argc, evt.argv );
                    exit( 1 );
                }
            }
        }
        else {
            if( getenv( "PSC_REGISTRY" ) ) {
                char* tmp;
                strcpy( envRegSpec, getenv( "PSC_REGISTRY" ) );

                if( !( tmp = strchr( envRegSpec, ':' ) ) ) {
                    fprintf( stderr, "Unrecognized registry setting: %s\n", envRegSpec );
                    exit( 1 );
                }
                else {
                    ( *tmp ) = 0;
                    strcpy( reg_host, envRegSpec );
                    reg_port = atoi( tmp + 1 );
                }
            }
            else {
                // this is the new version of this block
                if( psc_config_open() ) {
                    if( psc_config_reghost_init( reg_host, 2000 ) == 0 ) {
                        psc_errmsg( "Error reading the initial host (REGSERVICE_HOST_INIT) "
                                    "from the config file! Aborting...\n" );
                        exit( EXIT_FAILURE );
                    }

                    if( ( reg_port = psc_config_regport_init() ) == 0 ) {
                        psc_errmsg( "Error reading the initial port (REGSERVICE_PORT_INIT) "
                                    "from the config file! Aborting...\n" );
                        exit( EXIT_FAILURE );
                    }

                    psc_config_close();
                }
                else {
                    psc_errmsg( "Error opening config file! Aborting...\n" );
                    exit( EXIT_FAILURE );
                }
            }
        }

        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ), "Calling the registry "
                    "constructor in the frontend main state-machine (setup_registry)...\n" );
        regsrv = new RegistryService( reg_host, reg_port, true );
        fe->set_registry( regsrv );
        psc_dbgmsg( FRONTEND_HIGH_DEBUG_LEVEL,
                    "Using registry service at '%s:%d'\n", regsrv->get_reghost(), regsrv->get_regport() );
    }
}


void frontend_main_msm::Initializing_::setup_network( setup_network_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    psc_dbgmsg( FRONTEND_MSM_DEBUG_LEVEL,
                "---- FRONTEND MAIN SM ---- SUBMACHINE: Action: -- Setting up network\n" );

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

    fe->set_machinename( psc_machine );
    fe->set_sitename( psc_site );

    // If MACHINE is localhost -> set force-localhost by default
    if( strcmp( psc_machine, "localhost" ) == 0 ) {
        opts.has_force_localhost = 1;
    }

    if( opts.has_force_localhost ) {
        psc_dbgmsg( FRONTEND_GENERAL_DEBUG_LEVEL,
                    "Localhost startup activated! Will NOT use SSH for starting the agents!\n" );
    }

    myport = 30000;
    if( opts.has_desired_port ) {
        myport = atoi( opts.port_string );
        if( myport <= 0 ) {
            psc_errmsg( "Bad value for desired port: %s\n",
                        opts.port_string );
            exit( 1 );
        }
    }
    else {
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
    psc_dbgmsg( FRONTEND_HIGH_DEBUG_LEVEL, "Using own port number '%d'\n", myport );

    if( opts.has_delay ) {
        int delay = atoi( opts.delay_string );
        if( delay < 0 ) {
            psc_errmsg( "invalid delay %i specified, using 0 (disabled)\n", delay );
            opts.has_delay = 0;
        }
    }

    if( opts.has_iterations ) {
        int iterations = atoi( opts.delay_string );
        if( iterations < 0 ) {
            psc_errmsg( "invalid number of iterations %i specified, using default value\n", iterations );
            opts.has_iterations = 0;
        }
    }
}

void frontend_main_msm::Initializing_::setup_phases( setup_phases_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    psc_dbgmsg( FRONTEND_MSM_DEBUG_LEVEL,
                "---- FRONTEND MAIN SM ---- SUBMACHINE: Action: -- Setting up phases\n" );



    if( applUninstrumented() ) {
        if( opts.has_phase ) {
            psc_errmsg( "Uninstrumented program. Phase specification is disabled\n" );
            opts.has_phase = 0;
        }
    }
    else {               //application is instrumented
        if( !opts.has_phase ) {
            psc_errmsg( "Phase region was not provided!\n" );
            exit( 1 );
        }
        /* Since there is no SIR, application class can not be initialized before the first experiment is done */
        appl->set_phase_region( NULL );
        psc_dbgmsg( 1, "Using phase region \"%s\" \n", opts.phase_string );
    }
}

void frontend_main_msm::Initializing_::setup_processes( setup_processes_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    psc_dbgmsg( FRONTEND_MSM_DEBUG_LEVEL,
                "---- FRONTEND MAIN SM ---- SUBMACHINE: Action: -- Setting up processes\n" );
    mpinumprocs_val = 1;             // processes
    if( opts.has_mpinumprocs ) {
        mpinumprocs_val = atoi( opts.mpinumprocs_string );
        if( mpinumprocs_val <= 0 )
            psc_abort("Number of processes set to less than 1!");
    }
    else {
        strcpy( opts.mpinumprocs_string, "1" );
    }
    fe->set_mpinumprocs( mpinumprocs_val );
}

void frontend_main_msm::Initializing_::setup_threads( setup_threads_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    psc_dbgmsg( FRONTEND_MSM_DEBUG_LEVEL,
                "---- FRONTEND MAIN SM ---- SUBMACHINE: Action: -- Setting up threads\n" );
    ompnumthreads_val = 1;             // threads
    if( opts.has_ompnumthreads ) {
        ompnumthreads_val = atoi( opts.ompnumthreads_string );
        if( ompnumthreads_val < 0 ) {
            psc_errmsg( "invalid ompnumthreads %i specified, using 1\n",
                        ompnumthreads_val );
            ompnumthreads_val = 1;
        }
    }
    else {
        strcpy( opts.ompnumthreads_string, "1" );
    }
    fe->set_ompnumthreads( ompnumthreads_val );
}

void frontend_main_msm::Initializing_::setup_timeouts( setup_timeouts_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    psc_dbgmsg( FRONTEND_MSM_DEBUG_LEVEL,
                "---- FRONTEND MAIN SM ---- SUBMACHINE: Action: -- Setting up timeouts\n" );

    timeout = 18000;           // timeouts
//    if ( mpinumprocs_val > 16 )
//    {
//        timeout = 360;
//    }
//    else if ( mpinumprocs_val > 128 )
//    {
//        timeout = 600;
//    }
//    else if ( mpinumprocs_val > 1024 )
//    {
//        timeout = 1200;
//    }

    if( opts.has_timeout ) {
        timeout = atoi( opts.timeout_string );
        if( timeout <= 0 ) {
            psc_errmsg( "Bad value for timeout: %s\n", opts.timeout_string );
            exit( 1 );
        }
    }

    psc_dbgmsg( FRONTEND_HIGH_DEBUG_LEVEL, "Using %d seconds timeout\n", timeout );
    fe->set_global_timeout( timeout );
    fe->set_timeout_delta( timeout );
}

void frontend_main_msm::Initializing_::setup_application_data( setup_application_data_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    psc_dbgmsg( FRONTEND_MSM_DEBUG_LEVEL,
                "---- FRONTEND MAIN SM ---- SUBMACHINE: Action: -- Setting up application data\n" );

    // application
    char app_parameter[ 2000 ];
    if( opts.has_appname ) {
        strcpy( app_name, opts.appname_string );
    }
    else {
        if( getenv( "PSC_APPNAME" ) ) {
            strcpy( app_name, getenv( "PSC_APPNAME" ) );
        }
        else {
            sprintf( app_name, "appl%d",      ( int )getpid() );
        }
    }
    fe->set_appname( app_name );

    // command
    if( opts.has_apprun ) {
        strcpy( app_run_name, opts.app_run_string );
    }
    else {
        *app_run_name = 0;
    }

    if ( opts.has_input_desc ) {
        psc_infomsg( "Reading input description identifiers: %s\n", opts.input_desc_string );
        appl->loadInputIdentifiers( opts.input_desc_string );
    }
}

void frontend_main_msm::Initializing_::setup_agents( setup_agents_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    psc_dbgmsg( FRONTEND_MSM_DEBUG_LEVEL,
                "---- FRONTEND MAIN SM ---- SUBMACHINE: Action: -- Setting up agents\n" );

    psc_dbgmsg( FRONTEND_HIGH_DEBUG_LEVEL, "Using port %d\n", myport );
    if( fe->open( myport ) == -1 ) {
        psc_errmsg( "Error opening frontend on port %d\n", myport );
        exit( 1 );
    }
    else {
        psc_dbgmsg( FRONTEND_GENERAL_DEBUG_LEVEL, "Frontend up and running on %s:%d\n",
                    fe->get_local_hostname().c_str(), fe->get_local_port() );
    }

    // max cluster option
    int maxcluster_val = 4;
    if( opts.has_maxcluster ) {
        maxcluster_val = atoi( opts.maxcluster_string );
        if( maxcluster_val <= 0 ) {
            psc_errmsg( "Bad value for maxcluster: %s\n", opts.maxcluster_string );
            exit( 1 );
        }
    }
    fe->set_maxcluster( maxcluster_val );

    // max fan
    int maxfan = maxfan_default;
    if( opts.has_maxfan ) {
        maxfan = atoi( opts.maxfan_string );
        if( maxfan <= 0 ) {
            psc_errmsg( "Bad value for maxfan: %s\n", opts.maxfan_string );
            exit( 1 );
        }
    }
    fe->set_maxfan( maxfan );
    psc_dbgmsg( FRONTEND_GENERAL_DEBUG_LEVEL, "Using maxfan=%d\n", maxfan );

    // automatic
    if( opts.has_manual ) {
        fe->set_automatic( false );
    }
}

void frontend_main_msm::Initializing_::setup_outputfile( setup_outputfile_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    psc_dbgmsg( FRONTEND_MSM_DEBUG_LEVEL,
                "---- FRONTEND MAIN SM ---- SUBMACHINE: Action: -- Setting up output file\n" );

    // output file
    if( !opts.has_propfile ) {
        if( opts.has_strategy ) {
            strcpy( strategy_name, opts.strategy );
        }
        else {
            strcpy( strategy_name, "default" );
        }
        sprintf( opts.prop_file, "properties_%s_%d.psc", strategy_name, ( int )getpid() );
    }
    fe->set_outfilename( opts.prop_file );
    psc_dbgmsg( FRONTEND_GENERAL_DEBUG_LEVEL, "Using output file: %s\n", opts.prop_file );
}

void frontend_main_msm::Initializing_::connect_to_registry( connect_to_registry_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    if( fe->get_fastmode() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                    "Skipping registry connection (fast hierarchy setup)...\n" );
    }
    else {
        psc_dbgmsg( FRONTEND_MSM_DEBUG_LEVEL, "---- FRONTEND MAIN SM ---- SUBMACHINE: "
                    "Action: -- Contacting registry service\n" );
        psc_dbgmsg( FRONTEND_GENERAL_DEBUG_LEVEL, "Contacting registry service...\n" );
        while( fe->register_self() == -1 && !fe->timed_out() ) {
            ;
        }
        if( fe->timed_out() ) {
            psc_errmsg( "Error registering frontend\n" );
            exit( 1 );
        }
        else {
            psc_dbgmsg( FRONTEND_GENERAL_DEBUG_LEVEL, "Registration successful!\n" );
        }
    }
}

void frontend_main_msm::Initializing_::select_starter( select_starter_event const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    psc_dbgmsg( FRONTEND_MSM_DEBUG_LEVEL, "---- FRONTEND MAIN SM ---- SUBMACHINE: "
                "Action: -- Selecting launcher \n" );

    starter = new ApplicationStarter();
}

void frontend_main_msm::start_experiments( start_run const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    psc_dbgmsg( FRONTEND_MSM_DEBUG_LEVEL,
                "---- FRONTEND MAIN SM ---- Action: Starting experiments\n" );

    int iter_count = 0;
    //initialize the final thread no. and the no.of runs
    for( int i = initial_thread; i <= ompnumthreads_val; i += i ) {
        if( i * i > ompnumthreads_val ) {
            fe->set_ompfinalthreads( i );
            fe->set_maxiterations( iter_count + 1 );
        }
        iter_count++;
    }
    //Start scalability analysis for OpenMP codes
    if( opts.has_scalability_OMP ) {
        for( int i = initial_thread; i <= ompnumthreads_val; i += i ) {
            fe->set_appname( app_name );
            if( ompnumthreads_val >= i ) {
                fe->set_ompnumthreads( i );
            }
            fe->set_global_timeout( timeout );
            fe->set_timeout_delta( timeout );

            //Rerun the frontend for different configurations
            fe->run();
            std::cout << std::endl;
            std::cout << "----------------" << std::endl;
            std::cout << " Ending Periscope run #" << i << "! Search took "
                      << psc_wall_time() << " seconds " << "( ";
            std::cout << fe->get_startup_time() << " seconds for startup "
                      << " )" << std::endl;
            std::cout << "----------------" << std::endl;
        }
        //Print the time taken by the frontend to complete a configurational run
        std::cout << std::endl;
        std::cout << "----------------" << std::endl;
        std::cout << " Scalability experiment finished! Search took "
                  << psc_wall_time() << " seconds " << "( ";
        std::cout << fe->get_startup_time() << " seconds for startup " << " )"
                  << std::endl;
        std::cout << "----------------" << std::endl;

        error_code = EXIT_SUCCESS;
    }
    else {
        fe->run();
        std::cout << std::endl;
        std::cout << "----------------" << std::endl;
        std::cout << " End Periscope run! Search took " << psc_wall_time() << " seconds " << "( ";
        std::cout << fe->get_startup_time() << " seconds for startup " << " )" << std::endl;
        std::cout << "----------------" << std::endl;

        psc_infomsg( "Experiment completed!\n" );
        error_code = EXIT_SUCCESS;
    }
}


void frontend_main_msm::do_finalization( finalize const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    // TODO call each destructor
    psc_dbgmsg( FRONTEND_MSM_DEBUG_LEVEL,
                "---- FRONTEND MAIN SM ---- Action: Doing finalization\n" );

    RegistryService* regSrv = fe->get_registry();
    fe->quit();
    fe->~PeriscopeFrontend();
    if( !fe->get_fastmode() ) {          // there is no registry in fast mode
        psc_dbgmsg( 2,  "Trying to stop the registry...\n" );
        regSrv->stop_registry_server();
    }
}
}
