/**
   @file    AdaptiveSequencePlugin.cc
   @ingroup AdaptiveSequencePlugin
   @brief   Plugin that executes an adaptive sequence of sub-plugins
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

#include "AdaptiveSequencePlugin.h"

// TODO remove this and have a variable that is user defined (a runtime option such as --separator="something")
#define ADAPTIVE_SEQUENCE_SEPARATOR_STRING "\n--------------------------------------------------------------------------------\n"
#define ADAPTIVE_SEQUENCE_PLUGIN_OUTPUT "Adaptive-sequence: "

enum plugin_selection_type { PS_UNDEFINED = 0, PS_COMMAND_LINE_ARGUMENTS, PS_CONFIGURATION_FILE, PS_DEFAULT_LIST };


AdaptiveSequencePlugin::AdaptiveSequencePlugin() : appl( Application::instance() ) {
}


/**
 * @brief Adds a plugin into the plugin list and prepares it for execution.
 * @ingroup AdaptiveSequencePlugin
 *
 * This method loads the plugin creates a context and a new scenario set for it,
 * then it initializes a plugin.
 *
 * @param pluginName Name of the plugin to initialize.
 * @param threshold Threshold that plugins property has to exceed to execute a plugin.
 */
void AdaptiveSequencePlugin::addPlugin( string* pluginName,
                                        double  threshold ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: About to add %s plugin\n", pluginName->c_str() );

    int    major, minor;      // version information
    string name, description; // version information

    PluginInfo pluginInfo;
    pluginInfo.pluginName = *pluginName;
    pluginInfo.threshold  = threshold;

    context->loadPlugin( *pluginName, &major, &minor, &name, &description );

    print_loaded_plugin( major, minor, name, description );

    pluginInfo.plugin = context->getTuningPluginInstance( *pluginName );

    pluginInfo.context = new DriverContext( *context );

    generate_context_argc_argv( pluginInfo.context, subplugin_parameters, ( char* )plugin_selection_data.c_str() );

    pluginInfo.pool_set = new ScenarioPoolSet;
    pluginInfoList.push_back( pluginInfo );

    pluginInfo.plugin->initialize( pluginInfo.context, pluginInfo.pool_set );
}

/**
 * @brief Finalizes a plugin from the plugin list and removes it.
 * @ingroup AdaptiveSequencePlugin
 *
 * This method removes a plugin from the list and finalizes it.
 *
 * @param pluginName Name of the plugin to finalize and remove.
 */
void AdaptiveSequencePlugin::removePlugin( string* pluginName ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: About to remove %s plugin\n", pluginName->c_str() );

    list<PluginInfo>::iterator plugin_it;
    for( plugin_it = pluginInfoList.begin(); plugin_it != pluginInfoList.end(); plugin_it++ ) {
        if( ( *plugin_it ).pluginName.compare( *pluginName ) == 0 ) {
            //psc_infomsg(ADAPTIVE_SEQUENCE_PLUGIN_OUTPUT "Plugin %s found and will be deleted\n", (*plugin_it).pluginName.c_str());
            ( *plugin_it ).plugin->finalize();
            plugin_it = pluginInfoList.erase( plugin_it );
            plugin_it--;
        }
    }


    if( pluginInfoList.empty() ) {
        finalize();
    }

    pluginInfo_it = pluginInfoList.begin();
}

/**
 * @brief Parses the command line arguments.
 * @ingroup AdaptiveSequencePlugin
 *
 * This method parses the command line arguments. The command line arguments for
 * now only specifies which plugins to execute, from which configuration file to
 * read them from.
 *
 * @return Plugin selection type, i.e., where from to read the plugin list.
 */
