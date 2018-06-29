/**
   @file    PipelinePlugin.cc
   @ingroup PipelinePlugin
   @brief   Pipeline Plugin
   @author  Research Group Scientific Computing, University of Vienna
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

#include "PipelinePlugin.h"
#include <boost/foreach.hpp>
#include <boost/program_options.hpp>

#define foreach BOOST_FOREACH

namespace po = boost::program_options;

long tuningStep;
bool has_prune_flag = false;

/**
 * @brief extracts tuning parameters for Pipline Plugin
 * @ingroup PipelinePlugin
 *
 **/
vector<TuningParameter*>PipelinePlugin::extractPipelineTuningParameters( void ) {
    Application&             app = Application::instance();
    vector<TuningParameter*> tps;

    if( app.get_regions().empty() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "[#### AUTOTUNE ####]: No Plugin found. Exiting.\n" );
        throw 0;
    }

    TuningParameter*        tp;
    list<Region*>::iterator reg;
    list<Region*>           regions = app.get_regions();
    for( reg = regions.begin(); reg != regions.end(); ++reg ) {
        if( !( *reg )->plugins.empty() ) {
            list<Plugin*>::iterator plugin_iterator;
            list<Plugin*>           plugins = ( *reg )->plugins;
            int                     pid     = 0;
            for( plugin_iterator = plugins.begin(); plugin_iterator != plugins.end(); ++plugin_iterator, ++pid ) {
                tp = new TuningParameter();
                tp->setId( pid );
                tp->setName( ( *plugin_iterator )->getTuningActionName() );
                tp->setPluginType( Pipeline );
                const runtimeTuningActionType tuningActionType = ( *plugin_iterator )->getTuningActionType();
                tp->setRuntimeActionType( tuningActionType );
                int rstep = ( *plugin_iterator )->getRstep();
                int rmin  = ( *plugin_iterator )->getRmin();
                int rmax  = ( *plugin_iterator )->getRmax();
                if( rstep <= 0 ) {
                    rstep = 1;
                }
                if( !( rmin == 0 && rmax == 0 ) ) {
                    tp->setRange( rmin, rmax, rstep );
                }
                else {
                    tp->setRange( 1, ( *plugin_iterator )->getNumberOfVariants(), rstep );
                }

                Region*      r   = ( *reg );
                Restriction* res = new Restriction();
                res->setRegion( r );
                res->setRegionDefined( true );
                tp->setRestriction( res );
                tps.push_back( tp );
            }
        }
    }

    return tps;
}

/**
 * @brief Initialized the Pipeline Plugin
 * @ingroup PipelinePlugin
 *
 * Initialize the plugin's data structures.
 *
 * The tuning parameter list needs to be created.
 *
 * Search algorithms are loaded here when required. This can be done as follows:
 *    searchAlgorithm = loadSearchAlgorithm("name");
 *
 * where "name" refers to one of the available search algorithms (currently only exhaustive).
 *
 * @param context Context.
 * @param pool_set Set of Scenario Pools
 *
 **/
