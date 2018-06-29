/**
   @file    MasterWorkerPlugin.cc
   @ingroup MasterWorkerPlugin
   @brief   Master-Worker Plugin
   @author  Toni Pimenta, Gertvjola Saveta, Eduardo César, Anna Sikora
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

#include "MasterWorkerPlugin.h"
#include <sstream>
#include <global.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "DriverContext.h"
#include <application.h>
#include "StrategyRequest.h"
#include <cmath>
#include <float.h>

/**
 * @brief Estimates the partition factor using analytical simulation
 *
 * This method uses the measurements made during the first application pre-analysis and
 * the inputs provided in the configuration file to simulate (using an analytical model)
 * the application execution using different partition factors.
 * The algorithm starts with a partition factor of 0.1 (distributes 10% of tasks in the first
 * iteration, 10% of the remainder tasks in the second and so on and so forth). The partition
 * factor is incremented by 0.05 in each simulation.
 * For each partition factor simulates the distribution of tasks among workers and uses the following
 * expression to estimate the execution time of each worker:
 *
 * 2* netl + ((task_sz * chunk)+avg_answer_size) * nets + wmexct[k] * chunk
 *
 * Time is advanced to the first worker to finish its assigned chunk of tasks and, if available,
 * a new chunk is assigned to the worker. Time is then advanced to the next worker to finish its
 * assigned chunk, continuing the process until all tasks has been processed.
 * The partition factor that leads to the lowest execution time is returned by the function.
 *
 * @param num_w number of current application workers
 * @param num_tasks total number of tasks to be processed (Bytes of data/size of task)
 * @param avg_answer_size bytes sent back to the master by each worker
 * @param task_sz size in bytes of each task to be processed by the workers
 * @param netl mean latency of MPI functions (user provided)
 * @param nets mean inverse bandwidth (user provided)
 * @param wmexct[] mean execution time of each worker
 *
 * @ingroup MasterWorkerPlugin
 */
float MasterWorkerPlugin::pfsim( unsigned int num_w,
                                 unsigned int num_tasks,
                                 unsigned int avg_answer_size,
                                 unsigned int task_sz,
                                 float        netl,
                                 float        nets,
                                 float        wmexct[] ) {
    float  maxext, simt, min_simt = INFINITY;
    float  pf = 0.1, bpf;
    float* exect;
    int    pend_tks, chunk, i, j, k;

    exect = ( float* )malloc( num_w * sizeof( float ) );

    while( pf < 1.0 ) {
        pend_tks = num_tasks;
        for( i = 0; i < num_w; i++ ) {
            exect[ i ] = 0.0;
        }
        while( pend_tks > 0 ) {
            if( pend_tks <= 2 * num_w ) {
                chunk    = ceil( pend_tks / ( num_w * 1.0 ) );
                pend_tks = 0;
            }
            else {
                chunk     = ceil( ( pf * pend_tks ) / ( num_w * 1.0 ) );
                pend_tks -= ( chunk * num_w );
            }

            for( i = 0; i < num_w; i++ ) {
                simt = exect[ 0 ];
                k    = 0;
                for( j = 1; j < num_w; j++ ) {
                    if( exect[ j ] < simt ) {
                        k    = j;
                        simt = exect[ j ];
                    }
                }
                exect[ k ] += 2 * netl + ( ( task_sz * chunk ) + avg_answer_size ) * nets + wmexct[ k ] * chunk;
            }
        }
        maxext = exect[ 0 ];
        for( i = 1; i < num_w; i++ ) {
            if( exect[ i ] > maxext ) {
                maxext = exect[ i ];
            }
        }
        if( maxext < min_simt ) {
            min_simt = maxext;
            bpf      = pf;
        }
        pf += 0.05;
    }
    free( exect );
    return bpf;
}

/**
 * @brief Estimates the proper number of workers for the application.
 *
 * This method uses the measurements made during the second tuning step (if necessary) and user
 * provided parameters (configuration file) for estimating the best number of workers for the
 * application. It uses analytical expressions for estimating the application execution time for a
 * given number of workers, then it computes the relationship between the efficiency for the same
 * number of workers and the total computation time. This operation is done iterativelly increasing
 * the number of workers by 1. Stoping when the difference between two iterations is smaller than 0.25%.
 * Finally, the number used in the last iteration -1 is returned.
 *
 * @param total_bytes_transferred number of bytes communicated between the master and the workers
 * @param total_comp_time total execution time spent by the workers to process all tasks
 * @param lambda network inverse bandwidth (user provided)
 * @param latency MPI mean calls communication latency (user provided)
 *
 * @ingroup MasterWorkerPlugin
 */