int AdaptiveSequencePlugin::parseOpts() {
    // TODO: Fix the bug if both are specified in the command line that the last one is really done
    plugin_selection_type ps_type = plugin_selection_type( PS_DEFAULT_LIST );
    int                   size;


    const struct option long_opts[] = {
        { "plugins",                 required_argument,                 0,                 'p'                 },
        { "adaptivesequence-config", required_argument,                 0,                 'W'                 },
        { "threshold",               required_argument,                 0,                 't'                 },
    };

    optind = 1;

    while( optind < context->getArgc() ) {
        int            index = -1;
        struct option* opt   = 0;

        opterr = 0; // options for subplugins not parsed; suppress errors
        int result = getopt_long( context->getArgc(), context->getArgv(), "", long_opts, &index );
        if( result == -1 ) {
            psc_errmsg( "Error parsing command line parameters in the Adaptive-sequence plugin.\n" );
            throw 0;
        }

        opt = ( struct option* )&( long_opts[ index ] );

        switch( result ) {
        case 'p':
            psc_infomsg( ADAPTIVE_SEQUENCE_PLUGIN_OUTPUT "List of plugins to be executed: \"%s\".\n", optarg );
            ps_type               = plugin_selection_type( PS_COMMAND_LINE_ARGUMENTS );
            plugin_selection_data = string( optarg );
            break;
        case 'W':
            psc_infomsg( ADAPTIVE_SEQUENCE_PLUGIN_OUTPUT "Configuration file to be parsed: \"%s\".\n", optarg );
            ps_type               = plugin_selection_type( PS_CONFIGURATION_FILE );
            plugin_selection_data = string( optarg );
            break;
        case 't':
            psc_infomsg( ADAPTIVE_SEQUENCE_PLUGIN_OUTPUT "Plugin threshold values to be set: \"%s\".\n", optarg );
            plugin_threshold_data = string( optarg );
            break;
        case '?':
            psc_infomsg( ADAPTIVE_SEQUENCE_PLUGIN_OUTPUT "Non Adaptive-sequence plugin parameter \"%s\" will be passed to the Autotune plugin to be debugged.\n",
                         context->getArgv()[ optind - 1 ] );
            subplugin_parameters.push_back( string( context->getArgv()[ optind - 1 ] ) );
            break;
        default:
            psc_infomsg( ADAPTIVE_SEQUENCE_PLUGIN_OUTPUT "Unknown parameter! Will terminate.\n" );
            throw 0;
        }
    }
    return ps_type;
}

/**
 * @brief Loads a set of plugins and initializes them.
 * @ingroup AdaptiveSequencePlugin
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
void AdaptiveSequencePlugin::initialize( DriverContext*   context,
                                         ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: call to initialize()\n" );
    int    major, minor;      // version information
    string name, description; // version information
    // Assigns a context and a pool set for the plugin
    this->context  = context;
    this->pool_set = pool_set;

    plugin_selection_type ps_type = plugin_selection_type( PS_UNDEFINED );
    string configFilename;
    int size;
    // Parse command line options
    ps_type = ( plugin_selection_type )parseOpts();

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: Loaded plugins:\n" );


    //TODO: Tuning plugins should be done add once the pre-analysis is done and not here.
    if( ps_type == PS_COMMAND_LINE_ARGUMENTS ) {
        std::stringstream ts( plugin_threshold_data );
        list<float>       thresholds;
        string            temp_string;
        int               threshold;
        while( getline( ts, temp_string, ',' ) ) {
            thresholds.push_back( atof( temp_string.c_str() ) );
//      printf("Threshold = %s\n", temp_string.c_str());
        }

        std::stringstream ps( plugin_selection_data );
        string            pluginName;

        while( getline( ps, pluginName, ',' ) ) {
            if( !thresholds.empty() ) {
                psc_infomsg( "Adding %s plugin and its threshold = %f\n", pluginName.c_str(), thresholds.front() );
                addPlugin( &pluginName, thresholds.front() );
                thresholds.pop_front();
            }
            else {
                psc_infomsg( "Plugin %s threshold not specified! Default value, i.e. 10, is set.\n", pluginName.c_str() );
                addPlugin( &pluginName, 10 );
            }
        }
    }
    else if( ps_type == PS_CONFIGURATION_FILE ) {
        parseConfig( plugin_selection_data.c_str(), this );
    }
    else if( ps_type == PS_DEFAULT_LIST ) {
        psc_infomsg( "Plugin list not specified. Default configuration selected!\n" );
        addPlugin( new string( "compilerflags" ), 1 );
        addPlugin( new string( "dvfs" ), 1 );
        addPlugin( new string( "mpiparameters" ), 0.1 );
    }
    else {
        psc_infomsg( "Warning: plugin selection type not specified! Will apply the default one.\n" );
        addPlugin( new string( "compilerflags" ), 1 );
        addPlugin( new string( "dvfs" ), 1 );
        addPlugin( new string( "mpiparameters" ), 0.1 );
    }

    if( appl.get_regions().empty() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "[#### AUTOTUNE ####]: No regions found. Exiting.\n" );
        exit( EXIT_FAILURE );
    }

    pluginInfo_it = pluginInfoList.begin();

    psc_infomsg( ADAPTIVE_SEQUENCE_PLUGIN_OUTPUT "Running with \"%s\" plugin.\n", pluginInfo_it->pluginName.c_str() );
}


/**
 * @brief Copies the plugin arp into the sub-plugin arp and starts a tuning step of the sub-plugin.
 * @ingroup AdaptiveSequencePlugin
 *
 * This method copies the plugin arp properties into the sub-plugin arp pool.
 * It starts a sub-plugin's tuning step.
 *
 */
