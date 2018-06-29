/**
   @file    CompilerFlagsPlugin.cc
   @ingroup CompilerFlagsPlugin
   @brief   Compiler Flags Selection Plugin
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


#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "AutotunePlugin.h"
#include "AutotuneSearchAlgorithm.h"
#include "DriverContext.h"
#include "TuningDatabase.h"
#include "frontend.h"


#include "IndividualSearch.h"
#include "RandomSearch.h"
#include "GDE3Search.h"
#include "CFSTuningParameter.h"
#include "CompilerFlagsPlugin.h"

vector<std::list<TuningSpecification*>*> tuningSpecs;


void
parse_opts( int   argc,
            char* argv[] );

static const struct option long_opts[] = {
    { "cfs-config", required_argument, 0, 'W' },
    0
};

static bool  has_cfs_config         = false;
static char* cfs_config_file_string = NULL;


void parse_opts( int   argc,
                 char* argv[] ) {
    optind = 1;
    while( optind < argc ) {
        int index = -1;

        opterr = 0;
        int result = getopt_long( argc, argv, "", long_opts, &index );

        if( result == -1 ) {
            psc_errmsg( "Error parsing command line parameters.\n" );
        }

        switch( result ) {
        case 'W':
        {
            has_cfs_config = true;
            int size = strlen( optarg ) + 1;
            cfs_config_file_string = ( char* )malloc( size * sizeof( char ) );
            strcpy( cfs_config_file_string, optarg );
        }
        break;
        default:
            psc_errmsg( "Unrecognized option passed to the compiler flags plugin.\n" );
            break;
        }
    }
}


int    bestScenario; // Currently best scenario
double bestTime;     // Currently best time
int    tpEliminated; // TP that only gave worse results
int    lastScenario; // last scenario in previous tuning step
bool   preAnalysisDone = false;

std::string result_output;
std::string searchalgorithm;

std::ostringstream flags_oss;
map<int, int>      selection;


namespace {
/*
 * Helper functions
 */
inline bool isPerformanceCounters( MetaProperty metaProperty ) {
    return atoi( metaProperty.getId().c_str() ) == PERFORMANCECOUNTERS;
}

inline bool isExecTime( MetaProperty metaProperty ) {
    return atoi( metaProperty.getId().c_str() ) == EXECTIME;
}


ProgramID calculateProgId( const std::string& makefile_src ) {
    string        command;
    char          buf[ 64 ];
    int           ret;
    ostringstream ss;

    ss << PERISCOPE_PLUGINS_DIRECTORY << "/compilerflags/cfs_sha1.sh " << makefile_src;
    command = ss.str();

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                "CompilerFlagsPlugin: sending signature command: %s ...\n",
                command.c_str() );

    FILE* out = popen( command.c_str(), "r" );
    if( !fscanf( out, "%s", buf ) ) {
        psc_abort( "Error: %s Unable to read from the file. Terminating...\n", __func__ );

    }

    fclose( out );

    return ProgramID( buf );
}
} /* unnamed namespace */

/**
 * @brief Adds a tuning parameter to the TP list.
 * @ingroup CompilerFlagsPlugin
 *
 * Each compiler flag specified in the configuration file is a tuning parameter.
 *
 * @param tp New tuning parameter.
 **/
void CompilerFlagsPlugin::addTP( CFSTuningParameter* tp ) {
    tuningParameters.push_back( tp );
    tp->setId( tpID );
    tpID++;
}

/**
 * @brief Creates the list of selective files from the output of profiling with the Intel compiler.
 * @ingroup CompilerFlagsPlugin
 *
 * Starts the cfs_selected_files.sh script in the application directory.
 *
 **/

vector<string> getSelectedFilesToCompile() {
    vector<string> files;
    char           cmnd[ 1000 ];
    sprintf( cmnd, "%s/compilerflags/cfs_selected_files.sh", PERISCOPE_PLUGINS_DIRECTORY );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                "%s #################### <---------------> ######################\n", cmnd );
    int retVal = system( cmnd );

    return files;
}

void CompilerFlagsPlugin::addRoutine( string str ) {
    //search routine in the list of collected regions
    SignificantRegionInProcess* regionInfo;
    Region*                     r;

    r = appl->get_subroutine( str );
    if( r == NULL ) {
        psc_errmsg( "CFS Plugin: Region %s not found\n", str.c_str() );
    }
    else {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CFS Plugin: Significant region %s found\n", str.c_str() );
        regionInfo = new SignificantRegionInProcess( r, 0 );
        //add region and process 0 to regInProc
        regInProc.push_back( regionInfo );
    }
}


/**
 * @brief Initializes the CFS tuning plugin.
 * @ingroup CompilerFlagsPlugin
 *
 * It initializes the plugin from the default compiler configuration file and an application-specific configuration file.
 * If the application-specific file is missing or has not compiler specification, the default compiler configuration is omitted.
 *
 * @param context Context.
 * @param pool_set Set of Scenario Pools.
 **/
