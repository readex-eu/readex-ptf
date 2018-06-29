/**
   @file    FixedSequencePlugin.cc
   @ingroup FixedSequencePlugin
   @brief   Plugin that executes a fixed sequence of sub-plugins
   @author  Robert Mijakovic
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

#include "FixedSequencePlugin.h"

// TODO remove this and have a variable that is user defined (a runtime option such as --separator="something")
#define FIXED_SEQUENCE_SEPARATOR_STRING "\n--------------------------------------------------------------------------------\n"
#define FIXED_SEQUENCE_PLUGIN_OUTPUT "Fixed-sequence: "

enum plugin_selection_type { PS_UNDEFINED = 0, PS_COMMAND_LINE_ARGUMENTS, PS_CONFIGURATION_FILE, PS_DEFAULT_LIST };


/**
 * @brief Adds a plugin into the plugin list and prepares it for execution.
 * @ingroup FixedSequencePlugin
 *
 * This method loads the plugin creates a context and a new scenario set for it,
 * then it initializes a plugin.
 *
 * @param pluginName Name of the plugin to initialize.
 */
void FixedSequencePlugin::addPlugin( string* pluginName ) {
    int    major, minor;      // version information
    string name, description; // version information

    PluginInfo pluginInfo;
    pluginInfo.pluginName = *pluginName;

    context->loadPlugin( *pluginName, &major, &minor, &name, &description );

    print_loaded_plugin( major, minor, name, description );

    pluginInfo.plugin = context->getTuningPluginInstance( *pluginName );

    pluginInfo.context = new DriverContext;

    generate_context_argc_argv( pluginInfo.context, subplugin_parameters, ( char* )plugin_selection_data.c_str() );
    pluginInfo.context->setApplInstrumented( context->applInstrumented() );
    pluginInfo.context->setOmpnumthreads( context->getOmpnumthreads() );

    pluginInfo.pool_set = new ScenarioPoolSet;
    pluginInfoList.push_back( pluginInfo );

    pluginInfo.plugin->initialize( pluginInfo.context, pluginInfo.pool_set );
}

/**
 * @brief Parses the command line arguments.
 * @ingroup FixedSequencePlugin
 *
 * This method parses the command line arguments. The command line arguments for
 * now only specifies which plugins to execute, from which configuration file to
 * read them from.
 *
 * @return Plugin selection type, i.e., where from to read the plugin list.
 */
int FixedSequencePlugin::parseOpts() {
    // TODO: Fix the bug if both are specified in the command line that the last one is really done
    plugin_selection_type ps_type = plugin_selection_type( PS_DEFAULT_LIST );

    const struct option long_opts[] = {
        { "plugins",               required_argument,               0,               'p'               },
        { "fixed-sequence-config", required_argument,               0,               'W'               },
    };

    optind = 1;

    while( optind < context->getArgc() ) {
        int            index = -1;
        struct option* opt   = 0;

        opterr = 0; // options for subplugins not parsed; suppress errors
        int result = getopt_long( context->getArgc(), context->getArgv(), "", long_opts, &index );
        if( result == -1 ) {
            psc_errmsg( "Error parsing command line parameters in the Fixed-sequence plugin.\n" );
            throw 0;
        }

        opt = ( struct option* )&( long_opts[ index ] );

        switch( result ) {
        case 'p':
            psc_infomsg( FIXED_SEQUENCE_PLUGIN_OUTPUT "List of plugins to be executed: \"%s\".\n", optarg );
            ps_type               = plugin_selection_type( PS_COMMAND_LINE_ARGUMENTS );
            plugin_selection_data = string( optarg );
            break;
        case 'W':
            psc_infomsg( FIXED_SEQUENCE_PLUGIN_OUTPUT "Configuration file to be parsed: \"%s\".\n", optarg );
            ps_type               = plugin_selection_type( PS_CONFIGURATION_FILE );
            plugin_selection_data = string( optarg );
            break;
        case '?':
            psc_infomsg( FIXED_SEQUENCE_PLUGIN_OUTPUT "Non Fixed-sequence plugin parameter \"%s\" will be passed to the Autotune plugin to be debugged.\n",
                         context->getArgv()[ optind - 1 ] );
            subplugin_parameters.push_back( string( context->getArgv()[ optind - 1 ] ) );
            break;
        default:
            throw 0;
        }
    }
    return ps_type;
}

/**
 * @brief Loads a set of plugins and initializes them.
 * @ingroup FixedSequencePlugin
 *
 * This method loads a set of sub-plugins specified in the list, instantiates
 * and initialize them. Each sub-plugin has its own context and a pool set.
 * Sub-plugin information is combined and contains its plugin instance, a
 * context and a pool set.
 *
 * Plugins to execute are defined with the following priorities:
 * 1. Command line arguments
 * 2. Configuration file
 * 3. Default list in the plugin
 *
 * @param context defines a context for a plugin
 * @param pool_set defines a set of pools for a plugin
 */
