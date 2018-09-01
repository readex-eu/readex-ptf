#include "frontend_statemachine.h"
#include "frontend_main_statemachine.h"
#include "readex_configuration.h"
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/optional/optional.hpp>
#include <boost/optional.hpp>
#include <list>
using namespace frontend_statemachine;
using boost::property_tree::ptree;


IPlugin* plugin;
bool     search_done;

static bool             just_started = true;
static double           non_instumented_runtime;
static bool             is_instrumented                  = true;
static bool             scenarios_executed               = true;
static StrategyRequest* prepared_strategy_request        = NULL;
static StrategyRequest* analysis_strategy_request        = NULL;
static int              new_properties_index             = 0;
static bool             analysis_per_experiment_required = false;

extern int  application_pid;
extern char user_specified_environment[ 5000 ]; // user-provided environment variables for the starter


void ace_communication_phase( PeriscopeFrontend*             fe,
                              PeriscopeFrontend::TimerAction action,
                              bool                           fast_mode ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ),
                "ace_communication_phase: fast mode: %s; timer action: %d;\n",
                fast_mode ? "true" : "false", ( int )action );

    if( fast_mode ) { // fast timer-less mode
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ),
                    "fast ace_communication_phase\n" );
        fe->handle_step( action );
        search_done = false;
        int error_code;
        while( !search_done ) {
            error_code = fe->get_reactor()->handle_events();
            if( error_code == -1 ) {
                perror( "could not handle events from the reactor...\n" );
                if( errno == EAGAIN ) {
                    PSC_10MS_DELAY;
                }
                else if( errno == EWOULDBLOCK ) {
                    PSC_10MS_DELAY;
                }
                else {
                    throw 0;
                }
            }
            else if( error_code == 0 ) {
                perror( "Remote socket was closed!\n" );
                throw 0;
            }
        }
    }
    else {   // original timer mode
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ),
                    "timer based ace_communication_phase\n" );
        fe->set_timer( 2, 1, fe->timeout_delta(), action );
        fe->get_reactor()->run_reactor_event_loop();
    }
}


void restart_sequence( PeriscopeFrontend* fe,
                       string             location,
                       bool               push_request,
                       bool               reinit ) {
    int                                        status;
    std::map<std::string, AgentInfo>::iterator it;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                "%s: terminating application\n", location.c_str() );
    //Send terminate message to all agents.
    for( it = fe->get_child_agents()->begin(); it != fe->get_child_agents()->end(); it++ ) {
        AgentInfo& ag = it->second;
        if( ag.status != AgentInfo::CONNECTED && fe->connect_to_child( &ag ) == -1 ) {
            psc_errmsg( "Error FE not connected to child\n" );
            throw 0;
        }
        else {
            ag.appl_terminated = false;
            ag.handler->terminate();
        }
    }
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                "%s: waiting for application to terminate\n", location.c_str() );
    fe->get_reactor()->reset_reactor_event_loop();
    fe->get_reactor()->register_handler( 0, fe, ACE_Event_Handler::READ_MASK );
    fe->get_reactor()->register_handler( SIGINT, fe );
    ace_communication_phase( fe, PeriscopeFrontend::APPLICATION_TERMINATION, fe->get_fastmode() );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                "%s: application terminated.\n", location.c_str() );
    waitpid( application_pid, &status, 0 );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                "%s: waited for process id.\n", location.c_str() );
    fe->increment_global_timeout();
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                "%s: Restarting application\n", location.c_str() );

    starter->rerunApplication();
    psc_dbgmsg( 0, "Restart successful\n" );
    fe->set_need_restart( false );
    if( push_request ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "pushing the request in restart_sequence\n" );
        pushStrategyRequest( prepared_strategy_request );
    }
    fe->increment_global_timeout();
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "%s: Waiting for restart of application\n", location.c_str() );
    fe->get_reactor()->reset_reactor_event_loop();
    fe->get_reactor()->register_handler( 0, fe, ACE_Event_Handler::READ_MASK );
    fe->get_reactor()->register_handler( SIGINT, fe );

    // requests a startup or a re-init, depending on the call-site
    if( reinit ) {
        ace_communication_phase( fe, PeriscopeFrontend::STARTUP_REINIT, fe->get_fastmode() );

        if( fe->get_fastmode() ) {
            // extra step required for the fast protocols
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Extra comm. in restart with fast mode...\n" );
            ace_communication_phase( fe, PeriscopeFrontend::STARTUP_REINIT, fe->get_fastmode() );
        }
    }
    else {
        ace_communication_phase( fe, PeriscopeFrontend::STARTUP, fe->get_fastmode() );
    }
}