void CompilerFlagsPlugin::initialize( DriverContext*   context,
                                      ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: call to initialize()\n" );
    parse_opts( context->getArgc(), context->getArgv() );

    this->context  = context;
    this->pool_set = pool_set;

    individual_keep   = 1;
    sample_count      = 2;
    minutes_to_search = 0;
    population_size   = 0;
    results_file      = "cfs_results.txt";
    search_algorithm  = "exhaustive";


    std::string configFilename;
    preAnalysisDone = false;


    if( has_cfs_config ) {
        configFilename = cfs_config_file_string;
    }
    else {
        configFilename = "cfs_config.cfg";
    }

    FILE* fp;
    char  line[ 2000 ];
    char  command[ 2000 ];

    sprintf( command, "fgrep -i \"compiler\" %s", configFilename.c_str() );

    fp = popen( command, "r" );
    if( fp == NULL ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                    "CompilerFlagsPlugin: Failed to run command. Exiting.\n" );
        throw 0;
    }

    if( fgets( line, sizeof( line ) - 1, fp ) != NULL ) {
        //printf("%s", line); fflush(stdout);

        int start = -1, stop = -1;
        for( int i = 0; i < strlen( line ); i++ ) {
            if( line[ i ] == '\"' ) {
                if( start == -1 ) {
                    start = i;
                }
                else {
                    stop = i;
                }
            }
            else if( line[ i ] == '/' && line[ i + 1 ] == '/' ) {
                break;
            }
        }

        if( !( start < 0 || stop < 0 ) ) {
            char* compilerName = ( char* )calloc( 2000, 1 );
            strncpy( compilerName, &line[ start + 1 ], stop - start - 1 );
            //printf("compiler name= >>%s<<", compilerName); fflush(stdout);

            char compilerFilename[ 2000 ];
            sprintf( compilerFilename, "%s/templates/cfs_%s.cfg", PERISCOPE_INSTALL_DIRECTORY, compilerName );
            parseConfig( compilerFilename, this );
        }
    }

    pclose( fp );

    parseConfig( configFilename.c_str(), this );


    if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ) ) > 0 ) {
        printf( "\nAll created tuning parameters\n=======================\n" );
        for( int i = 0; i < tuningParameters.size(); i++ ) {
            string str = tuningParameters[ i ]->toString();
            cout << str <<  "=====================" << endl;
        }
    }


    if( has_cfs_config ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: Parsed %s:\n", cfs_config_file_string );
    }
    else {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: Parsed cfs_config.cfg:\n" );
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: \tmakefile\n" );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: \t\tpath: %s\n", makefile_path.c_str() );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: \t\tremote make: %s\n", remote_make.c_str() );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: \t\tidentity path: %s\n", identity_path.c_str() );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: \t\tremote make machine name: %s\n", remote_make_machine_name.c_str() );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: \t\tflags_variable: %s\n", makefile_flags_variable.c_str() );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: \t\trequired_flags: %s\n", required_flags.c_str() );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: \t\targuments: %s\n", makefile_arguments.c_str() );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: \t\tsource folder: %s\n", makefile_src.c_str() );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: \t\tselective compilation: %s\n", makefile_selective.c_str() );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: \t\t\tselected files: %s\n", file_list.c_str() );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: \tsearch-algorithm\n" );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: \t\talgorithm: %s\n", search_algorithm.c_str() );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: \t\tElements to keep in individual: %d\n", individual_keep );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: \t\tAmount of random samples: %d\n", sample_count );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: \t\tGDE3 population size: %d\n", population_size );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: \t\tminutes to search: %d\n", minutes_to_search );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: \t\tresults_file: %s\n", results_file.c_str() );

    if( makefile_selective.compare( "" ) != 0 ) {
        if( file_list.compare( "" ) != 0 ) {
            ofstream results_file;
            results_file.open( "files2touch" );
            results_file << file_list << endl;
            results_file.close();
        }
        else {
            //getSelectedFilesToCompile();
        }
    }

    if( tuningParameters.empty() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                    "CompilerFlagsPlugin: No tuning parameters found. Exiting.\n" );
        throw PTF_PLUGIN_ERROR( TUNING_PARAMETERS_NOT_FOUND );
    }
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: obtain getSearchInstance\n" );
    int    major, minor;
    string name, description;

    context->loadSearchAlgorithm( search_algorithm, &major, &minor, &name, &description );
    searchAlgorithm = context->getSearchAlgorithmInstance( search_algorithm );

    if( searchAlgorithm != NULL ) {
        print_loaded_search( major, minor, name, description );
        searchAlgorithm->initialize( context, pool_set );
    }
    else {
        perror( "NULL pointer in searchAlgorithm\n" );
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }

    if( search_algorithm.compare( "individual" ) == 0 ) {
        IndividualSearch* alg = reinterpret_cast<IndividualSearch*>( searchAlgorithm );
        alg->setScenariosToKeep( individual_keep );
        for( int i = 0; i < tuningParameters.size(); i++ ) {
            tuningParameters[ i ]->removeEmptyValues();
            //tuningParameters[i]->toString();
        }
    }
    else if( search_algorithm.compare( "random" ) == 0 ) {
        RandomSearch* alg = reinterpret_cast<RandomSearch*>( searchAlgorithm );
        alg->setSampleCount( sample_count );
    }
    else if( search_algorithm.compare( "gde3" ) == 0 ) {
        GDE3Search* alg = reinterpret_cast<GDE3Search*>( searchAlgorithm );
        if( population_size != 0 ) {
            alg->setPopulationSize( population_size );
        }
        if( minutes_to_search != 0 ) {
            alg->setTimer( ( double )minutes_to_search * 60 );
        }
    }
}

