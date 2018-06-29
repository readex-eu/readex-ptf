/**
   @file    ROMIOPlugin.cc
   @ingroup Autotune
   @brief   ROMIO Plugin
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

#include "ROMIOPlugin.h"
#include "Property.h"
/*
 * Initialize the plugin's data structures.
 *
 * The tuning parameter list needs to be created.
 *
 * Search algorithms are loaded here when required. This can be done as follows:
 *
 *    searchAlgorithm = loadSearchAlgorithm("name");
 *
 * where "name" refers to one of the available search algorithms (currently only exhaustive).
 *
 */
void ROMIOPlugin::initialize( DriverContext*   context,
                              ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ROMIOPlugin: call to initialize()\n" );
    this->context  = context;
    this->pool_set = pool_set;

    regions = app.get_regions();
    if( regions.empty() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "No Plugin found. Exiting.\n" );
        exit( EXIT_FAILURE );
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ROMIOPlugin: obtain getSearchInstance\n" );
    int         major, minor;
    string      name, description;
    char const* selected_search = getenv( "PSC_SEARCH_ALGORITHM" );
    if( selected_search != NULL ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Specified search algorithm: %s\n", selected_search );
        string selected_search_string = string( selected_search );
        context->loadSearchAlgorithm( selected_search_string, &major, &minor, &name, &description );
        searchAlgorithm = context->getSearchAlgorithmInstance( selected_search_string );
    }
    else {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "DVFSPlugin: Default search algorithm: exhaustive\n" );
        context->loadSearchAlgorithm( "exhaustive", &major, &minor, &name, &description );
        searchAlgorithm = context->getSearchAlgorithmInstance( "exhaustive" );
    }

    if( searchAlgorithm != NULL ) {
        searchAlgorithm->initialize( context, pool_set );
    }
    else {
        perror( "NULL pointer in searchAlgorithm\n" );
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }
    //throw 0;
}

/*
 * Defines a pre-analysis strategy.
 *
 * Found properties of the pre-analysis strategy are stored in the arp.
 *
 */
bool ROMIOPlugin::analysisRequired( StrategyRequest** strategy ) {
    /**
       StrategyRequestGeneralInfo* strategyRequestGeneralInfo = new StrategyRequestGeneralInfo;
       strategyRequestGeneralInfo->strategy_name = "MPI";
       strategyRequestGeneralInfo->pedantic = 1;
       strategyRequestGeneralInfo->delay_phases = 0;
       strategyRequestGeneralInfo->delay_seconds = 0;
     * strategy = new StrategyRequest(strategyRequestGeneralInfo);

       return true;
     **/
    return false;
}

/*
 * Operations to be done at the start of a tuning step.
 *
 * The tuning parameters that were created at initialization are used to create a variant space.
 * The variant space and the regions are then used to create a search space that is passed to the
 * search algorithm.
 *
 */
