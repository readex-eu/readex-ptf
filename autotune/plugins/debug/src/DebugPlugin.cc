/**
   @file    DebugPlugin.cc
   @ingroup DebugPlugin
   @brief   This is a meta-plugin that provides debug information about a selected plugin.
   @author  Isaias Compres
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include "DebugPlugin.h"

// TODO remove this and have a variable that is user defined (a runtime option such as --separator="something")
#define DEBUG_SEPARATOR_STRING "\n--------------------------------------------------------------------------------\n"
#define DEBUG_PLUGIN_OUTPUT "Debugger: "

/**
 * @brief Initialize the plugin's data structures.
 *
 * @ingroup DebugPlugin
 *
 */
void DebugPlugin::initialize( DriverContext*   context,
                              ScenarioPoolSet* pool_set ) {
    psc_infomsg( DEBUG_SEPARATOR_STRING DEBUG_PLUGIN_OUTPUT "Debugging %s::initialize()...\n\n", subplugin_name.c_str() );

    this->context  = context;
    this->pool_set = pool_set;

    bool plugin_selected = false;

    const struct option long_opts[] = {
        { "plugin", required_argument, 0, 'p' },
    };

    int    major, minor;      // version information
    string name, description; // version information

    optind = 1;
    while( optind < context->getArgc() ) {
        int            index = -1;
        struct option* opt   = 0;

        opterr = 0;         // options for subplugin not parsed; suppress errors
        int result = getopt_long( context->getArgc(), context->getArgv(), "", long_opts, &index );
        if( result == -1 ) {
            psc_errmsg( "Error parsing command line parameters in the debug plugin.\n" );
            throw 0;
        }

        opt = ( struct option* )&( long_opts[ index ] );

        switch( result ) {
        case 'p':
            psc_infomsg( DEBUG_PLUGIN_OUTPUT "Plugin to be debugged: \"%s\".\n", optarg );
            subplugin_name  = string( optarg );
            plugin_selected = true;
            break;
        case '?':
            psc_infomsg( DEBUG_PLUGIN_OUTPUT "Non debug plugin parameter \"%s\" will be passed to the Autotune plugin to be debugged.\n",
                         context->getArgv()[ optind - 1 ] );
            subplugin_parameters.push_back( string( context->getArgv()[ optind - 1 ] ) );
            break;
        default:
            throw 0;
        }
    }

    if( !plugin_selected ) {
        psc_errmsg( "no plugin was selected to be loaded by the Debug plugin\n" );
        throw 0;
    }

    subplugin_context = new DriverContext;
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "Generating a new DriverContext for the child plugin...\n" );
    generate_context_argc_argv( subplugin_context, subplugin_parameters, ( char* )subplugin_name.c_str() );
    subplugin_context->setApplInstrumented( context->applInstrumented() );
    subplugin_context->setOmpnumthreads( context->getOmpnumthreads() );
    context->loadPlugin( subplugin_name, &major, &minor, &name, &description );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "Debugging the following Autotune plugin:\n" );
    print_loaded_plugin( major, minor, name, description );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "Attempting to load Autotune plugin: %s\n", subplugin_name.c_str() );
    subplugin = context->getTuningPluginInstance( subplugin_name );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "DriverContext to be passed to the child plugin:\n %s\n", subplugin_context->toString( 0, "\t" ).c_str() );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "The pool set from the frontend will be passed directly.\n" );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "Attempting to initialize Autotune plugin: %s\n", subplugin_name.c_str() );
    subplugin->initialize( subplugin_context, pool_set );

    psc_infomsg( DEBUG_PLUGIN_OUTPUT "Returning from %s::initialize()..." DEBUG_SEPARATOR_STRING, subplugin_name.c_str() );
}

/**
 * @brief Defines a pre-analysis strategy.
 *
 * @ingroup DebugPlugin
 *
 */
