/**
   @file    dvfs_taurus.cc
   @ingroup DvfsTaurusPlugin
   @brief   Dvfs on the taurus system
   @author  Andreas Gocht
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
    Copyright (c) 2016, Technische Universitaet Dresden, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include "../include/dvfs_taurus.h"
#include <vector>
#include <list>
#include <map>
#include <limits>

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
 * @ingroup DvfsTaurusPlugin
 *
 * @param context a pointer to a context for a plugin
 * @param pool_set a pointer to a set of pools for a plugin
 */
void DvfsTaurusPlugin::initialize( DriverContext*   context,
                                   ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: call to initialize()\n" );


    this->context = context;
    this->pool_set = pool_set;

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: got pool and context\n" );
    /*
     * parse command line options
     */
    int argc = context->getArgc();
    char **argv = context->getArgv();

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: got argc and argv\n" );

    int c;
    optind = 1; // ptf parses opts with get_opt as well. So set optind to default, to avoid trouble
    while (1)
    {
         int option_index = 0;
         static struct option long_options[] = {
             {"min_freq",   required_argument, 0,  0 },
             {"max_freq",   required_argument, 0,  1 },
             {"freq_step",  required_argument, 0,  2 },
             {0,            0,                 0,  0 }
         };

         c = getopt_long(argc, argv, "",
                  long_options, &option_index);
         if (c == -1)
             break;

         switch (c) {
         case 0:
             this->min_freq = atoi(optarg);
             break;
         case 1:
             this->max_freq = atoi(optarg);
             break;
         case 2:
             this->freq_step = atoi(optarg);
             break;
         case '?':
             break;
         default:
             break;
         }
    }
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: got options\n" );

    TuningParameter* cpuFrequnecy = new TuningParameter();
    cpuFrequnecy->setId( 0 );
    cpuFrequnecy->setName( "CPU_FREQ" );
    cpuFrequnecy->setPluginType( UNKOWN_PLUGIN );
    cpuFrequnecy->setRange( min_freq, max_freq, freq_step );
    cpuFrequnecy->setRuntimeActionType( TUNING_ACTION_FUNCTION_POINTER );
    tuningParameters.push_back(cpuFrequnecy);
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: set freq\n" );

//    std::string min_freq_str = std::to_string(min_freq);
//    std::string max_freq_str = std::to_string(max_freq);
//    std::string freq_step_str = std::to_string(freq_step);


//    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin:"
//            " min_freq: %s, max_freq: %s, freq_step: %s \n\n",
//            itoa(min_freq), itoa(max_freq), itoa(freq_step));
//                min_freq_str.c_str(), max_freq_str.c_str(), freq_step_str.c_str());

    string results = cpuFrequnecy->toString();
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin plugin: Tuning parameter is: %s\n\n",
                results.c_str() );

    int    major, minor;
    string name, description;
    context->loadSearchAlgorithm( "exhaustive", &major, &minor, &name, &description );
    searchAlgorithm = context->getSearchAlgorithmInstance( "exhaustive" );
    if( searchAlgorithm ) {
        print_loaded_search( major, minor, name, description );
        searchAlgorithm->initialize( context, pool_set );
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: initialize() finished \n\n");
}

/**
 * @brief Operations to be done at the start of a tuning step.
 *
 * The tuning parameters that were created at initialization are used to create a variant space.
 * The variant space and the regions are then used to create a search space that is passed to the
 * search algorithm.
 *
 * @ingroup DvfsTaurusPlugin
 *
 */
void DvfsTaurusPlugin::startTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: call to startTuningStep()\n" );

    VariantSpace* variantSpace = new VariantSpace();
    SearchSpace*  searchSpace  = new SearchSpace();
    for( int i = 0; i < tuningParameters.size(); i++ ) {
        variantSpace->addTuningParameter( tuningParameters[ i ] );
    }
    searchSpace->setVariantSpace( variantSpace );
    searchSpace->addRegion( appl->get_phase_region() );
    searchAlgorithm->addSearchSpace( searchSpace );

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: call to startTuningStep() finished\n" );
}

/**
 * @brief Defines a pre-analysis strategy.
 *
 * Found properties of the pre-analysis strategy are stored in the arp.
 *
 * @ingroup DvfsTaurusPlugin
 *
 * @return true if pre-analysis is required false otherwise
 *
 */