void ROMIOPlugin::startTuningStep( void ) {
    tp_romio_cb_read = new TuningParameter;
    tp_romio_cb_read->setName( "ROMIO_CB_READ" );
    tp_romio_cb_read->setPluginType( MPI );
    tp_romio_cb_read->setRuntimeActionType( TUNING_ACTION_VARIABLE_INTEGER );
    tp_romio_cb_read->setId( 0 );

    tp_romio_cb_write = new TuningParameter;
    tp_romio_cb_write->setName( "ROMIO_CB_WRITE" );
    tp_romio_cb_write->setPluginType( MPI );
    tp_romio_cb_write->setRuntimeActionType( TUNING_ACTION_VARIABLE_INTEGER );
    tp_romio_cb_write->setId( 0 );

    tp_cb_nodes = new TuningParameter;
    tp_cb_nodes->setName( "CB_NODES" );
    tp_cb_nodes->setPluginType( MPI );
    tp_cb_nodes->setRuntimeActionType( TUNING_ACTION_VARIABLE_INTEGER );
    tp_cb_nodes->setId( 0 );

    tp_cb_buffer_size = new TuningParameter;
    tp_cb_buffer_size->setName( "CB_BUFFER_SIZE" );
    tp_cb_buffer_size->setPluginType( MPI );
    tp_cb_buffer_size->setRuntimeActionType( TUNING_ACTION_VARIABLE_INTEGER );
    tp_cb_buffer_size->setId( 0 );

    tp_ind_rd_buffer_size = new TuningParameter;
    tp_ind_rd_buffer_size->setName( "IND_RD_BUFFER_SIZE" );
    tp_ind_rd_buffer_size->setPluginType( MPI );
    tp_ind_rd_buffer_size->setRuntimeActionType( TUNING_ACTION_VARIABLE_INTEGER );
    tp_ind_rd_buffer_size->setId( 0 );

    tp_ind_wr_buffer_size = new TuningParameter;
    tp_ind_wr_buffer_size->setName( "IND_WR_BUFFER_SIZE" );
    tp_ind_wr_buffer_size->setPluginType( MPI );
    tp_ind_wr_buffer_size->setRuntimeActionType( TUNING_ACTION_VARIABLE_INTEGER );
    tp_ind_wr_buffer_size->setId( 0 );

    tp_romio_ds_read = new TuningParameter;
    tp_romio_ds_read->setName( "ROMIO_DS_READ" );
    tp_romio_ds_read->setPluginType( MPI );
    tp_romio_ds_read->setRuntimeActionType( TUNING_ACTION_VARIABLE_INTEGER );
    tp_romio_ds_read->setId( 0 );

    tp_romio_ds_write = new TuningParameter;
    tp_romio_ds_write->setName( "ROMIO_DS_WRITE" );
    tp_romio_ds_write->setPluginType( MPI );
    tp_romio_ds_write->setRuntimeActionType( TUNING_ACTION_VARIABLE_INTEGER );
    tp_romio_ds_write->setId( 0 );
}

void ROMIOPlugin::createScenariosForCollectiveWrite( Region* regionPtr ) {
    vector<TuningParameter*> tuningParameters;
    VariantSpace*            variantSpace;
    SearchSpace*             searchSpace;

    variantSpace = new VariantSpace();
    searchSpace  = new SearchSpace();
    tuningParameters.clear();

    tp_romio_cb_write->setRange( 0, 2, 1 );
    tuningParameters.push_back( tp_romio_cb_write );

    for( int i = 0; i < tuningParameters.size(); ++i ) {
        variantSpace->addTuningParameter( tuningParameters[ i ] );
    }
    searchSpace->setVariantSpace( variantSpace );
    searchSpace->addRegion( regionPtr );
    searchAlgorithm->addSearchSpace( searchSpace );
    searchAlgorithm->createScenarios();

    searchAlgorithm->clear();
    delete variantSpace;
    delete searchSpace;

    variantSpace = new VariantSpace();
    searchSpace  = new SearchSpace();
    tuningParameters.clear();

    tp_romio_cb_write->setRange( 2, 2, 1 );
    tp_cb_nodes->setRange( 0, ( int )( log( ( double )TASKS_PER_NODE ) / log( ( double )2 ) ), 1 );
    tuningParameters.push_back( tp_romio_cb_write );
    tuningParameters.push_back( tp_cb_nodes );
    for( int i = 0; i < tuningParameters.size(); ++i ) {
        variantSpace->addTuningParameter( tuningParameters[ i ] );
    }

    searchSpace->setVariantSpace( variantSpace );
    searchSpace->addRegion( regionPtr );
    searchAlgorithm->addSearchSpace( searchSpace );
    searchAlgorithm->createScenarios();

    searchAlgorithm->clear();
    delete variantSpace;
    delete searchSpace;

    for( int i = 0; i <= ( int )( log( ( double )TASKS_PER_NODE ) / log( ( double )2 ) ); ++i ) {
        variantSpace = new VariantSpace();
        searchSpace  = new SearchSpace();
        tuningParameters.clear();

        tp_romio_cb_write->setRange( 2, 2, 1 );
        tp_cb_nodes->setRange( i, i, 1 );
        tp_cb_buffer_size->setRange( 0, ( int )( log( ( double )MAX_COLLECTIVE_BUFFER ) / log( ( double )2 ) ), 1 );

        tuningParameters.push_back( tp_romio_cb_write );
        tuningParameters.push_back( tp_cb_nodes );
        tuningParameters.push_back( tp_cb_buffer_size );
        for( int i = 0; i < tuningParameters.size(); ++i ) {
            variantSpace->addTuningParameter( tuningParameters[ i ] );
        }

        searchSpace->setVariantSpace( variantSpace );
        searchSpace->addRegion( regionPtr );
        searchAlgorithm->addSearchSpace( searchSpace );
        searchAlgorithm->createScenarios();

        searchAlgorithm->clear();
        delete variantSpace;
        delete searchSpace;
    }
}