int MasterWorkerPlugin::CalculatePi( int    total_bytes_transferred,
                                     double total_comp_time,
                                     double lambda,
                                     double latency ) {
    double Tt;
    double previous_index;
    int    workers = 2;
    double workers2;
    double index = DBL_MAX;
    do {
        previous_index = index;
        if( latency >= ( ( lambda * total_bytes_transferred ) / workers ) ) {
            Tt = ( ( workers + 1 ) * latency + ( ( total_comp_time + ( total_bytes_transferred * lambda ) ) / workers ) );
        }
        else {
            Tt = ( ( 2 * latency ) + ( ( ( workers - 1 ) * lambda * total_bytes_transferred ) + total_comp_time ) / workers );
        }
        index = ( workers * ( ( Tt ) * ( Tt ) ) ) / total_comp_time;
        workers++;
    }
    while( ( previous_index > index ) && ( abs( previous_index - index ) > ( 0.0025 * previous_index ) ) );

    return workers - 1;
}


/**
 * @brief Extracting Tuning Parameters from the Configuration File
 *
 * This method initizalizes the following member variables using the values obtained form the
 * configuration file:
 *
 * mpi_worker_recv: vector for storing the lines of all MPI_Recv calls in the workers used for
 *                  receiving tasks from the master.
 * mpi_master_recv: vector for storing the lines of all MPI_Recv calls in the master used for
 *                  receiving results from the workers.
 * mpifunction: initial line of the function called in the workers to process tasks.
 * mpilatency: MPI calls latency in seconds.
 * mpilambda: mean inverse bandwidth in seconds (time to send a byte)
 * mpitksz: task size in bytes.
 * mpithreshold: relative differentce between the slowest and the fastest worker from which a
 *               imbalance is considered to exist (%)
 *
 * @ingroup MasterWorkerPlugin
 */
void MasterWorkerPlugin::extractTuningParametersFromConfigurationFile() {
    mpitpnw.ptype     = unknown;
    mpifunction.ptype = unknown;
    mpilatency.ptype  = unknown;
    mpilamda.ptype    = unknown;
    char const* param_spec_file = getenv( "PSC_PARAM_SPEC_FILE" );

    if( param_spec_file != NULL ) {
        conftps = getTuningParametersMW( param_spec_file );
    }
    else {
        conftps = getTuningParametersMW( "./param_spec.conf" );
    }

    for( int i = 0; i < conftps.size(); i++ ) {
        MW_TP aux = conftps.at( i );
        if( aux.ptype == WRECV ) {
            mpi_worker_recv.push_back( aux );
        }
        if( aux.ptype == MRECV ) {
            mpi_master_recv.push_back( aux );
        }
        if( aux.ptype == WFUNCT ) {
            mpifunction = aux;
        }
        if( aux.ptype == NLATENCY ) {
            mpilatency = aux;
        }
        if( aux.ptype == NSPEED ) {
            mpilamda = aux;
        }
        if( aux.ptype == TSIZE ) {
            mpitksz = aux;
        }
        if( aux.ptype == UNTHR ) {
            mpithreshold = aux;
        }
    }
    if( conftps.empty() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                    "[#### AUTOTUNE ####]: No tuning parameters found in specification file file, set PSC_PARAM_SPEC_FILE to correct file path. Exiting.\n" );
        throw PTF_PLUGIN_ERROR( TUNING_PARAMETERS_NOT_FOUND );
    }
}

/**
 * @brief Initializes the MasterWorker plugin.
 * @ingroup MasterWorkerPlugin
 *
 * Plugin initialization. contex and pool_set are assigned to the plugin and the exhaustive search
 * algorithm loaded.
 *
 * @param context Context.
 * @param pool_set Set of Scenario Pools
 *
 */