void PipelinePlugin::initialize( DriverContext*   context,
                                 ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "[#### PipelinePlugin ####]: Call to initialize\n" );
    tuningStep = 0;

    this->context  = context;
    this->pool_set = pool_set;

    po::options_description optdesc( "Options" );
    optdesc.add_options()
        ( "vpattern-focused", "Use plugin pruning" );
    po::variables_map vm;
    po::store( po::parse_command_line( context->getArgc(), context->getArgv(), optdesc ), vm );
    if( vm.count( "vpattern-focused" ) ) {
        has_prune_flag = true;
    }

    // Should be adapted for Score-P
    vector<TuningParameter*> tps = extractPipelineTuningParameters();
    if( tps.empty() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                    "[#### PipelinePlugin ####]: No tuning parameters found. Exiting.\n" );
        throw PTF_PLUGIN_ERROR( TUNING_PARAMETERS_NOT_FOUND );
    }
    tuningParameters = tps;
    std::cout << "tuningParameters size: " << tuningParameters.size() << std::endl;
    foreach( TuningParameter * tp, tps ) {
        tp->setPluginType( Pipeline );
        std::cout << "\t" << tp->getId() << ": " << tp->getName() << " (" << tp->getRangeFrom() << "," << tp->getRangeTo() << "," << tp->getRangeStep() << ")" << std::endl;
    }
    // later on we should have a max or min and a function that evaluates the objective
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "obtain getSearchInstance\n" );
    int    major, minor;
    string name, description;

    char const* selected_search = getenv( "PSC_SEARCH_ALGORITHM" );
    if( selected_search != NULL ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "User specified search algorithm: %s\n",
                    selected_search );
        string selected_search_string = string( selected_search );
        context->loadSearchAlgorithm( selected_search_string, &major, &minor, &name, &description );
        searchAlgorithm = context->getSearchAlgorithmInstance( selected_search_string );
    }
    else {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Selecting default search algorithm: exhaustive\n" );
        context->loadSearchAlgorithm( "exhaustive", &major, &minor, &name, &description );
        searchAlgorithm = context->getSearchAlgorithmInstance( "exhaustive" );
    }

    if( searchAlgorithm != NULL ) {
        print_loaded_search( major, minor, name, description );
        searchAlgorithm->initialize( context, pool_set );
    }
    else {
        perror( "NULL pointer in searchAlgorithm\n" );
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "searchAlgorithm instance obtained\n" );
}

/**
 * @brief No pre-analysis is required for the current version of the Pipeline Plugin
 * @ingroup PipelinePlugin
 *
 * @param strategy Not used.
 * @return FALSE
 *
 **/
bool PipelinePlugin::analysisRequired( StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "[#### PipelinePlugin ####]: Call to analysisRequired.\n" );
    StrategyRequestGeneralInfo* strategyRequestGeneralInfo = new StrategyRequestGeneralInfo;

    strategyRequestGeneralInfo->strategy_name     = "ConfigAnalysis";
    strategyRequestGeneralInfo->pedantic          = 1;
    strategyRequestGeneralInfo->delay_phases      = 0;
    strategyRequestGeneralInfo->delay_seconds     = 0;
    strategyRequestGeneralInfo->analysis_duration = 1;

    PropertyRequest* req = new PropertyRequest();

    list<PropertyRequest*>* reqList = new list<PropertyRequest*>;

    list<Region*> regions = appl->get_regions();

    foreach( Region * reg, regions ) {
        if( reg->get_type() == VIE_PIPE_STAGE_REGION ) {
            req = new PropertyRequest();
            req->addPropertyID( PIPESTAGEEXECTIME );
            req->addRegion( appl->searchRegion( reg->get_ident().file_id, reg->getFirstLine() ) );
            req->addAllProcesses();
            reqList->push_back( req );
            req = new PropertyRequest();
            req->addPropertyID( PIPESTAGEBUFWAITTIME );
            req->addRegion( appl->searchRegion( reg->get_ident().file_id, reg->getFirstLine() ) );
            req->addAllProcesses();
            reqList->push_back( req );
        }
    }

    *strategy = new StrategyRequest( reqList, strategyRequestGeneralInfo );
    ( *strategy )->printStrategyRequest();
    //(*analysisRequired)=true;
    return true;
}

/**
 * @brief Operations to be done at the start of a tuning step.
 * @ingroup PipelinePlugin
 *
 * The tuning parameters that were created at initialization are used to create a variant space.
 * The variant space and the regions are then used to create a search space that is passed to the
 * search algorithm.
 *
 **/
void PipelinePlugin::startTuningStep( void ) {
    tuningStep++;
}

/**
 * @brief The Created Scenario Pool (csp) is populated here.
 * @ingroup PipelinePlugin
 *
 * The scenarios need to be created and added to the first pool.  To create the scenarios, a
 * search algorithm can be used or it can be done directly by the plugin.
 *
 * After this step, the Periscope will verify that scenarios were added to the csp.
 *
 **/
