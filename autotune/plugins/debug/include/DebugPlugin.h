/**
   @file    DebugPlugin.h
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

/// @defgroup DebugPlugin Debug plugin
/// @ingroup AutotunePlugins

#ifndef SKELETON_PLUGIN_H_
#define SKELETON_PLUGIN_H_

#include "AutotunePlugin.h"

class DebugPlugin : public IPlugin {
private:
    IPlugin*       subplugin;
    string         subplugin_name;
    DriverContext* subplugin_context;
    list<string>   subplugin_parameters;

public:
    void initialize( DriverContext*   context,
                     ScenarioPoolSet* pool_set );

    void startTuningStep( void );

    bool analysisRequired( StrategyRequest** strategy );

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

#endif