void MasterWorkerPlugin::initialize( DriverContext*   context,
                                     ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: call to initialize()\n" );
    this->context  = context;
    this->pool_set = pool_set;
    extractTuningParametersFromConfigurationFile();

    tuningStep = 0;

    int    major, minor;
    string name, description;

    context->loadSearchAlgorithm( "exhaustive", &major, &minor, &name, &description );
    searchAlgorithm = context->getSearchAlgorithmInstance( "exhaustive" );

    if( searchAlgorithm != NULL ) {
        print_loaded_search( major, minor, name, description );
        searchAlgorithm->initialize( context, pool_set );
    }
    else {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: NULL pointer in searchAlgorithm\n" );
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: initialize() done\n" );
}

/**
 * @brief Defines a pre-analysis strategy.
 *
 * Pre-analysis is required for both tuning steps of the plugin.
 * For the first tuning step: the Configurable Analysis Strategy is used.
 * Asking for two different properties. The MPITIME property, to calculate the
 * aggregated average message size. This property is asked for the worker recv operations that receive
 * tasks from the master. And the EXECTIME property, for the execution time of the workers,
 * for the function where tasks to be executed by each worker are processed.
 * For the second tuning step: a second
 * pre-analysis is done for tuning step two. In this step, we use the Autotune Analysis Strategy,
 * and as a sub-strategy we make use of the Configurable Analysis Strategy. This strategy allows
 * for using the partition factor estimated in the first tuning step in the second pre-analysis.
 * The regions and the properties defined are the same as in the first tuning step.
 *
 * Note: during the second tuning step the pre-analysis is not necessary if the partition factor
 *	 estimated during the first tuning step is 1 (all tasks will be distributed at once). So,
 *       this modification (that will decrease the tuning time in this case) should be implemented.
 *
 * @ingroup MasterWorkerPlugin
 * @return true in both tuning steps if the application is instrumented, false if not.
 */
bool MasterWorkerPlugin::analysisRequired( StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: call to analysisRequired()\n" );
    if( context->applUninstrumented() ) {
        for( int i = 0; i < conftps.size(); i++ ) {
            MW_TP aux = conftps.at( i );
            if( aux.ptype == NUM_W ) {
                mpitpnw = aux;
            }
        }
        return false;
    }
    else {
        if( ( tuningStep == 1 ) && !context->applUninstrumented() ) {
            StrategyRequestGeneralInfo* analysisStrategyGeneralInfo = new StrategyRequestGeneralInfo;
            list<PropertyRequest*>*     reqList                     = new list<PropertyRequest*>;

            analysisStrategyGeneralInfo->strategy_name     = "ConfigAnalysis";
            analysisStrategyGeneralInfo->pedantic          = 0;
            analysisStrategyGeneralInfo->delay_phases      = 0;
            analysisStrategyGeneralInfo->delay_seconds     = 0;
            analysisStrategyGeneralInfo->analysis_duration = 1;

            for( vector<MW_TP>::iterator aux  = mpi_worker_recv.begin(); aux < mpi_worker_recv.end(); aux++ ) {
                PropertyRequest* req = new PropertyRequest();

                req->addPropertyID( MPITIME );
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPI Worker Recv Region (%d,%d) \n", aux->start, aux->end );

                Region* reg = appl->searchRegion( aux->start, aux->end );
                if( !reg ) {
                    psc_errmsg( "Task distribution region (%d,%d) not found!\n", aux->start, aux->end );
                    throw 0;
                }

                req->addRegion( reg );

                req->addAllProcesses();

                reqList->push_back( req );
            }

            for( vector<MW_TP>::iterator aux  = mpi_master_recv.begin(); aux < mpi_master_recv.end(); aux++ ) {
                PropertyRequest* req = new PropertyRequest();

                req->addPropertyID( MPITIME );
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MPI Master Recv Region (%d,%d) \n", aux->start, aux->end );

                Region* reg = appl->searchRegion( aux->start, aux->end );
                if( !reg ) {
                    psc_errmsg( "Aswer back region (%d,%d) not found!\n", aux->start, aux->end );
                    throw 0;
                }

                req->addRegion( reg );

                req->addSingleProcess( 0 );

                reqList->push_back( req );
            }


            PropertyRequest* req = new PropertyRequest();
            req->addPropertyID( EXECTIME );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Do-Work Region (%d,%d) \n", mpifunction.start, mpifunction.end );

            Region* reg1 = appl->searchRegion( mpifunction.start, mpifunction.end );
            if( !reg1 ) {
                psc_errmsg( "Do-Work region (%d,%d) not found!\n", mpifunction.start, mpifunction.end );
                throw 0;
            }

            req->addRegion( reg1 );
            req->addAllProcesses();
            reqList->push_back( req );

            *strategy = new StrategyRequest( reqList, analysisStrategyGeneralInfo );
            ( *strategy )->printStrategyRequest();

            return true;
        }
        else {
            list<PropertyRequest*>* reqList = new list<PropertyRequest*>;
            TuningParameter*        tp;

            tp = new TuningParameter();
            tp->setId( 0 );
            tp->setName( "PartitionFactor" );
            tp->setPluginType( MPI );
            tp->setRuntimeActionType( TUNING_ACTION_VARIABLE_INTEGER );
            tp->setRange( bestPartitionFactor, bestPartitionFactor, 1 );

            variantSpace.clear();
            variantSpace.addTuningParameter( tp );

            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Creating a SearchSpace from the tuning parameters.\n" );

            searchAlgorithm->clear();
            searchSpace.setVariantSpace( &variantSpace );
            if( context->applUninstrumented() ) {
                searchSpace.addRegion( new Region() );
            }
            else {
                searchSpace.addRegion( appl->get_phase_region() );
            }
            searchAlgorithm->addSearchSpace( &searchSpace );

            searchAlgorithm->createScenarios();

            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "pre-analysis with tune-strategy...\n" );
            Scenario*             scenario;
            std::list<Scenario*>* scenariosList = new std::list<Scenario*>;

            scenario = pool_set->csp->pop();
            scenario->setSingleTunedRegionWithPropertyRank( appl->get_main_region(), EXECTIME, 0 );
            scenariosList->push_back( scenario );

            StrategyRequestGeneralInfo* strategyRequestGeneralInfo = new StrategyRequestGeneralInfo;
            strategyRequestGeneralInfo->strategy_name     = "Autotune";
            strategyRequestGeneralInfo->pedantic          = false;
            strategyRequestGeneralInfo->delay_phases      = 0;
            strategyRequestGeneralInfo->delay_seconds     = 0;
            strategyRequestGeneralInfo->analysis_duration = 1;
            StrategyRequest* strategy_request = new StrategyRequest( scenariosList, strategyRequestGeneralInfo );

            StrategyRequestGeneralInfo* subAnalysisStrategyRequest = new StrategyRequestGeneralInfo;
            subAnalysisStrategyRequest->strategy_name     = "ConfigAnalysis";
            subAnalysisStrategyRequest->pedantic          = 1;
            subAnalysisStrategyRequest->delay_phases      = 0;
            subAnalysisStrategyRequest->delay_seconds     = 0;
            subAnalysisStrategyRequest->analysis_duration = 1;

            for( vector<MW_TP>::iterator aux = mpi_worker_recv.begin(); aux < mpi_worker_recv.end(); aux++ ) {
                PropertyRequest* req = new PropertyRequest();

                req->addPropertyID( MPITIME );
                req->addRegion( appl->searchRegion( aux->start, aux->end ) );
                req->addAllProcesses();

                reqList->push_back( req );
            }

            for( vector<MW_TP>::iterator aux = mpi_master_recv.begin(); aux < mpi_master_recv.end(); aux++ ) {
                PropertyRequest* req = new PropertyRequest();

                req->addPropertyID( MPITIME );
                req->addRegion( appl->searchRegion( aux->start, aux->end ) );
                req->addSingleProcess( 0 );

                reqList->push_back( req );
            }


            PropertyRequest* req = new PropertyRequest();
            req->addPropertyID( EXECTIME );
            req->addRegion( appl->searchRegion( mpifunction.start, mpifunction.end ) );
            req->addAllProcesses();
            reqList->push_back( req );
            StrategyRequest* sub_strategy = new StrategyRequest( reqList, subAnalysisStrategyRequest );
            strategy_request->setSubStrategyRequest( sub_strategy );

            ( *strategy ) = strategy_request;
            ( *strategy )->printStrategyRequest();

            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: analysisRequired() done\n" );

            return true;
        }
    }
}