void PipelinePlugin::createScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "[#### PipelinePlugin ####]: Call to createScenarios\n" );

    if( searchAlgorithm == NULL ) {
        perror( "Search algorithm not instantiated\n" );
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }

    int                           experimentId, propertyCount = 0;
    map<int, list<MetaProperty> > preAnalysisPropertiesMap;
    preAnalysisPropertiesMap = pool_set->arp->getAllPreAnalysisProperties();
    map<int, list<MetaProperty> >::const_iterator propertyMapIter;
    list<MetaProperty>                            properties;
    list<MetaProperty>::iterator                  property;
    MetaProperty*                                 maxstageprop;
    double                                        maxstagetime = 0.0;
    for( propertyMapIter = preAnalysisPropertiesMap.begin(); propertyMapIter != preAnalysisPropertiesMap.end(); propertyMapIter++ ) {
        experimentId = propertyMapIter->first;
        properties   = propertyMapIter->second;

        for( property = properties.begin(); property != properties.end(); property++, propertyCount++ ) {
            if( maxstagetime <= property->getSeverity() && property->getName().compare( "PipelineStageExecutionTime" ) == 0 ) {
                maxstagetime = property->getSeverity();
                maxstageprop = &( *property );
            }
        }
    }
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "[#### PipelinePlugin ####]: Create a VariantSpace from the tuning parameters.\n" );
    for( int i = 0; i < tuningParameters.size(); i++ ) {
        TuningParameter* currenttp = tuningParameters[ i ];
        Region*          tpregion  = currenttp->getRestriction()->getRegion();
        if( tpregion->get_type() == VIE_PIPE_STAGE_REGION && ( tpregion->getFirstLine() != maxstageprop->getStartPosition() ) && has_prune_flag ) {
            currenttp->setRange( currenttp->getRangeFrom(), currenttp->getRangeFrom(), currenttp->getRangeStep() );
        }
        variantSpace.addTuningParameter( tuningParameters[ i ] );
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "[#### PipelinePlugin ####]: Create a SearchSpace from the tuning parameters.\n" );
    searchSpace.setVariantSpace( &variantSpace );
    searchSpace.addRegion( tuningParameters[ 0 ]->getRestriction()->getRegion() );
    searchAlgorithm->addSearchSpace( &searchSpace );
    searchAlgorithm->createScenarios();
}

/**
 * @brief Preparatory steps for the scenarios are done here.
 * @ingroup PipelinePlugin
 *
 * If there are any preparatory steps required by some or all scenarios in the csp (for example:
 * the project may need to be re-compiled), then they are to be performed here.  After each
 * scenario is prepared, they are migrated from the csp to the Prepared Scenario Pool (psp).
 *
 * In some cases, no preparation may be necessary and the plugin can simply move all scenarios
 * from the csp to the psp.
 *
 * After this step, the Periscope will verify that scenarios were added to the psp.
 *
 **/
void PipelinePlugin::prepareScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                "[#### PipelinePlugin ####]: Preparing the scenarios.\n" );

    //no preparation is necessary, just move the elements on the pools
    while( !pool_set->csp->empty() ) {
        pool_set->psp->push( pool_set->csp->pop() );
    }

    // if compilation is necessary, a compile() method can be called here
}

/**
 * @brief Populate the Experiment Scenario Pool (esp) for the next experiment.
 * @ingroup PipelinePlugin
 *
 * This is the final step before the experiments are executed. Scenarios are moved from the
 * psp to the esp, depending on the number of processes and whether they can be executed
 * in parallel.
 *
 * After this step, the Periscope will verify that scenarios were added to the esp.
 *
 * @param numprocs Not changed.
 * @param analysisRequired Not used.
 * @param strategy No analysis strategy requested.
 *
 **/
void PipelinePlugin::defineExperiment( int               numprocs,
                                       bool&             analysisRequired,
                                       StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "[#### PipelinePlugin ####]: Call to defineExperiment.\n" );
    int       i;
    Scenario* scenario;

    for( i = 0; !pool_set->psp->empty() && i < numprocs; i++ ) {
        //select scenario of this rank
        scenario = pool_set->psp->pop();
        const list<TuningSpecification*>* ts = scenario->getTuningSpecifications();

        if( ts->size() != 1 ) {
            perror( "PipelinePlugin can't currently handle multiple TuningSpecifications\n" );
            throw 0;
        }
        //define rank in the tuning specification
        ts->front()->setSingleRank( i );

        //define tuned region, property, and rank for the objective evaluation
        scenario->setSingleTunedRegionWithPropertyRank( tuningParameters[ 0 ]->getRestriction()->getRegion(), PIPEEXECTIME, i );
        pool_set->esp->push( scenario );
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "[#### PipelinePlugin ####]: Added %d scenarios in the experiment.\n", i );
}