bool intelProfiling = false;


/**
 * @brief For the CFS plugin, no pre-analysis is required.
 * @ingroup CompilerFlagsPlugin
 *
 *
 * @return FALSE.
 **/
bool CompilerFlagsPlugin::analysisRequired( StrategyRequest** strategy ) {
    FILE* fp  = popen( "ls loop_prof_funcs_* 2> /dev/null", "r" );
    char* str = ( char* )calloc( 2000, 1 );

    if( !fscanf( fp, "%s", str ) ) {
        psc_abort( "Error: %s Unable to read from the file. Terminating...\n", __func__ );
    }

    pclose( fp );

    if( strlen( str ) > 5 ) {
        intelProfiling = true;
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Intel Profiling\n" );
    }
    else {
        intelProfiling = false;
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "No Intel Profiling\n" );
    }

    if( !getMachineLearning() && makefile_selective == "true" && file_list.empty() && !intelProfiling && !preAnalysisDone ) {
        StrategyRequestGeneralInfo* analysisStrategyGeneralInfo = new StrategyRequestGeneralInfo;
        analysisStrategyGeneralInfo->strategy_name     = "Importance";
        analysisStrategyGeneralInfo->pedantic          = 0;
        analysisStrategyGeneralInfo->delay_phases      = 0;
        analysisStrategyGeneralInfo->delay_seconds     = 0;
        analysisStrategyGeneralInfo->analysis_duration = 1;

        StrategyRequest* analysisStrategy = new StrategyRequest( analysisStrategyGeneralInfo );

        if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ) ) ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Strategy requests in the plugin:\n" );
            analysisStrategy->printStrategyRequest();
        }
        *strategy = analysisStrategy;

        return true;
    }
    else if( makefile_selective == "true" && file_list.empty() && intelProfiling ) {
        printf( "Getting files from Intel profile.\n" );

        getSelectedFilesToCompile();
        // Intel profiling needs no pre-analysis request, so it can be combined with machine learning.
    }

    if( getMachineLearning() ) {
        // recompile with -O1, so that PAPI counters are always gathered
        // with the same compiler flags
        reCompileUsingFlags( std::string( "\"-O1 " ) + required_flags + "\"" );

        // calculate SHA1 of (hopefully) all source code as Program ID
        // (reCompileUsingFlags may change makefile_src, so the order is significant here)
        progId = calculateProgId( makefile_src );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: ProgramID: %s\n", progId.toString().c_str() );

        StrategyRequestGeneralInfo* generalInfo = new StrategyRequestGeneralInfo;
        generalInfo->strategy_name     = "ConfigAnalysis";
        generalInfo->pedantic          = true;
        generalInfo->delay_phases      = 0;
        generalInfo->delay_seconds     = 0;
        generalInfo->analysis_duration = 1;

        PropertyRequest* request = new PropertyRequest;
        request->addPropertyID( PERFORMANCECOUNTERS );
        request->addRegion( appl->get_phase_region() );
        request->addAllProcesses();

        std::list<PropertyRequest*>* requests = new std::list<PropertyRequest*>;
        requests->push_back( request );

        *strategy = new StrategyRequest( requests, generalInfo );
        return true;
    }

    return false;
}

/**
 * @brief Starts a tuning step by creating the search space from all tuning parameters.
 * @ingroup CompilerFlagsPlugin
 *
 * The search space is create from the tuning parameters and added to the search algorithm.
 *
 **/
void CompilerFlagsPlugin::startTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: call to startTuningStep()\n" );

    searchAlgorithm->clear();
    selection.clear();
    variantSpace.clear();
    pool_set->csp->clear();
    pool_set->psp->clear();
    pool_set->esp->clear();

    for( int i = 0; i < tuningParameters.size(); i++ ) {
        variantSpace.addTuningParameter( tuningParameters[ i ] );
    }
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: Create a SearchSpace from the tuning parameters.\n" );
    searchSpace.setVariantSpace( &variantSpace );
    searchSpace.addRegion( new Region() );
    // adding search space is moved to processAnalysisResult()
}