void AdaptiveSequencePlugin::startTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: call to startTuningStep()\n" );

    //RM: Here I copy the arp into the local pool set
    list<MetaProperty>           arp_properties = pool_set->arp->getPreAnalysisProperties( this->context->tuning_step - 1 );
    list<MetaProperty>::iterator prop_it;
    for( prop_it = arp_properties.begin(); prop_it != arp_properties.end(); prop_it++ ) {
        pluginInfo_it->pool_set->arp->pushPreAnalysisProperty( *prop_it, this->context->tuning_step - 1 );
    }

    if( context->tuning_step ) {
        pluginInfo_it->plugin->startTuningStep();
    }
}

/**
 * @brief This method calls a sub-plugin function of the same name.
 * @ingroup AdaptiveSequencePlugin
 *
 * This method calls a sub-plugin function of the same name.
 *
 * @param strategy defines a context for a plugin
 * @return does a sub-plugin requires a pre-analysis.
 */
bool AdaptiveSequencePlugin::analysisRequired( StrategyRequest** strategy ) {
    printf( "context->tuning_step = %d\n", context->tuning_step );
    if( context->tuning_step == 0 ) {
        StrategyRequestGeneralInfo* analysisStrategyGeneralInfo = new StrategyRequestGeneralInfo;

        analysisStrategyGeneralInfo->strategy_name     = "ConfigAnalysis";
        analysisStrategyGeneralInfo->pedantic          = 1;
        analysisStrategyGeneralInfo->delay_phases      = 0;
        analysisStrategyGeneralInfo->delay_seconds     = 0;
        analysisStrategyGeneralInfo->analysis_duration = 1;

        PropertyRequest*        req     = new PropertyRequest();
        list<PropertyRequest*>* reqList = new list<PropertyRequest*>;

        req->addPropertyID( HPCCONDITIONAL );
        req->addRegion( appl.get_phase_region() );
        req->addAllProcesses();
        reqList->push_back( req );

        StrategyRequest* analysisStrategy = new StrategyRequest( reqList, analysisStrategyGeneralInfo );

        if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ) ) ) {
            printf( "Strategy requests in the plugin:\n" );
            analysisStrategy->printStrategyRequest();
        }
        *strategy = analysisStrategy;

        return true;
    }
    else {
        return pluginInfo_it->plugin->analysisRequired( strategy );
    }
}

/**
 * @brief This method calls a sub-plugin function of the same name.
 * @ingroup AdaptiveSequencePlugin
 *
 * This method calls a sub-plugin function of the same name.
 *
 *
 */