void ROMIOPlugin::createScenariosForCollectiveRead( Region* regionPtr ) {
    vector<TuningParameter*> tuningParameters;
    VariantSpace*            variantSpace;
    SearchSpace*             searchSpace;

    variantSpace = new VariantSpace();
    searchSpace  = new SearchSpace();
    tuningParameters.clear();

    tp_romio_cb_read->setRange( 0, 2, 1 );
    tuningParameters.push_back( tp_romio_cb_read );

    for( int i = 0; i < tuningParameters.size(); ++i ) {
        variantSpace->addTuningParameter( tuningParameters[ i ] );
    }
    searchSpace->setVariantSpace( variantSpace );
    searchSpace->addRegion( regionPtr );
    searchAlgorithm->addSearchSpace( searchSpace );
    searchAlgorithm->createScenarios();

    searchAlgorithm->clear();
    delete variantSpace;
    delete searchSpace;

    variantSpace = new VariantSpace();
    searchSpace  = new SearchSpace();
    tuningParameters.clear();

    tp_romio_cb_read->setRange( 2, 2, 1 );
    tp_cb_nodes->setRange( 0, ( int )( log( ( double )TASKS_PER_NODE ) / log( ( double )2 ) ), 1 );
    tuningParameters.push_back( tp_romio_cb_read );
    tuningParameters.push_back( tp_cb_nodes );
    for( int i = 0; i < tuningParameters.size(); ++i ) {
        variantSpace->addTuningParameter( tuningParameters[ i ] );
    }

    searchSpace->setVariantSpace( variantSpace );
    searchSpace->addRegion( regionPtr );
    searchAlgorithm->addSearchSpace( searchSpace );
    searchAlgorithm->createScenarios();

    searchAlgorithm->clear();
    delete variantSpace;
    delete searchSpace;

    for( int i = 0; i <= ( int )( log( ( double )TASKS_PER_NODE ) / log( ( double )2 ) ); ++i ) {
        variantSpace = new VariantSpace();
        searchSpace  = new SearchSpace();
        tuningParameters.clear();

        tp_romio_cb_read->setRange( 2, 2, 1 );
        tp_cb_nodes->setRange( i, i, 1 );
        tp_cb_buffer_size->setRange( 0, ( int )( log( ( double )MAX_COLLECTIVE_BUFFER ) / log( ( double )2 ) ), 1 );

        tuningParameters.push_back( tp_romio_cb_read );
        tuningParameters.push_back( tp_cb_nodes );
        tuningParameters.push_back( tp_cb_buffer_size );
        for( int i = 0; i < tuningParameters.size(); ++i ) {
            variantSpace->addTuningParameter( tuningParameters[ i ] );
        }

        searchSpace->setVariantSpace( variantSpace );
        searchSpace->addRegion( regionPtr );
        searchAlgorithm->addSearchSpace( searchSpace );
        searchAlgorithm->createScenarios();

        searchAlgorithm->clear();
        delete variantSpace;
        delete searchSpace;
    }
}

void ROMIOPlugin::createScenariosForNonCollectiveWrite( Region* regionPtr ) {
    vector<TuningParameter*> tuningParameters;
    VariantSpace*            variantSpace;
    SearchSpace*             searchSpace;

    variantSpace = new VariantSpace();
    searchSpace  = new SearchSpace();
    tuningParameters.clear();

    tp_romio_ds_write->setRange( 0, 2, 1 );
    tuningParameters.push_back( tp_romio_ds_write );

    for( int i = 0; i < tuningParameters.size(); ++i ) {
        variantSpace->addTuningParameter( tuningParameters[ i ] );
    }
    searchSpace->setVariantSpace( variantSpace );
    searchSpace->addRegion( regionPtr );
    searchAlgorithm->addSearchSpace( searchSpace );
    searchAlgorithm->createScenarios();

    searchAlgorithm->clear();
    delete variantSpace;
    delete searchSpace;

    variantSpace = new VariantSpace();
    searchSpace  = new SearchSpace();
    tuningParameters.clear();

    tp_romio_ds_write->setRange( 2, 2, 1 );
    tp_ind_wr_buffer_size->setRange( 0, ( int )( log( ( double )MAX_WRITE_DATA_SIEVING_BUFFER ) / log( ( double )2 ) ), 1 );
    tuningParameters.push_back( tp_romio_ds_write );
    tuningParameters.push_back( tp_ind_wr_buffer_size );
    for( int i = 0; i < tuningParameters.size(); ++i ) {
        variantSpace->addTuningParameter( tuningParameters[ i ] );
    }

    searchSpace->setVariantSpace( variantSpace );
    searchSpace->addRegion( regionPtr );
    searchAlgorithm->addSearchSpace( searchSpace );
    searchAlgorithm->createScenarios();

    searchAlgorithm->clear();
    delete variantSpace;
    delete searchSpace;
}

