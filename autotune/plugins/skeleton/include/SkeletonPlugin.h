/**
   @file    SkeletonPlugin.h
   @ingroup SkeletonPlugin
   @brief   Skeleton of a Plugin
   @author  Author's name
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
   @defgroup SkeletonPlugin SkeletonPlugin
   @ingroup AutotunePlugins
 */

#ifndef SKELETON_PLUGIN_H_
#define SKELETON_PLUGIN_H_

#include "AutotunePlugin.h"
// uncomment the line below if your plugin will load search algorithms
//#include "ISearchAlgorithm.h"

class SkeletonPlugin : public IPlugin {
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
                          std::string& cmd,
                          bool&        instrumented );
    bool searchFinished( void );
    void finishTuningStep( void );
    bool tuningFinished( void );
    Advice* getAdvice( void );
    void finalize( void );
    void terminate( void );
};

#endif