void agents_then_application_start( PeriscopeFrontend*  fe,
                                    ApplicationStarter* starter ) {
    fe->set_shutdown_allowed( false );
    psc_infomsg( "Starting agents network...\n" );
    starter->runAgents();
    while( !fe->get_agent_hierarchy_started() ) { // in fast mode, agents must arrive before the application is started
        ace_communication_phase( fe, PeriscopeFrontend::STARTUP, fe->get_fastmode() );
    }

    psc_infomsg( "Starting application %s using %s MPI procs and %s OpenMP threads...\n",
                 opts.app_run_string,
                 ( strlen( opts.mpinumprocs_string ) ? opts.mpinumprocs_string : "0" ),
                 ( strlen( opts.ompnumthreads_string ) ? opts.ompnumthreads_string : "0" ) );
    starter->runApplication();
    psc_dbgmsg( 1, "Application started after %5.1f seconds\n", psc_wall_time() );
}


void application_then_agents_start( PeriscopeFrontend*  fe,
                                    ApplicationStarter* starter ) {
    fe->set_shutdown_allowed( false );
    psc_infomsg( "Starting application %s using %s MPI procs and %s OpenMP threads...\n",
                 opts.app_run_string,
                 ( strlen( opts.mpinumprocs_string ) ? opts.mpinumprocs_string : "0" ),
                 ( strlen( opts.ompnumthreads_string ) ? opts.ompnumthreads_string : "0" ) );
    starter->runApplication();
    psc_dbgmsg( 1, "Application started after %5.1f seconds\n", psc_wall_time() );

    psc_infomsg( "Starting agents network...\n" );
    starter->runAgents();
}


namespace frontend_statemachine {
void handle_timers( PeriscopeFrontend* fe ) {
    double                                                 now = psc_wall_time();
    std::map<void ( * )( double ), timer_entry >::iterator iter;
    for( iter = fe->plugin_context->get_timers()->begin(); iter != fe->plugin_context->get_timers()->end(); ++iter ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Handling timer with delay: %f; initial time: %f; (current: %f)\n",
                    iter->second.delay, iter->second.initial_time, now );

        if( now - iter->second.initial_time > iter->second.delay ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Time exceeded; calling the handler at %f seconds.\n", now );
            ( *iter->second.call_back )( now );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Timer removed.\n" );
            fe->plugin_context->get_timers()->erase( iter );
        }
    }
}


void autotune_msm::initialize_plugin_action( initialize_plugin const& evt ) {
    if( !applUninstrumented() ) {
        start_application_and_agent_network( *evt.reactor );
    }

    PSC_SM_TRACE_SET_EVENT_NAME;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Sanity checks...\n" );

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Loading Plugin\n" );
    if( opts.has_plugin ) {
        int    major, minor;      // version information
        string name, description; // version information
        fe->fe_context->loadPlugin( opts.plugin, &major, &minor, &name, &description );
        plugin = fe->fe_context->getTuningPluginInstance( opts.plugin );
        cout << endl << "Loaded Autotune components: " << endl;
        print_loaded_plugin( major, minor, name, description );
    }
    else {
        psc_errmsg( "A plugin must be selected for autotuning.\n" );
        throw 0;
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Initializing plugin\n" );

    // pass all parameters to the plugin in the context
    evt.fe->plugin_context->setApplicationName( evt.fe->get_appname() );
    evt.fe->plugin_context->setApplInstrumented( !applUninstrumented() );
    evt.fe->plugin_context->setOmpnumthreads( evt.fe->get_ompnumthreads() );
    evt.fe->plugin_context->setMPINumProcs( evt.fe->get_mpinumprocs() );
    plugin->initialize( evt.fe->plugin_context.get(), evt.fe->frontend_pool_set.get() );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Plugin Initialized\n" );
}


void autotune_msm::start_tuning_step_action( start_tuning_step const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    handle_timers( fe );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "prepare_search: initPlugin\n" );
    plugin->startTuningStep();
}


