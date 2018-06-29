/**
   @file    MPITCPlugin.cc
   @ingroup MPITCPlugin
   @brief   MPIT Custom Plugin
   @author  Aras Atalar
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

#include "MPITCPlugin.h"

int MPITCPlugin::collectiveRange( string  coll_name,
                                  Region* phase_region,
                                  Region* coll_region ) {
    if( coll_name == "MPI_BCAST" || coll_name == "MPI_ALLTOALL" || coll_name == "MPI_ALLGATHER"
        || coll_name == "MPI_ALLGATHERV" || coll_name == "MPI_REDUCE_SCATTER_BLOCK" || coll_name == "MPI_REDUCE_SCATTER" ) {
        return 3;
    }
    else if( coll_name == "MPI_REDUCE" || coll_name == "MPI_ALLREDUCE" ) {
        return 2;
    }
    else {
        return 0;
    }
}
/*
 * Initialize the plugin's data structures.
 *
 * The tuning parameter list needs to be created.
 *
 * Search algorithms are loaded here when required.  This can be done as follows:
 *
 *    searchAlgorithm = loadSearchAlgorithm("name");
 *
 * where "name" refers to one of the available search algorithms (currently only exhaustive).
 *
 */
void MPITCPlugin::initialize( DriverContext*   context,
                              ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPITCPlugin: call to initialize()\n" );

    Region* user_region = app.get_phase_region();

    vector<TuningParameter*> tps = extractTuningParameters();
    extractMPICallRegions();
    if( tps.empty() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                    "[#### AUTOTUNE ####]: No tuning parameters found. Exiting.\n" );
        throw PTF_PLUGIN_ERROR( TUNING_PARAMETERS_NOT_FOUND );
    }
    tuningParameters = tps;

    if( !user_region ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "[#### AUTOTUNE ####]: No phase region found. Exiting.\n" );
        throw PTF_PLUGIN_ERROR( TUNING_PARAMETERS_NOT_FOUND );
    }

    coll_tp_count = 0;
    p2p_tp_count  = 0;
    TuningParameter* tp;

    stringstream temp;

    /*Get phase region from instrumentation and only search collectives encapsulated by it*/
    for( int i = 0; i < 2; i++ ) {
        tp = new TuningParameter();
        tp->setId( i );
        if( !i ) {
            tp->setName( "PSC_INTERNODE" );
        }
        else {
            tp->setName( "PSC_INTRANODE" );
        }
        tp->setPluginType( UNKOWN_PLUGIN );
        tp->setRuntimeActionType( TUNING_ACTION_VARIABLE_INTEGER );
        //tp->setRuntimeActionType(TUNING_ACTION_FUNCTION_POINTER);
        tp->setRange( 1, 10, 1 ); // start, end, step
        //tp->setRegion(user_region /*regions.front() tps[0]->getRegion()*/);

        Restriction* res = new Restriction();
        res->setRegion( user_region );
        res->setRegionDefined( true );
        tp->setRestriction( res );

        p2p_tuning_parameters.push_back( tp );
        p2p_tp_count++;
    }

    mpi_coll_regions = extractMPICallRegions();

    std::list<Region*>::const_iterator iterator;
    int                                range = 0;
    for( iterator = mpi_coll_regions.begin(); iterator != mpi_coll_regions.end(); ++iterator ) {
        std::string upper_case_name = ( *iterator )->get_name();
        boost::to_upper( upper_case_name );
        cout << upper_case_name << endl;
        range = collectiveRange( upper_case_name, user_region, ( *iterator ) );
        cout << upper_case_name << " and " << range << endl;

        if( range /*&& (*iterator)->get_parent()->get_ident() == p2p_tuning_parameters[0]->getRegion()->get_ident()*/ ) {
            std::cout << ( *iterator )->get_name() << endl;
            tp = new TuningParameter();
            tp->setId( p2p_tp_count + coll_tp_count );
            tp->setName( "PSC_" + upper_case_name );
            cout << tp->getName() << endl;
            tp->setPluginType( UNKOWN_PLUGIN );
            tp->setRuntimeActionType( TUNING_ACTION_VARIABLE_INTEGER );
            //tp->setRuntimeActionType(TUNING_ACTION_FUNCTION_POINTER);
            tp->setRange( 1, range, 1 ); // start, end, step
            //tp->setRegion(*iterator);
            Restriction* res = new Restriction();
            res->setRegion( *iterator );
            res->setRegionDefined( true );
            tp->setRestriction( res );
            coll_tuning_parameters.push_back( tp );
            coll_tp_count++;
        }
    }

    p2p_best_value_time[ 1 ] = 3; //16K, default internode threshold - MVAPICH
    p2p_best_value_time[ 3 ] = 5; //64K, default intranode threshold
    coll_best_value_time     = NULL;

    /*For each collective paramter, 3 doubles are allocated. In the first one, the best time, the best algorithm and
     * the previous experimented algorithms is kept.*/
    if( coll_tp_count ) {
        tuning_step          = TUNING_COLLECTIVES;
        coll_best_value_time = ( double* )calloc( 3 * coll_tp_count, sizeof( double ) );
        if( !coll_best_value_time ) {
            perror( "Can not allocate memory for collective optimal values\n" );
            throw 0;
        }
    }
    else {
        tuning_step = TUNING_P2P;
    }
}

