/**
   @file    DataProvider.cc
   @ingroup AnalysisAgent
   @brief   Internal data management
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2013, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include "DataProvider.h"
#include "psc_errmsg.h"
#include "analysisagent.h"
#include "Metric.h"
#include "TuningSpecification.h"
#include "stringutil.h"
#include "global.h"
#include <sys/time.h>
#include <boost/foreach.hpp>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <inttypes.h>
#include <map>

#ifdef __p575
#include "PerformanceDataBase.h"
#include <strings.h>
#include "Metric.h"
#endif


/* These variables are used internally by DataProvider to store the port and sockets for communicating with
 * application processes.*/
int agent_port = 51000;
int agent_sock = -1;

double myWallClock() {
    struct timeval tp;
    double         sec, usec, start;

    // Time stamp before the computations
    gettimeofday( &tp, NULL );
    sec   = static_cast<double> ( tp.tv_sec );
    usec  = static_cast<double> ( tp.tv_usec ) / 1E6;
    start = sec + usec;
    return start;
}

DataProvider::DataProvider( Application*     appl,
                            std::string      phase_name_,
                            RegistryService* registry ) :
        num_iterations_per_experiment(1),
        old_requests_pending(false),
        appl_(appl),
        phase_name(phase_name_),
        registry_(registry) {

    initialize_metric2papiset();
    burst_counter     = 1;
    current_iteration = 0;
    burst_counter=0;
    status            = NOT_ALL_INFO_GATHERED;
}


int DataProvider::getCurrentIterationNumber() {
    return current_iteration;
}


const std::list<Scenario*>* ApplProcess::getScenariosPerTuningSpecification() {
    return &scenariosPerTuningSpecification;
}

const std::list<Scenario*>* ApplProcess::getScenariosPerPropertyRequest() {
    return &scenariosPerPropertyRequest;
}


Request::Request( Context* ct_p,
                  Metric   m_p ) :
    ct( ct_p ),
    m( m_p ) {
}


void DataProvider::write_line( ApplProcess* process,
                               const char*  str ) {
    if( !process ) {
        return;
    }
    psc_dbgmsg( 9, "Sending line to process %d: <%s>\n", process->rank, str );

    socket_write_line( process->sock, str );
}


int DataProvider::read_line( ApplProcess* process,
                             char*        str,
                             int          maxlen ) {
    if( !process ) {
        return -1;
    }

    return socket_read_line( process->sock, str, maxlen );
}


int DataProvider::read_block( ApplProcess* process,
                              char*        ptr,
                              int          size ) {
    if( !process ) {
        return -1;
    }

    return socket_blockread( process->sock, ptr, size );
}


void DataProvider::addMeasurementRequest( Context* ct,
                                          Metric   m ) {
    if( old_requests_pending ) {
        psc_errmsg( "Data provider: There are still old requests pending - no new requests can be added right now!\n" );
        psc_errmsg( "Data provider: Hint: Not all metrics could be measured during the last experiment.\n" );
        psc_errmsg( "Data provider: Hint: Re-run the current experiment with no new measurement requests.\n" );
        abort();
    }

    if( m == PSC_INSTANCES ) {
        psc_errmsg( "Warning: PSC_INSTANCES cannot be requested explicitly; it is always provided by Score-P.\n" );
        return;
    }

    measurement_requests.push_back( m );
}


void DataProvider::addTuningRequest( int                  rank,
                                     TuningSpecification* ts ) {
    tuning_requests[ rank ].push_back( ts );
}


void DataProvider::cleanAllRequests() {
    measurement_requests.clear();
    tuning_requests.clear();
}


bool DataProvider::areOldRequestsPending() const {
    return old_requests_pending;
}


Gather_Required_Info_Type DataProvider::getResults() {
    std::list<ApplProcess>::iterator process;
    int                              number, length;
    char                             str[ 2000 ];
    Gather_Required_Info_Type        gather_info = ALL_INFO_GATHERED;

    if( status == ALL_INFO_GATHERED ) {
        return ALL_INFO_GATHERED;
    }

    /* Loop over processes controlled by this instance of Data Provider */
    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        psc_dbgmsg( 4, "Requesting summary data from process %d\n", process->rank );

        /* Send GETSUMMARYDATA request to the process */

        write_line( &( *process ), "getsummarydata;\n" );

        /* Receiving Merged Region Definitions */

        /* region definition buffer */
        SCOREP_OA_CallPathRegionDef* definitions;
        int                          definition_buffer_size = 0;

        /* Receive region definitions*/
        if( !blockingBufferReceive( ( void** )( &definitions ), &definition_buffer_size,
                                    &( *process ), MERGED_REGION_DEFINITIONS ) ) {
            psc_abort( "Error: Unable to receive region definitions (I/O error)!\n" );
        }


        /* Store and index region definitions */
        definition_mapping = storeAndIndexRegionDefinitions( definitions, definition_buffer_size,
                                                             process->rank );


        /* Free definitions buffer, it is not needed any more */
        free( definitions );

        /* Receiving Flat Profile */

        /* flat profile measurement buffer */
        SCOREP_OA_FlatProfileMeasurement* profile;
        int                               profile_buffer_size = 0;

        /* Receive region definitions */
        if( !blockingBufferReceive( ( void** )( &profile ), &profile_buffer_size,
                                    &( *process ), FLAT_PROFILE ) ) {
            psc_abort( "Error: Unable to receive flat profile (I/O error)!\n" );
        }

        /* Receiving Metric Definitions */

        /*  metric definition buffer */
        SCOREP_OA_CallPathCounterDef* metrics;
        int                           metrics_buffer_size = 0;

        /* Receive region definitions*/
        if( !blockingBufferReceive( ( void** )( &metrics ), &metrics_buffer_size,
                                    &( *process ), COUNTER_DEFINITIONS ) ) {
            psc_abort( "Error: Unable to receive metrics (I/O error)!\n" );
        }

        storeFlatProfiles( profile, profile_buffer_size, metrics );

        /* Release flat profile measurements buffer since it is not needed */
        free( profile );

        /* Receiving call-tree definitions */

        /* call-tree definitions buffer */
        SCOREP_OA_CallTreeDef* callpaths;
        int                    callpath_buffer_size = 0;

        /* Receive call-tree definitions*/
        if( !blockingBufferReceive( ( void** )( &callpaths ), &callpath_buffer_size,
                                    &( *process ), CALLTREE_DEFINITIONS ) ) {
            psc_abort( "Error: Unable to receive call-tree (I/O error)!\n" );
        }

//        if( !( agent->get_strategy()->name().compare("ImportanceStrategy") ) ) {

            SCOREP_OA_CallTreeDef* buffer_casted = ( SCOREP_OA_CallTreeDef* )callpaths;

            if( withRtsSupport() ) {
                Rts::construct_aagent_calltree( buffer_casted,
                                                callpath_buffer_size,
                                                &scorep_region_id_mappings );
            }
//        }
        /* Free call-tree definitions buffer*/
        free( callpaths );

        /* Receiving rts measurements */

        /* rts measurements buffer */
        SCOREP_OA_RtsMeasurement* rts_meas;
        int                       rts_meas_buffer_size = 0;

        /* Receive rts measurements */
        if( !blockingBufferReceive( ( void** )( &rts_meas ), &rts_meas_buffer_size,
                                    &( *process ), RTS_MEASUREMENTS ) ) {
            psc_abort( "Error: Unable to receive rts measurements (I/O error)!\n" );
        }

        if( withRtsSupport() ) {
            storeRtsProfiles( rts_meas, rts_meas_buffer_size, metrics );
            rtstree::clear_scorepid_to_rts_mapping();
        }

        /* Free rts measurement buffer*/
        free( rts_meas );
        /* Release metrics definition buffer since it is not needed */
        free( metrics );
    }     //loop over processes

    //decrement burst counter if it is greater then zero
    if( burst_counter > 0 ) {
        burst_counter--;
    }

    // if there are pending global requests...
    if( !measurement_requests.empty() ) {
        //... then not all info gathered
        gather_info = NOT_ALL_INFO_GATHERED;
        psc_dbgmsg( 4, "Data provider: Not all metrics have been gathered yet; remaining: %i\n",
                    static_cast<int>( measurement_requests.size() ) );
    }
    else {
        // if all requests were completed, then check whether the burst is over
        if( burst_counter > 0 ) {
            //if not then load the backed-up request to the request list and start over again
           psc_dbgmsg( 4, "Data provider: burst_counter %d\n",burst_counter );
            gather_info          = NOT_ALL_INFO_GATHERED;
            measurement_requests = measurement_requests_backup;
        }
    }
    // ... otherwise all info gathered!

    if( gather_info == ALL_INFO_GATHERED ) {
        // clear subscribers lists after all requests were completed
        notify_for_region_types();
        notify_for_global_metrics();
        metric_group_subscribers_map.clear();
        metric_subscribers_map.clear();
        region_type_subscribers_map.clear();
        ++current_iteration;
        old_requests_pending = false;
    }
    else if( burst_counter > 0 ) {
        ++current_iteration;
    }
    else {
        old_requests_pending = true;
    }

    status = gather_info;
    return status;
}


void DataProvider::wait_for_ok( ApplProcess process ) {

    char str[ 2000 ];
    int  length;

    bzero( str, 2000 );
    while( 1 ) {
        while( ( length = read_line( &process, str, 2000 ) ) == 0 ) {
        }
        ;
        psc_dbgmsg( 8, "Received from application process %d: <%s>\n",
                    process.rank, str );
        if( strcmp( str, "OK" ) != 0 ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ),
                        " %d: ::%s:: instead of OK\n",
                        process.registry_entry_id, str );
        }
        else {
            break;
        }
    }
}

std::string metric2reqSpec( Metric m ) {
    std::stringstream codeStream;

    switch( m ) {
    //Any Overhead, exclusive or not, is translated to PSC_MRI_OVERHEAD
    //PSC_MRI_OVERHEAD measures in the monitor both overhead metrics
    //and returns both
    case PSC_MRI_OVERHEAD:
        codeStream << ( int )PSC_MRI_OVERHEAD;
        break;
    case PSC_MRI_LIBCALLS:
        codeStream << ( int )PSC_MRI_LIBCALLS;
        break;
    case PSC_MRI_EXCL_OVERHEAD:
        codeStream << ( int )PSC_MRI_OVERHEAD;
        break;

    case PSC_IMPLICIT_BARRIER_TIME:
    case PSC_CRITICAL_REGION_CYCLE:
    case PSC_CRITICAL_BODY_CYCLE:
    case PSC_SINGLE_REGION_CYCLE:
    case PSC_SINGLE_BODY_CYCLE:
    case PSC_MASTER_BODY_CYCLE:
    case PSC_PARALLEL_REGION_CYCLE:
    case PSC_PARALLEL_REGION_BODY_CYCLE:
    case PSC_OMP_BARRIER_CYCLE:
    case PSC_ORDERED_REGION_CYCLE:
    case PSC_OMP_ATOMIC_CYCLE:
    case PSC_OMP_SECTIONS_REGION_CYCLE:
    case PSC_OMP_SECTION_BODY_CYCLE:
    case PSC_OMP_DO_REGION_CYCLE:
    case PSC_TASK_REGION_CYCLE:
    case PSC_TASK_REGION_BODY_CYCLE:
    case PSC_TASKS_CREATED:
    case PSC_TASKS_EXECUTED:
        codeStream << ( int )PSC_EXECUTION_TIME;
        break;
    default:
        codeStream << ( int )m;
        break;
    }
    return codeStream.str();
}