void autotune_msm::perform_analysis_action( perform_analysis const& evt ) {
    // TODO need to add code for the case where the application terminated before the strategy is evaluated -IC
    // check the run_phase_experiments_action for reference code
    std::map<std::string, AgentInfo>::iterator it;

    handle_timers( evt.fe );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "call to perform_analysis_action()\n" );
    pushStrategyRequest( evt.strategy );

    if( is_instrumented ) {
        if( just_started ) {  // if we just started, there is no need for a restart
            start_application_and_agent_network( *evt.reactor );
        }
        else {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Agent network is already running. \n" );
            evt.reactor->reset_reactor_event_loop();

            //TODO: solve properly the workaround below
            //
            // The start command commented below was followed by the duplicated start command issued inside the timer handling in the
            // ace_communication_phase call immediately below. The issuing of start command in the timer handling of the ace_communication_phase
            // is appears to be not optimal but is used everywhere. We need a more explicit way of issuing the start command.
            //
            // WORKAROUND BEGIN
            //evt.fe->start();
            // WORKAROUND END

            ace_communication_phase( evt.fe, PeriscopeFrontend::STARTUP, fe->get_fastmode() );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "After start and running event loop.\n" );
        }

        if( evt.fe->get_need_restart() ) {
            // frontend reference, string to prepend the debug messages, push strategy?, re-init?
            restart_sequence( evt.fe, "Perform Analysis", false, true );
        }
        else {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "PERFORM_ANALYSIS_ACTION: No restart was necessary\n" );
            delete evt.strategy;
        }

        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "PERFORM_ANALYSIS_ACTION: inserting results in the analysis results pool\n" );
        for(; new_properties_index < evt.fe->metaproperties_.size(); new_properties_index++ ) {
            evt.fe->frontend_pool_set->arp->pushPreAnalysisProperty( ( evt.fe->metaproperties_[ new_properties_index ] ), evt.tuning_step );
        }
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "PERFORM_ANALYSIS_ACTION: full property list: %d in total\n", evt.fe->metaproperties_.size() );

        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "PERFORM_ANALYSIS_ACTION: analysis done.\n" );
    }
    else {       // non-instrumented binary
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                    "RUN_PHASE_EXPERIMENTS_ACTION: nothing done since binary not instrumented in this step\n" );
    }
}


void autotune_msm::create_scenarios_action( create_scenarios const& evt ) {
    handle_timers( evt.fe );
    PSC_SM_TRACE_SET_EVENT_NAME;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "create_scenarios\n" );
    plugin->createScenarios();

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "create_scenarios_action printing scenarios:\n" );
    if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ) ) > 0 ) {
        evt.fe->frontend_pool_set->csp->print();
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "create_scenarios_action completed\n" );
}


void autotune_msm::prepare_scenarios_action( prepare_scenarios const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    handle_timers( fe );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "prepare_scenarios\n" );
    plugin->prepareScenarios();
}


void autotune_msm::define_experiment_action( define_experiment const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    handle_timers( fe );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "define_experiment\n" );
    plugin->defineExperiment( atoi( opts.mpinumprocs_string ), analysis_per_experiment_required, &analysis_strategy_request );
}