/**
 * @brief Defines a pre-analysis strategy.
 *
 * This plugin does not use a pre-analysis strategy.
 *
 * @ingroup MPITCPlugin
 *
 * @return false
 *
 */
bool MPITCPlugin::analysisRequired( StrategyRequest** strategy ) {
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
void MPITCPlugin::startTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPITCPlugin: call to StartTuningStep()\n" );

    switch( tuning_step ) {
    case TUNING_COLLECTIVES:
    case TUNING_COLLECTIVES_LAST:
        /*Start with tuning of collectives if any*/
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                    "[#### AUTOTUNE ####]: Collective Tuning Step is Starting\n" );
        coll_tuning_passes = 0;

        break;
    case TUNING_P2P:
    case TUNING_P2P_LAST:

        /*Second tuning step is to tune p2p parameters*/
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "[#### AUTOTUNE ####]:  P2P Tuning Step is Starting\n" );
        p2p_tuning_passes = 0;

        break;
    case TUNING_MAPPING:
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "[#### AUTOTUNE ####]:  Mapping Tuning Step is Starting\n" );

        break;
    default:

        perror( "Unexpected behavior in start tuning step\n" );
        throw 0;
        break;
    }
}

/*
 * The Created Scenario Pool (csp) is populated here.
 *
 * The scenarios need to be created and added to the first pool.  To create the scenarios, a
 * search algorithm can be used or it can be done directly by the plugin.
 *
 * After this step, the Periscope will verify that scenarios were added to the csp.
 *
 */