void ROMIOPlugin::createScenariosForNonCollectiveRead( Region* regionPtr ) {
    vector<TuningParameter*> tuningParameters;
    VariantSpace*            variantSpace;
    SearchSpace*             searchSpace;

    variantSpace = new VariantSpace();
    searchSpace  = new SearchSpace();
    tuningParameters.clear();

    tp_romio_ds_read->setRange( 0, 2, 1 );
    tuningParameters.push_back( tp_romio_ds_read );

    for( int i = 0; i < tuningParameters.size(); ++i ) {
        variantSpace->addTuningParameter( tuningParameters[ i ] );
    }
    searchSpace->setVariantSpace( variantSpace );
    searchSpace->addRegion( regionPtr );
    searchAlgorithm->addSearchSpace( searchSpace );
    searchAlgorithm->createScenarios();

    searchAlgorithm->clear();
    delete variantSpace;
    delete searchSpace;

    variantSpace = new VariantSpace();
    searchSpace  = new SearchSpace();
    tuningParameters.clear();

    tp_romio_ds_read->setRange( 2, 2, 1 );
    tp_ind_rd_buffer_size->setRange( 0, ( int )( log( ( double )MAX_READ_DATA_SIEVING_BUFFER ) / log( ( double )2 ) ), 1 );
    tuningParameters.push_back( tp_romio_ds_read );
    tuningParameters.push_back( tp_ind_rd_buffer_size );
    for( int i = 0; i < tuningParameters.size(); ++i ) {
        variantSpace->addTuningParameter( tuningParameters[ i ] );
    }

    searchSpace->setVariantSpace( variantSpace );
    searchSpace->addRegion( regionPtr );
    searchAlgorithm->addSearchSpace( searchSpace );
    searchAlgorithm->createScenarios();

    searchAlgorithm->clear();
    delete variantSpace;
    delete searchSpace;
}

/*
 * The Created Scenario Pool (csp) is populated here.
 *
 * The scenarios need to be created and added to the first pool. To create the scenarios, a
 * search algorithm can be used or it can be done directly by the plugin.
 *
 * After this step, the Periscope will verify that scenarios were added to the csp.
 *
 */
void ROMIOPlugin::createScenarios( void ) {
    diff_regions = 0;
    for( regionIterator = regions.begin(); regionIterator != regions.end(); regionIterator++ ) {
        if( ( *regionIterator )->get_name() == "mpi_file_write_all" ) {
            diff_regions++;
            createScenariosForCollectiveWrite( *regionIterator );
        }
        else if( ( *regionIterator )->get_name() == "mpi_file_read_all" ) {
            diff_regions++;
            createScenariosForCollectiveRead( *regionIterator );
        }
        else if( ( *regionIterator )->get_name() == "mpi_file_write" ) {
            diff_regions++;
            createScenariosForNonCollectiveWrite( *regionIterator );
        }
        else if( ( *regionIterator )->get_name() == "mpi_file_read" ) {
            diff_regions++;
            createScenariosForNonCollectiveRead( *regionIterator );
        }
    }
}

/*
 * Preparatory steps for the scenarios are done here.
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
 */
void ROMIOPlugin::prepareScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ROMIOPlugin: call to prepareScenarios()\n" );

    //no preparation is necessary, just move the elements on the pools
    while( !pool_set->csp->empty() ) {
        pool_set->psp->push( pool_set->csp->pop() );
    }
}