void DataProvider::transfer_requests_to_processes() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "DataProviderScoreP::transfer_requests_to_processes\n" );
    transfer_num_iterations_to_processes();
    std::list<ApplProcess>::iterator process;
    /* Send BEGINREQUEST message to all processes */
    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        write_line( &( *process ), "beginrequests;\n" );
        wait_for_ok( *process );
    }

    std::string substrate;
 	try {
		substrate = configTree.get < std::string > ("Configuration.scorep.tuningSubstrate").c_str();

 	} catch (exception &e) {
 		substrate = "rrl";
 	}

    if( withRtsSupport() && substrate=="rrl")
        transfer_rts_tuning_requests_to_processes();
    else {
        transfer_tuning_requests_to_processes();
    }

    transfer_measurement_requests_to_processes();

    /* Send ENDREQUEST message to all processes */
    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "Sending \"endrequests\" to process %i.\n", process->rank );
        write_line( &( *process ), "endrequests;\n" );
    }
    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "Waiting for OK...\n." );
        wait_for_ok( *process );
        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "OK received.\n." );
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "Sent end-requests message to all processes.\n" );
    status = NOT_ALL_INFO_GATHERED;
}


void DataProvider::transfer_num_iterations_to_processes() {
    std::stringstream requestTemp;
    requestTemp  <<  "setnumiterations "  <<  num_iterations_per_experiment  <<  ";\n";
    std::string request = requestTemp.str();
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), request.c_str() );

    BOOST_FOREACH( ApplProcess& process, controlled_processes )
        write_line(&process, request.c_str());
}


void DataProvider::transfer_measurement_requests_to_processes() {
    std::list<ApplProcess>::iterator process;
    std::list<Metric>::iterator      request_iter;
    std::string newMetric;

    /* Remove duplicated requests */
    measurement_requests.sort();
    measurement_requests.unique();

    /* Select requests for the submission during this online phase iteration */
    std::list<Metric> selected_requests = formRequestSet();

    // loop over requests
    for( request_iter = selected_requests.begin();
         request_iter != selected_requests.end();
         ++request_iter ) {
        // send request to each process
        for( process = controlled_processes.begin();
             process != controlled_processes.end();
             ++process ) {
            Metric metric_req_id = *request_iter;
            if( EventList[ metric_req_id ].EventMetric != metric_req_id ) {
                psc_errmsg( "Metric id doesn't match with the index of the metric in EventList array!!!\n" );
                continue;
            }

            /* Build the request string to send to Score-P OA interface */
            char* metric_name = (char*) EventList[ metric_req_id ].EventName;
            std::stringstream req_str;
            req_str <<  "REQUEST[0] GLOBAL ";

            /* Translate metric group to the Score-P metric group, and check whether the group is supported */
            switch( EventList[ metric_req_id ].EventGroup ) {
            case GROUP_PAPI_COUNTER:
            case GROUP_PAPI_POWER6_COUNTER:
            case GROUP_PAPI_NEHALEM_COUNTER:
                req_str <<  "METRIC PAPI \""  <<  metric_name  <<  "\"";
                break;
            case GROUP_ENOPT_ENERGY_COUNTER:
                req_str <<  "METRIC PLUGIN \"EnoptMP\" \""  <<  metric_name  <<  "\"";
                break;
            case GROUP_HDEEM:
                req_str <<  "METRIC PLUGIN \"hdeem_sync_plugin\" \""  <<  metric_name  <<  "\"";
                break;
            case GROUP_TIME_MEASUREMENT:
            case GROUP_MPI:
               req_str <<  metric_name;
               break;
            case GROUP_ENERGY: {
                //apply mapping for request
                //redefine metric_name>
                //metric_name="hdeem/CPU1/E";
                char* metricPlugin;
                std::string metricPlugin_s;
                if (opts.has_configurationfile) {
                    try {
                        metricPlugin_s = configTree.get < std::string > ("Configuration.periscope.metricPlugin.name");
                    } catch (exception &e) {
                        metricPlugin_s = "hdeem_sync_plugin";

                    }
                    switch (metric_req_id) {
                    case PSC_NODE_ENERGY: {
                        try {
                            newMetric = configTree.get < std::string > ("Configuration.periscope.metrics.node_energy");
                        } catch (exception &e) {
                            if( strcmp (metricPlugin_s.c_str(), "hdeem_sync_plugin") == 0)
                                newMetric = "hdeem/BLADE/E";
                            else
                                newMetric = "x86_energy/BLADE/E";

                        }
                        break;
                    }
                    case PSC_CPU0_ENERGY: {
                        try {
                            newMetric = configTree.get < std::string > ("Configuration.periscope.metrics.cpu0_energy");
                        } catch (exception &e) {
                            if( strcmp (metricPlugin_s.c_str(), "hdeem_sync_plugin") == 0)
                                newMetric = "hdeem/CPU0/E";
                        }
                        break;
                    }
                    case PSC_CPU1_ENERGY: {
                        try {
                            newMetric = configTree.get < std::string > ("Configuration.periscope.metrics.cpu1_energy");
                        } catch (exception &e) {
                            if( strcmp (metricPlugin_s.c_str(), "hdeem_sync_plugin") == 0)
                                newMetric = "hdeem/CPU1/E";
                        }
                        break;
                    }
                    default:
                        break;
                    }
                }
                metricPlugin = (char*) metricPlugin_s.c_str();
                metric_name = (char*) newMetric.c_str();
                req_str <<  "METRIC PLUGIN \"" << metricPlugin << "\"  \"" << metric_name  <<  "\"";
                break;
                }
            default:
                psc_abort( "Error: %s Metric '%s' is not supported by Score-P and should never have been queued!\n",
                           __func__, metric_name );
            }

            /* Store metric name and its id in the map of submitted requests for later name 2 id translation */
            submitted_request_map[ metric_name ] = metric_req_id;

            /* Compose and send request string */
            req_str <<  ";\n";
            std::string request_string = req_str.str();
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "Sending to process %i: %s", process->rank, request_string.c_str() );
            fflush( stdout );
            write_line( &( *process ), request_string.c_str() );

            /* Wait for confirmation */
            wait_for_ok( *process );
        }

        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "Sent to all processes.\n" );
    }
    /*std::cout << "Printing all the submitted requests" << endl;
    for(std::map<std::string, Metric>::iterator ii=submitted_request_map.begin(); ii!=submitted_request_map.end(); ++ii){
       std::cout << (*ii).first << endl;
    }*/
}

std::list<ApplProcess> DataProvider::get_controlled_processes() {
    return controlled_processes;
}

void DataProvider::transfer_rts_tuning_requests_to_processes() {
	std::list<ApplProcess>::iterator process;

	for (process = controlled_processes.begin(); process != controlled_processes.end(); process++) {
		std::list<TuningSpecification*> ts_list = tuning_requests[process->rank];
		std::list<TuningSpecification*>::iterator ts_it;

		//map of rts request string and list tuning specification
		for (ts_it = ts_list.begin(); ts_it != ts_list.end(); ts_it++) {
			if ((*ts_it)->getTypeOfVariantContext() != variant_context_type(RTS_LIST)) {
				continue;
			}

			map<TuningParameter *, int> values = (*ts_it)->getVariant()->getValue();
			VariantContext context = (*ts_it)->getVariantContext();
			std::map<TuningParameter *, int>::iterator tv_it;
			std::vector < std::string > rts_streams;
			std::list < std::pair<std::string, int> > tp_name_value;


			for (tv_it = values.begin(); tv_it != values.end(); tv_it++) {

				std::string tuningParameterName = tv_it->first->getName();
				int tuningParameterValue = tv_it->second;
				int tuningActionType = tv_it->first->getRuntimeActionType();

				tp_name_value.push_back(std::make_pair(tuningParameterName, tuningParameterValue));
			}

			list < string > *entities = context.context_union.entity_list;
			std::list<string>::iterator ent_it;
			for (ent_it = entities->begin(); ent_it != entities->end(); ent_it++) {
				int scorep_id;
				Rts* root_rts_node = Application::instance().getCalltreeRoot();
				Rts* rts_node = root_rts_node->getRtsByCallpath(*ent_it);

				if (rts_node == NULL) {
					psc_errmsg("RTS node not found for callpath %s\n", (*ent_it).c_str());
					continue;
				}

				Region* rootRegion = root_rts_node->getRegion();
				uint64_t aa_r_region_id = rootRegion->getLocalRegionID();
				int scorep_r_region_id = get_scorepid_by_aa_regionId_for_rank(aa_r_region_id, process->rank);

				if (scorep_r_region_id == -1) {
					psc_errmsg("ScoreP region ID of region %s was not identified!\n", rootRegion->getRegionID().c_str());
					break;
				}

				std::stringstream str_stream;
				std::list<Rts*> setOfRtss;
				setOfRtss.push_front(rts_node);

				/*Go from the rtsnode till the root and store it to a stack*/
				traverse_reverse(scorep_r_region_id, rts_node, &setOfRtss, process->rank);

				/* Pop from the stack and build the callpath */
				scorep_id = traverse_forward(rts_node, &setOfRtss, process->rank, str_stream);

				if (scorep_id == -1) {
					psc_errmsg("ScoreP region ID of the region was not identified!\n");
					break;
				}

				std::string rts_string = str_stream.str();
				std::stringstream req_stream;

				int cnt = 0;
				std::list<std::pair<std::string, int> >::const_iterator it_tp_name_val;

				req_stream << rts_string;
				req_stream << "=" << "(";
				for (it_tp_name_val = tp_name_value.begin(); it_tp_name_val != tp_name_value.end(); ++it_tp_name_val) {
					if (cnt > 0)
						req_stream << ",";
					req_stream << "\"" << it_tp_name_val->first << "\"" << "=" << it_tp_name_val->second;
					cnt++;
				}
				req_stream << ");\n"; //closing of tuning parameter// end of tuning specifications
				//psc_dbgmsg(6, "RTS based Tuning Request Format: %s\n", req_stream.str().c_str());
				psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AgentApplComm), "%s", req_stream.str().c_str());

				dp->write_line(&(*process), req_stream.str().c_str());
				dp->wait_for_ok(*process);
			} /*RTS based changes END */

		}
	}
	tuning_requests.clear();
}


void DataProvider::traverse_reverse( int root_scorep_id, Rts* rtsNode, std::list<Rts*>* rts_list, int rank )
{
    Region* region          = rtsNode->getRegion();
    uint64_t aa_region_id   = region->getLocalRegionID();
    int scorep_id           = get_scorepid_by_aa_regionId_for_rank( aa_region_id, rank );

    if( root_scorep_id == scorep_id )
        return;

    Rts* parentRts =  const_cast<Rts*>(rtsNode->getParent());
    rts_list->push_front( parentRts );

    traverse_reverse( root_scorep_id, parentRts, rts_list, rank );

}


int DataProvider::traverse_forward( Rts* rtsNode, std::list<Rts*>* rts_list, int rank, std::stringstream& req_stream )
{
    bool separator(false);
    std::stringstream streamPrint;
    req_stream << "RTSTUNINGREQUESTS(";
    // streamPrint
 	int main_id;
 	try {
 		main_id = atoi(configTree.get < std::string > ("Configuration.scorep.main_id").c_str());
 	    req_stream << "(" << main_id << ",INTPARAMS=(),UINTPARAMS=(),STRINGPARAMS=()),";
 	} catch (exception &e) {
 	}

    while( !rts_list->empty() )
    {
        Rts* rts_it               = rts_list->front();
        Region* region            = rts_it->getRegion();
        uint64_t aa_region_id     = region->getLocalRegionID();
        int scorep_region_id = get_scorepid_by_aa_regionId_for_rank( aa_region_id, rank );

        if( scorep_region_id == -1 )
            return -1;

        if( !rts_it->ValidRts() )
        {
            rts_list->pop_front();
            continue;
        }

        if( separator )
        {
            req_stream << ","; // comma separated rtss
        }
        req_stream << "(" << scorep_region_id << ","; // start of scorep id for a rts
        separator = create_param_req_stream( rts_it, req_stream );
        req_stream << ")" ; // end of scorep id for a rts

        rts_list->pop_front();
    }
    req_stream << ")"; // closing bracket RTSTUNINGREQUESTS(

    return 1;
}