void AdaptiveSequencePlugin::createScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: call to createScenarios()\n" );
    printf( "context->tuning_step = %d\n", context->tuning_step );
    if( context->tuning_step == 0 ) {
        int                                           experimentId, propertyCount = 0;
        map<int, list<MetaProperty> >                 preAnalysisPropertiesMap;
        map<int, list<MetaProperty> >                 perExperimentPropertiesMap;
        map<int, list<MetaProperty> >::const_iterator propertyMapIter;
        list<MetaProperty>                            properties;
        list<MetaProperty>::iterator                  property;
        double                                        ExecutionTime       = 0.0, TimePerInstance = 0.0, MPITime = 0.0;
        long long                                     RegionExecutedCount = 0, TotalInstructions = 0, EnergyCores = 0, EnergyDRAM = 0;

        double IPS = 0.0, IPJ = 0.0, fractionMPI = 0.0;

        preAnalysisPropertiesMap = pool_set->arp->getAllPreAnalysisProperties();
        for( propertyMapIter = preAnalysisPropertiesMap.begin(); propertyMapIter != preAnalysisPropertiesMap.end(); propertyMapIter++ ) {
            experimentId = propertyMapIter->first;
            properties   = propertyMapIter->second;
            psc_infomsg( ADAPTIVE_SEQUENCE_PLUGIN_OUTPUT "Properties returned from the pre-analysis\n" );

            for( property = properties.begin(); property != properties.end(); property++, propertyCount++ ) {
                if( property->getName().compare( "HPCConditional" ) == 0 ) {
                    RegionIdent  regident;
                    stringstream ss;
                    regident = appl.get_phase_region()->get_ident();
                    ss << regident.file_id << "-" << regident.rfl;

                    if( property->getRegionId().compare( ss.str() ) == 0 ) {
                        // Get values from the property and call the model.
                        if( property->getProcess() == 0 ) {
                            ExecutionTime       = atof( property->getExtraInfo().at( "ExecutionTime" ).c_str() );
                            RegionExecutedCount = atoi( property->getExtraInfo().at( "RegionExecutedCount" ).c_str() );
                            TimePerInstance     = atof( property->getExtraInfo().at( "TimePerInstance" ).c_str() );
                            MPITime             = atof( property->getExtraInfo().at( "MPITime" ).c_str() );
                            EnergyCores         = atoi( property->getExtraInfo().at( "EnergyCores" ).c_str() );
                            EnergyDRAM          = atoi( property->getExtraInfo().at( "EnergyDRAM" ).c_str() );
                            TotalInstructions   = atoi( property->getExtraInfo().at( "TotalInstructions" ).c_str() );
                            IPS                 = TotalInstructions / ExecutionTime;
                            IPJ                 = TotalInstructions / EnergyCores;
                            fractionMPI         = MPITime / ExecutionTime;
                        }
                    }
                    else {
                        psc_infomsg( ADAPTIVE_SEQUENCE_PLUGIN_OUTPUT "Property do not belong to the same region!\n" );
                    }

                    cout << propertyCount << ". Property: " << endl;
                    cout << " - Cluster: " << property->getCluster() << endl;
                    cout << " - Confidence: " << property->getConfidence() << endl;
                    cout << " - Configuration: " << property->getConfiguration() << endl;
                    //cout << " - Extra info: " << property->getExtraInfo() << endl;
                    cout << " - File id: " << property->getFileId() << endl;
                    cout << " - File name: " << property->getFileName() << endl;
                    cout << " - Property id: " << property->getId() << endl;
                    cout << " - Max processes: " << property->getMaxProcs() << endl;
                    cout << " - Max threads: " << property->getMaxThreads() << endl;
                    cout << " - Name: " << property->getName() << endl;
                    cout << " - Process: " << property->getProcess() << endl;
                    cout << " - Region id: " << property->getRegionId() << endl;
                    cout << " - Region name: " << property->getRegionName() << endl;
                    cout << " - Region type: " << property->getRegionType() << endl;
                    cout << " - Severity: " << property->getSeverity() << endl;
                    cout << " - Start position: " << property->getStartPosition() << endl;
                    cout << " - Thread: " << property->getThread() << endl;
                    cout << " - Time: " << property->getExtraInfo().at( "ExecutionTime" ) << endl;

                    cout << " - Execution time: " << ExecutionTime << endl;
                    cout << " - Region Executed Count: " << RegionExecutedCount << endl;
                    cout << " - Time per Instance: " << TimePerInstance << endl;
                    cout << " - MPI Time: " << MPITime << endl;
                    cout << " - Energy for Cores: " << EnergyCores << endl;
                    cout << " - Energy for DRAMs: " << EnergyDRAM << endl;
                    cout << " - Total Instructions: " << TotalInstructions << endl;

                    cout << " - Instructions Per Second: " << IPS << endl;
                    cout << " - Instructions Per Joule: " << IPJ << endl;
                    cout << " - Fraction of MPI time over complete execution time: " << fractionMPI << endl;
                } // if suited
            }     // for property
        }         // for propertyMapIter

        for( pluginInfo_it = pluginInfoList.begin(); pluginInfo_it != pluginInfoList.end(); pluginInfo_it++ ) {
            if( ( !pluginInfo_it->pluginName.compare( "compilerflags" ) && IPS < pluginInfo_it->threshold ) ||
                ( !pluginInfo_it->pluginName.compare( "dvfs" ) && IPJ < pluginInfo_it->threshold ) ||
                ( !pluginInfo_it->pluginName.compare( "mpiparameters" ) && fractionMPI < pluginInfo_it->threshold ) ) {
                psc_infomsg( ADAPTIVE_SEQUENCE_PLUGIN_OUTPUT "Unloading plugin %s. Criterium %f not satisfied! IPS = %f, IPJ = %f, fractionMPI = %f\n",
                             pluginInfo_it->pluginName.c_str(), pluginInfo_it->threshold, IPS, IPJ, fractionMPI );
                pluginInfo_it->plugin->finalize();
                pluginInfo_it = pluginInfoList.erase( pluginInfo_it );
                pluginInfo_it--;
            }
        }

        //TODO: If the plugin list is empty we should terminate, but now it crashes.
        //This does not work as the order is still forced by the statemachine.
        if( pluginInfoList.empty() ) {
            finalize();
        }

        pluginInfo_it = pluginInfoList.begin();
    }
    else {
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
}