void autotune_msm::consider_restart_action( consider_restart const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    handle_timers( evt.fe );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "call to check_restart_action()\n" );

    string env      = "";
    int    numprocs = evt.fe->get_mpinumprocs();
    string command  = string( opts.app_run_string );
    is_instrumented = true;
    std::map<std::string, AgentInfo>::iterator it;
    int                                        previous_process_count = evt.fe->get_mpinumprocs();

    bool restart_required = plugin->restartRequired( env, numprocs, command, is_instrumented );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Checking if a restart is necessary: %s\n", restart_required  ?  "yes" : "no" );

    if( numprocs != previous_process_count && numprocs > 0 ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                    "Changing mpinumprocs from %d to %d\n", previous_process_count, numprocs );
        evt.fe->set_mpinumprocs( numprocs );
    }
    else if( numprocs <= 0 ) {
        psc_errmsg( "Zero or negative process count specified by the plugin. Aborting...\n" );
        throw 0;
    }

    if( strcmp( command.c_str(), opts.app_run_string ) != 0 ) { // new command received
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                    "Changing command from %s to %s\n", opts.app_run_string, command.c_str() );
        if( command == "" ) {
            psc_errmsg( "Empty string received as command from the plugin. Aborting...\n" );
            throw 0;
        }

        strcpy( opts.app_run_string, command.c_str() );
    }

    if( strcmp( env.c_str(), user_specified_environment ) != 0 ) { // new environment definition received
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                    "Changing environment from %s to %s\n", user_specified_environment, env.c_str() );
        strcpy( user_specified_environment, env.c_str() );
    }

    if( is_instrumented ) {
        if( just_started ) {  // if we just started, there is no need for a restart
            start_application_and_agent_network( *evt.reactor );
        }
        else if( restart_required ) {    // a restart was requested with an already running application and hierarchy
            psc_infomsg( "Restarting application...\n" );
            if( numprocs != previous_process_count ) {
                psc_infomsg( "The number of processes changed. Rebuilding agent hierarchy along the restart...\n" );
                evt.fe->terminate_agent_hierarchy();
                just_started = true;
                start_application_and_agent_network( *evt.reactor );
            }
            else {       // a restart with the same number of processes was requested
                psc_infomsg( "The number of processes has not changed.\n" );
                evt.fe->set_need_restart( true );

                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "RESTART_ACTION: restarting application only.\n" );
                evt.reactor->register_handler( 0, evt.fe, ACE_Event_Handler::READ_MASK );
                evt.reactor->register_handler( SIGINT, evt.fe );
                if( !fe->get_fastmode() ) {  // this step is not necessary in fast mode
                    ace_communication_phase( evt.fe, PeriscopeFrontend::AGENT_STARTUP, fe->get_fastmode() );
                }

                restart_sequence( evt.fe, "Restart", false, false );
            }
        }
        else {     // no restart required
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "RESTART_ACTION: No restart required.\n" );
        }
        evt.reactor->reset_reactor_event_loop();
        evt.reactor->register_handler( 0, evt.fe, ACE_Event_Handler::READ_MASK );
        evt.reactor->register_handler( SIGINT, evt.fe );
    }
    else {     // not instrumented
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "RESTART_ACTION: non-instrumented run...\n" );

        char numprocs_string[ 2000 ];
        sprintf( numprocs_string, "%d", numprocs );
        psc_infomsg( "Starting non-instrumented application %s using %s MPI procs and %s OpenMP threads...\n",
                     opts.app_run_string,
                     numprocs_string,
                     ( strlen( opts.ompnumthreads_string ) ? opts.ompnumthreads_string : "0" ) );
        double start_time = psc_wall_time();
        evt.starter->runApplication();
        non_instumented_runtime = psc_wall_time() - start_time;
        MetaProperty mp;
        mp.setSeverity( non_instumented_runtime );
        Scenario* scenario = evt.fe->frontend_pool_set->esp->pop();
        mp.addExtraInfo( string( "ScenarioID" ),
                         string( boost::lexical_cast<std::string>( scenario->getID() ) ) );
        evt.fe->frontend_pool_set->fsp->push( scenario );
        evt.fe->frontend_pool_set->srp->push( mp, evt.search_step );
    }
}