bool DataProvider::create_param_req_stream( Rts* rts, std::stringstream& req_stream )
{

    std::vector<Parameter_t*> params = rts->getParameter();

    std::stringstream intparamStream;
    std::stringstream uintparamStream;
    std::stringstream stringparamStream;

    intparamStream << "INTPARAMS=(";
    uintparamStream << "UINTPARAMS=(";
    stringparamStream << "STRINGPARAMS=(";

    if( params.size()> 0 )
    {
        int cnt_int_comma = 0;
        int cnt_uint_comma = 0;
        int cnt_string_comma = 0;
        std::vector<Parameter_t*>::iterator param_it;
        for( param_it = params.begin(); param_it != params.end(); ++param_it )
        {
            std::string name  = (*param_it)->param_name;
            std::string value = (*param_it)->param_value;
            if( (*param_it)->param_type == NODE_PARAMETER_INTEGER )
            {
                if( cnt_int_comma > 0 )
                    intparamStream << ",";
                intparamStream << "\"" << name <<"\"" << "=" << value; //TO DO
                ++cnt_int_comma;
            }
            else if( (*param_it)->param_type == NODE_PARAMETER_UNSIGNEDINT )
            {
                if( cnt_uint_comma > 0 )
                    uintparamStream << ",";
                uintparamStream << "\"" << name <<"\"" << "=" << value;
                ++cnt_uint_comma;
            }
            else if( (*param_it)->param_type == NODE_PARAMETER_STRING )
            {
                if( cnt_string_comma > 0 )
                    intparamStream << ",";
                stringparamStream << "\"" << name <<"\"" << "=" << "\"" << value << "\"";
                ++cnt_string_comma;
            }
            else
            {
                psc_abort("Error: Invalid RTS paramter\n");
            }
        }
    }
    intparamStream << ")";
    uintparamStream << ")";
    stringparamStream << ")";
    req_stream << intparamStream.str() << "," << uintparamStream.str() << ","<< stringparamStream.str();
    return true;
}

/* Searching the ScoreP region ID based on the AA region ID*/
int DataProvider::get_scorepid_by_aa_regionId_for_rank( uint64_t aaRegionID, int rank )
{
    std::map<uint32_t, std::list<int> >  scorep_region_ids = scorep_region_id_mappings[ aaRegionID ];
    std::map<uint32_t, std::list<int> >::iterator id_it;
    int  scorep_region_id;
    for( id_it = scorep_region_ids.begin(); id_it != scorep_region_ids.end(); id_it++ ) {
        if( find( id_it->second.begin(), id_it->second.end(), rank ) != id_it->second.end() ) {
            scorep_region_id      = id_it->first;
            return scorep_region_id;
        }
    }
    return -1;
}

void DataProvider::transfer_tuning_requests_to_processes() {
    std::list<ApplProcess>::iterator process;

    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        std::list<TuningSpecification*>           ts_list = tuning_requests[ process->rank ];
        std::list<TuningSpecification*>::iterator ts_it;
        for( ts_it = ts_list.begin(); ts_it != ts_list.end(); ts_it++ ) {
//            if( ( *ts_it )->getTypeOfVariantContext() != variant_context_type( REGION_LIST ) ) {
//                continue;
//            }

            map<TuningParameter*, int>                values  = ( *ts_it )->getVariant()->getValue();
            VariantContext                            context = ( *ts_it )->getVariantContext();
            std::map<TuningParameter*, int>::iterator tv_it;

            for( tv_it = values.begin(); tv_it != values.end(); tv_it++ ) {
                std::string tuningParameterName  = tv_it->first->getName();
                int         tuningActionType     = tv_it->first->getRuntimeActionType();
                int         tuningParameterValue = tv_it->second;

                list<string>*               regions = context.context_union.entity_list;
                std::list<string>::iterator reg_it;
                for( reg_it = regions->begin(); reg_it != regions->end(); reg_it++ ) {
               	 Region*    r;
               	 if ((*ts_it)->getTypeOfVariantContext() == variant_context_type(RTS_LIST)){
               		 r=appl->getCalltreeRoot()->getRtsByCallpath(*reg_it)->getRegion();
               	 } else {
                      r= Application::instance().getRegionByID( *reg_it );
               	 }

                    char       str[ 2000 ];
                    RegionType regionType = r->get_type();

                    /* Searching the ScoreP region ID based on the AA region ID*/
                    int                                           aa_region_id      = r->getLocalRegionID();
                    std::map<uint32_t, std::list<int> >           scorep_region_ids = scorep_region_id_mappings[ aa_region_id ];
                    std::map<uint32_t, std::list<int> >::iterator id_it;
                    int                                           scorep_region_id;
                    bool                                          score_region_id_found = false;
                    for( id_it = scorep_region_ids.begin(); id_it != scorep_region_ids.end(); id_it++ ) {
                        if( find( id_it->second.begin(), id_it->second.end(), process->rank ) != id_it->second.end() ) {
                            scorep_region_id      = id_it->first;
                            score_region_id_found = true;
                            break;
                        }
                    }
                    if( !score_region_id_found ) {
                        psc_errmsg( "ScoreP region ID of region %s was not identified!\n",
                                    r->getRegionID().c_str() );
                        break;
                    }
                    /* Finished searching the ScoreP region ID based on the AA region ID*/

                    if( regionType == DO_REGION && strcmp( tuningParameterName.c_str(), "NUMTHREADS" ) == 0 ) {
                        regionType = PARALLEL_REGION;
                    }
                    if( tuningActionType == TUNING_ACTION_VARIABLE_INTEGER ) {
                        sprintf( str, "tuningaction (%d) = (VARIABLE, \"%s\", %d);\n",
                                 scorep_region_id,
                                 tuningParameterName.c_str(),
                                 tuningParameterValue );
                    }
                    else if( tuningActionType == TUNING_ACTION_FUNCTION_POINTER ) {
                        sprintf( str, "tuningaction (%d) = (FUNCTION, \"%s\", %d);\n",
                                 scorep_region_id,
                                 tuningParameterName.c_str(),
                                 tuningParameterValue );
                    }
                    else if( tuningActionType == TUNING_ACTION_NONE ) {
                        psc_dbgmsg( 6, "Tuning action type is NONE\n" );
                        break;
                    }
                    else {
                        psc_errmsg("Tuning action type %d is unknown\n", tuningActionType );
                        break;
                    }
                    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "%s", str );

                    dp->write_line( &( *process ), str );
                    dp->wait_for_ok( *process );
                }
            }
        }
    }

    tuning_requests.clear();
}

void DataProvider::print_controlled_processes() {
    std::list<ApplProcess>::iterator     process;
    unsigned int                         processNumber = 0;
    std::list<Scenario*>::const_iterator scenario;

    std::cout << "Controlled processes:" << std::endl;
    for( process = controlled_processes.begin();
         process != controlled_processes.end();
         process++, processNumber++ ) {
        std::cout << processNumber << ". Process" << std::endl;
        std::cout << " - Rank: " << process->rank << std::endl;
        std::cout << " - Port: " << process->port << std::endl;
        std::cout << " - Registry entry id: " << process->registry_entry_id << std::endl;
        std::cout << " - Socket number: " << process->sock << std::endl;
        std::cout << " - Status(" << process->status.file_id << ", " << process->status.rfl
                  << "): Position: " << process->status.position << " State: "
                  << process->status.state << std::endl;
        const std::list<Scenario*>* scenarios = process->getScenariosPerTuningSpecification();
        if( !scenarios->empty() ) {
            std::cout << " - List of scenarios controlled by the process (by tuning specification):" << std::endl;
            for( scenario = scenarios->begin();
                 scenario != scenarios->end();
                 scenario++ ) {
                ( *scenario )->print();
            }
        }
        scenarios = process->getScenariosPerPropertyRequest();
        if( !scenarios->empty() ) {
            std::cout << " - List of scenarios controlled by the process (by property requests):" << std::endl;
            for( scenario = scenarios->begin();
                 scenario != scenarios->end();
                 scenario++ ) {
                ( *scenario )->print();
            }
        }
    }
}

int DataProvider::get_total_num_processes() {
    return total_num_processes;
}

ErrorType DataProvider::set_num_iterations_per_phase( int iterations ) {
    if( iterations < 1 )
        psc_abort("Error: At least 1 iteration must be performed per experiment.");

    num_iterations_per_experiment = iterations;
    return OK;
}

ErrorType DataProvider::stop_on_start_region( const Region* reg ) {
    std::list<ApplProcess>::iterator process;
    char                             str[ 2000 ];
    int                              file_id = 0;
    int                              rfl     = 0;

    if( reg ) {
        RegionIdent ident = reg->get_ident();
        file_id = ident.file_id;
        rfl     = ident.rfl;
    }

    sprintf( str, "runtostart (%d,%d);\n", file_id, rfl );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), str );
    for( process = controlled_processes.begin();
         process != controlled_processes.end();
         process++ ) {
        write_line( &( *process ), str );
        process->status.state    = PR_WORKING;
        process->status.file_id  = file_id;
        process->status.rfl      = rfl;
        process->status.position = POS_REGION_START;
    }

    return OK;
}

ErrorType DataProvider::stop_on_end_region( const Region* reg ) {
    std::list<ApplProcess>::iterator process;
    char                             str[ 2000 ];
    int                              file_id = 0;
    int                              rfl     = 0;

    if( reg ) {
        RegionIdent ident = reg->get_ident();
        file_id = ident.file_id;
        rfl     = ident.rfl;
    }
    sprintf( str, "runtoend (%d,%d);\n", file_id, rfl );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), str );
    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        write_line( &( *process ), str );
        process->status.state    = PR_WORKING;
        process->status.file_id  = file_id;
        process->status.rfl      = rfl;
        process->status.position = POS_REGION_END;
    }

    return OK;
}



ErrorType DataProvider::app_finish() {
    std::list<ApplProcess>::iterator process;

    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        if( process->status.state != PR_TERMINATED ) {
            //MG: appfinish now runs to end
            write_line( &( *process ), "terminate;\n" );
            // seems like shutdown does not destroy the socket's file descriptor (we reach the limit in long running jobs); using close now -IC
            if( process->sock > 0 ) {
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                            "Closing file descriptor: %d; previous rank: %d;\n",
                            process->sock, process->rank );
                if( close( process->sock ) ) {            // this socket needs to be closed as well in agent termination
                    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                                "Warning: error while closing socket with FD %d!\n",
                                process->sock );
                }
            }
            else {
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                            "Warning: invalid file descriptor for process: %d (%d)!\n",
                            process->rank, process->sock );
            }
            process->sock            = -1;
            process->status.state    = PR_TERMINATED;
            process->status.position = POS_UNKNOWN;
            process->status.file_id  = -1;
            process->status.rfl      = -1;

            registry_->delete_entry( process->registry_entry_id );
        }
    }

    return OK;
}

bool compare_registry_entries( EntryData e1,
                               EntryData e2 ) {
    if( e1.id < e2.id ) {
        return true;
    }
    else {
        return false;
    }
}