/*
 * Populate the Experiment Scenario Pool (esp) for the next experiment.
 *
 * This is the final step before the experiments are executed. Scenarios are moved from the
 * psp to the esp, depending on the number of processes and whether they can be executed
 * in parallel.
 *
 * After this step, the Periscope will verify that scenarios were added to the esp.
 *
 */
void ROMIOPlugin::defineExperiment( int               numprocs,
                                    bool&             analysisRequired,
                                    StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ROMIOPlugin: call to defineExperiment()\n" );
    ScenarioPool* tmp_storage_pool   = new ScenarioPool();
    int           test_scenarios_num = 0;
    Scenario**    test_scenarios     = new Scenario*[ diff_regions ];

    while( test_scenarios_num < diff_regions && !pool_set->psp->empty() ) {
        Scenario*                         scenario       = pool_set->psp->pop();
        const list<TuningSpecification*>* tuningSpecList = scenario->getTuningSpecifications();
        if( tuningSpecList->size() != 1 ) {
            perror( "ROMIOPlugin can't handle multiple TuningSpecifications\n" );
            throw 0;
        }


        VariantContext context   = tuningSpecList->front()->getVariantContext();
        Region*        region    = ( context.context_union.region_list )->front();
        string         file_name = ( region->get_ident() ).file_name;
        stringstream   ss;
        ss << ( region->get_ident() ).start_position;
        string line_number         = ss.str();
        string key                 = file_name + "-" + line_number;
        bool   test_related_region = false;
        if( test_scenarios_num > 0 ) {
            for( int i = 0; i < test_scenarios_num; i++ ) {
                const list<TuningSpecification*>* test_tuningSpecList = test_scenarios[ i ]->getTuningSpecifications();
                if( test_tuningSpecList->size() != 1 ) {
                    perror( "ROMIOPlugin can't handle multiple TuningSpecifications\n" );
                    throw 0;
                }

                VariantContext test_context   = test_tuningSpecList->front()->getVariantContext();
                Region*        test_region    = ( test_context.context_union.region_list )->front();
                string         test_file_name = ( test_region->get_ident() ).file_name;
                stringstream   test_ss;
                test_ss << ( test_region->get_ident() ).start_position;
                string test_line_number = test_ss.str();
                string test_key         = test_file_name + "-" + test_line_number;

                cout << "key is: " << key << ", test region key is: " << test_key << "." << endl;
                if( key == test_key ) {
                    tmp_storage_pool->push( scenario );
                    test_related_region = true;
                    break;
                }
            }
        }
        if( test_related_region ) {
            continue;
        }
        test_scenarios_num++;
        test_scenarios[ test_scenarios_num - 1 ] = scenario;

        //effect range
        tuningSpecList->front()->setALLRanks();
        list<unsigned int>* rank_list = new list<unsigned int>;
        rank_list->push_back( 0 );
        // For having 1 properties:
        list<int>* property_ids = new list<int>;
        property_ids->push_back( EXECTIME );
        PropertyRequest*        propRequest     = new PropertyRequest( property_ids, rank_list );
        list<PropertyRequest*>* propRequestList = new list<PropertyRequest*>;
        propRequestList->push_back( propRequest );

        scenario->setPropertyRequests( propRequestList );
        scenario->setTunedRegion( region );
        pool_set->esp->push( scenario );
    }

    while( !tmp_storage_pool->empty() ) {
        pool_set->psp->push( tmp_storage_pool->pop() );
    }

    StrategyRequestGeneralInfo* strategyRequestGeneralInfo = new StrategyRequestGeneralInfo;
    strategyRequestGeneralInfo->strategy_name     = "MPI";
    strategyRequestGeneralInfo->pedantic          = 1;
    strategyRequestGeneralInfo->delay_phases      = 0;
    strategyRequestGeneralInfo->delay_seconds     = 0;
    strategyRequestGeneralInfo->analysis_duration = 1;
    *strategy                                     = new StrategyRequest( strategyRequestGeneralInfo );
    *analysisRequired                             = true;

    pool_set->esp->print();
    delete tmp_storage_pool;
    //delete[] test_scenarios;
}

/*
 * Return true if a restart of the application is required for the next experiment,
 * false otherwise.
 */
bool ROMIOPlugin::restartRequired( std::string& env,
                                   int&         numprocs,
                                   std::string& command,
                                   bool&        is_instrumented ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ROMIOPlugin: call to restartRequired()\n" );
    return false;     // no restart required
}