bool DvfsTaurusPlugin::analysisRequired( StrategyRequest** strategy ) {
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
 * @ingroup DvfsTaurusPlugin
 *
 */
void DvfsTaurusPlugin::createScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: call to createScenarios()\n" );

    searchAlgorithm->createScenarios();
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: call to createScenarios() finished\n" );
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
 * @ingroup DvfsTaurusPlugin
 *
 */
void DvfsTaurusPlugin::prepareScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: call to prepareScenarios()\n" );

    while( !pool_set->csp->empty() ) {
        pool_set->psp->push( pool_set->csp->pop() );
    }
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
 * @ingroup DvfsTaurusPlugin
 *
 * @param numprocs         Number of processes required for the experiment
 * @param analysisRequired Defines is per-experiment analysis required
 * @param strategy         Strategy request for per-experiment analysis
 *
 */
void DvfsTaurusPlugin::defineExperiment( int               numprocs,
                                         bool&             analysisRequired,
                                         StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: call to defineExperiment()\n" );

    Scenario* scenario = pool_set->psp->pop();
    scenario->setSingleTunedRegionWithPropertyRank( appl->get_phase_region(), HDEEM_ENERGY_CONSUMTION_BLADE, 0 );

    pool_set->esp->push( scenario );

        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: Processing significant regions in start tuning step\n" );
        code_significant_regions = appl->get_sig_regions_list();
        std::list<Region*>::iterator code_sig_regions_it;

    if( code_significant_regions.size() > 0 ) {
        StrategyRequestGeneralInfo* analysisStrategyRequest = new StrategyRequestGeneralInfo;
        analysisStrategyRequest->strategy_name     = "ConfigAnalysis";
        analysisStrategyRequest->pedantic          = 1;
        analysisStrategyRequest->delay_phases      = 0;
        analysisStrategyRequest->delay_seconds     = 0;
        analysisStrategyRequest->analysis_duration = 1;

        list<PropertyRequest*>* reqList = new list<PropertyRequest*>;

        for( code_sig_regions_it  = code_significant_regions.begin(); code_sig_regions_it != code_significant_regions.end(); code_sig_regions_it++ ) {
            PropertyRequest* req = new PropertyRequest();
            req->addPropertyID( HDEEM_ENERGY_CONSUMTION_BLADE );
            req->addRegion( ( *code_sig_regions_it ) );
            req->addSingleProcess( 0 );
            reqList->push_back( req );
        }

        StrategyRequest* sub_strategy = new StrategyRequest( reqList, analysisStrategyRequest );
        ( *strategy )    = sub_strategy;
        ( *strategy )->printStrategyRequest();
        analysisRequired = true;
    }
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: call to defineExperiment() finished\n" );
}

/**
 * @brief Returns does the application has to be restarted for the next experiment.
 *
 * Return true if a restart of the application is required for the next experiment,
 * false otherwise.
 *
 * @ingroup DvfsTaurusPlugin
 *
 * @return true if an application has to be restarted false otherwise
 *
 */
bool DvfsTaurusPlugin::restartRequired( std::string& env,
                                        int&         numprocs,
                                        std::string& command,
                                        bool&        is_instrumented ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: call to restartRequired()\n" );
    return false; // no restart required
}

/**
 * @brief Returns the status of the current search iteration.
 *
 * Returns true if the current search iteration is finished (by either the plugin itself or as
 * a result of the search algorithm), false otherwise.
 *
 * @ingroup DvfsTaurusPlugin
 *
 * @return true if the plugin has finished search false otherwise
 *
 */
bool DvfsTaurusPlugin::searchFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: call to searchFinished()\n" );
    return searchAlgorithm->searchFinished();
}

/**
 * @brief Final operation of a tuning step.
 *
 * If any post-processing is necessary before entering the next tuning iteration, it is to be
 * done here.
 *
 * @ingroup DvfsTaurusPlugin
 *
 */
void DvfsTaurusPlugin::finishTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: call to processResults()\n" );
}

/**
 * @brief Returns true if the plugin finished the tuning process, false otherwise.
 *
 * @ingroup DvfsTaurusPlugin
 *
 * @return true if the plugin has finished tuning false otherwise
 *
 */
bool DvfsTaurusPlugin::tuningFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: call to tuningFinished()\n" );
    return true;
}