bool DebugPlugin::analysisRequired( StrategyRequest** strategy ) {
    psc_infomsg( DEBUG_SEPARATOR_STRING DEBUG_PLUGIN_OUTPUT "Debugging %s::analysisRequired()...\n\n", subplugin_name.c_str() );

    bool return_bool;
    if( *strategy != NULL ) {
        psc_infomsg( DEBUG_PLUGIN_OUTPUT "WARNING: The StrategyRequest pointer was NOT initialized to NULL by the frontend\n." );
    }

    psc_infomsg( DEBUG_PLUGIN_OUTPUT "Calling %s::analysisRequired()...\n", subplugin_name.c_str() );
    return_bool = subplugin->analysisRequired( strategy );

    if( return_bool ) {
        psc_infomsg( DEBUG_PLUGIN_OUTPUT "The plugin requests an analysis.\n" );
        if( *strategy == NULL ) {
            psc_errmsg( "ERROR: the plugin returned a NULL pointer instead of a StrategyRequest reference!\n" );
            throw 0;
        }
        // TODO print the StrategyRequest here -IC
    }
    else {
        psc_infomsg( DEBUG_PLUGIN_OUTPUT "The plugin did not request an analysis for this tuning step.\n" );
    }

    psc_infomsg( DEBUG_PLUGIN_OUTPUT "Returning from %s::analysisRequired()..." DEBUG_SEPARATOR_STRING, subplugin_name.c_str() );
    return return_bool;
}

/**
 * @brief Operations to be done at the start of a tuning step.
 *
 * @ingroup DebugPlugin
 *
 */
void DebugPlugin::startTuningStep( void ) {
    psc_infomsg( DEBUG_SEPARATOR_STRING DEBUG_PLUGIN_OUTPUT "Debugging %s::startTuningStep()...\n\n", subplugin_name.c_str() );

    subplugin->startTuningStep();

    psc_infomsg( DEBUG_PLUGIN_OUTPUT "Returning from %s::startTuningStep()..." DEBUG_SEPARATOR_STRING, subplugin_name.c_str() );
}

/**
 * @brief The Created Scenario Pool (csp) is populated here.
 *
 * @ingroup DebugPlugin
 *
 */
void DebugPlugin::createScenarios( void ) {
    psc_infomsg( DEBUG_SEPARATOR_STRING DEBUG_PLUGIN_OUTPUT "Debugging %s::createScenarios()...\n\n", subplugin_name.c_str() );

    psc_infomsg( DEBUG_PLUGIN_OUTPUT "ARP after experiments:\n%s\n", pool_set->arp->toString( 0, "\t" ).c_str() );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "CSP before createScenarios():\n%s\n", pool_set->csp->toString( 0, "\t" ).c_str() );
    subplugin->createScenarios();
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "CSP after createScenarios():\n%s\n", pool_set->csp->toString( 0, "\t" ).c_str() );

    psc_infomsg( DEBUG_PLUGIN_OUTPUT "Returning from %s::createScenarios()..." DEBUG_SEPARATOR_STRING, subplugin_name.c_str() );
}

/**
 * @brief Preparatory steps for the scenarios are done here.
 *
 * @ingroup DebugPlugin
 *
 */
void DebugPlugin::prepareScenarios( void ) {
    psc_infomsg( DEBUG_SEPARATOR_STRING DEBUG_PLUGIN_OUTPUT "Debugging %s::prepareScenarios()...\n\n", subplugin_name.c_str() );

    psc_infomsg( DEBUG_PLUGIN_OUTPUT "PSP before prepareScenarios():\n%s\n", pool_set->psp->toString( 0, "\t" ).c_str() );
    subplugin->prepareScenarios();
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "CSP after prepareScenarios():\n%s\n", pool_set->csp->toString( 0, "\t" ).c_str() );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "PSP after prepareScenarios():\n%s\n", pool_set->psp->toString( 0, "\t" ).c_str() );

    psc_infomsg( DEBUG_PLUGIN_OUTPUT "Returning from %s::prepareScenarios()..." DEBUG_SEPARATOR_STRING, subplugin_name.c_str() );
}


/**
 * @brief Populate the Experiment Scenario Pool (esp) for the next experiment.
 *
 * @ingroup DebugPlugin
 *
 * @param numprocs         Number of processes required for the experiment
 * @param analysisRequired Defines is per-experiment analysis required
 * @param strategy         Strategy request for per-experiment analysis
 *
 */
