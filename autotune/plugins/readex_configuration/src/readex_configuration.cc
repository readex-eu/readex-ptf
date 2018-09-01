/**
   @file    readex_configuration.cc
   @ingroup readexConfigurationTuningPlugin
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

#include "readex_configuration_tuning.h"
#include "AppConfigParameter.h"
#include "AutotunePlugin.h"
#include <stdio.h>

#include "AutotuneSearchAlgorithm.h"
#include "DriverContext.h"
#include "TuningDatabase.h"
#include "frontend.h"


#include "IndividualSearch.h"
#include "RandomSearch.h"
#include "GDE3Search.h"


void
parse_opts( int   argc,
            char* argv[] );

static const struct option long_opts[] = {
    { "readex-app-config", required_argument, 0, 'W' },
    0
};

static bool  has_acp_config         = false;
static char* acp_config_file_string = NULL;
vector<std::list<TuningSpecification*>*> tuningSpecs;


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
            has_acp_config = true;
            int size = strlen( optarg ) + 1;
            acp_config_file_string = ( char* )malloc( size * sizeof( char ) );
            strcpy( acp_config_file_string, optarg );
        }
        break;
        default:
            psc_errmsg( "Unrecognized option passed to the readex application configuration tuning plugin.\n" );
            break;
        }
    }
}

void readexConfigurationTuningPlugin::copyTemplateFilesToInputFiles() {
    for( const auto& input : inputFile ) {
        stringstream cmd;
        int          retVal;

        string templateFilename = input.first;
        string inputFilename    = input.second;
        cmd << "cp " << templateFilename << " " << inputFilename;
        string cmdString = cmd.str();
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: command for copying template: %s\n", cmdString.c_str() );
        retVal = system( cmdString.c_str() );
        if ( retVal != 0 ) {
            psc_errmsg( "-----------------------------------------------------------\n" );
            psc_errmsg( "Fatal: The copying of template file FAILED! Analysis will be terminated.\n" );
            fe->quit();
            abort();
        }
    }
}





/**
 * @brief Adds a tuning parameter to the TP list.
 * @ingroup CompilerFlagsPlugin
 *
 * Each compiler flag specified in the configuration file is a tuning parameter.
 *
 * @param tp New tuning parameter.
 **/
void readexConfigurationTuningPlugin::addTP( AppConfigParameter* tp ) {
    tuningParameters.push_back( tp );
    tp->setId( tpID );
    tpID++;
}



/**
 * @brief Initialize the plugin's data structures.
 *
 * The tuning parameter list needs to be created.
 *
 * Search algorithms are loaded here when required. This can be done as follows:
 *
 *    searchAlgorithm = loadSearchAlgorithm("name");
 *
 * where "name" refers to one of the available search algorithms (currently only exhaustive).
 *
 * @ingroup readexConfigurationTuningPlugin
 *
 * @param context a pointer to a context for a plugin
 * @param pool_set a pointer to a set of pools for a plugin
 */