ErrorType DataProvider::init( RegistryService* registry,
                              std::list<int>   process_entries ) {
    std::list<int>::iterator it;
    std::list < EntryData >  rresult;
    EntryData                query;
    double                   start_clock, end_clock, diff_clock;
    double                   time = 0.0, maxTime, minTime;


    query.app = appl->get_app_name().c_str();
    query.tag = "none";

    // TODO check that we have all the information required without having to contact the reg -IC
    if( agent->get_fastmode() ) {
        /**
         * Fast mode is not possible with Score-P, since the agents are supposed to connect to the Score-P processes
         *
         * TODO: forbid fastmode before the connection is inverted in Score-P.
         */

        psc_errmsg( "Fast mode connection of application processes to the agents is not possible with Score-P!!!" );
        return ERROR;

        // TODO check what data can be had from the frontend now, as parameters or
        // environment variables, and what can be acquired later by the connecting application
        // processes (inverting startup order here).
        process_entries.sort();
        int rank            = 0;
        int mpinumprocs     = atoi( opts.mpinumprocs_string );
        int connected_count = 0;
        // TODO evaluate the best way to determine the default port (was done with the config file before) -IC
        agent_port = 51000;
        agent_sock = -1;

        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                    "Starting server for %d MRIs to connect ...\n",
                    mpinumprocs );
        // TODO need to change this call to socket_server_startup_retry() and progagate the port to the frontend in the heartbeat -IC
        agent_sock = socket_server_startup( agent_port );
        if( agent_sock < 0 ) {
            psc_errmsg( "Error opening socket at the AA with port: %d; abborting!\n", agent_port );
            exit( -1 );
        }
        // TODO update with proper values propagated and supermuc case; this should be error-checked at the FE on_heartbeat -IC
        if( agent->parent_handler_ ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                        "send the heartbeat for the parent with port: %d and host: %s\n",
                        agent_port, "localhost" );
            // TODO the last value is the number of ranks per AA; this is not available at this point in this mode
            // TODO update the port information if it changed (the FE then passed the agent's port:host to the MPI application) -IC
            agent->parent_handler_->heartbeat( agent->own_info_.hostname,
                                               agent->own_info_.port,
                                               agent->own_info_.tag,
                                               OWN_HEARTBEAT,
                                               1 );
        }
        else {
            psc_errmsg( "DP: Parent handler not set...\n" );
            exit( -1 );
        }

        std::list <ApplProcess>::iterator process_it;
        controlled_processes.clear();
        while( connected_count < mpinumprocs ) {
            ApplProcess process( socket_server_accept_client( agent_sock ), -1, -1 );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                        "doing accept=%d/%d; port=%d; \n",
                        connected_count, mpinumprocs, agent_port );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                        "the socket FD of the child is: %d\n",
                        process.sock );

            connected_count++;

            if( process.sock == -1 ) {
                psc_errmsg( "Error in accepting application process. connected_count=%d \n",
                            connected_count );
                exit( 1 );
            }
            else {
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                            "Application process accepted: connected_count=%d/%d\n",
                            connected_count, mpinumprocs );
            }
            controlled_processes.push_back( process );
        }
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                    "%d MRIs connected\n", connected_count );

        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                    "Asking for mpi ranks...\n", connected_count );
        int  length;
        char str[ 2000 ];
        for( process_it = controlled_processes.begin();
             process_it != controlled_processes.end();
             process_it++ ) {
            write_line( &( *process_it ), "getmpirank;\n" );

            bzero( str, 2000 );
            while( ( length = read_line( &( *process_it ), str, 2000 ) ) == 0 ) {
                PSC_10MS_DELAY;
            }
            ;

            for( int i = 0; i < length; i++ ) {
                str[ i ] = toupper( str[ i ] );
            }
            if( strcmp( str, "MPIRANK" ) == 0 ) {
                int rlen = 0;
                int rank;
                if( rlen = read_block( &( *process_it ), ( char* )( &rank ), sizeof( int ) ) != sizeof( int ) ) {
                    psc_errmsg( "socket block read %d!=%d", rlen, sizeof( int ) );
                }
                process_it->rank = rank;
                if( rank == 0 ) {
                    agent->set_leader();
                }
            }
            else {
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                            "Expected MPIRANK but got %s!",
                            str );
            }
        }
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                    "MPI ranks received\n" );
    }
    else {
        if( registry->query_entries( rresult, query, false ) == -1 ) {
            psc_errmsg( "Error querying registry for application %s\n",
                        appl->get_app_name().c_str() );
            exit( 1 );
        }

        rresult.sort( compare_registry_entries );

        process_entries.sort();
        std::list<EntryData>::iterator entryit = rresult.begin();
        for( it = process_entries.begin();
             it != process_entries.end() && entryit != rresult.end();
             it++ ) {
            char      hostname[ 2000 ];
            EntryData entry;
            bool      found = false;

            //search registry id of process in list obtained from registry

            while( !found && entryit != rresult.end() ) {
                if( entryit->id == *it ) {
                    entry = *entryit;
                    entryit++;
                    found = true;
                    break;
                }
                else {
                    entryit++;
                }
            }
            if( !found ) {
                psc_errmsg( "Error searching for process id: %d.\n", *it );
                exit( 1 );
            }

            //entry should be the information in the registry for the current process
            strcpy( hostname, entry.node.c_str() );


            start_clock = myWallClock();
            int sock_temp = socket_client_connect( hostname, entry.port );
            end_clock  = myWallClock();
            diff_clock = end_clock - start_clock;
            if( time == 0.0 ) {
                time    = ( double )diff_clock;
                minTime = time;
                maxTime = time;
            }
            else {
                double t = ( double )diff_clock;
                time += t;
                if( minTime > t ) {
                    minTime = t;
                }
                if( maxTime < t ) {
                    maxTime = t;
                }
            }

            ApplProcess process( sock_temp, *it, entry.port, entry.pid - 1 );

            if( process.rank == 0 ) {
                agent->set_leader();
            }

            if( process.sock == -1 ) {
                psc_errmsg( "Error in attaching to application process: %d (%s:%d)\n",
                            *it, hostname, entry.port );
                exit( 1 );
            }
            else {
                psc_dbgmsg( 5, " Attached to application process entry: %d port: %d\n",
                            *it, entry.port );
            }
            controlled_processes.push_back( process );
            total_num_processes = rresult.size();
        }
    }

    //psc_dbgmsg(1,"%d processes monitored out of %d processes total. Connect time:  mean %f, min %f, max %f\n",controlled_processes.size(), total_num_processes, time/controlled_processes.size(),minTime, maxTime);
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                "Connected to %d processes.\n", controlled_processes.size() );
    return OK;
}


ErrorType DataProvider::reinit( RegistryService* registry,
                                int              maplen,
                                int              idmap_from[ 8192 ],
                                int              idmap_to[ 8192 ] ) {
    std::list<int>::iterator it;
    std::list < EntryData >  rresult;
    EntryData                query;
    double                   start_clock, end_clock, diff_clock;
    double                   time = 0.0, maxTime, minTime;

    query.app = appl->get_app_name().c_str();
    query.tag = "none";

    if( agent->get_fastmode() ) {
        /**
         * Fast mode is not possible with Score-P, since the agents are supposed to connect to the Score-P processes
         *
         * TODO: forbid fastmode before the connection is inverted in Score-P.
         */

        psc_errmsg( "Fast mode connection of application processes to the agents is not possible with Score-P!!!" );
        return ERROR;

        // TODO this is a good candidate for code reuse; check init() -IC
        int connected_count = 0;
        // TODO need to get process host and port first
        std::list<ApplProcess>::iterator process;
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                    "re-init: attempting to accept connections from %ld processes...\n",
                    controlled_processes.size() );
        for( process = controlled_processes.begin();
             process != controlled_processes.end();
             process++ ) {
            process->sock = socket_server_accept_client( agent_sock );
            connected_count++;
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                        "re-init: the socket FD is: %d\n", process->sock );
            process->status.state    = PR_SUSPENDED;
            process->status.position = POS_UNKNOWN;
            process->status.rfl      = -1;
            process->status.file_id  = -1;

            if( process->rank == 0 ) {
                agent->set_leader();                    // TODO get this information from the child MPI proc.
            }

            if( process->sock == -1 ) {
                psc_errmsg( "Error in attaching to application process: %d\n",
                            process->registry_entry_id );
                exit( 1 );
            }
            else {
                psc_dbgmsg( 5, " Attached to application process entry: %d port: %d\n",
                            process->rank, process->port );
            }
        }
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                    "%d MRIs connected\n", connected_count );

        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                    "Asking for mpi ranks in re-init (fast mode)...\n",
                    connected_count );
        int                               length;
        std::list <ApplProcess>::iterator process_it;
        char                              str[ 2000 ];
        for( process_it = controlled_processes.begin();
             process_it != controlled_processes.end();
             process_it++ ) {
            write_line( &( *process_it ), "getmpirank;\n" );

            bzero( str, 2000 );
            while( ( length = read_line( &( *process_it ), str, 2000 ) ) == 0 ) {
                PSC_10MS_DELAY;
            }
            ;

            for( int i = 0; i < length; i++ ) {
                str[ i ] = toupper( str[ i ] );
            }
            if( strcmp( str, "MPIRANK" ) == 0 ) {
                int rlen = 0;
                int rank;
                if( rlen = read_block( &( *process_it ), ( char* )( &rank ), sizeof( int ) ) != sizeof( int ) ) {
                    psc_errmsg( "socket block read %d!=%d", rlen, sizeof( int ) );
                }
                process_it->rank = rank;
                if( rank == 0 ) {
                    agent->set_leader();
                }
            }
            else {
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                            "Expected MPIRANK but got %s in re-init!", str );
            }
        }
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                    "MPI ranks received\n" );
    }
    else {       // re-init in registry mode
        if( registry->query_entries( rresult, query, false ) == -1 ) {
            psc_errmsg( "Error querying registry for application %s\n",
                        appl->get_app_name().c_str() );
            exit( 1 );
        }
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                    "Analysisagent got info about %d processes\n",
                    rresult.size() );

        std::map<int, EntryData>       newEntry;
        std::list<EntryData>::iterator entryit;

        for( entryit = rresult.begin(); entryit != rresult.end(); entryit++ ) {
            newEntry[ entryit->id ] = *entryit;
        }

        std::list<ApplProcess>::iterator process;
        for( process = controlled_processes.begin();
             process != controlled_processes.end();
             process++ ) {
            int       rank  = process->rank;
            EntryData entry = newEntry[ idmap_to[ rank ] ];

            process->registry_entry_id = idmap_to[ rank ];
            process->port              = entry.port;
            start_clock                = myWallClock();
            process->sock              = socket_client_connect( ( char* )entry.node.c_str(),
                                                                entry.port );

            end_clock  = myWallClock();
            diff_clock = end_clock - start_clock;
            if( time == 0.0 ) {
                time    = ( double )diff_clock;
                minTime = time;
                maxTime = time;
            }
            else {
                double t = ( double )diff_clock;
                time += t;
                if( minTime > t ) {
                    minTime = t;
                }
                if( maxTime < t ) {
                    maxTime = t;
                }
            }

            process->status.state    = PR_SUSPENDED;
            process->status.position = POS_UNKNOWN;
            process->status.rfl      = -1;
            process->status.file_id  = -1;
            if( process->sock == -1 ) {
                psc_errmsg( "Error in attaching to application process: %d\n",
                            process->registry_entry_id );
                exit( 1 );
            }
            else {
                psc_dbgmsg( 5, " Attached to application process entry: %d port: %d\n",
                            process->registry_entry_id, process->port );
            }
        }
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ),
                "MRI::reinit, %d processes monitored out of %d processes total. Connect time:  mean %f, min %f, max %f\n",
                controlled_processes.size(), total_num_processes, time / controlled_processes.size(), minTime, maxTime );
    return OK;
}