void DebugPlugin::defineExperiment( int               numprocs,
                                    bool&             analysisRequired,
                                    StrategyRequest** strategy ) {
    psc_infomsg( DEBUG_SEPARATOR_STRING DEBUG_PLUGIN_OUTPUT "Debugging %s::defineExperiment()...\n\n", subplugin_name.c_str() );

    psc_infomsg( DEBUG_PLUGIN_OUTPUT "ESP before defineExperiment():\n%s\n", pool_set->esp->toString( 0, "\t" ).c_str() );
    subplugin->defineExperiment( numprocs, analysisRequired, strategy );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "PSP after defineExperiment():\n%s\n", pool_set->psp->toString( 0, "\t" ).c_str() );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "ESP after defineExperiment():\n%s\n", pool_set->esp->toString( 0, "\t" ).c_str() );

    psc_infomsg( DEBUG_PLUGIN_OUTPUT "Returning from %s::defineExperiment()..." DEBUG_SEPARATOR_STRING, subplugin_name.c_str() );
}

/**
 * @brief Returns does the application has to be restarted for the next experiment.
 *
 * @ingroup DebugPlugin
 *
 * @return Does the application has to be restarted for the next experiment.
 */
bool DebugPlugin::restartRequired( std::string& env,
                                   int&         numprocs,
                                   std::string& command,
                                   bool&        is_instrumented ) {
    psc_infomsg( DEBUG_SEPARATOR_STRING DEBUG_PLUGIN_OUTPUT "Debugging %s::restartRequired()...\n\n", subplugin_name.c_str() );

    bool return_bool;
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "Parameters were initialized by the frontend as follows:\n" );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "env: \"%s\"\n", env.c_str() );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "numprocs: \"%d\"\n", numprocs );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "command: \"%s\"\n", command.c_str() );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "is_instrumented: \"%s\"\n", is_instrumented ? "true" : "false" );
    return_bool = subplugin->restartRequired( env, numprocs, command, is_instrumented );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "After calling the plugin (%s), the parameters are as follows:\n", subplugin_name.c_str() );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "env: \"%s\"\n", env.c_str() );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "numprocs: \"%d\"\n", numprocs );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "command: \"%s\"\n", command.c_str() );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "is_instrumented: \"%s\"\n", is_instrumented ? "true" : "false" );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "The call (%s)::restartRequired() returned: %s\n", subplugin_name.c_str(), return_bool ? "true" : "false" );

    psc_infomsg( DEBUG_PLUGIN_OUTPUT "Returning from %s::restartRequired()..." DEBUG_SEPARATOR_STRING, subplugin_name.c_str() );
    return return_bool;
}

/*
 * @brief Returns the status of the current search iteration.
 *
 * @ingroup DebugPlugin
 *
 * @return The status of the current search iteration.
 */
bool DebugPlugin::searchFinished( void ) {
    psc_infomsg( DEBUG_SEPARATOR_STRING DEBUG_PLUGIN_OUTPUT "Debugging %s::searchFinished()...\n\n", subplugin_name.c_str() );

    psc_infomsg( DEBUG_PLUGIN_OUTPUT "SRP after experiments:\n%s\n", pool_set->srp->toString( 0, "\t" ).c_str() );
    bool return_bool;
    return_bool = subplugin->searchFinished();
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "The call (%s)::searchFinished() returned: %s\n", subplugin_name.c_str(), return_bool ? "true" : "false" );

    psc_infomsg( DEBUG_PLUGIN_OUTPUT "Returning from %s::searchFinished()..." DEBUG_SEPARATOR_STRING, subplugin_name.c_str() );
    return return_bool;
}

/**
 * @brief Final operation of a tuning step.
 *
 * @ingroup DebugPlugin
 *
 */
void DebugPlugin::finishTuningStep( void ) {
    psc_infomsg( DEBUG_SEPARATOR_STRING DEBUG_PLUGIN_OUTPUT "Debugging %s::finishTuningStep()...\n\n", subplugin_name.c_str() );
    subplugin->finishTuningStep();
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "Returning from %s::finishTuningStep()..." DEBUG_SEPARATOR_STRING, subplugin_name.c_str() );
}