/**
 * @brief Operations to be done at the start of a tuning step.
 *
 * The tuning parameters that were extracted from the configuration file,
 * at initialization are checked if they are enough for the tuning steps.
 * @ingroup MasterWorkerPlugin
 *
 */
void MasterWorkerPlugin::startTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: call to startTuningStep()\n" );
    tuningStep++;


    if( context->applUninstrumented() && ( conftps.size() < 1 ) ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: Not enough Tuning Parameters" );
        throw 0;
    }
    else if( !context->applUninstrumented() && ( conftps.size() < 5 ) ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: Not enough Tuning Parameters" );
        throw 0;
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: startTuningStep() Done\n" );
}

/**
 * @brief The Created Scenario Pool (csp) is populated here.
 *
 * This method populates the scenario pool in each tuning step accordingly to the measurements
 * obtained in the pre-analisys done in each step and the results returned by the stimation methods.
 *
 * Fisrt tuning step: The message sizes obtained from the MPITIME property are used to compute the
 * total number of bytes sent to the workers (aggregated_answer_message_size), the number of tasks
 * sent to each worker (tasksPerWorker[]) (in this case the user provided task size is also used).
 * While the execution times obtained from the EXECTIME property are used to compute
 * the execution time of each worker (timePerTaskPerWorker), and the fastest (minExecTime) and
 * slowest (maxExecTime) workers. If the difference between the fastest and the slowest worker is
 * greater than the user specified threshold then an imbalance must be solved and the pfsim method
 * is called to estimate a partition factor that should balance the load. If the difference is smaller
 * than the threshold the partition factor is set to 100 (all tasks are distributed at once). As far as,
 * Periscope needs to execute a least one experiment before going to the second tuning step, a dummy
 * scenario is created and passed to the search algorithm. Results from this experiment are ignored.
 *
 * Second tuning step: The message sizes obtained form the MPI properties are used to compute the total
 * number of bytes communicated (total_bytes_transferred), while the execution times obtained from
 * the EXECTIME property are used to compute the total execution time (total_execution_time) of the
 * workers. These measurements are passed to the CalculatePi method that estimates the proper number
 * of workers for the application. As a result of this tuning step two tuning parameters are
 * created. One for the partition factor with a range of three values (estimated partition factor +-
 * 10%) if the partition factor is smaller than 100, or only one value (100) if not. The second
 * tuning parameter is for the number of workers, again it indicates a range with three values
 * (estimated number of workers +-10%). The search space of the search algorithm is finally created
 * with the crossproduct of these two tuning parameters (a total of 9 scenarios).
 *
 * @ingroup MasterWorkerPlugin
 */