ErrorType DataProvider::wait() {
    char                             str[ 2000 ];
    std::list<ApplProcess>::iterator process;
    int                              length;
    bool                             executing;

    executing = true;
    while( executing ) {
        executing = false;

        for( process = controlled_processes.begin();
             process != controlled_processes.end();
             process++ ) {
            if( process->status.state == PR_WORKING ) {
                bzero( str, 2000 );
                length = read_line( &( *process ), str, 2000 );

                if( length != 0 ) {
                    for( int i = 0; i < length; i++ ) {
                        str[ i ] = toupper( str[ i ] );
                    }
                    if( strcmp( str, "TRACEDATA" ) == 0 ) {
                        executing = true;
                        continue;
                    }
                    if( strcmp( str, "SUSPENDED" ) == 0 ) {
                        process->status.state = PR_SUSPENDED;
                        continue;
                    }
                    if( strcmp( str, "SUSPENDEDATEND" ) == 0 ) {
                        process->status.state = PR_SUSPENDED_AT_END;
                        continue;
                    }

                    executing = true;
                    continue;
                }
            }
        }
    }             //Busy waiting loop for all processes to suspend
    return OK;
}

ApplicationState DataProvider::no_wait() {
    ApplicationState                 appState;
    std::list<ApplProcess>::iterator process;

    appState = APP_SUSPENDED;
    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        if( process->status.state == PR_WORKING ) {
            appState = APP_WORKING;
        }
        if( process->status.state == PR_SUSPENDED_AT_END && appState != APP_WORKING ) {
            appState = APP_SUSPENDED_AT_END;
        }
        if( process->status.state == PR_TERMINATED && appState != APP_WORKING ) {
            appState = APP_TERMINATED;
        }
    }

    return appState;
}

bool DataProvider::test_need_restart() {
    if( no_wait() == APP_TERMINATED || no_wait() == APP_SUSPENDED_AT_END ) {
        return true;
    }
    else {
        return false;
    }

    return false;
}

bool DataProvider::still_executing() {
    char                             str[ 2000 ];
    std::list<ApplProcess>::iterator process;
    int                              length;
    bool                             executing;

    executing = false;

    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        if( process->status.state == PR_WORKING ) {
            bzero( str, 2000 );
            length = read_line( &( *process ), str, 2000 );

            if( length != 0 ) {
                for( int i = 0; i < length; i++ ) {
                    str[ i ] = toupper( str[ i ] );
                }
                if( strcmp( str, "TRACEDATA" ) == 0 ) {
                    executing = true;
                    continue;
                }
                if( strcmp( str, "SUSPENDED" ) == 0 ) {
                    process->status.state = PR_SUSPENDED;
                    continue;
                }

                if( strcmp( str, "SUSPENDEDATEND" ) == 0 ) {
                    process->status.state = PR_SUSPENDED_AT_END;
                    continue;
                }

                executing = true;
                continue;
            }
        }
    }
    return executing;
}


std::list<Metric> DataProvider::formRequestSet() {
    std::list<Metric>                  result;
    std::map< int, std::list<Metric> > papi_sets;
    std::vector<Metric>                enopt_set;

    int largest_set_id   = -1;
    int largest_set_size = 0;

    std::list<Metric>::iterator request_iter;
    psc_dbgmsg( 6, "================Before selection==================\n" );
    for( request_iter = measurement_requests.begin();
         request_iter != measurement_requests.end();
         request_iter++ ) {
        psc_dbgmsg( 6, "Request: %s\n", EventList[ *request_iter ].EventName );
    }

    /* Loop over requested metrics and distribute them to sets */
    request_iter = measurement_requests.begin();
    while( request_iter != measurement_requests.end() ) {
        /* Different requested metric groups require corresponding handling*/
        switch( EventList[ *request_iter ].EventGroup ) {
        case GROUP_PAPI_COUNTER:
        case GROUP_PAPI_POWER6_COUNTER:
        case GROUP_PAPI_NEHALEM_COUNTER:
        case GROUP_PERISCOPE_COUNTER:
        {
            //
            // In case of PAPI metrics, request conflicts are possible. Score-P will throw an error and quit if the
            // requested PAPI events (metrics) combination is not valid. Therefore we will group them into valid sets.
            //

            /* get the papi set (available sets are acquired experimentally and hard-coded in the data provider see
             * initialize_metric2papiset) of the current metric. */
            int papi_set = metric2papiset[ *request_iter ];

            /* push the current request into the list of requests for the obtained before papi set */
            papi_sets[ papi_set ].push_back( *request_iter );

            /* select the most populated papi set for later use */
            if( papi_sets[ papi_set ].size() > largest_set_size ) {
                largest_set_id   = papi_set;
                largest_set_size = papi_sets[ papi_set ].size();
            }

            break;
        }
        case GROUP_TIME_MEASUREMENT:
        case GROUP_MPI:
        case GROUP_HDEEM:
        case GROUP_ENERGY:
        case GROUP_ENOPT_ENERGY_COUNTER:
            enopt_set.push_back( *request_iter );
            break;
        default:
            psc_errmsg( "Request for a metric %s is ignored since it is not supported by Score-P\n",
                        EventList[ *request_iter ].EventName );
            abort();
        }

        ++request_iter;
    }

    /* remove all request from the global request list. Only the requests from the most
     * populated set will be submitted (submitted requests are always removed from the global request list).
     * Those which were not submitted will be pushed back to the global request list. They will be again
     * grouped and submitted in the next online phase iteration. This is done so because removing and inserting
     * elements in the list is faster then traversing the list*/
    measurement_requests.clear();

    /* push request from the most populated set to the list of selected requests, others back to the global request list */
    std::map<int, std::list<Metric> >::iterator outer_iter;
    std::list<Metric>::iterator                 inner_iter;
    bool                                        papi_used = false;
    for( outer_iter = papi_sets.begin(); outer_iter != papi_sets.end(); outer_iter++ ) {
        std::list<Metric> set    = outer_iter->second;
        int               set_id = outer_iter->first;
        for( inner_iter = set.begin(); inner_iter != set.end(); inner_iter++ ) {
            if( set_id == largest_set_id ) {
                /* if this is the most populated set, push its requests to the list of selected requests */
                result.push_back( *inner_iter );
                papi_used = false;
            }
            else {
                /* else, push it back to the global request list */
                measurement_requests.push_back( *inner_iter );
            }
        }
    }

    //
    // If no PAPI metrics are requested, we can request ENOPT metrics; until we know more, it is safer
    // not to request them together, as ENOPT metrics can resolve to counters incompatible to already
    // requested PAPI metrics!
    //
    BOOST_FOREACH( Metric m, enopt_set ) {
        if( papi_used ) {
            measurement_requests.push_back( m );
        }
        else {
            result.push_back( m );
        }
    }

    // print and return the final set of metrics to be requested from Score-P
    psc_dbgmsg( 6, "================Selected metrics==================\n" );
    BOOST_FOREACH( Metric metric, result )
    psc_dbgmsg( 6, "Selected request: %s\n", EventList[ metric ].EventName );

    psc_dbgmsg( 6, "================Remaining metrics==================\n" );
    BOOST_FOREACH( Metric metric, measurement_requests )
    psc_dbgmsg( 6, "Remaining request: %s\n", EventList[ metric ].EventName );

    psc_dbgmsg( 6, "=======================END=========================\n" );
    return result;
}


void DataProvider::initialize_metric2papiset() {
    metric2papiset[ PSC_NP_THREAD_P ]                   = 0;
    metric2papiset[ PSC_NP_UOPS_EXECUTED_PORT015 ]      = 0;
    metric2papiset[ PSC_NP_UOPS_ISSUED_FUSED ]          = 0;
    metric2papiset[ PSC_NP_UOPS_ISSUED_ANY ]            = 0;
    metric2papiset[ PSC_NP_UOPS_RETIRED_ANY ]           = 0;
    metric2papiset[ PSC_NP_STALL_CYCLES ]               = 1;
    metric2papiset[ PSC_NP_RESOURCE_STALLS_ANY ]        = 1;
    metric2papiset[ PSC_NP_INSTRUCTION_RETIRED ]        = 1;
    metric2papiset[ PSC_NP_MEM_INST_RETIRED_LOADS ]     = 1;
    metric2papiset[ PSC_NP_MEM_INST_RETIRED_STORES ]    = 1;
    metric2papiset[ PSC_NP_DTLB_MISSES_ANY ]            = 2;
    metric2papiset[ PSC_NP_DTLB_LOAD_MISSES_ANY ]       = 2;
    metric2papiset[ PSC_NP_DTLB_MISSES_WALK_COMPLETED ] = 2;
    metric2papiset[ PSC_NP_ITLB_MISSES_ANY ]            = 2;
    metric2papiset[ PSC_NP_PARTIAL_ADDRESS_ALIAS ]      = 3;
    metric2papiset[ PSC_NP_UOPS_DECODED_MS ]            = 3;
    metric2papiset[ PSC_PAPI_L2_DCM ]                   = 4;
    metric2papiset[ PSC_PAPI_L2_DCA ]                   = 4;
    metric2papiset[ PSC_PAPI_TLB_DM ]                   = 4;
    metric2papiset[ PSC_PAPI_LST_INS ]                  = 5;
}

int countTrials=0;

char DataProvider::blockingBufferReceive(       void**                       buffer_out,
                                                int*                         buffer_size_out,
                                                ApplProcess*                 process,
                                                scorep_oaconsumer_data_types buffer_type ) {
    int         length;
    char        str[ 2000 ];
    std::string buffer_type_name;
    int         buffer_type_size;
    int         number_of_elements;

    if( !buffer_size_out ) {
        psc_errmsg( "DataProviderScoreP::blockingBufferReceive: buffer_size_out is NULL!\n" );
        return 0;
    }

    /* Get the expected header of the given buffer type */
    switch( buffer_type ) {
    case MERGED_REGION_DEFINITIONS:
        buffer_type_name = "MERGED_REGION_DEFINITIONS";
        buffer_type_size = sizeof( SCOREP_OA_CallPathRegionDef );
        break;
    case FLAT_PROFILE:
        buffer_type_name = "FLAT_PROFILE";
        buffer_type_size = sizeof( SCOREP_OA_FlatProfileMeasurement );
        break;
    case COUNTER_DEFINITIONS:
        buffer_type_name = "METRIC_DEFINITIONS";
        buffer_type_size = sizeof( SCOREP_OA_CallPathCounterDef );
        break;
    case CALLTREE_DEFINITIONS:
        buffer_type_name = "CALLTREE_DEFINITIONS";
        buffer_type_size = sizeof( SCOREP_OA_CallTreeDef );
        break;
    case RTS_MEASUREMENTS:
        buffer_type_name = "RTS_MEASUREMENTS";
        buffer_type_size = sizeof( SCOREP_OA_RtsMeasurement );
        break;
    default:
        psc_errmsg( "Unsupported buffer type %d!\n", buffer_type );
        return 0;
        break;
    }

    /* Receive header of the buffer */
    bzero( str, 2000 );
    while( ( length = read_line( process, str, 2000 ) ) == 0 ) {
        sleep( 1 );
        if (countTrials==60){
          psc_dbgmsg( 4, "..." );
          countTrials=0;
        } else {
          countTrials++;
        }
    };
    for( int i = 0; i < length; i++ ) {
        str[ i ] = toupper( str[ i ] );
    }

    /* Check if the buffer header is the expected one */
    if( strcmp( str, buffer_type_name.c_str() ) != 0 ) {
        psc_errmsg( "Expected %s but got %s", buffer_type_name.c_str(),str );
        return 0;
    }

    /* Receive number of the elements in the buffer*/
    if( length = read_block( process, ( char* )( &number_of_elements ), sizeof( int ) ) != sizeof( int ) ) {
        psc_errmsg( "wrong number of bytes read: %d!=%d\n", length, sizeof( int ) );
        abort();
    }

    psc_dbgmsg( 6, "Receiving %s from proc %d - %d entries of size %d \n",
                buffer_type_name.c_str(),
                process->rank,
                number_of_elements,
                buffer_type_size );
    if( number_of_elements == 0 )
        psc_abort( "Error: The profile has an invalid number of elements: %d\n", number_of_elements );
     if( buffer_type_size == 0 )
        psc_abort( "Error: The profile has an invalid element size: %d\n", number_of_elements );

    /* Allocate receive buffer */
    void* buffer = calloc( number_of_elements, buffer_type_size );
    if( !buffer )
        psc_abort("Error: Unable to allocate a receive buffer of size %i * %i!\n", number_of_elements, buffer_type_size);

    /* Blocking receive of the buffer */
    if( length = read_block( process, ( char* )( buffer ), number_of_elements * buffer_type_size )
                 != number_of_elements * buffer_type_size ) {
        free( buffer );
        psc_errmsg( "wrong number of bytes read: %d!=%d\n",
                    length, number_of_elements * buffer_type_size );
        abort();
    }

    /* Assign pointer to the received buffer the out pointer, assign number of elements to the corr. out pointer */
    *buffer_out      = buffer;
    *buffer_size_out = number_of_elements;

    /* Print out the received buffer */
    if( active_dbgLevel(AgentApplComm) ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "Got %s from process %d\n",
                    buffer_type_name.c_str(), process->rank );
        print_buffer( *buffer_out, *buffer_size_out, buffer_type );
    }

    return 1;
}


