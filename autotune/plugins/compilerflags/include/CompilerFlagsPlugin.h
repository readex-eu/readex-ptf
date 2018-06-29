/**
   @file    CompilerFlagsPlugin.h
   @ingroup CompilerFlagsPlugin
   @brief   Compiler Flags Plugin
   @author  Houssam Haitof, Michael Gerndt
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

/**
   @defgroup CompilerFlagsPlugin Compiler Flags Plugin
   @ingroup AutotunePlugins
 */

#ifndef COMPILERFLAGS_PLUGIN_H_
#define COMPILERFLAGS_PLUGIN_H_

#include <stdlib.h>
#include "AutotunePlugin.h"
// uncomment the line below if your plugin will load search algorithms
#include "ISearchAlgorithm.h"
#include "CFSTuningParameter.h"
#include "MetaProperty.h"
#include "ProgramID.h"
#include "ProgramSignature.h"
#include "PropertyID.h"
#include "application.h"

class SignificantRegionInProcess {
    Region* reg;
    int     proc;

public:
    SignificantRegionInProcess( Region* r,
                                int     p ) {
        reg  = r;
        proc = p;
    };
    virtual ~SignificantRegionInProcess() {
    };

    void setRegion( Region* r ) {
        reg = r;
    };
    void setProcess( int p ) {
        proc = p;
    };
    Region* getRegion() {
        return reg;
    };
    int getProcess() {
        return proc;
    };
    void print() {
        cout << "   ";
        reg->print();
        cout << "P" << proc << endl;
    }
};

class CompilerFlagsPlugin : public IPlugin {
    vector<CFSTuningParameter*>            tuningParameters;
    ISearchAlgorithm*                      searchAlgorithm;
    VariantSpace                           variantSpace;
    SearchSpace                            searchSpace;
    int                                    tpID;
    int                                    tuningStepCount;
    string                                 searchMode;
    std::list<SignificantRegionInProcess*> regInProc;
    ProgramID                              progId;
    ProgramSignature                       signature;

    vector<TuningParameter*>getTuningParameters();

    void parseConfigFile();

    bool reCompileUsingFlags( const string& AFLAGS );

    string getAFLAGS( map<TuningParameter*, int> values,
                      bool withQuotes,
                      bool withEscape,
                      bool withSemicolons );

    string makefile_path;
    string makefile_flags_variable;
    string required_flags;
    string makefile_arguments;
    string makefile_src;
    string makefile_selective;
    string file_list;
    string remote_make;
    string identity_path;
    string remote_make_machine_name;

    string search_algorithm;
    int    individual_keep;
    int    sample_count;
    int    population_size;
    int    minutes_to_search;
    string results_file;

    string machine_learning;
    bool   opencl_tuning;

    bool analysisResultProcessed;
    void processAnalysisResult();

public:
    CompilerFlagsPlugin() : tpID( 0 ), analysisResultProcessed( false ) {
    }

    void addTP( CFSTuningParameter* tp );

    void initialize( DriverContext*   context,
                     ScenarioPoolSet* pool_set );

    bool analysisRequired( StrategyRequest** strategy );

    void startTuningStep( void );

    void createScenarios( void );

    void prepareScenarios( void );

    void defineExperiment( int               numprocs,
                           bool&             analysisRequired,
                           StrategyRequest** strategy );

    bool restartRequired( string& env,
                          int&    numprocs,
                          string& command,
                          bool&   is_instrumented );

    bool searchFinished( void );

    void finishTuningStep( void );

    bool tuningFinished( void );

    Advice* getAdvice( void );

    void finalize( void );

    void terminate( void );

    void setMakefilePath( string str ) {
        makefile_path = str;
    }

    void setMakefileFlagsVariable( string str ) {
        makefile_flags_variable = str;
    }

    void setRequiredFlags( string str ) {
        required_flags = str;
    }

    void setMakefileArguments( string str ) {
        makefile_arguments = str;
    }

    void setMakefileSrc( string str ) {
        makefile_src = str;
    }

    void setMakefileSelective( string str ) {
        makefile_selective = str;
    }

    void setFileList( string str ) {
        file_list = str;
    }

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

    void setMinutesToSearch( int r ) {
        minutes_to_search = r;
    }

    void setResultsFile( string str ) {
        results_file = str;
    }

    void setMachineLearning( string str ) {
        machine_learning = str;
    }

    void setRemoteMake( string str ) {
        remote_make = str;
    }

    void setIdentityPath( string str ) {
        identity_path = str;
    }

    void setRemoteMakeMachineName( string str ) {
        remote_make_machine_name = str;
    }


    bool getMachineLearning() const;

    void setOpenCL() {
        opencl_tuning = true;
    }

    void addRoutine( string str );

    void writeCFSresults( string str );

    const int getNumParameters() const {
        return tuningParameters.size();
    }

    const int getNumValues( int i ) const {
        return ( tuningParameters.at( i )->getRangeTo() -
                 tuningParameters.at( i )->getRangeFrom() +
                 tuningParameters.at( i )->getRangeStep() ) /
               tuningParameters.at( i )->getRangeStep();
    }
};

void parseConfig( const char*          filename,
                  CompilerFlagsPlugin* plugin );

#endif