/**
 * @brief Requests a restart for the experiment.
 * @ingroup PipelinePlugin
 *
 * Return true if a restart of the application is required for the next experiment,
 * false otherwise.
 *
 * @param env Not changed.
 * @param numprocs Not changed.
 * @param command Not changed.
 * @param is_instrumented Not changed.
 *
 * @return True
 *
 **/
bool PipelinePlugin::restartRequired( std::string& env,
                                      int&         numprocs,
                                      std::string& command,
                                      bool&        is_instrumented ) {
    char const* force_restart = getenv( "PSC_FORCE_RESTART" );
    if( force_restart != NULL ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Forced restart.\n" );
        char const* restart_command = getenv( "PSC_RESTART_COMMAND" );
        if( restart_command != NULL ) {
            command.append( string( restart_command ) );
        }
        char const* process_count = getenv( "PSC_PROCESS_COUNT" );
        if( process_count != NULL ) {
            numprocs = atoi( process_count );
        }
        char const* env_is_instrumented = getenv( "PSC_NOT_INSTRUMENTED" );
        if( env_is_instrumented != NULL ) {
            is_instrumented = false;
        }
        return true;
    }
    return true;
}

/**
 * @brief  Returns true if the search algorithm finished.
 * @ingroup PipelinePlugin
 *
 * Return true if if the current search iteration is finished (by either the plugin itself or as
 * a result of the search algorithm), false otherwise.
 *
 * @return Value returned by the search algorithm.
 *
 **/
bool PipelinePlugin::searchFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PipelinePlugin: call to searchFinished()\n" );
    return searchAlgorithm->searchFinished();
}

/**
 * @brief Final operation of a tuning step.
 * @ingroup PipelinePlugin
 *
 * If any post-processing is necessary before entering the next tuning iteration, it is to be
 * done here.
 **/
void PipelinePlugin::finishTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PipelinePlugin: call to processResults()\n" );
}

/**
 * @brief Returns true if the plugin finished the tuning process.
 * @ingroup PipelinePlugin
 *
 * Returns true if the plugin finished the tuning process.
 *
 * @return True.
 **/
bool PipelinePlugin::tuningFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PipelinePlugin: call to tuningFinished()\n" );
    return true;
}

/**
 * @brief Prints the tuning advice to standard output.
 * @ingroup PipelinePlugin
 *
 * Prints the tuning advice to standard output as well as the list of all
 * tested scenarios.
 **/
