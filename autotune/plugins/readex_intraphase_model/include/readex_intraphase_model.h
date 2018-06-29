/**
   @file    readex_intraphase_model.cc
   @ingroup ReadexIntraphaseModelPlugin
   @brief   Readex intra-phase tuning plugin based on dvfs_taurus
   @author  Madhura Kumaraswamy
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
   @defgroup ReadexTuningPlugin ReadexTuningPlugin
   @ingroup AutotunePlugins
 */

#ifndef READEX_INTRAPHASE_MODEL_PLUGIN_H_
#define READEX_INTRAPHASE_MODEL_PLUGIN_H_

#include <unordered_map>
#include "AutotunePlugin.h"

#define ATP_PARAMETER_TUNING 1
#define HARDWARE_PARAMETER_TUNING 2
#define READEX_SCENARIO_TUNING 3


// uncomment the line below if your plugin will load search algorithms
//#include "ISearchAlgorithm.h"

class ReadexIntraphaseModelPlugin : public IPlugin {
        vector<TuningParameter*>        tuningParameters;
        ISearchAlgorithm*               searchAlgorithm;
        std::list<Region*>              code_significant_regions;
        std::vector<ObjectiveFunction*> objectives;
        int                             tuningStep;
        const char*                     atp_execution_mode;

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
        int  getTuningValue(int, int);
        void insertExtraInfo( list<MetaProperty> *properties, double& cpuEnergy, double& time, double& nodeEnergy, INT64& instances,
                              std::map<std::string, double>& extraInfo, double& totInstr );

    private:
        int min_freq;
        int max_freq;
        int freq_step;
        int scenario_no_atp;
        std::unordered_map<std::string, int> optimum_atp_map;

        static double objectiveFunction_Energy( int scenario_id,
                                                ScenarioResultsPool* );

};

#endif
