/**
   @file    PCAPPlugin.h
   @ingroup PCAPPlugin
   @brief   PCAP Plugin
   @author  Umbreen Sabir
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
   @defgroup PCAPPlugin OpenMP Power Capping Plugin
   @ingroup AutotunePlugins
 */

#ifndef PCAP_PLUGIN_H_
#define PCAP_PLUGIN_H_

#include "AutotunePlugin.h"
// uncomment the line below if your plugin will load search algorithms
#include "ISearchAlgorithm.h"
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost::algorithm;

class PCAPPlugin : public IPlugin {
public:
    PCAPPlugin();

    vector<TuningParameter*> tuningParameters;
    ISearchAlgorithm*        searchAlgorithm;
    Region*                  tuningRegion;
    Application&             app;
    //double (*ObjectiveFunction)(int);
    int numberOfThreads;

    vector<MetaProperty> energyOpenMP_scalability_prop;

    //Required Variables
    int              number_of_cores;
    TuningParameter* OpenMP_tp;
    int              tuningStep;

    // variables & functions for preanalysis & region selection
    std::map<string, Region*>               code_region_candidates;
    std::vector<std::pair<string, double> > regionExectimeMap;
    Region*                                 selected_region;
    bool                                    firstTuningStep;
    double                                  firstCutOff;
    double                                  secondCutOff;
    int                                     minNoRegions;
    int                                     maxNoRegions;

    void addRegionToSearchAlgorithm( Region* selectedRegion,
                                     double  execTime );

    string searchAlgorithmName;
    bool   has_pcap_config;
    string pcap_config_file;

    void
    parse_opts( int argc,
                char* argv[] );

    void parseConfigFile( void );

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

    void getTuningParameters( void );

    //AnalysisStep

    void startTuningStep1SpeedupAnalysis( void );

    void createScenarios1SpeedupAnalysis( void );

    void prepareScenarios1SpeedupAnalysis( void );

    void defineExperiment1SpeedupAnalysis( int               numprocs,
                                           bool&             analysisRequired,
                                           StrategyRequest** strategy );

    void finishTuningStep1SpeedupAnalysis( void );

    //EnergyStep

    void startTuningStep2EnergyTuning( void );

    void createScenarios2EnergyTuning( void );

    void prepareScenarios2EnergyTuning( void );

    void defineExperiment2EnergyTuning( int               numprocs,
                                        bool&             analysisRequired,
                                        StrategyRequest** strategy );

    void finishTuningStep2EnergyTuning( void );

    void writePCAPresults();

    void curveFitting();
};

#endif