void FixedSequencePlugin::initialize( DriverContext*   context,
                                      ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "FixedSequencePlugin: call to initialize()\n" );
    int    major, minor;      // version information
    string name, description; // version information
    // Assigns a context and a pool set for the plugin
    this->context  = context;
    this->pool_set = pool_set;

    plugin_selection_type ps_type = plugin_selection_type( PS_UNDEFINED );
    // Parse command line options
    ps_type = ( plugin_selection_type )parseOpts();

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "FixedSequencePlugin: Loaded plugins:\n" );

    if( ps_type == PS_COMMAND_LINE_ARGUMENTS ) {
        std::stringstream ss( plugin_selection_data );
        string            pluginName;
        while( getline( ss, pluginName, ',' ) ) {
            addPlugin( &pluginName );
        }
    }
    else if( ps_type == PS_CONFIGURATION_FILE ) {
        parseConfig( plugin_selection_data.c_str(), this );
    }
    else if( ps_type == PS_DEFAULT_LIST ) {
        psc_infomsg( "Plugin list not specified. Default configuration selected!\n" );
        addPlugin( new string( "compilerflags" ) );
        addPlugin( new string( "dvfs" ) );
        addPlugin( new string( "mpiparameters" ) );
    }
    else {
        psc_infomsg( "Warning plugin selection type not specified! Will apply the default one.\n" );
        addPlugin( new string( "compilerflags" ) );
        addPlugin( new string( "dvfs" ) );
        addPlugin( new string( "mpiparameters" ) );
    }

    pluginInfo_it = pluginInfoList.begin();

    psc_infomsg( FIXED_SEQUENCE_PLUGIN_OUTPUT "Running with \"%s\" plugin.\n", pluginInfo_it->pluginName.c_str() );
}


/**
 * @brief Copies the plugin arp into the sub-plugin arp and starts a tuning step of the sub-plugin.
 * @ingroup FixedSequencePlugin
 *
 * This method copies the plugin arp properties into the sub-plugin arp pool.
 * It starts a sub-plugin's tuning step.
 *
 */
void FixedSequencePlugin::startTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "FixedSequencePlugin: call to startTuningStep()\n" );

    //RM: Here I copy the arp into the local pool set
    list<MetaProperty>           arp_properties = pool_set->arp->getPreAnalysisProperties( this->context->tuning_step - 1 );
    list<MetaProperty>::iterator prop_it;
    for( prop_it = arp_properties.begin(); prop_it != arp_properties.end(); prop_it++ ) {
        pluginInfo_it->pool_set->arp->pushPreAnalysisProperty( *prop_it, this->context->tuning_step - 1 );
    }

    pluginInfo_it->plugin->startTuningStep();
}

/**
 * @brief This method calls a sub-plugin function of the same name.
 * @ingroup FixedSequencePlugin
 *
 * This method calls a sub-plugin function of the same name.
 *
 * @param strategy defines a context for a plugin
 * @return does a sub-plugin requires a pre-analysis.
 */
bool FixedSequencePlugin::analysisRequired( StrategyRequest** strategy ) {
    return pluginInfo_it->plugin->analysisRequired( strategy );
}

/**
 * @brief This method calls a sub-plugin function of the same name.
 * @ingroup FixedSequencePlugin
 *
 * This method calls a sub-plugin function of the same name.
 *
 *
 */
void FixedSequencePlugin::createScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "FixedSequencePlugin: call to createScenarios()\n" );

    pluginInfo_it->plugin->createScenarios();
    map<int, Scenario*>*          scenarios = pluginInfo_it->pool_set->csp->getScenarios();
    map<int, Scenario*>::iterator sc_it;
    //psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "Scenarios copied to a meta-plugin csp:\n");
    if( scenarios->empty() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "NONE" );
    }
    for( sc_it = scenarios->begin(); sc_it != scenarios->end(); sc_it++ ) {
        if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ) ) > 0 ) {
            sc_it->second->print();
        }
        pool_set->csp->push( sc_it->second );
    }
}

/**
 * @brief This method calls a sub-plugin function of the same name.
 * @ingroup FixedSequencePlugin
 *
 * This method calls a sub-plugin function of the same name.
 *
 */
void FixedSequencePlugin::prepareScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "FixedSequencePlugin: call to prepareScenarios()\n" );

    pluginInfo_it->plugin->prepareScenarios();
}