/**
 * @brief This method calls a sub-plugin function of the same name.
 * @ingroup AdaptiveSequencePlugin
 *
 * This method calls a sub-plugin function of the same name.
 *
 */
void AdaptiveSequencePlugin::prepareScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: call to prepareScenarios()\n" );

    if( context->tuning_step == 0 ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: call to prepareScenarios() should not happen!\n" );
    }
    else {
        pluginInfo_it->plugin->prepareScenarios();
    }
}

/**
 * @brief This method calls a sub-plugin function of the same name.
 * @ingroup AdaptiveSequencePlugin
 *
 * This method calls a sub-plugin function of the same name. The method prepares
 * a set of scenarios from the sub-plugin's csp and moves them to the
 * sub-plugin's psp.
 *
 * @param numprocs number of processes defined by a sub-plugin
 * @param analysisRequired defines does a sub-plugin requires a per-experiment analysis
 * @param strategy defines a sub-plugin's strategy request
 */
void AdaptiveSequencePlugin::defineExperiment( int               numprocs,
                                               bool&             analysisRequired,
                                               StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: call to defineExperiment()\n" );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: defining %d experiment\n", this->context->experiment_count );

    if( context->tuning_step == 0 ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: call to defineExperiment() should not happen!\n" );
        analysisRequired = false;
        ( *strategy )    = NULL;
        return;
    }
    else {
        pluginInfo_it->plugin->defineExperiment( numprocs, analysisRequired, strategy );
    }
}

/**
 * @brief This method keeps pools coherent and calls a sub-plugin function of the same name.
 * @ingroup AdaptiveSequencePlugin
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
bool AdaptiveSequencePlugin::restartRequired( std::string& env,
                                              int&         new_process_count,
                                              std::string& command,
                                              bool&        is_instrumented ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: call to restartRequired()\n" );

    if( context->tuning_step == 0 ) {
        is_instrumented = context->applInstrumented();
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: call to restartRequired() should not happen!\n" );
        return false;
    }

    pool_set->csp->clear();
    pool_set->psp->clear();
    pool_set->esp->clear();

    Scenario* sc;

    //RM: Here I move the fsp scenarios from global pool set into the local pool set
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Scenarios copied to a plugin-specific fsp: %lu\n",
                pool_set->fsp->size() );
    while( !pool_set->fsp->empty() ) {
        sc = pool_set->fsp->pop();
        if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ) ) > 0 ) {
            sc->print();
        }
        pluginInfo_it->pool_set->fsp->push( sc );
    }

    //RM: Here I move the esp scenarios from local pool set into the global pool set
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Scenarios copied to a meta-plugin esp: %lu\n",
                pluginInfo_it->pool_set->esp->size() );
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

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Scenarios copied to a meta-plugin psp: %lu\n",
                scenarios->size() );
    for( sc_it = scenarios->begin(); sc_it != scenarios->end(); sc_it++ ) {
        if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ) ) > 0 ) {
            sc_it->second->print();
        }
        pool_set->psp->push( sc_it->second );
    }

    //RM: Here I move the csp scenarios from local pool set into the global pool set
    scenarios = pluginInfo_it->pool_set->csp->getScenarios();
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Scenarios copied to a meta-plugin csp: %lu\n",
                scenarios->size() );
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
 * @ingroup AdaptiveSequencePlugin
 *
 * Keeps the srp pool coherent between the plugin and sub-plugins. Calls a sub-plugin's
 * function of the same name.
 *
 * @return has the sub-plugin finished search
 */