/**
 * @brief Processes the result of the pre-analysis.
 * @ingroup CompilerFlagsPlugin
 *
 * This function is called from createScenarios, because that is the earliest
 * stage when the analysis result is available. However, we do not want to do
 * this processing at every step when using a multi-step search algorithm.
 *
 * Therefore this function is a no-op on every call, except for the first one.
 */
void CompilerFlagsPlugin::processAnalysisResult() {
    if( analysisResultProcessed ) {
        return;
    }

    if( getMachineLearning() ) {
        addInfoType addInfo; ///< additional information from PerformanceCounters property

        // find PerformanceCounters property and write into addInfo
        map<int, list<MetaProperty> > properties = pool_set->arp->getAllPreAnalysisProperties();
        for( map<int, list<MetaProperty> >::iterator i = properties.begin(); i != properties.end(); i++ ) {
            list<MetaProperty>& metaProperties = ( *i ).second;

            list<MetaProperty>::iterator j;
            j = find_if( metaProperties.begin(), metaProperties.end(), &isPerformanceCounters );
            if( j != metaProperties.end() ) {
                addInfo = ( *j ).getExtraInfo();
            }
        }

        signature = ProgramSignature( addInfo );

        // Add search space with program signature for the case,
        // when searchAlgorithm is capable of using machine learning.
        searchAlgorithm->addSearchSpaceWithSignature( &searchSpace, signature );
    }
    else {
        if( makefile_selective.compare( "" ) != 0 && file_list.compare( "" ) == 0 && !intelProfiling && !preAnalysisDone ) {
            ofstream results_file;
            results_file.open( "files2touch" );

            //Go over found properties
            std::list < std::string >                     files;
            int                                           propertyCount = 0;
            map<int, list<MetaProperty> >                 preAnalysisPropertiesMap;
            map<int, list<MetaProperty> >                 perExperimentPropertiesMap;
            map<int, list<MetaProperty> >::const_iterator propertyMapIter;
            list<MetaProperty>                            properties;
            list<MetaProperty>::iterator                  property;

            preAnalysisPropertiesMap = pool_set->arp->getAllPreAnalysisProperties();
            for( propertyMapIter = preAnalysisPropertiesMap.begin(); propertyMapIter != preAnalysisPropertiesMap.end(); propertyMapIter++ ) {
                properties = propertyMapIter->second;
                for( property = properties.begin(); property != properties.end(); property++, propertyCount++ ) {
                    files.push_back( property->getFileName() );
                } // for property
            }     // for propertyMapIter

            files.sort();
            files.unique();
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Files found in pre-analysis properties" );
            std::list<std::string>::const_iterator file_it;
            for( file_it = files.begin(); file_it != files.end(); file_it++ ) {
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), " %s\n ", ( *file_it ).c_str() );
                results_file << ( *file_it ) << endl;
            }
            results_file.close();
            file_list = boost::algorithm::join( files, " " );

            for( file_it = files.begin(); file_it != files.end(); file_it++ ) {
                properties = pool_set->arp->getAllPreAnalysisProperties().begin()->second;
                Region* bestRegion   = NULL;
                int     bestProcess  = 0;
                double  bestSeverity = 0.0;

                for( property = properties.begin(); property != properties.end(); property++, propertyCount++ ) {
                    if( property->getFileName().compare( ( *file_it ) ) == 0 ) {
                        double severity = property->getSeverity();
                        if( severity > bestSeverity ) {
                            Region* reg = appl->searchRegion( property->getRegionId() );
                            bestRegion   = reg;
                            bestSeverity = severity;
                            bestProcess  = property->getProcess();
                        }
                    }
                } // for property
                SignificantRegionInProcess* info = new SignificantRegionInProcess( bestRegion, bestProcess );
                regInProc.push_back( info );
            }

            cout << "Significant region in process:" << endl;
            std::list<SignificantRegionInProcess*>::iterator r;
            for( r = regInProc.begin(); r != regInProc.end(); r++ ) {
                ( *r )->print();
            }
            cout << endl;

            preAnalysisDone = true;
        }

        searchAlgorithm->addSearchSpace( &searchSpace );
    }

    // future calls to this function will do nothing
    analysisResultProcessed = true;
}

/**
 * @brief Creates the scenarios based on the search algorithm.
 * @ingroup CompilerFlagsPlugin
 **/
void CompilerFlagsPlugin::createScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: call to createScenarios()\n" );

    processAnalysisResult();

    if( searchAlgorithm == NULL ) {
        perror( "Search algorithm not instantiated\n" );
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }
    searchAlgorithm->createScenarios();
}

/**
 * @brief Prepares scenario by recompiling the selected files.
 * @ingroup CompilerFlagsPlugin
 *
 * The variant specified in the scenario is used to compile the application with the appropriate flags.
 *
 **/