/**
 * @brief This method calls a sub-plugin function of the same name.
 * @ingroup FixedSequencePlugin
 *
 * This method calls a sub-plugin function of the same name. The method prepares
 * a set of scenarios from the sub-plugin's csp and moves them to the
 * sub-plugin's psp.
 *
 * @param numprocs number of processes defined by a sub-plugin
 * @param analysisRequired defines does a sub-plugin requires a per-experiment analysis
 * @param strategy defines a sub-plugin's strategy request
 */
void FixedSequencePlugin::defineExperiment( int               numprocs,
                                            bool&             analysisRequired,
                                            StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "FixedSequencePlugin: call to defineExperiment()\n" );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "FixedSequencePlugin: defining %d experiment\n", this->context->experiment_count );

    pluginInfo_it->plugin->defineExperiment( numprocs, analysisRequired, strategy );
}

/**
 * @brief This method keeps pools coherent and calls a sub-plugin function of the same name.
 * @ingroup FixedSequencePlugin
 *
 * Keeps pools coherent between the plugin and sub-plugins. Calls a sub-plugin's
 * function of the same name.
 *
 * @param env environment variables configured for a sub-plugin
 * @param new_process_count number of processes requested by a sub-plugin
 * @param command command requested by a sub-plugin
 * @param is_instrumented a sub-plugin defines is an application instrumented
 * @return does the sub-plugin requires restart of an application
 */
bool FixedSequencePlugin::restartRequired( std::string& env,
                                           int&         new_process_count,
                                           std::string& command,
                                           bool&        is_instrumented ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "FixedSequencePlugin: call to restartRequired()\n" );

    pool_set->csp->clear();
    pool_set->psp->clear();
    pool_set->esp->clear();

    Scenario* sc;

    //RM: Here I move the fsp scenarios from global pool set into the local pool set
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Scenarios copied to a plugin-specific fsp:\n" );
    if( pool_set->fsp->empty() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "NONE" );
    }
    while( !pool_set->fsp->empty() ) {
        sc = pool_set->fsp->pop();
        if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ) ) > 0 ) {
            sc->print();
        }
        pluginInfo_it->pool_set->fsp->push( sc );
    }

    //RM: Here I move the esp scenarios from local pool set into the global pool set
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Scenarios copied to a meta-plugin esp:\n" );
    if( pluginInfo_it->pool_set->esp->empty() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "NONE" );
    }
    while( !pluginInfo_it->pool_set->esp->empty() ) {
        sc = pluginInfo_it->pool_set->esp->pop();
        if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ) ) > 0 ) {
            sc->print();
        }
        pool_set->esp->push( sc );
    }

    //RM: Here I move the psp scenarios from local pool set into the global pool set
    map<int, Scenario*>*          scenarios = pluginInfo_it->pool_set->psp->getScenarios();
    map<int, Scenario*>::iterator sc_it;

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Scenarios copied to a meta-plugin psp:\n" );
    if( scenarios->empty() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "NONE" );
    }
    for( sc_it = scenarios->begin(); sc_it != scenarios->end(); sc_it++ ) {
        if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ) ) > 0 ) {
            sc_it->second->print();
        }
        pool_set->psp->push( sc_it->second );
    }

    //RM: Here I move the csp scenarios from local pool set into the global pool set
    scenarios = pluginInfo_it->pool_set->csp->getScenarios();
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Scenarios copied to a meta-plugin csp:\n" );
    if( scenarios->empty() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "NONE" );
    }
    for( sc_it = scenarios->begin(); sc_it != scenarios->end(); sc_it++ ) {
        if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ) ) > 0 ) {
            sc_it->second->print();
        }
        pool_set->csp->push( sc_it->second );
    }

    return pluginInfo_it->plugin->restartRequired( env, new_process_count, command, is_instrumented );
}

/**
 * @brief Keeps the srp pool coherent between the plugin and sub-plugins and calls a sub-plugin function of the same name.
 * @ingroup FixedSequencePlugin
 *
 * Keeps the srp pool coherent between the plugin and sub-plugins. Calls a sub-plugin's
 * function of the same name.
 *
 * @return has the sub-plugin finished search
 */
bool FixedSequencePlugin::searchFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "FixedSequencePlugin: call to searchFinished()\n" );

    //RM: Here I copy the srp properties from global pool set into the local pool set
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Search step = %d\n", this->context->search_step );
    list<ScenarioResult>           srp_results = pool_set->srp->getScenarioResultsPerSearchStep( this->context->search_step - 1 );
    list<ScenarioResult>::iterator scr_it;
    list<MetaProperty>             srp_properties;
    list<MetaProperty>::iterator   prop_it;
    for( scr_it = srp_results.begin(); scr_it != srp_results.end(); scr_it++ ) {
        srp_properties = scr_it->getProperties();
        for( prop_it = srp_properties.begin(); prop_it != srp_properties.end(); prop_it++ ) {
            pluginInfo_it->pool_set->srp->push( *prop_it, this->context->search_step - 1 );
        }
    }

    return pluginInfo_it->plugin->searchFinished();
}