void MasterWorkerPlugin::createScenarios( void ) {
    list < MetaProperty > properties;
    double                aggregated_average_message_sizes;
    double                aggregated_answer_message_size;
    double                execTime;
    double                average_message_size;
    int                   TotalCount;
    int*                  tasksPerWorker;
    float*                timePerTaskPerWorker;
    TuningParameter*      tp, * tp2;



    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: call to createScenarios()\n" );
    if( searchAlgorithm == NULL ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: Search algorithm not instantiated\n" );
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }

    if( tuningStep == 1 ) {
        if( context->applUninstrumented() ) {
            tp = new TuningParameter();
            tp->setId( 0 );
            tp->setName( "NumberOfWorkers" );
            tp->setPluginType( MPI );
            tp->setRuntimeActionType( TUNING_ACTION_NONE );
            tp->setRange( mpitpnw.start, mpitpnw.end, mpitpnw.step );
            numberOfWorkersTP = tp;
        }
        else {
            double                        msgSize;
            map<int, list<MetaProperty> > preAnalysisPropertiesMap;
            list<MetaProperty>::iterator  property;
            list<Region*>                 regions;
            list<Region*>::iterator       reg;

            int TotalCount;

            tasksPerWorker       = ( int* )calloc( context->getMPINumProcs(), sizeof( int ) );
            timePerTaskPerWorker = ( float* )calloc( context->getMPINumProcs() - 1, sizeof( float ) );

            map<int, list<MetaProperty> >::const_iterator propertyMapIter;
            int                                           total_mpitime_props = 0;
            double                                        minExecTime         = -1.0;
            double                                        maxExecTime         = -1.0;
            double                                        threshold;
            int                                           i = 0;
            preAnalysisPropertiesMap = pool_set->arp->getAllPreAnalysisProperties();
            for( propertyMapIter = preAnalysisPropertiesMap.begin(); propertyMapIter != preAnalysisPropertiesMap.end(); propertyMapIter++ ) {
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: Getting %d properties from element %d of the PreAnalysisPropertiesMap \n",  preAnalysisPropertiesMap.size(), i++ );

                properties = propertyMapIter->second;

                aggregated_average_message_sizes = 0.0;
                int propertyCount = 0;
                for( property = properties.begin(); property != properties.end(); property++, propertyCount++ ) {
                    if( atoi( property->getId().c_str() ) == MPITIME ) {
                        TotalCount = atoi( property->getExtraInfo().at( "TotalCount" ).c_str() );
                        if( property->getProcess() != 0 ) {
                            total_mpitime_props++;
                            aggregated_average_message_sizes            += atof( property->getExtraInfo().at( "MeanMsgSize" ).c_str() );
                            tasksPerWorker[ property->getProcess() - 1 ] = ( int )( atof( property->getExtraInfo().at( "MeanMsgSize" ).c_str() ) / mpitksz.start * TotalCount );
                        }
                        else {
                            aggregated_answer_message_size += atof( property->getExtraInfo().at( "MeanMsgSize" ).c_str() );
                        }
                    }
                    else if( atoi( property->getId().c_str() ) == EXECTIME ) {
                        execTime = atof( property->getExtraInfo().at( "ExecTime" ).c_str() );
                        if( minExecTime < 0.0 ) {
                            minExecTime = execTime;
                            maxExecTime = execTime;
                        }
                        else {
                            if( execTime < minExecTime ) {
                                minExecTime = execTime;
                            }
                            if( execTime > maxExecTime ) {
                                maxExecTime = execTime;
                            }
                        }
                        timePerTaskPerWorker[ property->getProcess() - 1 ] = atof( property->getExtraInfo().at( "ExecTime" ).c_str() );
                    }
                }
                threshold            = ( ( maxExecTime - minExecTime ) / minExecTime ) * 100;
                average_message_size = aggregated_average_message_sizes / total_mpitime_props;
            }
            for( int i = 0; i < context->getMPINumProcs() - 1; i++ ) {
                timePerTaskPerWorker[ i ] = timePerTaskPerWorker[ i ] / tasksPerWorker[ i ];
            }
            if( ( int )threshold < mpithreshold.start ) {
                bestPartitionFactor = 100;
            }
            else {
                bestPartitionFactor = ( int )(
                    pfsim( context->getMPINumProcs() - 1,                                            // workers
                           average_message_size / mpitksz.start * ( context->getMPINumProcs() - 1 ), // total number of tasks
                           aggregated_answer_message_size / ( context->getMPINumProcs() - 1 ),       //Average size of the workers' answer
                           mpitksz.start,                                                            // task size sent per worker
                           mpilatency.startfloat,                                                    // latency
                           mpilamda.startfloat,                                                      // lambda
                           timePerTaskPerWorker                                                      // average exec. time per worker
                           ) * 100 );
            }
            tp = new TuningParameter();
            tp->setId( 0 );
            tp->setName( "PartitionFactor" );
            tp->setPluginType( MPI );
            tp->setRuntimeActionType( TUNING_ACTION_VARIABLE_INTEGER );
            tp->setRange( bestPartitionFactor, bestPartitionFactor, 1 );
        }

        variantSpace.clear();
        variantSpace.addTuningParameter( tp );
        if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ) ) > 0 ) {
            string str = tp->toString();
            cout << str << "=====================" << endl;
        }
    }

    if( tuningStep == 2 ) {
        tp2 = new TuningParameter();
        tp2->setId( 0 );
        tp2->setName( "PartitionFactor" );
        tp2->setPluginType( MPI );
        tp2->setRuntimeActionType( TUNING_ACTION_VARIABLE_INTEGER );

        if( bestPartitionFactor == 100 ) {
            tp2->setRange( bestPartitionFactor, bestPartitionFactor, 1 );
        }
        else {
            double ten_percent = ( double )bestPartitionFactor / 10.0f;
            if( bestPartitionFactor <= 90 ) {
                tp2->setRange( bestPartitionFactor - ( int )ten_percent, bestPartitionFactor + ( int )ten_percent, ( int )ten_percent );
            }
            else if( bestPartitionFactor > 90 ) {
                tp2->setRange( bestPartitionFactor - ( 100 - bestPartitionFactor ), 100, ( 100 - bestPartitionFactor ) );
            }
        }

        int                          propertyCount       = 0;
        int                          total_mpitime_props = 0;
        double                       minExecTime         = -1.0;
        double                       maxExecTime         = -1.0;
        list<MetaProperty>::iterator property;

        properties = pool_set->arp->getPreAnalysisProperties( 0 );
        cout << "2nd Tuning step. Properties  from the pre-analysis" << endl;

        double total_bytes_transferred = 0.0;
        double total_execution_time    = 0.0;
        int    total_exectime_props    = 0;
        for( property = properties.begin(); property != properties.end(); property++, propertyCount++ ) {
            if( atoi( property->getId().c_str() ) == MPITIME ) {
                total_mpitime_props++;
                TotalCount               = atoi( property->getExtraInfo().at( "TotalCount" ).c_str() );
                total_bytes_transferred += atof( property->getExtraInfo().at( "MeanMsgSize" ).c_str() ) * TotalCount;
            }
            else if( atoi( property->getId().c_str() ) == EXECTIME ) {
                total_exectime_props++;
                total_execution_time += atof( property->getExtraInfo().at( "ExecTime" ).c_str() );
            }
        }

        int predicted_optimal_number_of_workers = ( int )CalculatePi( total_bytes_transferred, total_execution_time, mpilamda.startfloat, mpilatency.startfloat );

        tp = new TuningParameter();
        tp->setId( 1 );
        tp->setName( "NumberOfWorkers" );
        tp->setPluginType( MPI );
        tp->setRuntimeActionType( TUNING_ACTION_NONE );

        int ten_percent_nw = ( int )predicted_optimal_number_of_workers * 0.10f;
        if( predicted_optimal_number_of_workers <= 10 ) {
            tp->setRange( ( int )predicted_optimal_number_of_workers - 1, ( int )predicted_optimal_number_of_workers + 1, 1 );
        }
        else {
            tp->setRange( ( int )predicted_optimal_number_of_workers - ( int )ten_percent_nw, ( int )predicted_optimal_number_of_workers + ( int )ten_percent_nw, ( int )ten_percent_nw );
        }
        numberOfWorkersTP = tp;

        variantSpace.clear();
        variantSpace.addTuningParameter( tp2 );
        variantSpace.addTuningParameter( tp );

        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), ": Create a SearchSpace from the tuning parameters.\n" );
    }

    // create a clean exhaustive search
    context->unloadSearchAlgorithms();
    int    major, minor;
    string name, description;
    context->loadSearchAlgorithm( "exhaustive", &major, &minor, &name, &description );
    searchAlgorithm = context->getSearchAlgorithmInstance( "exhaustive" );
    searchAlgorithm->initialize( context, pool_set );

    searchSpace.setVariantSpace( &variantSpace );
    if( context->applUninstrumented() ) {
        searchSpace.addRegion( new Region() );
    }
    else {
        searchSpace.addRegion( appl->get_phase_region() );
    }
    searchAlgorithm->addSearchSpace( &searchSpace );

    pool_set->csp->clear();
    pool_set->psp->clear();
    pool_set->esp->clear();
    pool_set->fsp->clear();
    pool_set->srp->clear();
    Scenario::resetScenarioIds();
    searchAlgorithm->createScenarios();
}