void readexConfigurationTuningPlugin::initialize( DriverContext*   context,
                                                  ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: call to initialize()\n" );
    parse_opts( context->getArgc(), context->getArgv() );

    this->context  = context;
    this->pool_set = pool_set;

    individual_keep   = 1;
    sample_count      = 2;
    population_size   = 0;
    results_file      = "readex_configuration_parameter_result.txt";
    search_algorithm  = "exhaustive";
    objectiveName     = "Energy";


    std::string configFilename;


    if( has_acp_config ) {
        configFilename = acp_config_file_string;
    }
    else {
        configFilename = "appConfigParams.cfg";
    }

    parseConfig( configFilename.c_str(), this );


     if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ) ) > 0 ) {
         printf( "\nAll created tuning parameters\n=======================\n" );
         for( const auto& tuningParameter : tuningParameters ) {
             string str = tuningParameter->toString();
             cout << str <<  "=====================" << endl;
         }
     }

     if( has_acp_config ) {
         psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Readex Configuration Parameter Plugin: Parsed %s:\n", acp_config_file_string );
     }
     else {
         psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Readex Configuration Parameter Plugin: appConfigParams.cfg:\n" );
     }

     psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Readex Configuration Parameter Plugin: \tsearch-algorithm\n" );
     psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Readex Configuration Parameter Plugin: \t\talgorithm: %s\n", search_algorithm.c_str() );
     psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Readex Configuration Parameter Plugin: \t\tElements to keep in individual: %d\n", individual_keep );
     psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Readex Configuration Parameter Plugin: \t\tAmount of random samples: %d\n", sample_count );
     psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Readex Configuration Parameter Plugin: \t\tGDE3 population size: %d\n", population_size );
     psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Readex Configuration Parameter Plugin: \t\tresults_file: %s\n", results_file.c_str() );
     psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Readex Configuration Parameter Plugin: \t\tobjective: %s\n", objectiveName.c_str() );

     if( tuningParameters.empty() ) {
         psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                     "Readex Configuration Parameter Plugin: No tuning parameters found. Exiting.\n" );
         throw PTF_PLUGIN_ERROR( TUNING_PARAMETERS_NOT_FOUND );
     }
     psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Readex Configuration Parameter Plugin: obtain getSearchInstance\n" );
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
         IndividualSearch* alg = reinterpret_cast< IndividualSearch* >( searchAlgorithm );
         alg->setScenariosToKeep( individual_keep );
         for( const auto& tuningParameter : tuningParameters ) {
             tuningParameter->removeEmptyValues();
             //tuningParameter->toString();
         }
     }
     else if( search_algorithm.compare( "random" ) == 0 ) {
         RandomSearch* alg = reinterpret_cast< RandomSearch* >( searchAlgorithm );
         alg->setSampleCount( sample_count );
     }
     else if( search_algorithm.compare( "gde3" ) == 0 ) {
         GDE3Search* alg = reinterpret_cast< GDE3Search* >( searchAlgorithm );
         if( population_size != 0 ) {
             alg->setPopulationSize( population_size );
         }
     }

     std::string timeUnit;
     std::string energyUnit;
     std::string currencyUnit;

     //Read objectives
     try {
         if (opts.has_configurationfile) {
             timeUnit = configTree.get < std::string > ("Configuration.periscope.metricUnits.timeUnit");
             energyUnit = configTree.get < std::string > ("Configuration.periscope.metricUnits.energyUnit");
             currencyUnit = configTree.get < std::string > ("Configuration.periscope.metricUnits.currencyUnit");
         }

     } catch (exception &e) {
         psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexConfigurationPlugin: %s\n", e.what());
     }

     std::string s = objectiveName;
     if (s == "Energy") {
         std::string unit = energyUnit;
         searchAlgorithm->addObjectiveFunction(new EnergyObjective(energyUnit));
         objective=new EnergyObjective(energyUnit);
       } else if (s == "NormalizedEnergy") {
           std::string unit = energyUnit.append("/instr");
           searchAlgorithm->addObjectiveFunction(new NormalizedEnergyObjective(unit));
           objective=new NormalizedEnergyObjective(unit);
       } else if (s == "CPUEnergy") {
           std::string unit = energyUnit;
           searchAlgorithm->addObjectiveFunction(new CPUEnergyObjective(unit));
           objective=new CPUEnergyObjective(unit);
       } else if (s == "NormalizedCPUEnergy") {
           std::string unit = energyUnit.append("/instr");
           searchAlgorithm->addObjectiveFunction(new NormalizedCPUEnergyObjective(unit));
           objective=new NormalizedCPUEnergyObjective(unit);
       } else if (s == "EDP") {
           std::string unit = energyUnit.append("*").append(timeUnit);
           searchAlgorithm->addObjectiveFunction(new EDPObjective(unit));
           objective=new EDPObjective(unit);
       } else if (s == "NormalizedEDP") {
           std::string unit = energyUnit.append("*").append(timeUnit).append("/instr");
           searchAlgorithm->addObjectiveFunction(new NormalizedEDPObjective(unit));
           objective=new NormalizedEDPObjective(unit);
       } else if (s == "ED2P") {
           std::string unit = energyUnit.append("*").append(timeUnit).append("2");
           searchAlgorithm->addObjectiveFunction(new ED2PObjective(unit));
           objective=new ED2PObjective(unit);
       } else if (s == "NormalizedED2P") {
           std::string unit = energyUnit.append("*").append(timeUnit).append("2").append("/instr");
           searchAlgorithm->addObjectiveFunction(new NormalizedED2PObjective(unit));
           objective=new NormalizedED2PObjective(unit);
       } else if (s == "Time") {
           std::string unit = timeUnit;
           searchAlgorithm->addObjectiveFunction(new TimeObjective(unit));
           objective=new TimeObjective(unit);
       } else if (s == "NormalizedTime") {
           std::string unit = timeUnit.append("/instr");
           searchAlgorithm->addObjectiveFunction(new NormalizedTimeObjective(unit));
           objective=new NormalizedTimeObjective(unit);
       } else if (s == "TCO") {
           std::string unit = currencyUnit;
           searchAlgorithm->addObjectiveFunction(new TCOObjective(unit));
           objective=new TCOObjective(unit);
       } else if (s == "NormalizedTCO") {
           std::string unit = currencyUnit.append("/instr");
           searchAlgorithm->addObjectiveFunction(new NormalizedTCOObjective(unit));
           objective=new NormalizedTCOObjective(unit);
       } else {
         psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                     "Unknown objective %s.\n", objectiveName.c_str());
         throw PTF_PLUGIN_ERROR( UNDEFINED_OBJECTIVE );
     }
 }