void CompilerFlagsPlugin::prepareScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: call to prepareScenarios()\n" );


    //For the CSF plugin, we only expect 1 scenario at a time.
    string AFLAGS = "";
    flags_oss.clear();
    if( !pool_set->csp->empty() ) {
        Scenario* scenario;
        scenario = pool_set->csp->pop();
        const Variant* v = scenario->getTuningSpecifications()->front()->getVariant();

        if( remote_make.compare( "true" ) == 0 ) {
            AFLAGS = getAFLAGS( v->getValue(), true, true, false );
        }
        else {
            AFLAGS = getAFLAGS( v->getValue(), true, false, false );
        }
        pool_set->psp->push( scenario );
        flags_oss << "Scenario " << scenario->getID() << " flags: " << getAFLAGS( v->getValue(), true, false, false ) << endl;

        if( !reCompileUsingFlags( AFLAGS ) ) {
            psc_errmsg( "-----------------------------------------------------------\n" );
            psc_errmsg( "Fatal: The re-compilation has FAILED! Analysis will be terminated.\n" );
            psc_errmsg( "HINT: Check for possible compilation errors above.\n" );
            fe->quit();
            abort();
        }
    }
}


/**
 * @brief Defines an experiment based on a single scenario.
 * @ingroup CompilerFlagsPlugin
 *
 * The recompiled application is executed. No analysis strategy is applied.
 * @param numprocs         Not used.
 * @param analysisRequired Set to false.
 * @param strategy         No analysis strategy requested.
 **/
void CompilerFlagsPlugin::defineExperiment( int               numprocs,
                                            bool&             analysisRequired,
                                            StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: call to defineExperiment()\n" );
    Scenario* scenario;

    if( !pool_set->psp->empty() ) {
        scenario = pool_set->psp->pop();
        const list<TuningSpecification*>* ts = scenario->getTuningSpecifications();
        if( ts->size() != 1 ) {
            perror( "CompilerFlagsPlugin: can't currently handle multiple TuningSpecifications\n" );
            throw 0;
        }
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "setting single rank (%d) in scenario with id: %d\n", 0, scenario->getID() );

        if( context->applUninstrumented() ) {
            scenario->setSingleTunedRegionWithPropertyRank( NULL, EXECTIME, 0 );
        }
        else {
            scenario->setSingleTunedRegionWithPropertyRank( appl->get_phase_region(), EXECTIME, 0 );
        }

        //scenario->print();
        int id = scenario->getID();
        tuningSpecs.push_back( scenario->getTuningSpecifications() );
        tuningSpecs[ id ] = scenario->getTuningSpecifications();
        scenario->setTuningSpecifications( NULL );

        pool_set->esp->push( scenario );

        if( regInProc.size() > 0 ) {
            StrategyRequestGeneralInfo* analysisStrategyRequest = new StrategyRequestGeneralInfo;
            analysisStrategyRequest->strategy_name     = "ConfigAnalysis";
            analysisStrategyRequest->pedantic          = 1;
            analysisStrategyRequest->delay_phases      = 0;
            analysisStrategyRequest->delay_seconds     = 0;
            analysisStrategyRequest->analysis_duration = 1;

            list<PropertyRequest*>*                          reqList = new list<PropertyRequest*>;
            std::list<SignificantRegionInProcess*>::iterator r;
            for( r = regInProc.begin(); r != regInProc.end(); r++ ) {
                PropertyRequest* req = new PropertyRequest();
                req->addPropertyID( EXECTIME );
                req->addRegion( ( *r )->getRegion() );
                req->addSingleProcess( ( *r )->getProcess() );
                reqList->push_back( req );
            }

            StrategyRequest* sub_strategy = new StrategyRequest( reqList, analysisStrategyRequest );
            ( *strategy )    = sub_strategy;
            analysisRequired = true;
        }

        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: Added 1 scenario in the experiment.\n" );
    }
}

/**
 * @brief Requests a restart for the experiment.
 * @ingroup CompilerFlagsPlugin
 *
 *
 * @param env Not changed.
 * @param numprocs Not changed.
 * @param command Not changed.
 * @param is_instrumented Not changed.
 *
 * @return True.
 **/
bool CompilerFlagsPlugin::restartRequired( std::string& env,
                                           int&         numprocs,
                                           std::string& command,
                                           bool&        is_instrumented ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: call to restartRequired()\n" );
    is_instrumented = !context->applUninstrumented();
    return true;
}

/**
 * @brief Returns whether the search algorithm is finished.
 * @ingroup CompilerFlagsPlugin
 *
 * @return Value returned by the search algorithm.
 **/
bool CompilerFlagsPlugin::searchFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: call to searchFinished()\n" );

    map<int, Scenario*>::iterator scenario_iter;
    for( scenario_iter = pool_set->fsp->getScenarios()->begin(); scenario_iter != pool_set->fsp->getScenarios()->end(); scenario_iter++ ) {
        Scenario* sc = scenario_iter->second;
        sc->setTuningSpecifications( tuningSpecs[ sc->getID() ] );
    }


    return searchAlgorithm->searchFinished();
}


/**
 * @brief Empty.
 * @ingroup CompilerFlagsPlugin
 *
 *
 **/
void CompilerFlagsPlugin::finishTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: call to finishTuningStep()\n" );

    //cout << flags_oss.str() << endl;
}