/**
 * @brief Preparatory steps for the scenarios are done here.
 *
 * The created scenarios are simply moved to the PSP.
 * The number of workers should be at least 1.
 *
 * @ingroup MasterWorkerPlugin
 */
void MasterWorkerPlugin::prepareScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: call to prepareScenarios()\n" );

    if( !pool_set->csp->empty() ) {
        Scenario* scenario;
        scenario = pool_set->csp->pop();
        const list<TuningSpecification*>* ts     = scenario->getTuningSpecifications();
        map<TuningParameter*, int>        values = ts->front()->getVariant()->getValue();

        if( tuningStep == 2 || context->applUninstrumented() ) {
            numberOfWorkers = values.at( numberOfWorkersTP );
            if( numberOfWorkers < 1 ) {
                psc_errmsg( "Invalid number of workers: %d; setting it to 1...\n", numberOfWorkers );
                numberOfWorkers = 1;
            }
        }

        pool_set->psp->push( scenario );
    }
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: call to prepareScenarios() done.\n" );
}

/**
 * @brief Populate the Experiment Scenario Pool (esp) for the next experiment.
 *
 * A scenario from the prepare scenario pool (psp) is pushed into the experiments pool.
 * No analysis strategy is applied.
 *
 * @ingroup MasterWorkerPlugin
 * @param numprocs Not used.
 * @param analysisRequired Set to false.
 * @param strategy No analysis strategy requested.

 */