/**
 * @brief This method calls a sub-plugin function of the same name.
 * @ingroup FixedSequencePlugin
 *
 * This method calls a sub-plugin function of the same name.
 *
 */
void FixedSequencePlugin::finishTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "FixedSequencePlugin: call to finishTuningStep()\n" );

    pluginInfo_it->plugin->finishTuningStep();
}

/**
 * @brief Checks has the sub-plugin finished tuning and selects the next sub-plugin.
 * @ingroup FixedSequencePlugin
 *
 * This method checks has the sub-plugin finished tuning. If it has finished it
 * selects the next one in the list and returns false. If all the sub-plugins
 * have finished it returns true.
 *
 * @return status of the tuning for a plugin.
 *
 */
bool FixedSequencePlugin::tuningFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "FixedSequencePlugin: call to tuningFinished()\n" );

    bool tuningFinishedFlag = pluginInfo_it->plugin->tuningFinished();

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "FixedSequencePlugin: Tuning finished flag = %d\n", tuningFinishedFlag );

    if( tuningFinishedFlag ) {
        Scenario::resetScenarioIds();
        pluginInfo_it++;
        if( pluginInfo_it == pluginInfoList.end() ) {
            return tuningFinishedFlag;
        }
        psc_infomsg( FIXED_SEQUENCE_PLUGIN_OUTPUT "Running with \"%s\" plugin.\n", pluginInfo_it->pluginName.c_str() );
    }

    return false;
}

/**
 * @brief This method calls a function of the same name for all sub-plugins.
 * @ingroup FixedSequencePlugin
 *
 * This method calls a function of the same name for all sub-plugins.
 *
 */
Advice* FixedSequencePlugin::getAdvice( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "FixedSequencePlugin: call to getAdvice()\n" );

    list<ptree> adviceDataList;

    for( pluginInfo_it = pluginInfoList.begin(); pluginInfo_it != pluginInfoList.end(); pluginInfo_it++ ) {
        pluginInfo_it->advice = pluginInfo_it->plugin->getAdvice();
        adviceDataList.push_back( pluginInfo_it->advice->getInternalAdviceData() );
    }

    Advice* advice = new Advice();
    advice->mergeAdvices( adviceDataList, getName() );

    return advice;
}

/**
 * @brief This method calls a function of the same name for all sub-plugins.
 * @ingroup FixedSequencePlugin
 *
 * This method calls a function of the same name for all sub-plugins.
 *
 */
void FixedSequencePlugin::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "FixedSequencePlugin: call to finalize()\n" );

    for( pluginInfo_it = pluginInfoList.begin(); pluginInfo_it != pluginInfoList.end(); pluginInfo_it++ ) {
        pluginInfo_it->plugin->finalize();
    }
}

/**
 * @brief This method calls a function of the same name for all sub-plugins.
 * @ingroup FixedSequencePlugin
 *
 * This method calls a function of the same name for all sub-plugins.
 *
 */
void FixedSequencePlugin::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "FixedSequencePlugin: call to terminate()\n" );

    for( pluginInfo_it = pluginInfoList.begin(); pluginInfo_it != pluginInfoList.end(); pluginInfo_it++ ) {
        pluginInfo_it->plugin->terminate();
    }
}

/**
 * @brief Returns plugin instance.
 * @ingroup FixedSequencePlugin
 *
 * @return Plugin instance.
 **/
IPlugin* getPluginInstance( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "FixedSequencePlugin: call to getPluginInstance()\n" );

    return new FixedSequencePlugin();
}

/**
 * @brief Returns the major version number.
 * @ingroup FixedSequencePlugin
 *
 * @return The major version number.
 **/
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "FixedSequencePlugin: call to getVersionMajor()\n" );

    return 1;
}

/**
 * @brief Returns the minor version number.
 * @ingroup FixedSequencePlugin
 *
 * @return The minor version number.
 **/
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "FixedSequencePlugin: call to getVersionMinor()\n" );

    return 0;
}

/**
 * @brief Returns plugin name.
 * @ingroup FixedSequencePlugin
 *
 * @return Plugin name.
 **/
string getName( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "FixedSequencePlugin: call to getName()\n" );

    return "Fixed sequence plugin";
}

/**
 * @brief Returns a short description.
 * @ingroup FixedSequencePlugin
 *
 * @return Description.
 **/
string getShortSummary( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "FixedSequencePlugin: call to getShortSummary()\n" );

    return "Finds optimal combination for a set of plugins that run one after another";
}