void autotune_msm::run_phase_experiments_action( run_phase_experiments const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
     handle_timers( evt.fe );
     psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "running phase experiments...\n" );

     std::map<std::string, AgentInfo>::iterator it;
     if( is_instrumented ) {
         // only prepare a request if the scenarios are not executed
         // this can be due to the application terminating before we have explored the space -IC
         if( scenarios_executed ) {
             prepared_strategy_request = createStrategyRequest( evt.fe );
             if( analysis_per_experiment_required && analysis_strategy_request ) {
                 psc_dbgmsg( 3, "Frontend, Configuration type: %d\n", analysis_strategy_request->getTypeOfConfiguration() );
                 psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "per experiment analysis requested...\n" );
                 prepared_strategy_request->setSubStrategyRequest( analysis_strategy_request );
             }
             pushStrategyRequest( prepared_strategy_request );
         }

         evt.reactor->reset_reactor_event_loop();
         evt.reactor->register_handler( 0, evt.fe, ACE_Event_Handler::READ_MASK );
         evt.reactor->register_handler( SIGINT, evt.fe );

         ace_communication_phase( evt.fe, PeriscopeFrontend::STARTUP, fe->get_fastmode() );

         if( evt.fe->get_need_restart() ) {
             psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Restarting the application...\n" );
             scenarios_executed = *evt.executed = false;     // the scenarios are not executed immediately in case of restart

             restart_sequence( evt.fe, "Run Phase Experiments", true, false );
         }
         else {
             psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "No restart was necessary in run_phase_experiments_action...\n" );
             scenarios_executed = *evt.executed = true;     // the scenarios are executed if there is no restart
             delete prepared_strategy_request;
         }

         for(; new_properties_index < evt.fe->metaproperties_.size(); ++new_properties_index ) {
             MetaProperty& property = evt.fe->metaproperties_[ new_properties_index ];
             //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
             //            "Inserting new property to result pool: %s (%f)\n",
             //            property.getName().c_str(), property.getSeverity() );

             if( property.getPurpose() == PSC_PROPERTY_PURPOSE_TUNING ) {
                 evt.fe->frontend_pool_set->srp->push( property, evt.search_step );
             }
             else {
                 evt.fe->frontend_pool_set->arp->pushExperimentProperty( property, evt.experiment_count );
             }
         }


         if(withRtsSupport()) {
             evt.reactor->reset_reactor_event_loop();
             evt.reactor->register_handler( 0, evt.fe, ACE_Event_Handler::READ_MASK );
             evt.reactor->register_handler( SIGINT, evt.fe );
             psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( CallTree ), "Get call-tree\n" );
             ace_communication_phase( evt.fe, PeriscopeFrontend::REQUESTCALLTREE, fe->get_fastmode() );

             // Set the isValidRts flag for all leaf nodes
             appl->getCalltreeRoot()->setValidRtss(appl->getCalltreeRoot());

             //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Printing rts tree in frontend \n" );
             //rtstree::printTree( appl->getCalltreeRoot() );

             psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( CallTree ), "Get call-tree done\n" );


             /*Modifying significant regions dynamically. To make sure we get the objective values for significant regions even if
              * it was not encountered in the Importance strategy
              */
             if( opts.has_configurationfile ) {
                 std::list<std::string> names;

                 // extract the significant regions from configuration file provided by readex-dyn-detect tool
                 BOOST_FOREACH(ptree::value_type &v, configTree.get_child( "Configuration.readex-dyn-detect.Intra-phase" ) ) {
                     std::string name = v.second.get<std::string>("name");
                     //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "Significant region name: %s \n", name.c_str() );
                     names.push_back( name );
                 }

                 appl->markSignificantRegions( names );

             }
         }
     }
     else {       // non-instrumented binary
         psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ),
                     "RUN_PHASE_EXPERIMENTS_ACTION: nothing done since binary not instrumented in this step\n" );
     }
 }


void autotune_msm::finish_tuning_step_action( finish_tuning_step const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    handle_timers( fe );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "finish_tuning_step\n" );
    plugin->finishTuningStep();
}