Advice* PipelinePlugin::getAdvice( void ) {
    if( searchAlgorithm == NULL ) {
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }
    cout << "AutoTune Results:" << endl;
    cout << "\n-----------------------\n";
    map<int, double > path = searchAlgorithm->getSearchPath();
    cout << "Search Steps: " << path.size() << endl;

    cout << "Optimum Scenario: " << searchAlgorithm->getOptimum() << endl;
    list<MetaProperty> props = pool_set->srp->getScenarioResultsByID( searchAlgorithm->getOptimum() );

    cout << "Optimum Scenario Severity: " << props.front().getSeverity() << endl << endl;

    cout << "Optimum Scenario Configuration: " << endl;


    std::map<TuningParameter*, int>           tparams = pool_set->fsp->getTuningSpecificationByScenarioID( searchAlgorithm->getOptimum() )->front()->getVariant()->getValue();
    std::map<TuningParameter*, int>::iterator it;

    for( std::map<TuningParameter*, int>::iterator it = tparams.begin(); it != tparams.end(); ++it ) {
        std::cout << "\t" << it->first->getName() << ": " << it->second << std::endl;
    }
    std::cout << std::endl;

    /*
       cout << "Search Path:\n";
       cout << "Step      |  Best\n";
       for (int step=0; step < path.size(); step++){
       printf("%-10d|\t%f\n", step, path[step]);
       }
       cout << endl;
     */
    cout << "Explored Scenarios:\n";
    for( int scenario_id = 0; scenario_id < pool_set->fsp->size(); scenario_id++ ) {
        std::cout << "Scenario " << scenario_id << ": " << std::endl;

        std::map<TuningParameter*, int>           tparams2 = pool_set->fsp->getTuningSpecificationByScenarioID( scenario_id )->front()->getVariant()->getValue();
        std::map<TuningParameter*, int>::iterator it2;

        for( std::map<TuningParameter*, int>::iterator it2 = tparams2.begin(); it2 != tparams2.end(); ++it2 ) {
            std::cout << "\t" << it2->first->getName() << ": " << it2->second << std::endl;
        }
        std::cout << std::endl;
    }

    cout << "Scenario  |  Severity\n";
    for( int scenario_id = 0; scenario_id < pool_set->fsp->size(); scenario_id++ ) {
        if( !pool_set->srp->getScenarioResultsByID( scenario_id ).empty() ) {
            printf( "%-10d|", scenario_id );
            list<MetaProperty> properties = pool_set->srp->getScenarioResultsByID( scenario_id );
            for( std::list<MetaProperty>::iterator iterator = properties.begin(),
                 end = properties.end(); iterator != end; ++iterator ) {
                printf( "\t%f", iterator->getSeverity() );
            }
            printf( "\n" );
        }
    }
    cout << "----------|-------------" << endl;
    return new Advice( getName(), ( *pool_set->fsp->getScenarios() )[ searchAlgorithm->getOptimum() ],
                       searchAlgorithm->getSearchPath(), "Time", pool_set->fsp->getScenarios() );
}

/**
 * @brief Finalize the plugin normally.
 * @ingroup PipelinePlugin
 *
 * Finalize the plugin normally, removes any allocated memory, objects, file descriptors, etc.
 *
 **/
void PipelinePlugin::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PipelinePlugin: call to finalize()\n" );
    terminate();
}

/**
 * @brief Terminate the plugin.
 * @ingroup PipelinePlugin
 *
 * Terminate the plugin due to error. Safely remove any allocated memory, objects, file descriptors, etc.
 * This method should be able to be executed safely at any point.
 *
 **/
void PipelinePlugin::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PipelinePlugin: call to terminate()\n" );

    if( searchAlgorithm ) {
        searchAlgorithm->finalize();
        delete searchAlgorithm;
    }
    context->unloadSearchAlgorithms();
}

/*
 * The following are C functions that belong to the Plugin Management Interface, and not to the
 * IPlugin class.  Currently the interface includes information and instance creation functions.
 *
 * These are defined in PluginManagement.h .
 */


/**
 * @brief Returns plugin instance.
 * @ingroup PipelinePlugin
 *
 * Returns an instance of this particular plugin implementation.
 * Typically, a simple return with new is enough. For example:
 *    return new PipelinePlugin();
 *
 * @return pointer to a new plugin object.
 **/
IPlugin* getPluginInstance( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PipelinePlugin: call to getComponentInstance()\n" );

    return new PipelinePlugin();
}

/**
 * @brief Returns the major plugin interface version.
 * @ingroup PipelinePlugin
 *
 * Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 * @return major plugin interface version number.
 **/
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PipelinePlugin: call to getInterfaceVersionMajor()\n" );

    return PIPELINE_VERSION_MAJOR;
}

/**
 * @brief Returns the minor plugin interface version.
 * @ingroup PipelinePlugin
 *
 * Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 * @return minor plugin interface version number.
 **/
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PipelinePlugin: call to getInterfaceVersionMinor()\n" );

    return PIPELINE_VERSION_MINOR;
}

/**
 * @brief Returns plugin name string.
 * @ingroup PipelinePlugin
 *
 * Returns a string with the name of the plugin.
 *
 * @return Plugin name string.
 **/
string getName( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PipelinePlugin: call to getName()\n" );

    return "PipelinePlugin";
}

/**
 * @brief Returns a short description.
 * @ingroup PipelinePlugin
 *
 * Returns a string with a short description of the plugin.
 * @return Description string.
 **/
string getShortSummary( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PipelinePlugin: call to getShortSummary()\n" );

    return "Pipeline Tuning Plugin";
}