void MPITCPlugin::createScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPITCPlugin: call to createScenarios()\n" );

    map<TuningParameter*, int> phase_values;

    phase_values[ tuningParameters[ 0 ] ] = 777;
    switch( tuning_step ) {
    //uncomment the tuning parameter values, I get an error for FT if I set these parameters
    case TUNING_COLLECTIVES:
    case TUNING_COLLECTIVES_LAST:

//    phase_values[p2p_tuning_parameters[INTERNODE]] = 2048*pow(2, p2p_best_value_time[1]);
//    phase_values[p2p_tuning_parameters[INTRANODE]] = 2048*pow(2, p2p_best_value_time[3]);

        for( int i = 0; i < coll_tuning_parameters.size(); i++ ) {
//      phase_values[coll_tuning_parameters[i]] = ((int)coll_best_value_time[i * 3 + 2] %
//                                                coll_tuning_parameters[i]->getRangeTo()) + 1;

            cout << "In Scenario Collective " << coll_tuning_parameters[ i ]->getName() << " at line "
                 << coll_tuning_parameters[ i ]->getRestriction()->getRegion()->get_ident().start_position << " is set to: "
                 << ( ( int )coll_best_value_time[ i * 3 + 2 ] % coll_tuning_parameters[ i ]->getRangeTo() ) + 1 << endl;
        }
        break;
    case TUNING_P2P:
    case TUNING_P2P_LAST:

//    phase_values[p2p_tuning_parameters[INTERNODE]] = 2048 * pow(2, ((p2p_tuning_passes % 10) * (1- p2p_tuning_passes / 10)) +
//                                                     (p2p_best_value_time[1] * (p2p_tuning_passes / 10)));
//    phase_values[p2p_tuning_parameters[INTRANODE]] = 2048*pow(2, ((p2p_tuning_passes % 10)*(p2p_tuning_passes / 10)) +
//                                                     (p2p_best_value_time[3]*(1 - p2p_tuning_passes / 10)));

        /*Set best algorithms found for the collectives if enough experiments are done*/
//    for (int i = 0; i < coll_tuning_parameters.size(); i++) {
//      if (coll_best_value_time[i*3+2] < coll_tuning_parameters[i]->getRangeTo())
//        phase_values[coll_tuning_parameters[i]] = 0; //use default library selection
//      else
//        phase_values[coll_tuning_parameters[i]] = coll_best_value_time[i * 3 + 1];
//    }
        cout << "In Scenario Internode Value: "
             << 2048 * pow( 2, ( ( p2p_tuning_passes % 10 ) * ( 1 - p2p_tuning_passes / 10 ) )
                       + ( p2p_best_value_time[ 1 ] * ( p2p_tuning_passes / 10 ) ) ) << " and Intranode Value:"
             << 2048 * pow( 2, ( ( p2p_tuning_passes % 10 ) * ( p2p_tuning_passes / 10 ) )
                       + ( p2p_best_value_time[ 3 ] * ( 1 - p2p_tuning_passes / 10 ) ) ) << endl;

        break;
    case TUNING_MAPPING:

//    phase_values[p2p_tuning_parameters[INTERNODE]] = 2048*pow(2, p2p_best_value_time[1]);
//    phase_values[p2p_tuning_parameters[INTRANODE]] = 2048*pow(2, p2p_best_value_time[3]);
//    for (int i = 0; i < coll_tuning_parameters.size(); i++)
//      phase_values[coll_tuning_parameters[i]]= coll_best_value_time[i*3+1];
        break;
    default:

        perror( "Unexpected behavior in start tuning step\n" );
        throw 0;
        break;
    }

    Variant*                    variant = new Variant( phase_values );
    list<TuningSpecification*>* ts_in   = new list<TuningSpecification*>();
    ts_in->push_back( new TuningSpecification( variant ) );
    list<PropertyRequest*>* pr_in = new list<PropertyRequest*>();

    //here add regions to property request... mpi_coll_regions + user_region
    pr_in->push_back( new PropertyRequest( new list<int>) );
    pr_in->front()->addPropertyID( EXECTIME );

    //change region..
    Scenario* scenario = new Scenario( tuningParameters[ 0 ]->getRestriction()->getRegion(), ts_in, pr_in );
    csp->push( scenario );
    psc_dbgmsg( PSC_AUTOTUNE_ALL_DEBUG, "Added Scenario(%d) to the scenario pool\n", scenario->getID() );
}

/*
 * Preparatory steps for the scenarios are done here.
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
 */