void autotune_msm::create_tuning_advice_action( create_tuning_advice const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "create_tuning_advice\n" );
    Advice* advice = plugin->getAdvice();

    if( advice != NULL ) {
        try {
            using boost::property_tree::ptree;

            ptree adviceResultData = advice->getXMLOutputData();
            BOOST_FOREACH( const ptree::value_type &plugin_info, adviceResultData.get_child("Advices") ) {

                const std::string pl_name   = plugin_info.second.get<std::string>( "PluginName" );

                //Extracting phase specific scenario information
                const ptree scenario_result = plugin_info.second.get_child( "BestScenarios.ScenarioResult.Scenario.Region" );

                //Extract region information
                const std::string region_id = scenario_result.get<std::string>( "RegionID" );
                Region*           reg       = appl->getRegionByID( region_id );

                //Extract objective value
                double tuning_obj_value     = 0;
                if( plugin_info.second.get_child_optional( "BestScenarios.ScenarioResult.Result" ) ) {
                    tuning_obj_value        = plugin_info.second.get_child( "BestScenarios.ScenarioResult.Result" ).get<double>( "Value" );
                }

                const ptree tuning_info     = plugin_info.second.get_child( "BestScenarios.ScenarioResult.Scenario.TuningSpecification.Variant" );

                std::map<std::string, int> tuning_param_value_mapping;

                //Extract tuning parameter values
                BOOST_FOREACH( const ptree::value_type &tuning_param_info, tuning_info ) {
                    const std::string param_name  = tuning_param_info.second.get<std::string>( "Name" );
                    const int         param_value = tuning_param_info.second.get<int>( "Value" );
                    tuning_param_value_mapping.insert( std::pair<std::string, int>( param_name, param_value ) );
                }


//                if(withRtsSupport()) {
//                    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Inserting tuning results to rts node \n" );
//                    appl->getCalltreeRoot()->insertPluginResult( pl_name, tuning_obj_value, tuning_param_value_mapping, reg );
//
//                    if( plugin_info.second.get_child_optional( "BestScenarios.BestSignificantRegionScenarios" ) ) {
//                        //Extracting significant region specific information
//                        const ptree region_scenario_info = plugin_info.second.get_child( "BestScenarios.BestSignificantRegionScenarios" );
//                        BOOST_FOREACH( const ptree::value_type &reg_scen_info, region_scenario_info ) {
//
//                            //Extract region information
//                            const std::string region_id  = reg_scen_info.second.get_child( "Region" ).get<std::string>( "RegionID" );
//                            Region* reg = appl->getRegionByID( region_id );
//
//                            const ptree tuning_info      = reg_scen_info.second.get_child( "TuningParameters" );
//                            std::map<std::string, int> tuning_param_value_mapping;
//
//                            //Extract tuning parameter values
//                            BOOST_FOREACH( const ptree::value_type &tuning_param_info, tuning_info ) {
//                                const std::string param_name  = tuning_param_info.second.get<std::string>( "Name" );
//                                const int         param_value = tuning_param_info.second.get<int>( "Value" );
//                                tuning_param_value_mapping.insert( std::pair<std::string, int>( param_name, param_value ) );
//                            }
//
//                            double tuning_obj_value = 0;
//
//                            //Extract objective value
//                            tuning_obj_value = reg_scen_info.second.get_child( "Result" ).get<double>( "Value" );
//
//                            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Inserting tuning results to rts node \n" );
//                            //appl->getCalltreeRoot()->insertPluginResult( pl_name, tuning_obj_value, tuning_param_value_mapping, reg );
//                        }
//                    }
//                    appl->getCalltreeRoot()->printTree( appl->getCalltreeRoot() );
//                }
            }
        }

        catch( std::exception& ex ) {
            psc_errmsg( "Unable to parse the advice property tree: %s\n", ex.what() );
            abort();
        }
    }

    if( advice != NULL ) {
        char filename[ 1000 ];
        sprintf( filename, "advice_%d.xml", ( int )getpid() );
        advice->toXML( filename );
        psc_infomsg( "Plugin advice stored in: %s\n", filename );
    }
}


void autotune_msm::finalize_autotuning_action( finalize_autotuning const& evt ) {
    PSC_SM_TRACE_SET_EVENT_NAME;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "finalize_autotuning\n" );
    // delete all shared queues and plugin (matching initialization)
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Calling finalize in the plugin...\n" );
    plugin->finalize();
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Deleting all pools...\n" );
}