void MasterWorkerPlugin::defineExperiment( int               numprocs,
                                           bool&             analysisRequired,
                                           StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: call to defineExperiment()\n" );
    Scenario* scenario;
    if( !pool_set->psp->empty() ) {
        scenario = pool_set->psp->pop();

        const list<TuningSpecification*>* ts = scenario->getTuningSpecifications();

        if( ts->size() != 1 ) {
            perror( "MasterWorkerPlugin can't currently handle multiple tuning specs\n" );
            throw 0;
        }

        if( context->applUninstrumented() ) {
            scenario->setSingleTunedRegionWithPropertyRank( NULL, EXECTIME, 0 );
        }
        else {
            scenario->setSingleTunedRegionWithPropertyRank( appl->get_phase_region(), EXECTIME, 0 );
        }
        pool_set->esp->push( scenario );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                    "MasterWorkerPlugin: Added 1 scenario in the experiment.\n" );
        0;
    }
}

/**
 * @brief Return true if a restart of the application is required for the next experiment,
 * false otherwise.
 *
 * Restart is required for the number of workers. Either when the application is uninstrumented or when we
 * are in the second tuning step of the instrumented application.
 *
 * @ingroup MasterWorkerPlugin
 * @param env Not changed
 * @param numprocs used for setting the number of workers that will be used in each experiment during
 * the 2nd tuning step.
 * @param command Not changed
 * @param is_instrumented
 *
 */
bool MasterWorkerPlugin::restartRequired( std::string& env,
                                          int&         numprocs,
                                          std::string& command,
                                          bool&        is_instrumented ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: call to restartRequired()\n" );

    if( !pool_set->esp->empty() ) {
        if( tuningStep == 2 || context->applUninstrumented() ) {
            numprocs = numberOfWorkers + 1;
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Requires %d MPI processes\n", numprocs );

            is_instrumented = !context->applUninstrumented();
            return true;             // restart required to change the number of workers
        }
        else {
            return false;
        }
    }
}

/**
 * @brief Returns whether the search algorithm is finished.
 *
 * @ingroup MasterWorkerPlugin
 */
bool MasterWorkerPlugin::searchFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: call to searchFinished()\n" );

    return searchAlgorithm->searchFinished();
}

/**
 * @brief Final operation of a tuning step.
 *
 * Tuning steps are called to finish for the instrumented version of the application.
 * In this case we call finishTuningStep after tuning step 1, to obtain the optimum partition factor.
 *
 * @ingroup MasterWorkerPlugin
 */
void MasterWorkerPlugin::finishTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: call to processResults()\n" );

    if( tuningStep == 1 && !context->applUninstrumented() ) {
        const list<TuningSpecification*>* ts     = pool_set->fsp->getTuningSpecificationByScenarioID( searchAlgorithm->getOptimum() );
        map<TuningParameter*, int>        values = ts->front()->getVariant()->getValue();
    }
}

/**
 * @brief Returns true if the plugin finished the tuning process, false otherwise.
 *
 * Tuning is finished in the first tuning step for unistrumented and in the second tuning step
 * for instrumented.
 *
 * @ingroup  MasterWorkerPlugin
 *
 */