/**
 * @brief Tuning is finished after one tuning step.
 * @ingroup CompilerFlagsPlugin
 *
 * @return True.
 **/
bool CompilerFlagsPlugin::tuningFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: call to tuningFinished()\n" );

    return true;
}


class SeverityScenario {
    double sev;
    int    sce;

public:
    SeverityScenario( double severity, int scenario ) {
        sev = severity;
        sce = scenario;
    }
    ~SeverityScenario() {
    };

    double getSeverity() {
        return sev;
    };
    int getScenario() {
        return sce;
    };
};



/**
 * @brief Prints which flag combination is best.
 * @ingroup CompilerFlagsPlugin
 *
 * It outputs the best flag combination and the search path.
 **/
Advice* CompilerFlagsPlugin::getAdvice( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: call to getAdvice()\n" );

    if( searchAlgorithm == NULL ) {
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }

    map<int, Scenario*>* finishedScenarios = pool_set->fsp->getScenarios();

    std::ostringstream result_oss;
    map<int, double>   timeForScenario = searchAlgorithm->getSearchPath();

    int optimum = searchAlgorithm->getOptimum();
    result_oss << "Optimum Scenario: " << optimum << endl << endl;

    result_oss << "Compiler Flags tested:" << endl;
    result_oss << flags_oss.str() << endl;

    result_oss << "\nAll Results:\n";
    result_oss << "Scenario\t|  Severity\n";

    for( int scenario_id = 0; scenario_id < pool_set->fsp->size(); scenario_id++ ) {
        result_oss << scenario_id << "\t\t|  " << ( pool_set->srp->getScenarioResultsByID( scenario_id ).front() ).getSeverity() << endl;     //printf("%-10d|\t%f\n", scenario_id,	srp->getScenarioResultsByID(scenario_id).front()->getSeverity());
    }
    result_oss << "\n------------------------" << endl << endl;


    //reset tuning specifications and sets default description in the scenario and execution time.
    map<int, Scenario*>::iterator scenario_iter;
    for( scenario_iter = pool_set->fsp->getScenarios()->begin(); scenario_iter != pool_set->fsp->getScenarios()->end(); scenario_iter++ ) {
        Scenario*         sc = scenario_iter->second;
        std::stringstream strStream;

        strStream << "Flag combination: " << getAFLAGS( tuningSpecs[ sc->getID() ]->front()->getVariant()->getValue(), false, false, false );
        sc->setDescription( strStream.str() );
        sc->setTuningSpecifications( tuningSpecs[ sc->getID() ] );
        sc->addResult( "Time", timeForScenario[ sc->getID() ] );
    }

    //Build list of best scenarios: global best and best for individual files.
    list<Scenario*> best_scenarios;

    Scenario*         globalBestScenario = ( *pool_set->fsp->getScenarios() )[ searchAlgorithm->getOptimum() ];
    std::stringstream strStream;
    strStream << "Global best combination: " << getAFLAGS( tuningSpecs[ globalBestScenario->getID() ]->front()->getVariant()->getValue(), false, false, false );
    globalBestScenario->setDescription( strStream.str() );
    best_scenarios.push_back( globalBestScenario );




    //Determine best scenario for significant files and store execution time in scenario
    if( regInProc.size() > 0 ) {
        map<Region*, SeverityScenario*>           bestSeverityScenario;
        map<Region*, SeverityScenario*>::iterator bss;

        map<int, list<MetaProperty> >                 scenarioAnalysisPropertiesMap;
        map<int, list<MetaProperty> >::const_iterator propertyMapIter;
        list<MetaProperty>                            properties;
        list<MetaProperty>::iterator                  p;

        scenarioAnalysisPropertiesMap = pool_set->arp->getAllExperimentProperties();

        int numExp = scenarioAnalysisPropertiesMap.size();
        for( int exp = 0; exp < numExp; exp++ ) {
            properties = pool_set->arp->getExperimentProperties( exp );
            for( p = properties.begin(); p != properties.end(); p++ ) {
                Region* reg = appl->searchRegion( p->getRegionId() );
                bss = bestSeverityScenario.find( reg );
                int scenarioID = atoi( p->getExtraInfo().at( "ScenarioID" ).c_str() );
                if( bss != bestSeverityScenario.end() ) {
                    if( bss->second->getSeverity() > p->getSeverity() ) {
                        SeverityScenario* s = new SeverityScenario( p->getSeverity(), scenarioID );
                        bestSeverityScenario[ reg ] = s;
                    }
                }
                else {
                    SeverityScenario* s = new SeverityScenario( p->getSeverity(), scenarioID );
                    bestSeverityScenario[ reg ] = s;
                }
                //store execution time in scenario
                Scenario*         s = ( *finishedScenarios )[ scenarioID ];
                std::stringstream strStream;
                strStream <<  reg->getFileName() << "_Time";
                s->addResult( strStream.str(), p->getSeverity() );
            }
        }

        result_oss << "Best scenarios per file" << endl;
        result_oss << "-----------------------" << endl;
        for( bss = bestSeverityScenario.begin(); bss != bestSeverityScenario.end(); bss++ ) {
            Region*   reg      = bss->first;
            double    time     = bss->second->getSeverity();
            int       scenario = bss->second->getScenario();
            Scenario* s        = ( *finishedScenarios )[ scenario ];

            result_oss <<  reg->getFileName() << "    " << scenario << "     " << time << endl;
            std::stringstream strStream;
            strStream << "Best combination for " <<  reg->getFileName() << ";" << s->getDescription();
            s->setDescription( strStream.str() );

            if( s != globalBestScenario ) {
                best_scenarios.push_back( s );
            }
        }
        result_oss << endl << endl;
    }

    best_scenarios.sort();
    best_scenarios.unique();

    cout << result_oss.str();


    writeCFSresults( result_oss.str() );
    //if( applUninstrumented() ) return NULL;
    return new Advice( getName(), best_scenarios,    searchAlgorithm->getSearchPath(), "Time", finishedScenarios );
}

