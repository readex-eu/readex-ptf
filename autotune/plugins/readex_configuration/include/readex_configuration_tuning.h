/**
   @file    readex_configuration_tuning.h
   @ingroup readex_configuration_tuning
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
   @defgroup readex_configuration_tuning readex_configuration_tuning
   @ingroup AutotunePlugins
 */

#ifndef readex_configuration_tuning_PLUGIN_H_
#define readex_configuration_tuning_PLUGIN_H_

#include "AutotunePlugin.h"
#include "AppConfigParameter.h"
// uncomment the line below if your plugin will load search algorithms
//#include "ISearchAlgorithm.h"

class readexConfigurationTuningPlugin : public IPlugin {
    vector< AppConfigParameter* >   tuningParameters;
    ISearchAlgorithm*               searchAlgorithm;
    VariantSpace                    variantSpace;
    SearchSpace                     searchSpace;
    string                          searchMode;
    int                             tpID;
    map< std::string, std::string > inputFile;
    ObjectiveFunction*              objective;


    vector<TuningParameter*>getTuningParameters();
public:
    string search_algorithm;
    string objectiveName;
    int    individual_keep;
    int    sample_count;
    int    population_size;
    string results_file;

    void parseConfigFile();


    readexConfigurationTuningPlugin() : tpID( 0 ) {
     }

    void addTP( AppConfigParameter* tp );
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

    void addTemplateAndInputFile( string templateStr,
                                  string fileStr ) {
        inputFile[ templateStr ] = fileStr;
    }

    void copyTemplateFilesToInputFiles();


    void setSearchAlgorithm( string str ) {
        search_algorithm = str;
    }

    void setIndividualKeep( int i ) {
        individual_keep = i;
    }

    void setSampleCount( int r ) {
        sample_count = r;
    }

    void setGDE3PopulationSize( int r ) {
        population_size = r;
    }


    void setResultsFile( string str ) {
        results_file = str;
    }

    void setObjectiveName( string str ) {
        objectiveName = str;
    }

    string getObjectiveName() {
        return( objectiveName );
    }
};

void parseConfig( const char*                      filename,
                  readexConfigurationTuningPlugin* plugin );
#endif
