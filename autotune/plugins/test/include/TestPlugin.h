/**
   @file    TestPlugin.h
   @ingroup TestPlugin
   @brief   Test Plugin
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

/**
   @defgroup TestPlugin Test Plugin
   @ingroup AutotunePlugins
 */

#ifndef TEST_PLUGIN_H_
#define TEST_PLUGIN_H_

#include "AutotunePlugin.h"
// uncomment the line below if your plugin will load search algorithms
//#include "ISearchAlgorithm.h"

class TestPlugin : public IPlugin {
private:
    vector<TuningParameter*> tuning_parameters;
    VariantSpace             variantSpace;
    SearchSpace              searchSpace;
    ISearchAlgorithm*        search_algorithm;
    int                      test_step;
    int                      proc_count;
    int                      max_procs;
    bool                     just_started;
    DriverContext*           search_context;
    DriverContext*           context;

public:
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