bool AdaptiveSequencePlugin::searchFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: call to searchFinished()\n" );

    if( context->tuning_step == 0 ) {
        return true;
    }

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
 * @ingroup AdaptiveSequencePlugin
 *
 * This method calls a sub-plugin function of the same name.
 *
 */
void AdaptiveSequencePlugin::finishTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: call to finishTuningStep()\n" );

    if( context->tuning_step == 0 ) {
        return;
    }
    else {
        pluginInfo_it->plugin->finishTuningStep();
    }
}

/**
 * @brief Checks has the sub-plugin finished tuning and selects the next sub-plugin.
 * @ingroup AdaptiveSequencePlugin
 *
 * This method checks has the sub-plugin finished tuning. If it has finished it
 * selects the next one in the list and returns false. If all the sub-plugins
 * have finished it returns true.
 *
 * @return status of the tuning for a plugin.
 *
 */
bool AdaptiveSequencePlugin::tuningFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: call to tuningFinished()\n" );

    printf( "context->tuning_step = %d\n", context->tuning_step );
    if( context->tuning_step > 1 ) {
        bool tuningFinishedFlag = pluginInfo_it->plugin->tuningFinished();

        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: Tuning finished flag = %d\n", tuningFinishedFlag );

        if( tuningFinishedFlag ) {
            Scenario::resetScenarioIds();
            pluginInfo_it++;
            if( pluginInfo_it == pluginInfoList.end() ) {
                return tuningFinishedFlag;
            }
            psc_infomsg( ADAPTIVE_SEQUENCE_PLUGIN_OUTPUT "Running with \"%s\" plugin.\n", pluginInfo_it->pluginName.c_str() );
        }
    }


    return false;
}

/**
 * @brief This method calls a function of the same name for all sub-plugins.
 * @ingroup AdaptiveSequencePlugin
 *
 * This method calls a function of the same name for all sub-plugins.
 *
 */
Advice* AdaptiveSequencePlugin::getAdvice( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: call to getAdvice()\n" );

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
 * @ingroup AdaptiveSequencePlugin
 *
 * This method calls a function of the same name for all sub-plugins.
 *
 */
void AdaptiveSequencePlugin::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: call to finalize()\n" );

    for( pluginInfo_it = pluginInfoList.begin(); pluginInfo_it != pluginInfoList.end(); pluginInfo_it++ ) {
        pluginInfo_it->plugin->finalize();
    }
}

/**
 * @brief This method calls a function of the same name for all sub-plugins.
 * @ingroup AdaptiveSequencePlugin
 *
 * This method calls a function of the same name for all sub-plugins.
 *
 */
void AdaptiveSequencePlugin::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: call to terminate()\n" );

    for( pluginInfo_it = pluginInfoList.begin(); pluginInfo_it != pluginInfoList.end(); pluginInfo_it++ ) {
        pluginInfo_it->plugin->terminate();
    }
}

/**
 * @brief Returns plugin instance.
 * @ingroup AdaptiveSequencePlugin
 *
 * @return Plugin instance.
 **/
IPlugin* getPluginInstance( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: call to getPluginInstance()\n" );

    return new AdaptiveSequencePlugin();
}

/**
 * @brief Returns the major version number.
 * @ingroup AdaptiveSequencePlugin
 *
 * @return The major version number.
 **/
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: call to getVersionMajor()\n" );

    return 1;
}

/**
 * @brief Returns the minor version number.
 * @ingroup AdaptiveSequencePlugin
 *
 * @return The minor version number.
 **/
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: call to getVersionMinor()\n" );

    return 0;
}

/**
 * @brief Returns plugin name.
 * @ingroup AdaptiveSequencePlugin
 *
 * @return Plugin name.
 **/
string getName( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: call to getName()\n" );

    return "Adaptive sequence plugin";
}

/**
 * @brief Returns a short description.
 * @ingroup AdaptiveSequencePlugin
 *
 * @return Description.
 **/
string getShortSummary( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "AdaptiveSequencePlugin: call to getShortSummary()\n" );

    return "Finds optimal combination for a set of plugins that run one after another";
}