/**
 * @brief Finalizes the plugin.
 * @ingroup CompilerFlagsPlugin
 *
 *
 **/
void CompilerFlagsPlugin::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: call to finalize()\n" );

    if( getMachineLearning() ) {
        // Save signature
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: Machine Learning: saving signature\n" );
        tdb->saveSignature( progId, signature );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: Machine Learning: saving signature done\n" );

        // Save cases
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: Machine Learning: saving results\n" );
        map<int, Scenario*>* scenarios = pool_set->fsp->getScenarios();
        for( map<int, Scenario*>::const_iterator it = scenarios->begin(); it != scenarios->end(); ++it ) {
            int scenarioId = ( *it ).first;

            list<MetaProperty>           metaProperties = pool_set->srp->getScenarioResultsByID( scenarioId );
            list<MetaProperty>::iterator j;

            j = std::find_if( metaProperties.begin(), metaProperties.end(), &isExecTime );
            if( j != metaProperties.end() ) {
                Variant const* variant = tuningSpecs[ scenarioId ]->front()->getVariant();
                tdb->saveTuningCase( progId, TuningCase( *variant, ( *j ).getSeverity() ) );
            }
            else {
                psc_errmsg( "CompilerFlagsPlugin: a Scenario is finished, but ExecTime property is not found.\n" );
            }
        }
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: Machine Learning: saving results done\n" );

        // Commit
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: Machine Learning: committing\n" );
        tdb->commit();
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: Machine Learning: committing done\n" );
    }

    terminate();
}

/**
 * @brief Terminates the plugin.
 * @ingroup CompilerFlagsPlugin
 *
 **/
void CompilerFlagsPlugin::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: call to terminate()\n" );

    if( searchAlgorithm ) {
        searchAlgorithm->finalize();
        delete searchAlgorithm;
    }
    context->unloadSearchAlgorithms();
}

bool CompilerFlagsPlugin::getMachineLearning() const {
    if( boost::iequals( machine_learning, "true" ) ) {
        return true;
    }
    if( boost::iequals( machine_learning, "false" ) ) {
        return false;
    }
    return /* default: */ false;
}


/**
 * @brief Create the string of flags from the variant.
 * @ingroup CompilerFlagsPlugin
 *
 *
 * @return The variant defines the flags to use. Those have to be translated into a string that can be used at compile time.
 **/
string CompilerFlagsPlugin::getAFLAGS( map<TuningParameter*, int> values,
                                       bool                       withQuotes,
                                       bool                       withEscape,
                                       bool                       withSemicolons ) {
    //psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "CompilerFlagsPlugin: Constructing %d flag(s)\n", values.size());

    string flag;
    int    pos = 0;
    if( withQuotes && withEscape ) {
        flag += "\\\"";
    }
    if( withQuotes && !withEscape ) {
        flag += "\"";
    }
    for( map<TuningParameter*, int>::const_iterator it = values.begin(); it != values.end(); ++it ) {
        int i = ( *it ).second;
        if( CFSTuningParameter* tp = dynamic_cast<CFSTuningParameter*>( ( *it ).first ) ) {
            while( tp->getId() > pos && withSemicolons ) {
                flag += ";";
                pos++;
            }
            flag += tp->getFlagWithValue( i ) + " ";
        }
    }
    if( withSemicolons ) {
        while( pos < tuningParameters.size() - 1 ) {
            flag += ";";
            pos++;
        }

        if( required_flags.compare( "" ) != 0 ) {
            flag += ";";
        }
    }
    //appending the required flags to the generated selection
    flag += required_flags;
    if( withQuotes && withEscape ) {
        flag += "\\\"";
    }
    if( withQuotes && !withEscape ) {
        flag += "\"";
    }
    // psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "%s\n", flag.c_str());
    return flag;
}