/*
 * Return true if if the current search iteration is finished (by either the plugin itself or as
 * a result of the search algorithm), false otherwise.
 */
bool ROMIOPlugin::searchFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ROMIOPlugin: call to searchFinished()\n" );

    if( !pool_set->csp->empty() ) {
        return false;
    }
    return searchAlgorithm->searchFinished();
}

/*
 * Final operation of a tuning step.
 *
 * If any post-processing is necessary before entering the next tuning iteration, it is to be
 * done here.
 */
void ROMIOPlugin::finishTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ROMIOPlugin: call to processResults()\n" );
}

/*
 * Returns true if the plugin finished the tuning process, false otherwise.
 */
bool ROMIOPlugin::tuningFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ROMIOPlugin: call to tuningFinished()\n" );
    return true;
}

/*
 * Prints to the screen (and to a file, where necessary) the tuning advice.
 */
Advice* ROMIOPlugin::getAdvice( void ) {
    list<MetaProperty>                properties;
    std::list<MetaProperty>::iterator iterator;

    list<TuningSpecification*>*          ts;
    list<TuningSpecification*>::iterator tsit;

    map<TuningParameter*, int>           tp;
    map<TuningParameter*, int>::iterator tpit;

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ROMIO Plugin: call to getAdvice()\n" );

    if( searchAlgorithm == NULL ) {
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }

    cout << "\n-----------------------\n";
    cout << "AutoTune Results:" << endl;

    int    current_start_id = 0, next_start_id = 0;
    string current_region_key;
    int    checked_scenario_num = 0;
    int*   checked_scenario     = ( int* )calloc( pool_set->fsp->size(), sizeof( int ) );
    bool   finish               = false;

    while( !finish ) {
        current_start_id = next_start_id;
        bool found_next_start_id = false;
        //printf("current id is %d, next is %d.\n",current_start_id, next_start_id);
        for( int scenario_id = current_start_id; scenario_id < pool_set->fsp->size(); scenario_id++ ) {
            properties = pool_set->srp->getScenarioResultsByID( scenario_id );
            if( !properties.empty() ) {
                int* tag   = ( int* )calloc( 8, sizeof( int ) );
                int* value = ( int* )calloc( 8, sizeof( int ) );
                ts = ( list<TuningSpecification*>* )( pool_set->fsp->getTuningSpecificationByScenarioID( scenario_id ) );
                for( tsit = ts->begin(); tsit != ts->end(); ++tsit ) {
                    tp = ( *tsit )->getVariant()->getValue();
                    for( tpit = tp.begin(); tpit != tp.end(); ++tpit ) {
                        if( !tpit->first->getName().compare( "ROMIO_CB_READ" ) ) {
                            tag[ 0 ]   = 1;
                            value[ 0 ] = tpit->second;
                        }
                        else if( !tpit->first->getName().compare( "ROMIO_CB_WRITE" ) ) {
                            tag[ 1 ]   = 1;
                            value[ 1 ] = tpit->second;
                        }
                        else if( !tpit->first->getName().compare( "CB_NODES" ) ) {
                            tag[ 2 ]   = 1;
                            value[ 2 ] = tpit->second;
                        }
                        else if( !tpit->first->getName().compare( "CB_BUFFER_SIZE" ) ) {
                            tag[ 3 ]   = 1;
                            value[ 3 ] = tpit->second;
                        }
                        else if( !tpit->first->getName().compare( "ROMIO_DS_WRITE" ) ) {
                            tag[ 4 ]   = 1;
                            value[ 4 ] = tpit->second;
                        }
                        else if( !tpit->first->getName().compare( "IND_WR_BUFFER_SIZE" ) ) {
                            tag[ 5 ]   = 1;
                            value[ 5 ] = tpit->second;
                        }
                        else if( !tpit->first->getName().compare( "ROMIO_DS_READ" ) ) {
                            tag[ 6 ]   = 1;
                            value[ 6 ] = tpit->second;
                        }
                        else if( !tpit->first->getName().compare( "IND_RD_BUFFER_SIZE" ) ) {
                            tag[ 7 ]   = 1;
                            value[ 7 ] = tpit->second;
                        }
                    } // for tpit
                }     // for tsit
                for( iterator = properties.begin(); iterator != properties.end(); ++iterator ) {
                    PropertyPurpose purpose_for_tuning = PSC_PROPERTY_PURPOSE_TUNING;
                    if( iterator->getPurpose() != purpose_for_tuning ) {
                        continue;
                    }
                    //if(iterator->getPurpose()!=100) continue; //only get property for the region that we care, 100 is from aagent/include/Property.h

                    stringstream ss;
                    ss << iterator->getStartPosition();
                    string line_number = ss.str();
                    string key         = iterator->getFileName() + "-" + line_number;

                    if( scenario_id == current_start_id ) {
                        current_region_key = key;
                        if( tag[ 0 ] == 1 ) {
                            cout << "Scenario  |    FileName   | StartPosition | ROMIO_CB_READ |   CB_NODES    | CB_BUFFER_SIZE|  Runtime (s)  |\n";
                        }
                        else if( tag[ 1 ] == 1 ) {
                            cout << "Scenario  |    FileName   | StartPosition | ROMIO_CB_WRITE|   CB_NODES    | CB_BUFFER_SIZE|  Runtime (s)  |\n";
                        }
                        else if( tag[ 4 ] == 1 ) {
                            cout << "Scenario  |    FileName   | StartPosition | ROMIO_DS_WRITE | IND_WR_BUFFER_SIZE |  Runtime (s)  |\n";
                        }
                        else if( tag[ 6 ] == 1 ) {
                            cout << "Scenario  |    FileName   | StartPosition | ROMIO_DS_READ  | IND_RD_BUFFER_SIZE |  Runtime (s)  |\n";
                        }

                        if( tag[ 0 ] == 1 || tag[ 1 ] == 1 ) {
                            cout << "----------+---------------+---------------+---------------+---------------+---------------+---------------|\n";
                        }
                        else if( tag[ 4 ] == 1 || tag[ 6 ] == 1 ) {
                            cout << "----------+---------------+---------------+----------------+--------------------+---------------|\n";
                        }
                    }
                    if( key == current_region_key ) {
                        checked_scenario[ scenario_id ] = 1;
                        checked_scenario_num++;
                        if( checked_scenario_num == pool_set->fsp->size() ) {
                            finish = true;
                        }

                        if( tag[ 0 ] == 1 || tag[ 1 ] == 1 ) {
                            printf( "%-10d|", scenario_id );
                            printf( "%-15s|", ( iterator->getFileName() ).c_str() );
                            printf( "%-15d|", iterator->getStartPosition() );

                            if( tag[ 0 ] == 1 ) { //ROMIO_CB_READ,ROMIO_CB_WRITE
                                switch( value[ 0 ] ) {
                                case 0:
                                    printf( "%-15s|", "Disable" );
                                    break;
                                case 1:
                                    printf( "%-15s|", "Automatic" );
                                    break;
                                case 2:
                                    printf( "%-15s|", "Enable" );
                                    break;
                                default:
                                    break;
                                }
                            }
                            else if( tag[ 1 ] == 1 ) { //ROMIO_CB_READ,ROMIO_CB_WRITE
                                switch( value[ 1 ] ) {
                                case 0:
                                    printf( "%-15s|", "Disable" );
                                    break;
                                case 1:
                                    printf( "%-15s|", "Automatic" );
                                    break;
                                case 2:
                                    printf( "%-15s|", "Enable" );
                                    break;
                                default:
                                    break;
                                }
                            }

                            if( tag[ 2 ] == 1 ) {
                                printf( "%-15d|", ( int )pow( 2, value[ 2 ] ) );
                            }
                            else {
                                printf( "%-15s|", "Default" );
                            }

                            if( tag[ 3 ] == 1 ) {
                                printf( "%-14dM|", ( int )pow( 2, value[ 3 ] ) );
                            }
                            else {
                                printf( "%-15s|", "Default" );
                            }

                            printf( "%-14fs|", iterator->getSeverity() );
                        }
                        else if( tag[ 4 ] == 1 || tag[ 6 ] == 1 ) {
                            printf( "%-10d|", scenario_id );
                            printf( "%-15s|", ( iterator->getFileName() ).c_str() );
                            printf( "%-15d|", iterator->getStartPosition() );

                            if( tag[ 4 ] == 1 ) { //ROMIO_DS_WRITE ROMIO_DS_READ
                                switch( value[ 4 ] ) {
                                case 0:
                                    printf( "%-16s|", "Disable" );
                                    break;
                                case 1:
                                    printf( "%-16s|", "Automatic" );
                                    break;
                                case 2:
                                    printf( "%-16s|", "Enable" );
                                    break;
                                default:
                                    break;
                                }

                                if( tag[ 5 ] == 1 ) {
                                    printf( "%-19dM|", ( int )pow( 2, value[ 5 ] ) );
                                }
                                else {
                                    printf( "%-20s|", "Default" );
                                }
                            }
                            else if( tag[ 6 ] == 1 ) { //ROMIO_DS_WRITE ROMIO_DS_READ
                                switch( value[ 6 ] ) {
                                case 0:
                                    printf( "%-16s|", "Disable" );
                                    break;
                                case 1:
                                    printf( "%-16s|", "Automatic" );
                                    break;
                                case 2:
                                    printf( "%-16s|", "Enable" );
                                    break;
                                default:
                                    break;
                                }

                                if( tag[ 7 ] == 1 ) {
                                    printf( "%-19dM|", ( int )pow( 2, value[ 7 ] ) );
                                }
                                else {
                                    printf( "%-20s|", "Default" );
                                }
                            }

                            printf( "%-14fs|", iterator->getSeverity() );
                        }
                    }
                    else if( !found_next_start_id && checked_scenario[ scenario_id ] == 0 ) {
                        next_start_id       = scenario_id;
                        found_next_start_id = true;
                    }
                    printf( "\n" );
                }     //for property
                free( tag );
                free( value );
            } //if !properties.empty()
        }     //for scenario
              //printf("\n");
              //printf("\n");
    }    //while !finish

    free( checked_scenario );

    return new Advice( getName(), ( *pool_set->fsp->getScenarios() )[ searchAlgorithm->getOptimum() ],
                       searchAlgorithm->getSearchPath(), "Time", pool_set->fsp->getScenarios() );
}