/**
 * @brief Prints to the screen (and to a file, where necessary) the tuning advice.
 *
 * @ingroup DvfsTaurusPlugin
 */
Advice* DvfsTaurusPlugin::getAdvice( void ) {

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: call to getAdvice()\n" );
    std::ostringstream     result_oss;
    std::map<int, double>  energyForScenario = searchAlgorithm->getSearchPath();

    int optimum = searchAlgorithm->getOptimum();
    result_oss << "Optimum Scenario: " << optimum << endl << endl;

    list<Scenario* > bestScenarios;
    Scenario*        bestScenario = ( *pool_set->fsp->getScenarios() )[ optimum ];
    bestScenarios.push_back( bestScenario );

    list<TuningSpecification*>*          best_tuning_specifications = bestScenario->getTuningSpecifications();
    list<TuningSpecification*>::iterator ts;

    result_oss << "CPU Frequency: ";
    for( ts = best_tuning_specifications->begin(); ts != best_tuning_specifications->end(); ts++ ) {
        map<TuningParameter*, int> best_cpu_freq = ( *ts )->getVariant()->getValue();
        int                        cpu_freq      = best_cpu_freq.begin()->second;
        result_oss << cpu_freq << " " << endl;
    }

    result_oss << "\nAll Results:\n";
    result_oss << "Scenario\t|  Frequency\t|  Energy\t\n";
    for( int scenario_id = 0; scenario_id < pool_set->fsp->size(); scenario_id++ ) {
        Scenario*                   sc         = ( *pool_set->fsp->getScenarios() )[ scenario_id ];
        list<TuningSpecification*>* tuningSpec = sc->getTuningSpecifications();
        int                         cpu_freq   = 0;
        for( ts = tuningSpec->begin(); ts != tuningSpec->end(); ts++ ) {
            map<TuningParameter*, int> tpValues = ( *ts )->getVariant()->getValue();
            cpu_freq                            = tpValues.begin()->second;
        }
        double                      energy      = energyForScenario[ scenario_id ];
        result_oss << scenario_id << "\t\t|  " << cpu_freq << "\t\t|  " << energy << "\t  "<< endl;
    }

    list<MetaProperty>                                       properties;
    list<MetaProperty>::iterator                             p_it;
    std::multimap<Scenario*, std::map<std::string, double> > RegionBestConfig;

    int         num_sig_regions = code_significant_regions.size();
    std::string regionName[ num_sig_regions ];
    int         regionBestFreq[ num_sig_regions ];
    int         regionBestScenarioID[ num_sig_regions ];
    double      regionEnergy[ num_sig_regions ];
    Scenario*   regionBestScenario[ num_sig_regions ];
    std::string regionID[ num_sig_regions ];

    // initializing the array values
    for( int i = 0; i < num_sig_regions; i++ ) {
        regionName[ i ]           = "";
        regionBestFreq[ i ]       = 0;
        regionBestScenarioID[ i ] = 0;
        regionEnergy[ i ]         = INT_MAX;
        regionBestScenario[ i ]   = NULL;
        regionID[ i ]             = "";
    }

    int numExp  = pool_set->arp->getAllExperimentProperties().size();
    int counter = 0;
    result_oss << endl << "Results for individual regions:" << endl << endl;

    for( int experiment = 0; experiment < numExp; experiment++ ) {
        properties = pool_set->arp->getExperimentProperties( experiment );
        counter    = 0;

        for( p_it = properties.begin(); p_it != properties.end(); p_it++ ) {
            Region* reg = appl->getRegionByID( p_it->getRegionId() );
            result_oss << "Region: " << reg->get_name() << " Energy: " << p_it->getSeverity() << "\t";

            int                        scenario_id = atoi( p_it->getExtraInfo().at( "ScenarioID" ).c_str() );
            Scenario*                  sc          = ( *pool_set->fsp->getScenarios() )[ scenario_id ];
            map<TuningParameter*, int> tpValues    = sc->getTuningSpecifications()->front()->getVariant()->getValue();
            int                        cpu_freq    = tpValues.begin()->second;

            if( p_it->getSeverity() < regionEnergy[ counter ] ) {
                regionEnergy[ counter ]         = p_it->getSeverity();
                regionBestFreq[ counter ]       = cpu_freq;
                regionBestScenarioID[ counter ] = scenario_id;
                regionBestScenario[ counter ]   = sc;
                regionName[ counter ]           = reg->get_name();
                regionID[ counter ]             = reg->getRegionID();
            }
            counter++;
        }
        result_oss << endl;
    }

    result_oss << endl << "Best configurations for individual regions:" << endl << endl;

    std::multimap<Scenario*, std::map<std::string, double> >::iterator RegionBestConfig_it;

    for( int i = 0; i < num_sig_regions; i++ ) {
        result_oss << "Region: " << regionName[ i ] << " ";
        result_oss << "CPU Frequency: " << regionBestFreq[ i ] << " ";
        result_oss << "Energy: " << regionEnergy[ i ] << endl;

        std::map<std::string, double >         RegionScenarioRes;
        list<std::map<std::string, double > >  RegionScenarioList;

        RegionScenarioRes.insert( std::make_pair( regionID[i], regionEnergy[i] ) );
        RegionBestConfig.insert( std::make_pair( regionBestScenario[ i ], RegionScenarioRes ) );

        // accumulating best scenario description
        Scenario*         sc                  = ( *pool_set->fsp->getScenarios() )[ regionBestScenarioID[ i ] ];
        string            scenarioDescription = sc->getDescription();
        std::stringstream str_stream;
        str_stream << "Best scenario for region: " << regionName[ i ];
        if( scenarioDescription.empty() ) {
            sc->setDescription( str_stream.str() );
        }
        else {
            scenarioDescription = scenarioDescription + " | " + str_stream.str();
            sc->setDescription( scenarioDescription );
        }
    }

    result_oss << "\n------------------------" << endl << endl;
    cout       << result_oss.str();

    map<int, Scenario*>::iterator scenario_iter;
    for( scenario_iter = pool_set->fsp->getScenarios()->begin(); scenario_iter != pool_set->fsp->getScenarios()->end(); scenario_iter++ ) {
        Scenario* sc   = scenario_iter->second;
        sc->addResult( "Energy", energyForScenario[ sc->getID() ] );
    }

    if( RegionBestConfig.empty() ) {
        return new Advice( getName(), bestScenarios, energyForScenario, "Energy", pool_set->fsp->getScenarios() );
    }

    return new Advice( getName(), bestScenarios, energyForScenario, "Energy", pool_set->fsp->getScenarios(), RegionBestConfig );
}

