/**
   @file    FixedSequencePlugin.h
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

/**
   @defgroup FixedSequencePlugin Fixed Sequence Plugin
   @ingroup AutotunePlugins
 */

#ifndef FIXED_SEQUENCE_PLUGIN_H_
#define FIXED_SEQUENCE_PLUGIN_H_

#include "AutotunePlugin.h"
// uncomment the line below if your plugin will load search algorithms
//#include "ISearchAlgorithm.h"

typedef struct pluginInfo_t {
    string           pluginName;
    IPlugin*         plugin;
    DriverContext*   context;
    ScenarioPoolSet* pool_set;
    Advice*          advice;
} PluginInfo;

class FixedSequencePlugin : public IPlugin {
    friend class CompilerFlagsPlugin;
    list <IPlugin*>                 pluginList;
    list<PluginInfo>                pluginInfoList;
    std::list<PluginInfo>::iterator pluginInfo_it;
    string                          plugin_selection_data;
    list<string>                    subplugin_parameters;
protected:
    DriverContext*              context;
    ScenarioPoolSet*            pool_set;
    std::list<DriverContext*>   sub_contexts;
    std::list<ScenarioPoolSet*> sub_pools;
public:
    void addPlugin( string* pluginName );

    int parseOpts();

    void initialize( DriverContext*   context,
                     ScenarioPoolSet* pool_set );

    bool analysisRequired( StrategyRequest** strategy );

    void startTuningStep( void );

    void createScenarios( void );

    void prepareScenarios( void );

    void defineExperiment( int               numprocs,
                           bool&             analysisRequired,
                           StrategyRequest** strategy );

    bool restartRequired( std::string& env,
                          int&         numprocs,
                          std::string& command,
                          bool&        is_instrumented );

    bool searchFinished( void );

    void finishTuningStep( void );

    bool tuningFinished( void );

    Advice* getAdvice( void );

    void finalize( void );

    void terminate( void );
};

void parseConfig( const char*          filename,
                  FixedSequencePlugin* plugin );

#endif /* FIXED_SEQUENCE_PLUGIN_H_ */