void DataProvider::print_buffer( void*                        buffer,
                                 int                          buffer_size,
                                 scorep_oaconsumer_data_types buffer_type ) {
    switch( buffer_type ) {
    case MERGED_REGION_DEFINITIONS:
    {
        SCOREP_OA_CallPathRegionDef* buffer_casted = ( SCOREP_OA_CallPathRegionDef* )buffer;
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "\n\n-\t-\t-\tMERGED REGION DEFINITIONS\t-\t-\t-\n" );
        for( int i = 0; i < buffer_size; i++ ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "record %d: \t|region_id=%d \t| name=%s \t| file=%s \t| rfl=%d \t| rel=%d \t| adapter_type=%d\t|\n",
                    i,
                    buffer_casted[ i ].region_id,
                    buffer_casted[ i ].name,
                    buffer_casted[ i ].file,
                    buffer_casted[ i ].rfl,
                    buffer_casted[ i ].rel,
                    buffer_casted[ i ].adapter_type
                    );
        }
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "\n\n-\t-\t-\tMERGED_REGION_DEFINITIONS\t-\t-\t-\n\n" );
        break;
    }
    case FLAT_PROFILE:
    {
        SCOREP_OA_FlatProfileMeasurement* buffer_casted = ( SCOREP_OA_FlatProfileMeasurement* )buffer;
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "\n\n-\t-\t-\t       FLAT PROFILE      \t-\t-\t-\n" );
        for( int i = 0; i < buffer_size; i++ ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "record %d: \t|meas_id=%d \t| rank=%" PRId64 " \t| thread=%d \t| region_id=%d \t| samples=%" PRId64 "\t| counter=%d\t| int_val=%" PRId64 "\t|\n",
                    i,
                    buffer_casted[ i ].measurement_id,
                    buffer_casted[ i ].rank,
                    buffer_casted[ i ].thread,
                    buffer_casted[ i ].region_id,
                    buffer_casted[ i ].samples,
                    buffer_casted[ i ].metric_id,
                    buffer_casted[ i ].int_val
                    );
        }
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "-\t-\t-\t       FLAT PROFILE      \t-\t-\t-\n\n" );
        break;
    }
    case COUNTER_DEFINITIONS:
    {
        SCOREP_OA_CallPathCounterDef* buffer_casted = ( SCOREP_OA_CallPathCounterDef* )buffer;
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "\n\n-\t-\t-\t  COUNTER DEFINITIONS  \t-\t-\t-\n" );
        for( int i = 0; i < buffer_size; i++ ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "record %d: \t| counter name=%s \t| unit=%s \t| status=%d\n",
                    i,
                    buffer_casted[ i ].name,
                    buffer_casted[ i ].unit,
                    buffer_casted[ i ].status
                    );
        }
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "-\t-\t-\t  COUNTER DEFINITIONS  \t-\t-\t-\n\n" );
        break;
    }
    case CALLTREE_DEFINITIONS:
    {
        SCOREP_OA_CallTreeDef* buffer_casted = ( SCOREP_OA_CallTreeDef* )buffer;
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "\n\n-\t-\t-\t  CALL-TREE DEFINITIONS \t-\t-\t-\n" );
        for( int i = 0; i < buffer_size; i++ ) {
          psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "record %d: \t| region id = %d \t| region name=%s \t| score-p id = %d \t|parent score-p id = %d\n ",
                    i,
                    buffer_casted[ i ].region_id,
                    buffer_casted[ i ].name,
                    buffer_casted[ i ].scorep_id,
                    buffer_casted[ i ].parent_scorep_id
                    );
        }
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "-\t-\t-\t  CALL-TREE DEFINITIONS  \t-\t-\t-\n\n" );
        break;
    }
    case RTS_MEASUREMENTS:
    {
        SCOREP_OA_RtsMeasurement* buffer_casted = ( SCOREP_OA_RtsMeasurement* )buffer;
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "\n\n-\t-\t-\t       RTS MEASUREMENTS      \t-\t-\t-\n" );
        for( int i = 0; i < buffer_size; i++ ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "record %d: \t| rank=%" PRId64 " \t| thread=%d \t| count=%" PRId64 "\t| metric id=%d\t| int_val=%" PRId64 "\t| score-p id = %d \t|\n",
                    i,
                    buffer_casted[ i ].rank,
                    buffer_casted[ i ].thread,
                    buffer_casted[ i ].count,
                    buffer_casted[ i ].metric_id,
                    buffer_casted[ i ].int_val,
                    buffer_casted[ i ].scorep_id
            );
        }
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "-\t-\t-\t       RTS MEASUREMENTS      \t-\t-\t-\n\n" );
        break;
    }
    default:
    {
        psc_errmsg( "DataProviderScoreP::print_buffer: Buffer type not known\n" );
        break;
    }
    }
}


void DataProvider::notify_for_global_metrics() {
    psc_dbgmsg( 5, "Notifying subscribers of the global metrics\n" );

    // iterate over metrics to notify about
    typedef std::map<std::string, MetricNotification>::iterator Iterator;
    for( Iterator it = global_metrics_to_notify.begin();
         it != global_metrics_to_notify.end(); ++it ) {
        // notify subscribers
        const std::list<Strategy*>& subscribers = metric_subscribers_map[ it->second.m_id ];
        BOOST_FOREACH( Strategy * subscriber, subscribers )
        subscriber->metric_found_callback( it->second.m_id, *it->second.ct );

        // notify group subscribers
//        const std::list<Strategy*>& group_subscribers = metric_group_subscribers_map[ EventList[ it->second.m_id ].EventGroup ];
//        BOOST_FOREACH( Strategy * subscriber, group_subscribers )
//        subscriber->metric_found_callback( it->second.m_id, *it->second.ct );

        // delete the context object which was created in the add_metric_to_notification_list and used only here
        delete it->second.ct;
    }

    global_metrics_to_notify.clear();
}


void DataProvider::notify_for_region_types() {
    psc_dbgmsg( 5, "Notifying subscribers of the region definitions\n" );
    std::map <uint64_t, bool>::iterator reg_id_iter;
    /* Loop over region ids which are to be notified about*/
    for( reg_id_iter = regions_to_notify.begin(); reg_id_iter != regions_to_notify.end(); reg_id_iter++ ) {
        psc_dbgmsg( 7, "Region with id %" PRId64 " is ...", reg_id_iter->first );
        /* Check whether this region id was already notified about*/
        if( !reg_id_iter->second ) {
            psc_dbgmsg( 7, " reported!\n" );
            Region* reg = appl_->getRegionByKey( reg_id_iter->first );
            if( !reg ) {
                continue;
            }
            /* Notify region type subscribers */
            /* Get the list of subscribers for this metric */
            std::list<Strategy*>           subscribers = region_type_subscribers_map[ reg->get_type() ];
            std::list<Strategy*>::iterator subscribers_iter;

            /* Loop over subscribers of this region type */
            for( subscribers_iter = subscribers.begin();
                 subscribers_iter != subscribers.end();
                 subscribers_iter++ ) {
                /* notify the subscriber about region type received from ScoreP */
                ( *subscribers_iter )->region_definition_received_callback( reg );
            }
            reg_id_iter->second = true;
        }
        else {
            psc_dbgmsg( 7, " NOT reported!\n" );
        }
    }
    regions_to_notify.clear();
}

void DataProvider::storeFlatProfiles( SCOREP_OA_FlatProfileMeasurement* profiles,
                                  int                               profiles_size,
                                  SCOREP_OA_CallPathCounterDef*     metrics ) {
    for( int i = 0; i != profiles_size; ++i ) {
        const SCOREP_OA_FlatProfileMeasurement profile = profiles[ i ];
        const SCOREP_OA_CallPathCounterDef metric      = metrics[ profiles[ i ].metric_id ];
        // translate ScoreP metric to Periscope metric
        Metric metric_id = translateMetricSCOREP2PSC( metric.name, 0, profile.region_id );
        if( metric_id == PSC_UNDEFINED_METRIC ) {
            // Metric translation failure might be related to the metric text case. Please check that first.
            psc_errmsg( "storeFlatProfile: Metric translation failed: %s\n", metric.name );
            abort();
        }
        union {
            uint64_t in;
            INT64    out;
        } translate_profile_new_val;
        translate_profile_new_val.in = profile.int_val;
        // get the region in which the metric was measured
        Region* const region = appl->getRegionByKey( definition_mapping[ profile.region_id ] );
        // create a Periscope-style context for the measurement
        Context metric_context( region, profile.rank, profile.thread );
        storeProfile( metric_context, metric_id, translate_profile_new_val.out, region, profile.samples );
    }
}

void DataProvider::storeRtsProfiles( SCOREP_OA_RtsMeasurement*      rtsprofiles,
                                  int                               profiles_size,
                                  SCOREP_OA_CallPathCounterDef*     metrics ) {
    for( int i = 0; i < profiles_size; i++ ) {
        SCOREP_OA_RtsMeasurement rtsprofile = rtsprofiles[ i ];
        SCOREP_OA_CallPathCounterDef metric = metrics[ rtsprofiles[ i ].metric_id ];
        // translate ScoreP metric to Periscope metric
        Metric metric_id = translateMetricSCOREP2PSC( metric.name, rtsprofile.scorep_id, 0 );
        if( metric_id == PSC_UNDEFINED_METRIC ) {
            // Metric translation failure might be related to the metric text case. Please check that first.
            psc_errmsg( "storeRtsProfile: Metric translation failed: %s\n", metric.name );
            abort();
        }
        union {
            uint64_t in;
            INT64    out;
        } translate_profile_new_val;
        translate_profile_new_val.in = rtsprofile.int_val;

        // get the rts in which the metric was measured
        //TODO write appl->getRtsByScorepID( rtsprofile.scorep_id ) function for rts lookup based on rts scorep id
        std::map<int, Rts*> scorepid_rts_mapping = rtstree::get_scorepid_to_rts_mapping();
        std::map<int, Rts*>::iterator scorepid_rts_mapping_it = scorepid_rts_mapping.find( rtsprofile.scorep_id );
//        std::map<int, Rts*>::iterator scorepid_it;
//        for(scorepid_it = scorepid_rts_mapping.begin(); scorepid_it != scorepid_rts_mapping.end() ; scorepid_it++) {
//            psc_dbgmsg( 7, "Printing contents of scorepid_rts_mapping: scorep region %d -> %s\n",scorepid_it->first,scorepid_it->second->getRegion()->get_name().c_str());
//        }
        if( scorepid_rts_mapping_it != scorepid_rts_mapping.end() ){
            Region* const region = scorepid_rts_mapping_it->second->getRegion();
            Rts* const    rts    = scorepid_rts_mapping_it->second;
//            break;

            // create a Periscope-style context for the rts measurement
            Context rts_context( rts, rtsprofile.rank, rtsprofile.thread );
            storeProfile( rts_context, metric_id, translate_profile_new_val.out, region, rtsprofile.count );
//            printf("rts_context->isRtsBased() %d\n", rts_context.isRtsBased());
        }
    }
}