/**
 * @brief Returns true if the plugin finished the tuning process, false otherwise.
 *
 * @ingroup DebugPlugin
 *
 * @return True if the plugin has finished the tuning process, false otherwise.
 */
bool DebugPlugin::tuningFinished( void ) {
    psc_infomsg( DEBUG_SEPARATOR_STRING DEBUG_PLUGIN_OUTPUT "Debugging %s::tuningFinished()...\n\n", subplugin_name.c_str() );
    return subplugin->tuningFinished();
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "Returning from %s::tuningFinished()..." DEBUG_SEPARATOR_STRING, subplugin_name.c_str() );
}

/**
 * @brief Prints to the screen (and to a file, where necessary) the tuning advice.
 *
 * @ingroup DebugPlugin
 */
Advice* DebugPlugin::getAdvice( void ) {
    psc_infomsg( DEBUG_SEPARATOR_STRING DEBUG_PLUGIN_OUTPUT "Debugging %s::getAdvice()...\n\n", subplugin_name.c_str() );
    return subplugin->getAdvice();
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "Returning from %s::getAdvice()..." DEBUG_SEPARATOR_STRING, subplugin_name.c_str() );
}

/**
 * @brief Finalize the plugin normally.
 *
 * @ingroup DebugPlugin
 *
 */
void DebugPlugin::finalize() {
    psc_infomsg( DEBUG_SEPARATOR_STRING DEBUG_PLUGIN_OUTPUT "Debugging %s::finalize()...\n\n", subplugin_name.c_str() );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "State of all pools at finalize:\n" );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "CSP:\n%s\n", pool_set->csp->toString( 0, "\t" ).c_str() );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "PSP:\n%s\n", pool_set->psp->toString( 0, "\t" ).c_str() );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "ESP:\n%s\n", pool_set->esp->toString( 0, "\t" ).c_str() );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "FSP:\n%s\n", pool_set->fsp->toString( 0, "\t" ).c_str() );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "ARP:\n%s\n", pool_set->arp->toString( 0, "\t" ).c_str() );
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "SRP:\n%s\n", pool_set->srp->toString( 0, "\t" ).c_str() );
    subplugin->finalize();
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "Returning from %s::finalize()..." DEBUG_SEPARATOR_STRING, subplugin_name.c_str() );
}

/**
 * @brief Terminate the plugin due to error.
 *
 * @ingroup DebugPlugin
 *
 */
void DebugPlugin::terminate() {
    psc_infomsg( DEBUG_SEPARATOR_STRING DEBUG_PLUGIN_OUTPUT "Debugging %s::terminate()...\n\n", subplugin_name.c_str() );
    subplugin->terminate();
    psc_infomsg( DEBUG_PLUGIN_OUTPUT "Returning from %s::terminate()..." DEBUG_SEPARATOR_STRING, subplugin_name.c_str() );
}

/*
 * The following are C functions that belong to the Plugin Management Interface, and not to the
 * IPlugin class. Currently the interface includes information and instance creation functions.
 *
 * These are defined in PluginManagement.h .
 */

/**
 * @brief Returns an instance of this particular plugin implementation.
 *
 * @ingroup DebugPlugin
 *
 * @return An instance of this particular plugin implementation.
 */
IPlugin* getPluginInstance( void ) {
    return ( IPlugin* )new DebugPlugin();
}

/**
 * @brief Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 *
 * @ingroup DebugPlugin
 *
 * @return The major plugin interface version used by this plugin.
 *
 */
int getVersionMajor( void ) {
    return 0;
}

/**
 * @brief Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 *
 * @ingroup DebugPlugin
 *
 * @return The minor plugin interface version used by this plugin.
 *
 */
int getVersionMinor( void ) {
    return 9;
}

/**
 * @brief Returns a string with the name of the plugin.
 *
 * @ingroup DebugPlugin
 *
 * @ return A string with the name of the plugin.
 *
 */
string getName( void ) {
    return "Debugger Plugin";
}

/**
 * @brief Returns a string with a short description of the plugin.
 *
 * @ingroup DebugPlugin
 *
 * @return A string with a short description of the plugin.
 */
string getShortSummary( void ) {
    return "Prints the data exchanged (through the PTI) between the frontend and the plugin.";
}