/**
 * @brief Touches the files to recompile and start make.
 * @ingroup CompilerFlagsPlugin
 *
 *
 * It either touches the selected files or all the files of the application. It then starts the make process.
 **/
bool CompilerFlagsPlugin::reCompileUsingFlags( const string& AFLAGS ) {
    char cmnd[ 1000 ];
    int  retVal = 0;

    if( !makefile_src.compare( "" ) ) {
        makefile_src = "./";
    }

    if( makefile_selective.compare( "true" ) == 0 ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: selective compilation selected.\n" );
        sprintf( cmnd, "%s/compilerflags/cfs_touchselected.sh %s", PERISCOPE_PLUGINS_DIRECTORY, makefile_src.c_str() );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: sending touch command: %s\n", cmnd );
        retVal = system( cmnd );
    }
    else {
        sprintf( cmnd, "%s/compilerflags/touchall_cfs.sh %s", PERISCOPE_PLUGINS_DIRECTORY, makefile_src.c_str() );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: sending touchall command: %s ...\n", cmnd );
        retVal = system( cmnd );
    }

    if( remote_make.compare( "true" ) == 0 && identity_path.compare( "" ) != 0 && remote_make_machine_name.compare( "" ) != 0 ) {
        sprintf( cmnd, "ssh -i %s %s make --directory=%s %s %s=%s", identity_path.c_str(), remote_make_machine_name.c_str(), makefile_path.c_str(), makefile_arguments.c_str(), makefile_flags_variable.c_str(), AFLAGS.c_str() );
    }
    else {
        sprintf( cmnd, "make --directory=%s %s %s=%s", makefile_path.c_str(), makefile_arguments.c_str(), makefile_flags_variable.c_str(), AFLAGS.c_str() );
    }

    psc_dbgmsg( 1, "CompilerFlagsPlugin: sending make command: %s\n", cmnd );
    retVal = system( cmnd );
    return retVal == 0;
}


/**
 * @brief Returns plugin instance.
 * @ingroup CompilerFlagsPlugin
 *
 *
 * @return pointer to a new plugin object.
 **/
IPlugin* getPluginInstance( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: call to getPluginInstance()\n" );

    return new CompilerFlagsPlugin();
}

/**
 * @brief Returns the major version number.
 * @ingroup CompilerFlagsPlugin
 *
 *
 * @return Major version number.
 **/
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: call to getInterfaceVersionMajor()\n" );

    return COMPILERFLAGS_VERSION_MAJOR;
}

/**
 * @brief Returns the minor version number.
 * @ingroup CompilerFlagsPlugin
 *
 *
 * @return Minor version number.
 **/
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: call to getInterfaceVersionMinor()\n" );

    return COMPILERFLAGS_VERSION_MINOR;
}

/**
 * @brief Returns plugin name.
 * @ingroup CompilerFlagsPlugin
 *
 *
 * @return Plugin name.
 **/
string getName( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: call to getName()\n" );

    return "Compiler Flag Selection Plugin";
}

/**
 * @brief Returns a short description.
 * @ingroup CompilerFlagsPlugin
 *
 *
 * @return description.
 **/
string getShortSummary( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "CompilerFlagsPlugin: call to getShortSummary()\n" );

    return "The Compiler Flags Selection Plugin (CFS) determines the best selection of compiler switches.";
}


void CompilerFlagsPlugin::writeCFSresults( string str ) {
    if( results_file.compare( "" ) != 0 ) {
        int                Num_Scenarios = pool_set->fsp->size();
        std::ostringstream resultString;

        resultString << "Scenario ID; Flags;";
        for( int i = 0; i < tuningParameters.size(); i++ ) {
            resultString << tuningParameters[ i ]->getName();
            resultString << ";";
        }

        Scenario* s = ( *pool_set->fsp->getScenarios() )[ 0 ];

        map<string, double>           results = s->getResults();
        map<string, double>::iterator r;
        //results.sort();
        for( r = results.begin(); r != results.end(); r++ ) {
            resultString << r->first << ";";
        }

        resultString << endl;

        for( int scenario_id = 0; scenario_id < Num_Scenarios; scenario_id++ ) {
            Scenario* scenario = ( *pool_set->fsp->getScenarios() )[ scenario_id ];
            resultString << scenario_id << "; ";

            string flagsString = getAFLAGS( tuningSpecs[ scenario->getID() ]->front()->getVariant()->getValue(), false, false, false );
            resultString << flagsString << ";";
            flagsString = getAFLAGS( tuningSpecs[ scenario->getID() ]->front()->getVariant()->getValue(), false, false, true );
            resultString << flagsString << ";";

            map<string, double>           results = scenario->getResults();
            map<string, double>::iterator r;
            for( r = results.begin(); r != results.end(); r++ ) {
                resultString << r->second << ";";
            }

            resultString << endl;
        }

        resultString << endl << endl << str;
        //cout << result_oss.str();
        ofstream rf;
        rf.open( results_file.c_str() );
        rf << resultString.str();
        rf.close();
    }
}
