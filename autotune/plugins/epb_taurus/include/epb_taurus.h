/**
   @file    epb_taurus.h
   @ingroup EpbTaurusPlugin
   @brief   EPB on the taurus system
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
   @defgroup EpbTaurusPlugin EpbTaurusPlugin
   @ingroup AutotunePlugins
 */

#ifndef EPB_TAURUS_PLUGIN_H_
#define EPB_TAURUS_PLUGIN_H_

#include "AutotunePlugin.h"
// uncomment the line below if your plugin will load search algorithms
//#include "ISearchAlgorithm.h"

class EpbTaurusPlugin : public IPlugin {
        vector<TuningParameter*> tuningParameters;
        ISearchAlgorithm*        searchAlgorithm;
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
    private:
        int min_epb;
        int max_epb;
        int epb_step;

};

#endif