void DataProvider::storeProfile( Context& context, Metric& metric_id, INT64& int_val, Region* region, uint64_t samples ) {

    // store measurement in database
    pdb->store( &context, ( Metric )metric_id, int_val );
    pdb->store( &context, PSC_INSTANCES, samples );
    //printf("\n\n -->storeProfile() Metric %s region name =%s region type=%d  instances=%ld value=%ld\n", metric_id , region->get_name(), region->get_type(), samples, int_val);

    /* Notify subscribers of this global metric */
    add_metric_to_notification_list( metric_id, context );

    /* Store this metric as EXECUTION_TIME for this region/rts in case it is  */
    if( EventList[ metric_id ].EventGroup == GROUP_OMP ) {
        pdb->store( context.getFileId(), context.getRfl(),
                    context.getRank(), context.getThread(),
                    PSC_EXECUTION_TIME, int_val);
        /* Notify subscribers of this global metric */
        add_metric_to_notification_list( PSC_EXECUTION_TIME, context );
    }

    /* If it is a TASK_REGION_BODY, then we need to store samples of this region also as  PSC_TASKS_EXECUTED */
    if( region->get_type() == TASK_REGION_BODY ) {
        pdb->store( context.getFileId(), context.getRfl(),
                    context.getRank(), context.getThread(),
                    PSC_TASKS_EXECUTED, samples );
        /* Notify subscribers of this global metric */
        add_metric_to_notification_list( PSC_TASKS_EXECUTED, context );
    }
    /* If it is a MPI_CALL, then we need to store samples of this region also as MPI_CALL_COUNT */
    if( region->get_type() == MPI_CALL && metric_id == PSC_MPI_TIME_SPENT ) {
       pdb->store( &context, PSC_MPI_CALL_COUNT, samples );
       pdb->store( &context, PSC_INSTANCES, samples );
       pdb->store( &context, PSC_EXECUTION_TIME, int_val );
        /* Notify subscribers of this global metric */
       add_metric_to_notification_list( PSC_MPI_CALL_COUNT, context );
       add_metric_to_notification_list( PSC_EXECUTION_TIME, context );
       add_metric_to_notification_list( PSC_INSTANCES, context );
    }
    /* If it is a TASK_REGION, then we need to store samples of this region also as  PSC_TASKS_CREATED */
    if( region->get_type() == TASK_REGION ) {
        pdb->store( context.getFileId(), context.getRfl(),
                    context.getRank(), context.getThread(),
                    PSC_TASKS_CREATED, samples );
        /* Notify subscribers of this global metric */
        add_metric_to_notification_list( PSC_TASKS_CREATED, context );
    }
    /* Finally store region samples as PSC_INSTANCES as well */
    if( region->get_type() != MPI_CALL ) {
        pdb->store( context.getFileId(), context.getRfl(),
                    context.getRank(), context.getThread(),
                    PSC_INSTANCES, samples );
        /* Notify subscribers of this global metric */
        add_metric_to_notification_list( PSC_INSTANCES, context );
    }
}

RegionType DataProvider::convertRegionTypeSCOREP2PSC( uint32_t scorep_region_type,
                                                      char*    region_name ) {
    RegionType            periscope_region_type;
    SCOREP_OA_RegionTypes scorep_region_type_ = ( SCOREP_OA_RegionTypes )scorep_region_type;
    switch( scorep_region_type_ ) {
    case SCOREP_ADAPTER_COMPILER:
        periscope_region_type = CALL_REGION;
        break;
    case SCOREP_ADAPTER_USER:
        /* Only Online Access Phase region has a honor to be Periscope's USER_REGION */
        /* Check if it is a phase region. Phase region is defined by adapter_type=SCOREP_ADAPTER_USER and
         * a name provided to the Periscope at command line */

        if( !strcasecmp( region_name, phase_name.c_str() ) ) {
            periscope_region_type = USER_REGION;
        }
        else {
            periscope_region_type = CALL_REGION;
        }
        break;

    case SCOREP_ADAPTER_POMP:
        periscope_region_type = refineOMPregionByName( region_name );
        break;
    case SCOREP_ADAPTER_MPI:
        periscope_region_type = MPI_CALL;
        break;
    default:
        periscope_region_type = CALL_REGION;
        break;
    }
    psc_dbgmsg( 7, "convertRegionTypeSCOREP2PSC: scorep region %d <%s> -> psc region type=%d\n",
                scorep_region_type, region_name, periscope_region_type );
    return periscope_region_type;
}

RegionType DataProvider::refineOMPregionByName( std::string region_name ) {
    RegionType result = CALL_REGION;

    /*ScoreP OpenMP region template: !$omp <OpenMP region type> @filename:rfl*/

    /* cut @filename:rfl */
    int cut_position = region_name.find_first_of( "@" );
    if( cut_position != std::string::npos ) {
        region_name.resize( cut_position );
    }

    /* break remaining string into tokens with " " being delimeter and put them into list*/
    std::list<std::string> tokens;
    int                    pos   = 0;
    std::string            delim = " ";
    while( pos != std::string::npos ) {
        std::string token_str;
        pos = get_token( region_name, pos, delim, token_str );
        tokens.push_back( token_str );
    }

    std::list<std::string>::iterator token_iter;
    token_iter = tokens.begin();
    /* skip !$omp token */
    token_iter++;
    if( token_iter == tokens.end() ) {
        return result;
    }
    /* parse tokens*/
    if( !strcasecmp( ( *token_iter ).c_str(), "create" ) ) { //!$omp create task
        result =  TASK_REGION;
        return result;
    }
    if( !strcasecmp( ( *token_iter ).c_str(), "task" ) ) { //!$omp task
        result =  TASK_REGION_BODY;
        return result;
    }
/* Don't map this region, since there is no corresponding region type in Periscope. It will get CALL_REGION type */
//    if ( !strcasecmp( ( *token_iter ).c_str(), "taskwait" ) ) //!$omp taskwait
//    {
//        result =  TASK_REGION;
//        return result;
//    }
    if( !strcasecmp( ( *token_iter ).c_str(), "parallel" ) ) { //!$omp parallel
        result =  PARALLEL_REGION;
        token_iter++;
        if( !strcasecmp( ( *token_iter ).c_str(), "do" ) ) {  //!$omp parallel do
            result = DO_REGION;
        }
        return result;
    }
    if( !strcasecmp( ( *token_iter ).c_str(), "do" ) ) { //!$omp parallel
        result = DO_REGION;
        return result;
    }
    if( !strcasecmp( ( *token_iter ).c_str(), "single" ) ) { //!$omp single
        result =  SINGLE_REGION;
        token_iter++;
        if( !strcasecmp( ( *token_iter ).c_str(), "sblock" ) ) {  //!$omp single sblock
            result = SINGLE_REGION_BODY;
        }
        return result;
    }
    if( !strcasecmp( ( *token_iter ).c_str(), "implicit" ) ) { //!$omp implicit barrier
        result =  IMPLICIT_BARRIER_REGION;
        return result;
    }
    if( !strcasecmp( ( *token_iter ).c_str(), "for" ) ) { //!$omp for
        result =  DO_REGION;
        return result;
    }
    if( !strcasecmp( ( *token_iter ).c_str(), "master" ) ) { //!$omp master
        result =  MASTER_REGION_BODY;
        return result;
    }
    if( !strcasecmp( ( *token_iter ).c_str(), "barrier" ) ) { //!$omp barrier
        result =  BARRIER_REGION;
        return result;
    }
    if( !strcasecmp( ( *token_iter ).c_str(), "critical" ) ) { //!$omp critical
        result =  CRITICAL_REGION;
        token_iter++;
        int test = ( *token_iter ).find_first_of( "(" );
        if( test != std::string::npos ) {
            result =  CRITICAL_REGION_BODY;
        }
        return result;
    }
    if( !strcasecmp( ( *token_iter ).c_str(), "atomic" ) ) { //!$omp atomic
        result =  ATOMIC_REGION;
        return result;
    }
    if( !strcasecmp( ( *token_iter ).c_str(), "sections" ) ) { //!$omp sections
        result =  SECTIONS_REGION;
        return result;
    }
    if( !strcasecmp( ( *token_iter ).c_str(), "section" ) ) { //!$omp section
        result =  SECTION_REGION;
        return result;
    }
    if( !strcasecmp( ( *token_iter ).c_str(), "section" ) ) { //!$omp section
        result =  SECTION_REGION_BODY;
        return result;
    }
    if( !strcasecmp( ( *token_iter ).c_str(), "flush" ) ) { //!$omp section
        result =  OMP_FLASH_REGION;
        return result;
    }


    return result;
}

std::map<int32_t, uint64_t> DataProvider::storeAndIndexRegionDefinitions(
    SCOREP_OA_CallPathRegionDef* definitions,
    int                          number_of_definitions,
    int                          rank ) {
    std::map<int32_t, uint64_t> local_to_global_region_ids;
    RegionType                  region_type;


    bool phase_found = false;

    /* Loop over definitions buffer */
    for( int j = 0; j < number_of_definitions; j++ ) {
        /* Convert ScoreP region type to the corresponding Periscope region type */
        region_type = convertRegionTypeSCOREP2PSC( definitions[ j ].adapter_type, definitions[ j ].name );

        /* Add (if not already exists) region definition to the region list of the application class.
         * Get process-global (within the corresponding AAgent) id of the region */

        Region* region = appl_->addSCOREPRegion( definitions[ j ].rfl,
                                                 definitions[ j ].file,
                                                 definitions[ j ].name,
                                                 region_type,
                                                 definitions[ j ].rfl,
                                                 definitions[ j ].rel );
        uint64_t region_id = region->getLocalRegionID();

        /* map process-global to process-local region id for later use*/
        local_to_global_region_ids[ j ] = region_id;

        /* keep scorep region id and map it to the rank and the AA global region id */
        scorep_region_id_mappings[ region_id ][ definitions[ j ].region_id ].push_back( rank );
        scorep_region_id_mappings[ region_id ][ definitions[ j ].region_id ].unique();

        /* Store the id of this region for later notification about it, if it was not already stored*/
        if( regions_to_notify.find( region_id ) == regions_to_notify.end() ) {
            psc_dbgmsg( 7, "Region %s with id %" PRId64 " is added to notification list\n",
                        definitions[ j ].name, region_id );
            regions_to_notify[ region_id ] = false;
        }
        /* If it is a phase region store it in application object */
        if( region_type == USER_REGION ) {
            phase_found = true;
            appl_->set_phase_region( region );
        }
    }

    if( !phase_found ) {
        psc_errmsg( "Provided phase region <%s> was not found among the region definitions provided by Score-P! Further analysis might not be complete.\n",
                    phase_name.c_str() );
    }

    return local_to_global_region_ids;
}