/**
 * @brief Operations to be done at the start of a tuning step.
 *
 * The tuning parameters that were created at initialization are used to create a variant space.
 * The variant space and the regions are then used to create a search space that is passed to the
 * search algorithm.
 *
 * @ingroup readexConfigurationTuningPlugin
 *
 */
void readexConfigurationTuningPlugin::startTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: call to startTuningStep()\n" );

    searchAlgorithm->clear();
    variantSpace.clear();
    pool_set->csp->clear();
    pool_set->psp->clear();
    pool_set->esp->clear();

    for( const auto& tuningParameter : tuningParameters ) {
        variantSpace.addTuningParameter( tuningParameter );
    }
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: Create a SearchSpace from the tuning parameters.\n" );
    searchSpace.setVariantSpace( &variantSpace );

    std::list< Rts* > rtsList = appl->getCalltreeRoot()->getValidRtsOfRegion( appl->get_phase_region(), NULL );
    if ( rtsList.empty() ) {
        psc_abort( "ReadexConfigurationTuningPlugin: No rts for phase\n" );
    } else {
        for ( const auto& rts : rtsList ) {
            searchSpace.addRts( rts );
        }
    }

    //searchSpace.addRts( appl->get_phase_region() );
    searchAlgorithm->addSearchSpace( &searchSpace );
}

/**
 * @brief Defines a pre-analysis strategy.
 *
 * Found properties of the pre-analysis strategy are stored in the arp.
 *
 * @ingroup readexConfigurationTuningPlugin
 *
 * @return true if pre-analysis is required false otherwise
 *
 */
bool readexConfigurationTuningPlugin::analysisRequired( StrategyRequest** strategy ) {
    return false;
}

/**
 * @brief The Created Scenario Pool (csp) is populated here.
 *
 * The scenarios need to be created and added to the first pool. To create the scenarios, a
 * search algorithm can be used or it can be done directly by the plugin.
 *
 * After this step, the Periscope will verify that scenarios were added to the csp.
 *
 * @ingroup readexConfigurationTuningPlugin
 *
 */
void readexConfigurationTuningPlugin::createScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: call to createScenarios()\n" );

    if( searchAlgorithm == NULL ) {
        perror( "Search algorithm not instantiated\n" );
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }
    searchAlgorithm->createScenarios();
    //pool_set->csp->print();
}

/**
 * @brief Preparatory steps for the scenarios are done here.
 *
 * If there are any preparatory steps required by some or all scenarios in the csp (for example:
 * the project may need to be re-compiled), then they are to be performed here. After each
 * scenario is prepared, they are migrated from the csp to the Prepared Scenario Pool (psp).
 *
 * In some cases, no preparation may be necessary and the plugin can simply move all scenarios
 * from the csp to the psp.
 *
 * After this step, the Periscope will verify that scenarios were added to the psp.
 *
 * @ingroup readexConfigurationTuningPlugin
 *
 */