void autotune_msm::start_application_and_agent_network( ACE_Reactor& reactor ) {
    // check some pre-conditions
    handle_timers( fe );

    if( !appl ) {
        psc_errmsg( "Application cannot be null when starting up!\n" );
        abort();
    }

    if( !just_started ) {
        psc_errmsg( "Application is already started, some state logic is broken!\n" );
        abort();
    }


    // startup the application the agent network
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Starting application and agent network...\n" );
    if( fe->get_fastmode() ) {
        agents_then_application_start( fe, starter );
    }
    else {
        application_then_agents_start( fe, starter );
    }

    just_started = false;
    reactor.reset_reactor_event_loop();
    reactor.register_handler( 0, fe, ACE_Event_Handler::READ_MASK );
    reactor.register_handler( SIGINT, fe );
    if( fe->get_fastmode() && fe->get_agent_hierarchy_started() ) {
        fe->start();
    }

    if( !is_instrumented ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Application is not instrumented, no region information will be gathered.\n" );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Startup completed.\n" );
        return;
    }


    // gather all regions by performing an analysis with the importance strategy
    std::unique_ptr<StrategyRequestGeneralInfo> info( new StrategyRequestGeneralInfo() );
    info->strategy_name     = "Importance";
    info->pedantic          = 0;
    if( opts.has_delay ) {
        //common_command << " --delay=" << opts.delay_string;
        info->delay_phases      = atoi(opts.delay_string);
    } else {
   	 info->delay_phases=0;
    }
    info->delay_seconds     = 0;
    info->analysis_duration = 1;
    std::unique_ptr<StrategyRequest> importance( new StrategyRequest( info.release() ) );
    pushStrategyRequest( importance.get() );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Gather regions: entering communication phase\n" );
    ace_communication_phase( fe, PeriscopeFrontend::STARTUP, fe->get_fastmode() );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Gather regions: communication phase done\n" );


    // restart the application as necessary
    if( fe->get_need_restart() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Gather regions: application restart is required after gathering\n" );
        restart_sequence( fe, "Gather regions", false, true );
    }


    // extract found regions from the properties and add them to the application
    BOOST_FOREACH( MetaProperty & prop, fe->metaproperties_ ) {
        int start = prop.getStartPosition();
        appl->addRegion( prop.getRegionName(), prop.getRFL(), prop.getFileName(), prop.getRegionType(), start, start );
    }

    if(withRtsSupport()) {
        std::list<std::string> names;
        std::list<std::string> objectiveFunctionList;
        std::vector<ObjectiveFunction*> objectives;
        if( opts.has_configurationfile ) {
            // extract the significant regions from configuration file provided by readex-dyn-detect tool
            BOOST_FOREACH(ptree::value_type &v, configTree.get_child( "Configuration.readex-dyn-detect.Intra-phase" ) ) {
                std::string name = v.second.get<std::string>("name");
                names.push_back( name );
            }

            appl->markSignificantRegions( names );

            std::string timeUnit;
            std::string energyUnit;
            std::string currencyUnit;

            //Read objectives
            psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: Obtaining objectives.\n");
            std::list < std::string > objectiveFunctionList;


            try {
                if (opts.has_configurationfile) {
                    // extract the significant regions from configuration file provided by readex-dyn-detect tool
                    BOOST_FOREACH(ptree::value_type & v, configTree.get_child("Configuration.objectives"))
                    {
                        std::string name = v.second.data();
                        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "Objective Function: %s \n", name.c_str());
                        objectiveFunctionList.push_back(name);
                    }
                    timeUnit = configTree.get < std::string > ("Configuration.periscope.metricUnits.timeUnit");
                    energyUnit = configTree.get < std::string > ("Configuration.periscope.metricUnits.eneryUnit");
                    currencyUnit = configTree.get < std::string > ("Configuration.periscope.metricUnits.currencyUnit");
                }

            } catch (exception &e) {
                psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: %s\n", e.what());
            }

            if(objectiveFunctionList.empty()) {
                objectives.push_back(new EnergyObjective(energyUnit));
            }

            std::string s = objectiveFunctionList.front();
            if (s == "Energy") {
                std::string unit = energyUnit;
                objectives.push_back(new EnergyObjective(energyUnit));
              } else if (s == "NormalizedEnergy") {
                  std::string unit = energyUnit.append("/instr");
                  objectives.push_back(new NormalizedEnergyObjective(unit));
              } else if (s == "CPUEnergy") {
                  std::string unit = energyUnit;
                  objectives.push_back(new CPUEnergyObjective(unit));
              } else if (s == "NormalizedCPUEnergy") {
                  std::string unit = energyUnit.append("/instr");
                  objectives.push_back(new NormalizedCPUEnergyObjective(unit));
              } else if (s == "EDP") {
                  std::string unit = energyUnit.append("*").append(timeUnit);
                  objectives.push_back(new EDPObjective(unit));
              } else if (s == "NormalizedEDP") {
                  std::string unit = energyUnit.append("*").append(timeUnit).append("/instr");
                  objectives.push_back(new NormalizedEDPObjective(unit));
              } else if (s == "ED2P") {
                  std::string unit = energyUnit.append("*").append(timeUnit).append("2");
                  objectives.push_back(new ED2PObjective(unit));
              } else if (s == "NormalizedED2P") {
                  std::string unit = energyUnit.append("*").append(timeUnit).append("2").append("/instr");
                  objectives.push_back(new NormalizedED2PObjective(unit));
              } else if (s == "Time") {
                  std::string unit = timeUnit;
                  objectives.push_back(new TimeObjective(unit));
              } else if (s == "NormalizedTime") {
                  std::string unit = timeUnit.append("/instr");
                  objectives.push_back(new NormalizedTimeObjective(unit));
              } else if (s == "TCO") {
                  std::string unit = currencyUnit;
                  objectives.push_back(new TCOObjective(unit));
              } else if (s == "NormalizedTCO") {
                  std::string unit = currencyUnit.append("/instr");
                  objectives.push_back(new NormalizedTCOObjective(unit));
              }
              else
                  psc_errmsg("Unknown objective %s\n", s.c_str());
          }


        reactor.reset_reactor_event_loop();
        reactor.register_handler( 0, fe, ACE_Event_Handler::READ_MASK );
        reactor.register_handler( SIGINT, fe );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Get call-tree: entering communication phase\n" );
        ace_communication_phase( fe, PeriscopeFrontend::REQUESTCALLTREE, fe->get_fastmode() );

        // Set the isValidRts flag for all leaf nodes
        appl->getCalltreeRoot()->setValidRtss(appl->getCalltreeRoot());

        if( opts.has_configurationfile ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Inserting the default energy values for rts's\n" );

            std::string s = objectives.front()->getName();
            std::list<Region*> code_significant_regions = appl->get_sig_regions_list();
            double obj_val(0.0);
            for (auto sig_region : code_significant_regions) {
                std::list<Rts*> rtsList = appl->getCalltreeRoot()->getValidRtsOfRegion(sig_region, NULL);
                for (auto rts : rtsList) {
                    std::list<MetaProperty> rts_properties;
                    for( MetaProperty & prop : fe->metaproperties_ ) {
                        if( prop.getCallpath() == rts->getCallPath() ) {
                            rts_properties.push_back(prop);
                        }
                    }
                    //Insert DEFAULT objective values for current rts
                    obj_val = objectives[0]->objective(rts_properties);
                    appl->getCalltreeRoot()->getRtsByCallpath(rts->getCallPath())->insertDefaultObjValue(s,obj_val);
                }
            }

            std::list<MetaProperty> phase_properties;
            for( MetaProperty & prop : fe->metaproperties_ ) {
                if( prop.getCallpath() == appl->getCalltreeRoot()->getCallPath() ) {
                    phase_properties.push_back(prop);
                }
            }
            //Insert DEFAULT objective values for current rts
            obj_val = objectives[0]->objective(phase_properties);
            appl->getCalltreeRoot()->insertDefaultObjValue(s,obj_val);
        }

