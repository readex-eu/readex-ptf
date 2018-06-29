/**
   @file    PipelinePlugin.h
   @ingroup PipelinePlugin
   @brief   Pipeline Plugin
   @author  Research Group Scientific Computing, University of Vienna
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
   @defgroup PipelinePlugin Pipeline Plugin
   @ingroup AutotunePlugins
 */

#ifndef PIPELINE_PLUGIN_H_
#define PIPELINE_PLUGIN_H_

#include "AutotunePlugin.h"
// uncomment the line below if your plugin will load search algorithms
//#include "ISearchAlgorithm.h"

// Autotune headers
#include "AutotunePlugin.h"
#include "ISearchAlgorithm.h"

using namespace std;

class PipelinePlugin : public IPlugin {
    bool                     scenario_pool_not_empty;
    bool                     prepared_scenario_pool_not_empty;
    vector<TuningParameter*> tuningParameters;
    ISearchAlgorithm*        searchAlgorithm;
    Region*                  tuningRegion;
    VariantSpace             variantSpace;
    SearchSpace              searchSpace;

public:
    vector<TuningParameter*>extractPipelineTuningParameters( void );

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

#endif