void MPITCPlugin::prepareScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPITCPlugin: call to prepareScenarios()\n" );
    while( !csp->empty() ) {
        psp->push( csp->pop() );
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
void MPITCPlugin::defineExperiment( int               numprocs,
                                    bool&             analysisRequired,
                                    StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPITCPlugin: call to defineExperiment()\n" );
    while( !psp->empty() ) {
        esp->push( psp->pop() );
    }
}

/*
 * Return true if a restart of the application is required for the next experiment,
 * false otherwise.
 */
bool MPITCPlugin::restartRequired( std::string& env,
                                   int&         numprocs,
                                   std::string& command,
                                   bool&        is_instrumented ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPITCPlugin: call to restartRequired()\n" );
    /*Restart to ensure application termination to create communication matrix in MPI_Finalize*/
    if( tuning_step == TUNING_MAPPING ) {
        return true;
    }
    return false; // no restart required
}

double MPITCPlugin::getRegionTime( int              scenario_id,
                                   TuningParameter* tp ) {
    /*scenario out of range*/
    if( scenario_id > fsp->size() - 1 ) {
        return -1.0;
    }

    stringstream temp;
    temp << tp->getRestriction()->getRegion()->get_ident().file_id << "-"
         << tp->getRestriction()->getRegion()->get_ident().rfl;
    double max = -1.0;

    if( !srp->getScenarioResultsByID( scenario_id ).empty() ) {
        /*Not sure whether it is the right way to obtain region measurements for each parameter*/
        list<MetaProperty> properties = srp->getScenarioResultsByID( scenario_id );
        for( std::list<MetaProperty>::iterator iterator = properties.begin(), end = properties.end(); iterator != end;
             ++iterator ) {
            cout << "	"<< iterator->getSeverity();
            if( temp.str() == iterator->getRegionId() && iterator->getSeverity() > max ) {
                max = iterator->getSeverity();
            }
        }
    }
    cout << endl;
    cout << "Scenario " << scenario_id << " :" << max << endl;
    return max;
}

/*
 * Return true if if the current search iteration is finished (by either the plugin itself or as
 * a result of the search algorithm), false otherwise.
 */
bool MPITCPlugin::searchFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPITCPlugin: call to searchFinished()\n" );

    double max;

    switch( tuning_step ) {
    case TUNING_COLLECTIVES:
    case TUNING_COLLECTIVES_LAST:

        for( int i = 0; i < coll_tp_count; i++ ) {
            max = getRegionTime( fsp->size() - 1, coll_tuning_parameters[ i ] );

            //this line is put to show the parameter space is swept is measurement is availble, if not remains same..
            if( coll_tuning_parameters[ i ]->getRestriction()->getRegion()->get_ident().start_position == 34 ) {
                max = 1;
            }
            /*If region is executed*/
            if( max > 0.0 ) {
                if( ( coll_best_value_time[ i * 3 ] > max
                      || ( !coll_best_value_time[ i * 3 + 2 ] && tuning_step == TUNING_COLLECTIVES ) ) ) {
                    coll_best_value_time[ i * 3 ]     = max;
                    coll_best_value_time[ i * 3 + 1 ] = ( ( int )coll_best_value_time[ i * 3 + 2 ]
                                                          % coll_tuning_parameters[ i ]->getRangeTo() ) + 1;
                }
                coll_best_value_time[ i * 3 + 2 ]++;     //increment the experiment counter if a region is executed
            }
        }

        coll_tuning_passes++;
        if( coll_tuning_passes >= 6 ) {
            return true;
        }
        break;

    case TUNING_P2P:
    case TUNING_P2P_LAST:

        if( p2p_tuning_passes < 10 ) {
            //get the time for the last scenario
            max = getRegionTime( fsp->size() - 1, p2p_tuning_parameters[ INTERNODE ] );
            if( ( p2p_best_value_time[ 0 ] > max || ( !p2p_tuning_passes && tuning_step == TUNING_P2P ) ) && max > 0.0 ) {
                p2p_best_value_time[ 0 ] = max;
                p2p_best_value_time[ 1 ] = p2p_tuning_passes % 10;
            }
        }
        else {
            //get the time for the last scenario
            max = getRegionTime( fsp->size() - 1, p2p_tuning_parameters[ INTRANODE ] );
            if( ( p2p_best_value_time[ 2 ] > max || ( !( p2p_tuning_passes % 10 ) && tuning_step == TUNING_P2P ) ) && max > 0.0 ) {
                p2p_best_value_time[ 2 ] = max;
                p2p_best_value_time[ 3 ] = p2p_tuning_passes % 10;
            }
        }

        /*Scenarios are overwritten in automatic restart, this avoids that*/
        if( max >= 0.0 ) {
            p2p_tuning_passes++;
        }
        if( p2p_tuning_passes >= 20 ) {
            return true;
        }
        break;
    case TUNING_MAPPING:
        return true;
        break;
    default:
        perror( "Unexpected behavior in start tuning step\n" );
        throw 0;
        break;
    }
    return false;
}

/*
 * Final operation of a tuning step.
 *
 * If any post-processing is necessary before entering the next tuning iteration, it is to be
 * done here.
 */
void MPITCPlugin::finishTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPITCPlugin: call to processResults()\n" );
}

/*
 * Returns true if the plugin finished the tuning process, false otherwise.
 */
bool MPITCPlugin::tuningFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPITCPlugin: call to tuningFinished()\n" );

    if( !coll_tp_count && tuning_step == TUNING_P2P ) {
        tuning_step = tuning_step_t( int( tuning_step ) + 2 );
    }
    else {
        tuning_step = tuning_step_t( int( tuning_step ) + 1 );
    }
    return tuning_step == TUNING_DONE;
}