bool MasterWorkerPlugin::tuningFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: call to tuningFinished()\n" );

    if( tuningStep == 1 && !context->applUninstrumented() ) {
        return false;
    }
    else {
        return true;
    }
}

/**
 * @brief Prints to the screen (and to a file, where necessary) the tuning advice.
 *
 * In the case of uninstrumented getAdivce will provide the user with the best number of workers.
 * In the case of the instrumented application both best partition factor and number of workers are provided.
 *
 * @ingroup MasterWorkerPlugin
 */
Advice* MasterWorkerPlugin::getAdvice() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: call to getAdvice()\n" );

    std::ostringstream result_oss;

    int optimum, scenario_id;

    const list<TuningSpecification*>* ts =
        pool_set->fsp->getTuningSpecificationByScenarioID( searchAlgorithm->getOptimum() );
    map<TuningParameter*, int> values = ts->front()->getVariant()->getValue();
    typedef map<TuningParameter*, int>::iterator tp_iter;

    optimum = searchAlgorithm->getOptimum();
    result_oss << "Found best Scenario: " << optimum << endl;

    int par_count = 0;
    for( tp_iter j = values.begin(); j != values.end(); j++, par_count++ ) {
        result_oss << "TuningParameter: " << par_count << " Name: " << j->first->getName().c_str() << " Value: " << j->second << endl;
    }

    result_oss << "\nAll Results (" << pool_set->fsp->size() << "): \n";
    result_oss << "Scenario\t|  Severity\t\t Tuning Parameters" << endl;

    for( scenario_id = 0; scenario_id < pool_set->fsp->size(); scenario_id++ ) {
        list < MetaProperty > props = pool_set->srp->getScenarioResultsByID( scenario_id );

        MetaProperty prop     = props.front();
        double       severity = prop.getSeverity();

        ts     = pool_set->fsp->getTuningSpecificationByScenarioID( scenario_id );
        values = ts->front()->getVariant()->getValue();
        result_oss << scenario_id << "\t\t|  " << severity;
        for( tp_iter j = values.begin(); j != values.end(); j++, par_count++ ) {
            result_oss << " Name: " << j->first->getName().c_str() << " Value: " << j->second << "\t";
        }
        result_oss << endl;
    }
    result_oss << "\n------------------------" << endl << endl;

    cout << result_oss.str();
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: call to getAdvice() done\n" );

    return new Advice( getName(), ( *pool_set->fsp->getScenarios() )[ searchAlgorithm->getOptimum() ],
                       searchAlgorithm->getSearchPath(), "Time", pool_set->fsp->getScenarios() );
}

/**
 * @brief Finalizes the plugin.
 * @ingroup MasterWorkerPlugin
 *
 *
 */
void MasterWorkerPlugin::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: call to finalize()\n" );

    terminate();
}

/**
 * @brief Terminate the plugin due to error.
 *
 * Safely remove any allocated memory, objects, file descriptors, etc. This method should
 * be able to be executed safely at any point.
 *
 * @ingroup MasterWorkerPlugin
 */
void MasterWorkerPlugin::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: call to terminate()\n" );

    if( searchAlgorithm ) {
        searchAlgorithm->finalize();
        delete searchAlgorithm;
    }
    context->unloadSearchAlgorithms();
}

/**
 * The following are C functions that belong to the Plugin Management Interface, and not to the
 * IPlugin class. Currently the interface includes information and instance creation functions.
 *
 * These are defined in PluginManagement.h .
 */

/**
 * @brief Returns Plugin's instance.

 * Returns an instance of this particular plugin implementation.
 * Typically, a simple return with new is enough. For example:
 *
 * @ingroup MasterWorkerPlugin
 * @return new MasterWorkerPlugin();
 */
IPlugin* getPluginInstance( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: call to getPluginInstance()\n" );

    return new MasterWorkerPlugin();
}

/**
 * @brief Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 * @ingroup MasterWorkerPlugin
 * @return 1
 */
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: call to getInterfaceVersionMajor()\n" );

    return MASTERWORKER_VERSION_MAJOR;
}

/**
 * @brief Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 * @ingroup MasterWorkerPlugin
 * @return 0
 */
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: call to getInterfaceVersionMinor()\n" );

    return MASTERWORKER_VERSION_MINOR;
}

/**
 * @brief Returns a string with the name of the plugin.
 * @ingroup MasterWorkerPlugin
 *
 * @return A string with the name of the plugin.
 */
string getName( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: call to getName()\n" );

    return "Master Worker plugin";
}

/**
 * @brief Returns a string with a short description of the plugin.
 * @ingroup MasterWorkerPlugin
 *
 * @return A string with a short description of the plugin.
 *
 */
string getShortSummary( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "MasterWorkerPlugin: call to getShortSummary()\n" );

    return "Tunes the partition strategy and load balancing of MPI Master-Worker applications.";
}