void readexConfigurationTuningPlugin::prepareScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: call to prepareScenarios()\n" );
    int  retVal = 0;

    if( pool_set->csp->empty() ) {
        return;
    }

    Scenario* scenario = pool_set->csp->pop();
    copyTemplateFilesToInputFiles();

    vector< bool > doneTP;
    for( size_t i = 0; i < tuningParameters.size(); i++ ) {
        doneTP.push_back( false );
    }

    const Variant* v = scenario->getTuningSpecifications()->front()->getVariant();
    map< TuningParameter*, int > values = v->getValue();
    for( const auto& value : values ) {
        int i = value.second;
        if( AppConfigParameter* tp = dynamic_cast< AppConfigParameter* >( value.first ) ) {
            doneTP[ tp->getId() ] = true;
            stringstream cmd;
            cmd << PERISCOPE_PLUGINS_DIRECTORY << "/readex_configuration/set_configuration_parameter.sh " << tp->getfilePath() << " \"" << tp->getName() << "\" \"" << *(tp->getValueString(i)) << "\"";
            string cmdString = cmd.str();
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: command for setting ACP: %s\n", cmdString.c_str() );
            retVal = system( cmdString.c_str() );
            if ( retVal != 0 ) {
                psc_errmsg( "-----------------------------------------------------------\n" );
                psc_errmsg( "Fatal: The application configuration parameter setting has FAILED! Analysis will be terminated.\n" );
                fe->quit();
                abort();
            }
        }
    }


    for( size_t i = 0; i < tuningParameters.size(); i++ ) {
        if ( !doneTP[ i ] ) {
            AppConfigParameter* tp = dynamic_cast< AppConfigParameter* >( tuningParameters[ i ] );

            stringstream cmd;
            cmd << PERISCOPE_PLUGINS_DIRECTORY << "/readex_configuration/set_configuration_parameter.sh " << tp->getfilePath() << " \"" << tp->getName() << "\" \"" << *(tp->getValueString(1)) << "\"";
            string cmdString = cmd.str();
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: command for setting ACP: %s\n", cmdString.c_str() );
            retVal = system( cmdString.c_str() );
            if ( retVal != 0 ) {
                psc_errmsg( "-----------------------------------------------------------\n" );
                psc_errmsg( "Fatal: The application configuration parameter setting has FAILED! Analysis will be terminated.\n" );
                fe->quit();
                abort();
            }
        }
    }

    pool_set->psp->push( scenario );
    //flags_oss << "Scenario " << scenario->getID() << " flags: " << getAFLAGS( v->getValue(), true, false, false ) << endl;
}


/**
 * @brief Populate the Experiment Scenario Pool (esp) for the next experiment.
 *
 * This is the final step before the experiments are executed. Scenarios are moved from the
 * psp to the esp, depending on the number of processes and whether they can be executed
 * in parallel.
 *
 * After this step, the Periscope will verify that scenarios were added to the esp.
 *
 * @ingroup readexConfigurationTuningPlugin
 *
 * @param numprocs         Number of processes required for the experiment
 * @param analysisRequired Defines is per-experiment analysis required
 * @param strategy         Strategy request for per-experiment analysis
 *
 */
void readexConfigurationTuningPlugin::defineExperiment( int               numprocs,
                                                        bool&             analysisRequired,
                                                        StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: call to defineExperiment()\n" );

    if( !pool_set->psp->empty() ) {
        Scenario* scenario;
        scenario = pool_set->psp->pop();

        std::list<Rts*> rtsList = appl->getCalltreeRoot()->getValidRtsOfRegion( appl->get_phase_region(), NULL );
        if ( rtsList.empty() ) {
            psc_abort( "ReadexConfigurationTuningPlugin: No rts for phase\n" );
        }
        for ( const auto& rts : rtsList ) {
            scenario->setSingleTunedRtsWithPropertyALLRanks( rts, ENERGY_CONSUMPTION );
        }
        //scenario->setSingleTunedRegionWithPropertyALLRanks( appl->get_phase_region(), ENERGY_CONSUMPTION );
        //scenario->print();
        int id = scenario->getID();
        tuningSpecs.push_back( scenario->getTuningSpecifications() );
        tuningSpecs[ id ] = scenario->getTuningSpecifications();
        scenario->setTuningSpecifications( NULL );

        pool_set->esp->push( scenario );

        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: Added 1 scenario in the experiment.\n" );
    }
}