/**
 * @brief Finalize the plugin normally.
 *
 * Remove any allocated memory, objects, file descriptors, etc.
 *
 * @ingroup DvfsTaurusPlugin
 *
 */
void DvfsTaurusPlugin::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: call to finalize()\n" );

    delete searchAlgorithm;
    for( int i = 0; i < tuningParameters.size(); i++ ) {
        delete tuningParameters[ i ];
    }
}

/**
 * @brief Terminate the plugin due to error.
 *
 * Safely remove any allocated memory, objects, file descriptors, etc. This method should
 * be able to be executed safely at any point.
 *
 * @ingroup DvfsTaurusPlugin
 *
 */
void DvfsTaurusPlugin::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: call to terminate()\n" );
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
 * return a pointer to a new DvfsTaurusPlugin();
 *
 * @ingroup DvfsTaurusPlugin
 *
 * @return A pointer to a new DvfsTaurusPlugin
 */
IPlugin* getPluginInstance( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: call to getPluginInstance()\n" );
    return new DvfsTaurusPlugin();
}

/**
 * @brief Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 *
 * @ingroup DvfsTaurusPlugin
 *
 * @return The major plugin interface version used by the plugin
 */
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: call to getInterfaceVersionMajor()\n" );
    return 1;
}

/**
 * @brief Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 *
 * @ingroup DvfsTaurusPlugin
 *
 * @return The minor plugin interface version used by the plugin
 *
 */
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: call to getInterfaceVersionMinor()\n" );

    return 0;
}

/**
 * @brief Returns a string with the name of the plugin.
 *
 * @ingroup DvfsTaurusPlugin
 *
 * @return Returns a string with the name of the plugin
 *
 */
string getName( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: call to getName()\n" );
    return "DvfsTaurusPlugin";
}

/**
 * @brief Returns a string with a short description of the plugin.
 *
 * @ingroup DvfsTaurusPlugin
 *
 * @return A string with a short description of the plugin
 *
 */
string getShortSummary( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DvfsTaurusPlugin: call to getShortSummary()\n" );
    return "DVFS plugin for taurus";
}