/*
 * Prints to the screen (and to a file, where necessary) the tuning advice.
 */
Advice* MPITCPlugin::getAdvice( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPITCPlugin: call to getAdvice()\n" );

    /*Present best values*/
    cout << "Optimal Intranode Threshold: " << 2048 * pow( 2, p2p_best_value_time[ 3 ] ) << endl;
    cout << "Optimal Internode Threshold: " << 2048 * pow( 2, p2p_best_value_time[ 1 ] ) << endl;

    for( int i = 0; i < coll_tp_count; i++ ) {
        /*present the optimal value only if enough experiments are done*/
        if( coll_best_value_time[ i * 3 + 2 ] >= coll_tuning_parameters[ i ]->getRangeTo() ) {
            cout << "Optimal Algorithm for " << coll_tuning_parameters[ i ]->getName() << " at line "
                 << coll_tuning_parameters[ i ]->getRestriction()->getRegion()->get_ident().start_position << " is: "
                 << coll_best_value_time[ i * 3 + 1 ] << endl;
        }
    }
    /*Mapping Advice*/
    createMapping( atoi( opts.mpinumprocs_string ) );
    // TODO make the proper advice here
    return new Advice();
}

/*
 * Finalize the plugin normally.
 *
 * Remove any allocated memory, objects, file descriptors, etc.
 *
 */
void MPITCPlugin::finalize() {
    terminate();
}

/*
 * Terminate the plugin due to error.
 *
 * Safely remove any allocated memory, objects, file descriptors, etc. This method should
 * be able to be executed safely at any point.
 *
 */
void MPITCPlugin::terminate() {
    if( coll_best_value_time ) {
        free( coll_best_value_time );
    }
}

/**
 * @brief Returns an instance of this particular plugin implementation.
 *
 * @ingroup MPITCPlugin
 *
 * @return A pointer to an instance of this particular plugin implementation.
 */
IPlugin* getPluginInstance( void ) {
    return new MPITCPlugin();
}

/**
 * @brief Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 *
 * @ingroup MPITCPlugin
 *
 * @return The major plugin interface version used by the plugin
 */
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPITCPlugin: call to getInterfaceVersionMajor()\n" );

    return 1;
}

/**
 * @brief Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 *
 * @ingroup MPITCPlugin
 *
 * @return The minor plugin interface version used by the plugin
 *
 */
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPITCPlugin: call to getInterfaceVersionMinor()\n" );

    return 0;
}

/**
 * @brief Returns a string with the name of the plugin.
 *
 * @ingroup MPITCPlugin
 *
 * @return A string with the name of the plugin
 *
 */
string getName( void ) {
    return "MPIT Custom Plugin";
}

/**
 * @brief Returns a string with a short description of the plugin.
 *
 * @ingroup MPITCPlugin
 *
 * @return A string with a short description of the plugin
 *
 */
string getShortSummary( void ) {
    return "MPI Runtime Communication Parameters Tuning Plugin";
}