/**
 * @brief Returns does the application has to be restarted for the next experiment.
 *
 * Return true if a restart of the application is required for the next experiment,
 * false otherwise.
 *
 * @ingroup readexConfigurationTuningPlugin
 *
 * @return true if an application has to be restarted false otherwise
 *
 */
bool readexConfigurationTuningPlugin::restartRequired( std::string& env,
                                                       int&         numprocs,
                                                       std::string& command,
                                                       bool&        is_instrumented ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: call to restartRequired()\n" );

    return true; // no restart required
}

/**
 * @brief Returns the status of the current search iteration.
 *
 * Returns true if the current search iteration is finished (by either the plugin itself or as
 * a result of the search algorithm), false otherwise.
 *
 * @ingroup readexConfigurationTuningPlugin
 *
 * @return true if the plugin has finished search false otherwise
 *
 */
bool readexConfigurationTuningPlugin::searchFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: call to searchFinished()\n" );
    map<int, Scenario*>* scenarios = pool_set->fsp->getScenarios();
    for( const auto& scenario: *scenarios ) {
        Scenario* sc = scenario.second;
        sc->setTuningSpecifications( tuningSpecs[ sc->getID() ] );
    }

    return searchAlgorithm->searchFinished();
}

/**
 * @brief Final operation of a tuning step.
 *
 * If any post-processing is necessary before entering the next tuning iteration, it is to be
 * done here.
 *
 * @ingroup readexConfigurationTuningPlugin
 *
 */
void readexConfigurationTuningPlugin::finishTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: call to processResults()\n" );

}

/**
 * @brief Returns true if the plugin finished the tuning process, false otherwise.
 *
 * @ingroup readexConfigurationTuningPlugin
 *
 * @return true if the plugin has finished tuning false otherwise
 *
 */
bool readexConfigurationTuningPlugin::tuningFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: call to tuningFinished()\n" );

    return true;
}

/**
 * @brief Prints to the screen (and to a file, where necessary) the tuning advice.
 *
 * @ingroup readexConfigurationTuningPlugin
 */
Advice* readexConfigurationTuningPlugin::getAdvice( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: call to getAdvice()\n" );

    if( searchAlgorithm == NULL ) {
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }
    std::ostringstream result_oss;
    map< int, double > timeForScenario = searchAlgorithm->getSearchPath();

    copyTemplateFilesToInputFiles();

    vector< bool > doneTP;
    for( size_t i = 0; i < tuningParameters.size(); i++ ) {
        doneTP.push_back( false );
    }

    int optimum = searchAlgorithm->getOptimum();
    result_oss << "Optimum Scenario: " << optimum << endl << endl;
    const list<TuningSpecification*>* tuningSpecifications = pool_set->fsp->getTuningSpecificationByScenarioID( optimum );
    const Variant*                    v                    = tuningSpecifications->front()->getVariant();
    map< TuningParameter*, int >      values               = v->getValue();

    result_oss << "\nConfiguration Parameter settings:\n";
    for( const auto& value : values ) {
        int i = value.second;
        if( AppConfigParameter* tp = dynamic_cast< AppConfigParameter* >( value.first ) ) {
            const std::string *v = tp->getValueString( i );
            doneTP[ tp->getId() ] = true;

            result_oss << "\t" << tp->getName() << ": " << *v << "\n";

            stringstream cmd;
            cmd << PERISCOPE_PLUGINS_DIRECTORY << "/readex_configuration/set_configuration_parameter.sh " << tp->getfilePath() << " \"" << tp->getName() << "\" \"" << *v << "\"";
            string cmdString = cmd.str();
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: command for setting ACP: %s\n", cmdString.c_str() );
            const int retVal = system( cmdString.c_str() );
            if ( retVal != 0 ) {
                psc_errmsg( "-----------------------------------------------------------\n" );
                psc_errmsg( "Fatal: The application configuration parameter setting has FAILED! Analysis will be terminated.\n" );
                fe->quit();
                abort();
            }
        }
    }


    for( size_t i = 0; i < tuningParameters.size(); i++ ) {
        if ( !doneTP[ i ] ) {
            const std::string*  v  = tuningParameters[ i ]->getValueString( i );
            AppConfigParameter* tp = dynamic_cast< AppConfigParameter* >( tuningParameters[ i ] );

            result_oss << "\t" << tp->getName() << ": " << *v << "\n";

            stringstream cmd;
            cmd << PERISCOPE_PLUGINS_DIRECTORY << "/readex_configuration/set_configuration_parameter.sh " << tp->getfilePath() << " \"" << tp->getName() << "\" \"" << *(tp->getValueString(1)) << "\"";
            string cmdString = cmd.str();
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: command for setting ACP: %s\n", cmdString.c_str() );
            int retVal = system( cmdString.c_str() );
            if ( retVal != 0 ) {
                psc_errmsg( "-----------------------------------------------------------\n" );
                psc_errmsg( "Fatal: The application configuration parameter setting has FAILED! Analysis will be terminated.\n" );
                fe->quit();
                abort();
            }
        }
    }

    result_oss << "\n------------------------" << endl << endl;
    cout << result_oss.str();

    std::ofstream ofs;
    ofs.open (results_file, std::ofstream::out);

    ofs << result_oss.str();

    ofs.close();
}

