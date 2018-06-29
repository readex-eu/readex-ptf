/**
   @file    DemoPlugin.h
   @ingroup DemoPlugin
   @brief   Demo Plugin
   @author  Houssam Haitof
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
   @defgroup DemoPlugin Demo Plugin
   @ingroup AutotunePlugins
 */

#ifndef DEMOPLUGIN_H_
#define DEMOPLUGIN_H_

// Autotune headers
#include "AutotunePlugin.h"
#include "ISearchAlgorithm.h"

using namespace std;

class DemoPlugin : public IPlugin {
    bool                     scenario_pool_not_empty;
    bool                     prepared_scenario_pool_not_empty;
    vector<TuningParameter*> tuningParameters;
    //TP constraints as iterator facade. (To be done)
    ISearchAlgorithm* searchAlgorithm;
    Region*           tuningRegion;
    VariantSpace      variantSpace;
    SearchSpace       searchSpace;

public: // plublic only what is part of the plugin interface
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

#endif /* DEMOPLUGIN_H_ */