//        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Printing rts tree in frontend \n" );
//        rtstree::printTree( appl->getCalltreeRoot() );

        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Get call-tree: communication phase done\n" );
    }
    fe->metaproperties_.clear();
    new_properties_index = 0;
}


StrategyRequest* createStrategyRequest( PeriscopeFrontend* fe ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Preparing strategy request...\n" );
    Scenario*             scenario;
    std::list<Scenario*>* scenariosList = new std::list<Scenario*>;

    while( !fe->frontend_pool_set->esp->empty() ) {
        scenario = fe->frontend_pool_set->esp->pop();
        // Pushes scenarios from the experiment scenario pool to a list which is serialized.
        scenariosList->push_back( scenario );
        // Pushes scenarios that are sent to AA from the experiment scenario pool to the finished scenarios pool.
        fe->frontend_pool_set->fsp->push( scenario );
    }

    StrategyRequestGeneralInfo* strategyRequestGeneralInfo = new StrategyRequestGeneralInfo;
    strategyRequestGeneralInfo->strategy_name     = "Autotune";
    strategyRequestGeneralInfo->pedantic          = false;
    strategyRequestGeneralInfo->delay_phases      = 0;
    strategyRequestGeneralInfo->delay_seconds     = 0;
    strategyRequestGeneralInfo->analysis_duration = 1;

    StrategyRequest* strategy_request = new StrategyRequest( scenariosList, strategyRequestGeneralInfo );
    return strategy_request;
}


void pushStrategyRequest( StrategyRequest* strategy_request ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Pushing strategy request...\n" );
    if( !strategy_request ) {
        throw std::invalid_argument( "The strategy request cannot be a nullptr." );
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( FrontendStateMachines ), "Strategy requests in the frontend:\n" );
    if( psc_get_debug_level() >= 4 ) {
        strategy_request->printStrategyRequest();
    }
    fe->serializeStrategyRequests( strategy_request );
    if( fe->get_fastmode() && fe->get_agent_hierarchy_started() ) {
        fe->start();
    }
}
}
