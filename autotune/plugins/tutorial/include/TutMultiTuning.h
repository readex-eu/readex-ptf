/**
   @file    TutMultiTuning.h
   @ingroup TutMultiTuning
   @brief   Tuning significant regions in multiple tuning steps
   @author  Shrikant Vinchurkar
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
   @defgroup TutMultiTuning Tutorial Multistep tuning
   @ingroup AutotunePlugins
 */

#ifndef TUTORIAL_PLUGIN_H_
#define TUTORIAL_PLUGIN_H_

#include "AutotunePlugin.h"

class TutMultiTuning : public IPlugin {
    vector<TuningParameter*> tuningParameters;
    ISearchAlgorithm*        searchAlgorithm;

    std::map<string, Region*> code_region_candidates;
    std::map<int, double>     localSearchPath;
    std::map<int, int>        scenIdTuningStepMap;

    std::vector<std::pair<string, double> > regionExectimeMap;
    Region*                                 selected_region;
    int                                     maxTuningSteps;
    int                                     curTuningStep;

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

    bool restartRequired( string& env,
                          int&    np,
                          string& cmd,
                          bool&   instrumented );

    bool searchFinished( void );

    void finishTuningStep( void );

    bool tuningFinished( void );

    Advice* getAdvice( void );

    void finalize( void );

    void terminate( void );
};

#endif