/*
 * Finalize the plugin normally.
 *
 * Remove any allocated memory, objects, file descriptors, etc.
 *
 */
void ROMIOPlugin::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ROMIOPlugin: call to finalize()\n" );

    //psc_errmsg("ROMIOPlugin: finalize() not implemented\n");
    //throw 0;
    terminate();
}

/*
 * Terminate the plugin due to error.
 *
 * Safely remove any allocated memory, objects, file descriptors, etc. This method should
 * be able to be executed safely at any point.
 *
 */
void ROMIOPlugin::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ROMIOPlugin: call to terminate()\n" );

    //psc_errmsg("ROMIOPlugin: terminate() not implemented\n");
    //throw 0;
    if( searchAlgorithm ) {
        searchAlgorithm->finalize();
        delete searchAlgorithm;
    }

    delete tp_romio_cb_read;
    delete tp_romio_cb_write;
    delete tp_cb_nodes;
    delete tp_cb_buffer_size;
    delete tp_ind_rd_buffer_size;
    delete tp_ind_wr_buffer_size;
    delete tp_romio_ds_read;
    delete tp_romio_ds_write;

    context->unloadSearchAlgorithms();
}

/*
 * The following are C functions that belong to the Plugin Management Interface, and not to the
 * IPlugin class. Currently the interface includes information and instance creation functions.
 *
 * These are defined in PluginManagement.h .
 */

/*
 * Returns an instance of this particular plugin implementation.
 *
 * Typically, a simple return with new is enough. For example:
 *
 * return new ROMIOPlugin();
 *
 */
IPlugin* getPluginInstance( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ROMIOPlugin: call to getPluginInstance()\n" );
    //return (IPlugin*) new ROMIOPlugin();
    return new ROMIOPlugin();
}

/*
 * Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 */
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ROMIOPlugin: call to getInterfaceVersionMajor()\n" );
    return 1;
}

/*
 * Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 */
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ROMIOPlugin: call to getInterfaceVersionMinor()\n" );
    return 0;
}

/*
 * Returns a string with the name of the plugin.
 */
string getName( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ROMIOPlugin: call to getName()\n" );
    return "ROMIO Plugin";
}

/*
 * Returns a string with a short description of the plugin.
 */
string getShortSummary( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "ROMIOPlugin: call to getShortSummary()\n" );

    return "Tunes the runtime parameters of ROMIO MPI-IO implementations";
}