void MPITCPlugin::createMapping( int proc_no ) {
    char const* mapping_file = getenv( "ATALAR_MAPPING_FILE" );
    if( !mapping_file ) {
        cout << "Not creating mapping information: Communication matrix path is not specified" << endl;
        return;
    }

    FILE* fp;

    long** comm_Matrix;
    comm_Matrix = ( long** )malloc( proc_no * sizeof( long* ) );
    for( int i = 0; i < proc_no; i++ ) {
        comm_Matrix[ i ] = ( long* )calloc( proc_no, sizeof( long ) );
        if( !comm_Matrix[ i ] ) {
            cout << "Can not allocate memory for comm matrix" << endl;
            throw 0;
        }
    }

    std::ostringstream oss;
    int                rank;
    long               value;
    long               average    = 0;
    int                element_no = 0;

    for( int i = 0; i < proc_no; i++ ) {
        oss.str( std::string() );
        oss.clear();
        oss << mapping_file << "_" << i;
        fp = fopen( oss.str().c_str(), "r" );
        if( !fp ) {
            cout << "Not creating mapping information" << endl;
            for( int i = 0; i < proc_no; i++ ) {
                free( comm_Matrix[ i ] );
            }
            free( comm_Matrix );
            return;
        }

        rank = 0;
        while( fscanf( fp, "%d %ld", &rank, &value ) != EOF && ( rank < proc_no ) ) {
            /*Performance model minimum value*/
            if( value > 20000 ) {
                comm_Matrix[ i ][ rank ] += value;
                comm_Matrix[ rank ][ i ] += value;
                element_no++;
                average = average + ( value * 2 - average ) / element_no;
            }
        }
        fclose( fp );
    }

    typedef adjacency_list<vecS, vecS, undirectedS,
                           property<vertex_color_t, default_color_type, property<vertex_degree_t, int> > > Graph;
    typedef graph_traits<Graph>::vertex_descriptor Vertex;
    typedef graph_traits<Graph>::vertices_size_type size_type;
    Graph G( proc_no );

    for( int i = 0; i < proc_no; i++ ) {
        for( int j = i; j < proc_no; j++ ) {
            //should be bigger than 80 percent than average
            if( comm_Matrix[ i ][ j ] > average * 8 / 10 ) {
                add_edge( ( size_t )i, ( size_t )j, G );
            }
        }
    }

    graph_traits<Graph>::vertex_iterator ui, ui_end;

    property_map<Graph, vertex_degree_t>::type deg = get( vertex_degree, G );
    for( boost::tie( ui, ui_end ) = vertices( G ); ui != ui_end; ++ui ) {
        deg[ *ui ] = degree( *ui, G );
    }

    property_map<Graph, vertex_index_t>::type index_map = get( vertex_index, G );

    std::cout << "original bandwidth: " << bandwidth( G ) << std::endl;

    std::vector<Vertex>    inv_perm( num_vertices( G ) );
    std::vector<size_type> perm( num_vertices( G ) );

//  {
//    Vertex s = vertex(proc_no/2, G);
//    //reverse cuthill_mckee_ordering
//    cuthill_mckee_ordering(G, s, inv_perm.rbegin(), get(vertex_color, G),
//                           get(vertex_degree, G));
//    cout << "Reverse Cuthill-McKee ordering starting at: " << s << endl;
//    cout << "  ";
//    for (std::vector<Vertex>::const_iterator i = inv_perm.begin();
//         i != inv_perm.end(); ++i)
//      cout << index_map[*i] << " ";
//      cout << endl;
//
//      for (size_type c = 0; c != inv_perm.size(); ++c)
//        perm[index_map[inv_perm[c]]] = c;
//        std::cout << "  bandwidth: "
//                  << bandwidth(G, make_iterator_property_map(&perm[0], index_map, perm[0]))
//                  << std::endl;
//  }
//  {
//    Vertex s = vertex(0, G);
//    //reverse cuthill_mckee_ordering
//    cuthill_mckee_ordering(G, s, inv_perm.rbegin(), get(vertex_color, G),
//                           get(vertex_degree, G));
//    cout << "Reverse Cuthill-McKee ordering starting at: " << s << endl;
//    cout << "  ";
//    for (std::vector<Vertex>::const_iterator i = inv_perm.begin();
//         i != inv_perm.end(); ++i)
//      cout << index_map[*i] << " ";
//      cout << endl;
//
//      for (size_type c = 0; c != inv_perm.size(); ++c)
//        perm[index_map[inv_perm[c]]] = c;
//        std::cout << "  bandwidth: "
//                  << bandwidth(G, make_iterator_property_map(&perm[0], index_map, perm[0]))
//                  << std::endl;
//  }

    {
        //reverse cuthill_mckee_ordering
        cuthill_mckee_ordering( G, inv_perm.rbegin(), get( vertex_color, G ), make_degree_map( G ) );

        cout << "Reverse Cuthill-McKee ordering:" << endl;
        cout << "  ";
        for( std::vector<Vertex>::const_iterator i = inv_perm.begin(); i != inv_perm.end(); ++i ) {
            cout << index_map[ *i ] << " ";
        }
        cout << endl;

        for( size_type c = 0; c != inv_perm.size(); ++c ) {
            perm[ index_map[ inv_perm[ c ] ] ] = c;
        }
        std::cout << "  bandwidth: " << bandwidth( G, make_iterator_property_map( &perm[ 0 ], index_map, perm[ 0 ] ) ) << std::endl;
    }

    for( int i = 0; i < proc_no; i++ ) {
        free( comm_Matrix[ i ] );
    }
    free( comm_Matrix );
}