/**
 * @brief Finalize the plugin normally.
 *
 * Remove any allocated memory, objects, file descriptors, etc.
 *
 * @ingroup readexConfigurationTuningPlugin
 *
 */
void readexConfigurationTuningPlugin::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: call to finalize()\n" );
}

/**
 * @brief Terminate the plugin due to error.
 *
 * Safely remove any allocated memory, objects, file descriptors, etc. This method should
 * be able to be executed safely at any point.
 *
 * @ingroup readexConfigurationTuningPlugin
 *
 */
void readexConfigurationTuningPlugin::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: call to terminate()\n" );

    if( searchAlgorithm ) {
        searchAlgorithm->finalize();
        delete searchAlgorithm;
    }
    context->unloadSearchAlgorithms();
}

/*
 * The following are C functions that belong to the Plugin Management Interface, and not to the
 * IPlugin class. Currently the interface includes information and instance creation functions.
 *
 * These are defined in PluginManagement.h .
 */

/**
 * @brief Returns an instance of this particular plugin implementation.
 *
 * Typically, a simple return with new is enough. For example:
 *
 * return a pointer to a new readexConfigurationTuningPlugin();
 *
 * @ingroup readexConfigurationTuningPlugin
 *
 * @return A pointer to a new readexConfigurationTuningPlugin
 */
IPlugin* getPluginInstance( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: call to getComponentInstance()\n" );

    return new readexConfigurationTuningPlugin();
}

/**
 * @brief Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 *
 * @ingroup readexConfigurationTuningPlugin
 *
 * @return The major plugin interface version used by the plugin
 */
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: call to getInterfaceVersionMajor()\n" );

    return READEX_CONFIGURATION_VERSION_MAJOR;
}

/**
 * @brief Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 *
 * @ingroup readexConfigurationTuningPlugin
 *
 * @return The minor plugin interface version used by the plugin
 *
 */
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: call to getInterfaceVersionMinor()\n" );

    return READEX_CONFIGURATION_VERSION_MINOR;
}

/**
 * @brief Returns a string with the name of the plugin.
 *
 * @ingroup readexConfigurationTuningPlugin
 *
 * @return Returns a string with the name of the plugin
 *
 */
std::string getName( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: call to getName()\n" );

    return "Application Configuration Plugin";
}

/**
 * @brief Returns a string with a short description of the plugin.
 *
 * @ingroup readexConfigurationTuningPlugin
 *
 * @return A string with a short description of the plugin
 *
 */
std::string getShortSummary( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "readexConfigurationTuningPlugin: call to getShortSummary()\n" );

    return "The READEX Application Configuration Plugin (ACP) determines the best selection of application configuration parameters.";
}