Metric DataProvider::interpretExecutionTimeOfRegion( RegionType region_type ) {
    switch( region_type ) {
    case IMPLICIT_BARRIER_REGION:
        return PSC_IMPLICIT_BARRIER_TIME;          //check
    case CRITICAL_REGION:
        return PSC_CRITICAL_REGION_CYCLE;          //check
    case CRITICAL_REGION_BODY:
        return PSC_CRITICAL_BODY_CYCLE;            //check
    case SINGLE_REGION_BODY:
        return PSC_SINGLE_BODY_CYCLE;              //check
    case SINGLE_REGION:
        return PSC_SINGLE_REGION_CYCLE;            //check
    case MASTER_REGION_BODY:
        return PSC_MASTER_BODY_CYCLE;              //check
    case PARALLEL_REGION:
        return PSC_PARALLEL_REGION_CYCLE;          //check
    case PARALLEL_REGION_BODY:
        return PSC_PARALLEL_REGION_BODY_CYCLE;     //check
    case TASK_REGION:
        return PSC_TASK_REGION_CYCLE;              //check
    case TASK_REGION_BODY:
        return PSC_TASK_REGION_BODY_CYCLE;         //check
    case BARRIER_REGION:
        return PSC_OMP_BARRIER_CYCLE;              //check
    case DO_REGION:
        return PSC_OMP_DO_REGION_CYCLE;            //check
    case WORKSHARE_DO:
        return PSC_OMP_DO_REGION_CYCLE;            //check
    case ORDERED_REGION:
        return PSC_ORDERED_REGION_CYCLE;           //check
    case ATOMIC_REGION:
        return PSC_OMP_ATOMIC_CYCLE;               //check
    case OMP_FLASH_REGION:
        return PSC_FLUSH_CYCLES;                   //check
    case SECTIONS_REGION:
        return PSC_OMP_SECTIONS_REGION_CYCLE;      //check
    case SECTION_REGION_BODY:
        return PSC_OMP_SECTION_BODY_CYCLE;         //check
    case MPI_CALL:
        return PSC_MPI_TIME_SPENT;
    default:
        return PSC_EXECUTION_TIME;
    }
}

Metric DataProvider::translateMetricSCOREP2PSC( std::string metric_scorep_name,
                                                uint32_t    scorep_node_id,
                                                uint32_t    scorep_region_id = 0 ) {
    Metric result = PSC_UNDEFINED_METRIC;

    /* Check if the provided metric name is in the requested metric map (in the list of submitted metrics by the DataProvider)*/
    if( submitted_request_map.count( metric_scorep_name ) == 0 ) {
        /* Special case. MPI late send metrics are called late_send. We need to rename them to MPI to match Periscope
         * PSC_MPI metric. We will further refine it based on region type */
        if( metric_scorep_name == "late_send" ) {
            metric_scorep_name = "MPI";
        }
        else if( metric_scorep_name == "late_receive" ) {
            // TODO: Investigate how to handle LATE_RECEIVE metric returned by Score-P
            psc_dbgmsg( 6, "Late receive type of the metric was reported by Score-P. Since this can not be interpreted \n as a performance problem it will be ignored.\n " );
        }
        else {
            psc_dbgmsg( 6, "Metric %s was not found in requested metric map!\n",
                        metric_scorep_name.c_str() );
            return PSC_UNDEFINED_METRIC;
        }
    }

    /* Restore the periscope metric id from the map of submitted metric requests. Further we will refine the metric based on the region it was found for*/
    Metric requested_metric_psc_id = submitted_request_map[ metric_scorep_name ];

    Region* region = NULL;
    int rts_id;

    /* Check if the region definition specified by ScoreP id is stored and indexed as a Periscope region */
    if( withRtsSupport() && scorep_node_id != 0 ) {
        std::map<int, Rts*> scorepid_rts_mapping = rtstree::get_scorepid_to_rts_mapping();
        std::map<int, Rts*>::iterator scorepid_rts_mapping_it;
        scorepid_rts_mapping_it = scorepid_rts_mapping.find( scorep_node_id );
        if( scorepid_rts_mapping_it != scorepid_rts_mapping.end() ){
            region = scorepid_rts_mapping_it->second->getRegion();
            rts_id = scorepid_rts_mapping_it->second->getRtsID();
        }

        if ( region == NULL ) {
            /* if not, we can not identify this metric */
            psc_dbgmsg( 6, "ScoreP node id %d was not found in the map of stored rts's!\n", scorep_node_id );
            return PSC_UNDEFINED_METRIC;
        }
    }
    else {
        if( definition_mapping.count( scorep_region_id ) == 0 ) {
            /* if not, we can not identify this metric */
            psc_dbgmsg( 6, "ScoreP Region id %d was not found in the map of indexed and stored regions!\n",
                    scorep_region_id );
            return PSC_UNDEFINED_METRIC;
        }
        /* Get region object by the Periscope region key obtained from the ScoreP region id using the map*/
        region = appl->getRegionByKey( definition_mapping[ scorep_region_id ] );
    }

    if( !region ) {
        psc_dbgmsg( 6, "Region corresponding metric %s was not found!\n",
                    metric_scorep_name.c_str() );
        return PSC_UNDEFINED_METRIC;
    }

    /* Get region type */
    RegionType region_type = region->get_type();

    /* Refine the metric requested to the Score-P based on the region for which it was measured*/
    switch( requested_metric_psc_id ) {
    case PSC_EXECUTION_TIME:
        /* Refine EXECUTION_TIME metric of the region based on the region's type */
        result = interpretExecutionTimeOfRegion( region_type );
        break;
    case PSC_MPI:
        /* Refine MPI metric of the region based on the region's name*/
        result = interpret_MPI_LATE_SEND_OfRegion( region->get_name() );
        break;
    default:
        /* Return the Periscope Metric id as it was requested, without further refinement */
        result = requested_metric_psc_id;
        break;
    }

    if( withRtsSupport() && scorep_node_id != 0 ) {
        psc_dbgmsg( 7, "Translating metric %s of the rts ID %d = %d\n",
                    metric_scorep_name.c_str(), rts_id, result );
    }
    else {
        psc_dbgmsg( 7, "Translating metric %s of the region %s = %d\n",
                    metric_scorep_name.c_str(), region->get_name().c_str(), result );
    }

    return result;
}

Metric DataProvider::interpret_MPI_LATE_SEND_OfRegion( std::string region_name ) {
    psc_dbgmsg( 7, "TRANSLATING MPI METRIC of the region %s\n",
                region_name.c_str() );
    Metric result = PSC_UNDEFINED_METRIC;
    if( !strcasecmp( region_name.c_str(), "MPI_Barrier" ) ) {
        result =  PSC_MPI_LATE_BARRIER;
    }
    if( !strcasecmp( region_name.c_str(), "mpi_waitall" ) ) {
        result =  PSC_MPI_EARLY_RECV;
    }
    if( !strcasecmp( region_name.c_str(), "MPI_Wait" ) ) {
        result =  PSC_MPI_EARLY_RECV;
    }
    if( !strcasecmp( region_name.c_str(), "MPI_Waitsome" ) ) {
        result =  PSC_MPI_EARLY_RECV;
    }
    if( !strcasecmp( region_name.c_str(), "MPI_Waitany" ) ) {
        result =  PSC_MPI_EARLY_RECV;
    }
    if( !strcasecmp( region_name.c_str(), "MPI_Recv" ) ) {
        result =  PSC_MPI_EARLY_RECV;
    }
    if( !strcasecmp( region_name.c_str(), "MPI_Bcast" ) ) {
        result =  PSC_MPI_EARLY_BCAST;
    }
    if( !strcasecmp( region_name.c_str(), "MPI_Scatter" ) ) {
        result =  PSC_MPI_EARLY_SCATTER;
    }
    if( !strcasecmp( region_name.c_str(), "MPI_Scatterv" ) ) {
        result =  PSC_MPI_EARLY_SCATTER;
    }
    if( !strcasecmp( region_name.c_str(), "MPI_Alltoall" ) ) {
        result =  PSC_MPI_LATE_ALLTOALL;
    }
    if( !strcasecmp( region_name.c_str(), "MPI_Alltoallv" ) ) {
        result =  PSC_MPI_LATE_ALLTOALL;
    }
    if( !strcasecmp( region_name.c_str(), "MPI_Reduce" ) ) {
        result =  PSC_MPI_LATE_REDUCE;
    }
    if( !strcasecmp( region_name.c_str(), "MPI_Gather" ) ) {
        result =  PSC_MPI_LATE_GATHER;
    }
    if( !strcasecmp( region_name.c_str(), "MPI_Gatherv" ) ) {
        result =  PSC_MPI_LATE_GATHER;
    }
    if( !strcasecmp( region_name.c_str(), "MPI_Allgather" ) ) {
        result =  PSC_MPI_LATE_ALLGATHER;
    }
    if( !strcasecmp( region_name.c_str(), "MPI_Allgatherv" ) ) {
        result =  PSC_MPI_LATE_ALLGATHER;
    }
    if( !strcasecmp( region_name.c_str(), "MPI_Allreduce" ) ) {
        result =  PSC_MPI_LATE_ALLREDUCE;
    }

    return result;
}

void DataProvider::globalMetricSubscribe( Strategy* strategy,
                                          Metric    m ) {
    if( !strategy ) {
        return;
    }
    /* Add this strategy to the list of subscribers for this metric */
    metric_subscribers_map[ m ].push_back( strategy );
    /* Remove duplicates, to prevent multiple notifications of the same Strategy in the future */
    metric_subscribers_map[ m ].sort();
    metric_subscribers_map[ m ].unique();
}

void DataProvider::globalMetricUnSubscribe( Strategy* strategy,
                                            Metric    m ) {
    if( !strategy ) {
        return;
    }
    /* Remove this strategy from the list of subscribers for this metric */
    metric_subscribers_map[ m ].remove( strategy );
}

void DataProvider::metricGroupSubscribe( Strategy* strategy,
                                         Group     group ) {
    if( !strategy ) {
        return;
    }
    /* Add this strategy to the list of subscribers for this metric group */
    metric_group_subscribers_map[ group ].push_back( strategy );
    /* Remove duplicates, to prevent multiple notifications of the same Strategy in the future */
    metric_group_subscribers_map[ group ].sort();
    metric_group_subscribers_map[ group ].unique();
}

void DataProvider::metricGroupUnSubscribe( Strategy* strategy,
                                           Group     group ) {
    if( !strategy ) {
        return;
    }
    /* Remove this strategy from the list of subscribers for this metric */
    metric_group_subscribers_map[ group ].remove( strategy );
}

void DataProvider::regionTypeSubscribe( Strategy*  strategy,
                                        RegionType type ) {
    if( !strategy ) {
        return;
    }
    /* Add this strategy to the list of subscribers for this region type */
    region_type_subscribers_map[ type ].push_back( strategy );
    /* Remove duplicates, to prevent multiple notifications of the same Strategy in the future */
    region_type_subscribers_map[ type ].sort();
    region_type_subscribers_map[ type ].unique();
}

void DataProvider::regionTypeUnSubscribe( Strategy*  strategy,
                                          RegionType type ) {
    if( !strategy ) {
        return;
    }
    /* Remove this strategy from the list of subscribers for this region type */
    region_type_subscribers_map[ type ].remove( strategy );
}

void DataProvider::setIterationBurstLength( int length ) {
    burst_counter               = length;
    measurement_requests_backup = measurement_requests;
}

void DataProvider::add_metric_to_notification_list( Metric  m,
                                                    Context ct ) {
    Context*           ct_to_store = new Context( ct );
    MetricNotification mn;
    mn.ct   = ct_to_store;
    mn.m_id = m;
    std::stringstream key;
    if( ct.isRtsBased() ) {
        key << EventList[ m ].EventName << ct.getRegion()->get_name()
            << ct.getFileId() << ct.getRfl() << ct.getRank() << ct.getThread() << ct.getRtsID();

    }
    else {
        key << EventList[ m ].EventName << ct.getRegion()->get_name()
            << ct.getFileId() << ct.getRfl() << ct.getRank() << ct.getThread();
    }
    global_metrics_to_notify[ key.str() ] = mn;
}
