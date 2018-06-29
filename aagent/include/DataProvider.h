/**
   @file    DataProvider.h
   @ingroup AnalysisAgent
   @brief   Internal data management
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2011, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#define __STDC_FORMAT_MACROS

#ifndef DATAPROVIDER_H_
#define DATAPROVIDER_H_
#include <map>
#include <stdio.h>
#include <string>
#include <list>
#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "regxx.h"
#include "application.h"
#include "SCOREP_OA_ReturnTypes.h"
#include <vector>
#include "rts.h"

class Scenario;
class Strategy;
class TuningSpecification;

/**
 * A metric notification struct containing the type of the metric and the
 * context where it was measured
 */
struct MetricNotification {
    Context* ct;
    Metric   m_id;
};


/**
 * @brief Errors that can be reported by the data provider.
 */
enum ErrorType {
    OK,
    ERROR,
    RUNTIME_INFORMATION_NOT_AVAILABLE,
    AGGREGATION_NOT_AVAILABLE,
    NO_DATA,
    MAX_NR_REQUESTS_REACHED,
    TRACE_DATA_NOT_AVAILABLE_THROUGH_ROUTINE
};


/**
 * @brief Application (for all processes) states
 */
enum ApplicationState {
    APP_WORKING,
    APP_SUSPENDED,
    APP_SUSPENDED_AT_END,
    APP_TERMINATED,
    APP_GOT_DATA,
    APP_TRACE_BUFFER_FULL,
    APP_STATE_UNKNOWN
};


/**
 * @brief Application process (local) status. Consider moving to cc
 */
enum ApplProcessState {
    PR_WORKING,
    PR_SUSPENDED,
    PR_SUSPENDED_AT_END,
    PR_TERMINATED,
    PR_STATE_UNKNOWN
};


/**
 * @brief Application process position relative to the current region
 */
enum ApplProcessPosition {
    POS_REGION_START,
    POS_REGION_END,
    POS_UNKNOWN
};


/**
 * @brief A data structure storing the current state of an application process
 */
struct ApplProcessStatus {
    int                 file_id;
    int                 rfl;
    ApplProcessPosition position;
    ApplProcessState    state;

    ApplProcessStatus( int file_id, int rfl, ApplProcessPosition position = POS_UNKNOWN, ApplProcessState state = PR_SUSPENDED ) :
        file_id( file_id ), rfl( rfl ), position( position ), state( state ) {
    };
};


/**
 * @brief Represents an application process
 */
class ApplProcess {
    /**
     * @brief a list of Scenarios containing a tuning specification
     * intended for this process
     */
    std::list<Scenario*> scenariosPerTuningSpecification;

    /**
     * @brief a list of Scenarios containing a property requests intended
     * for this process
     */
    std::list<Scenario*> scenariosPerPropertyRequest;
public:
    ApplProcess( int sock, int registry_entry_id, int port, int rank = -1, int file_id = -1, int rfl = -1 ) :
        sock( sock ), registry_entry_id( registry_entry_id ), port( port ), rank( rank ), status( file_id, rfl ) {
    }

    ~ApplProcess() {
    }

    int               registry_entry_id;
    int               port;
    int               sock;
    ApplProcessStatus status;
    int               rank;

    const std::list<Scenario*>* getScenariosPerTuningSpecification();

    const std::list<Scenario*>* getScenariosPerPropertyRequest();
};


/**
 * @brief a request (context-local or -global) for a measurement
 */
struct Request {
    Request( Context* ct,
             Metric   m );

    Context* ct;
    Metric   m;
};


/**
 * The data provider is basically a driver for the monitoring library (Score-P).
 *
 * It offers the following services:
 * 1. Request for a measurement
 * 2. Request for the measurement result
 * 3. Request for an execution command
 * 4. Request for a tuning action
 */
class DataProvider {
    /** A list of objects containing all the information relevant to an application processes managed by this DataProvider*/
    std::list <ApplProcess> controlled_processes;
    /** Total number of processes in the analyzed application */
    int total_num_processes;

    /** The number of iterations to be performed per experiment. */
    int num_iterations_per_experiment;

    /** The status of the DataProvider request and measurement handling*/
    Gather_Required_Info_Type status;

    /** A pointer to the registry service object */
    RegistryService* const registry_;
    /** A pointer to the application object */
    Application* const appl_;
    /** The name given to the Score-P Online Access Phase region */
    const std::string phase_name;

    /** A list of global metric requests */
    std::list<Metric> measurement_requests;
    /** A backup list for the metrics requested when a burst of iterations needs to be executed */
    std::list<Metric> measurement_requests_backup;
    /** A list of tuning requests per application process rank */
    std::map<int, std::list<TuningSpecification*> > tuning_requests;

    /** A map of storing a list of strategy pointers subscribed for a metric type */
    std::map< Metric, std::list<Strategy*> > metric_subscribers_map;
    /** A map of storing a list of strategy pointers subscribed for a metric type group */
    std::map< Group, std::list<Strategy*> > metric_group_subscribers_map;
    /** A map of storing a list of strategy pointers subscribed for a region type */
    std::map< RegionType, std::list<Strategy*> > region_type_subscribers_map;
    /** Region definitions to be notified about */
    std::map<uint64_t, bool> regions_to_notify;
    /** A map of global metric and their notification data structures */
    std::map<std::string, MetricNotification> global_metrics_to_notify;

    /** The number of the last completed phase iteration */
    int current_iteration;
    /** the number of the phase iterations to be completed in the requested burst */
    int burst_counter;

    /** A map for mapping of the process-local region IDs to global (AAgent wise) IDs */
    std::map<int32_t, uint64_t> definition_mapping;
    /** A map mapping global AA region ID to the map of Score-P region definition id mapping
     * to a list of application processes on which the id was recorded */
    std::map<uint64_t, std::map<uint32_t, std::list<int> > > scorep_region_id_mappings;

    /** A map of metric names to metric types of metric requests submitted to the processes */
    std::map<std::string, Metric> submitted_request_map;

    /** A map storing the PAPI set for a metric type */
    std::map<Metric, int> metric2papiset;

    /** Indicates whether old requests are still pending and the last experiment needs to be re-run. */
    bool old_requests_pending;


    //
    // Request handling
    //

    /** Sends the number of iterations to be performed during the next experiment. */
    void transfer_num_iterations_to_processes();

    /** Sends all previously added measurement requests to the monitoring library. */
    void transfer_measurement_requests_to_processes();

    /** Sends all previously added tuning requests to the monitoring library. */
    void transfer_tuning_requests_to_processes();

    /** Sends all previously added rts based tuning requests to the monitoring library. */
    void transfer_rts_tuning_requests_to_processes();

    /* Searching the ScoreP region ID based on the AA region ID*/
    int get_scorepid_by_aa_regionId_for_rank( uint64_t aaRegionID, int rank );

    /*Create Stream for rts Parameter Node type*/
    bool create_param_req_stream( Rts* rts, std::stringstream& req_stream );

    /* Traverse from the valid rts up to the root rts node and store it into the rts_list stack */
    void traverse_reverse(  int root_scorep_id, Rts* rtsNode, std::list<Rts*>* rts_list, int rank );

    /*traverse the rts_list stack and create the rts_based tuning request format*/
    int traverse_forward( Rts* rtsNode, std::list<Rts*>* rts_list, int rank, std::stringstream& req_stream );


    /** Returns an optimal non-conflicting set of metric requests, based on the initialized metric sets */
    std::list<Metric>formRequestSet();

    /** Initializes metric sets containing metrics which can be measured together */
    void initialize_metric2papiset();


    //
    // Communication with application processes
    //

    /* Writes a string specified by str to the socket of the application process */
    void write_line( ApplProcess* process,
                     const char*  str );

    /* Reads a string to the str from the socket of the application process */
    int read_line( ApplProcess* process,
                   char*        str,
                   int          maxlen );

    /* Reads a block of size to the ptr from the socket of the application process */
    int read_block( ApplProcess* process,
                    char*        ptr,
                    int          size );

    /** Receives a buffer of type buffer_type from the process*/
    char blockingBufferReceive( void**                       buffer_out,
                                int*                         buffer_size_out,
                                ApplProcess*                 process,
                                scorep_oaconsumer_data_types buffer_type );

    /** Prints a buffer of size buffer_size of type buffer_type*/
    void print_buffer( void*                        buffer,
                       int                          buffer_size,
                       scorep_oaconsumer_data_types buffer_type );


    //
    // Measurement notification
    //

    /** Adds a metric to the notification list from which subscribers are fed. */
    void add_metric_to_notification_list( Metric  m,
                                          Context ct );

    /** Notifies all subscribers of measured metrics. */
    void notify_for_global_metrics();

    /** Notifies all subscribers for measurements taken on regions of specific types. */
    void notify_for_region_types();


    //
    // Score-P to PSC matching magic
    //

    /** Parses the flat measurement profiles received from Score-P and stores the metrics in the PDB. */
    void storeFlatProfiles( SCOREP_OA_FlatProfileMeasurement* profiles,
                        int                               profiles_size,
                        SCOREP_OA_CallPathCounterDef*     metrics );

    /** Parses the rts measurement profiles received from Score-P and stores the metrics in the PDB. */
    void storeRtsProfiles( SCOREP_OA_RtsMeasurement*         profiles,
                           int                               profiles_size,
                           SCOREP_OA_CallPathCounterDef*     metrics );

    /** Parses the measurement profile received from Score-P and stores the metrics the PDB. */
//    void storeProfile( const SCOREP_OA_FlatProfileMeasurement& profile,
//                       const SCOREP_OA_CallPathCounterDef&     metric );

    void storeProfile( Context& context, Metric& metric_id, INT64& int_val, Region* region, uint64_t samples );

    /** Converts the Score-P region type to a PSC region type*/
    RegionType convertRegionTypeSCOREP2PSC( uint32_t scorep_region_type,
                                            char*    region_name );

    /** Refines OpenMP region type to a PSC region type with the help of the region name*/
    RegionType refineOMPregionByName( std::string region_name );

    /** Stores and indexes region definitions received from Score-P */
    std::map<int32_t, uint64_t>storeAndIndexRegionDefinitions(     SCOREP_OA_CallPathRegionDef* definitions,
                                                                   int                          number_of_definitions,
                                                                   int                          rank );

    /** Interprets a measurement of type execution time based on the type of the region where it was taken*/
    Metric interpretExecutionTimeOfRegion( RegionType region_type );

    /** Translates Score-P metric type given by metric name and a region where it was taken to a PSC metric type*/
    Metric translateMetricSCOREP2PSC( std::string metric_scorep_name,
                                      uint32_t    scorep_node_id,
                                      uint32_t    scorep_region_id);

    /** Translates MPI_LATE_SEND_ metric to a specific metric based on the region type it was measured at */
    Metric interpret_MPI_LATE_SEND_OfRegion( std::string region_name );

public:
    DataProvider( Application*     appl,
                  std::string      phase_name_,
                  RegistryService* registry );


    //
    // DataProvider Management
    //

    /** Initializes the DataProvider*/
    ErrorType init( RegistryService* registry,
                    std::list<int>   process_entries );

    /** Re-initializes the DataProvider after the application restart*/
    ErrorType
    reinit( RegistryService* registry,
            int maplen,
            int idmap_from[ 8192 ],
            int idmap_to[ 8192 ] );

    /** Returns a list of objects representing application processes handled by this DataProvider*/
    std::list <ApplProcess>get_controlled_processes();

    /** Prints application processes handled by this DataProvider*/
    void print_controlled_processes();

    /** Returns the number of application processes handled by this DataProvider */
    int get_total_num_processes();


    //
    // Handling measurement requests
    //

    /** adds a request for measuring metric of type m in the context specified by ct*/
    void addMeasurementRequest( Context* ct,
                                Metric   m );

    /** adds a tuning request specified by a tuning specification for a process specified by rank*/
    void addTuningRequest( int                  rank,
                           TuningSpecification* ts );

    /** cleans the list of requests */
    void cleanAllRequests();

    /** Sends all previously added requests to the monitoring library. */
    void transfer_requests_to_processes();

    /**
     * Subscription and un-subscription functions. All subscribers are notified about all measurements relevant to their
     * subscriptions after all measurements are received and processed by the DataProvider
     */
    /** Subscribes the strategy specified by the pointer for notifications about all measurements of the metric type m */
    void globalMetricSubscribe( Strategy* strategy,
                                Metric    m );

    /** Un-subscribes the strategy specified by the pointer from notifications about all measurements of the metric type m*/
    void globalMetricUnSubscribe( Strategy* strategy,
                                  Metric    m );

    /** Subscribes the strategy specified by the pointer for notifications about all measurements of the metric group type group*/
    void metricGroupSubscribe( Strategy* strategy,
                               Group     group );

    /** Un-subscribes the strategy specified by the pointer from notifications about all measurements of the metric group type group*/
    void metricGroupUnSubscribe( Strategy* strategy,
                                 Group     group );

    /** Subscribes the strategy specified by the pointer for notifications about all measurements for the region type*/
    void regionTypeSubscribe( Strategy*  strategy,
                              RegionType type );

    /** Un-subscribes the strategy specified by the pointer from notifications about all measurements for the region type*/
    void regionTypeUnSubscribe( Strategy*  strategy,
                                RegionType type );

    /**
     * Indicates whether old metric requests are still pending because they could not be submitted yet
     * without conflicting with other metrics. If this is the case, no new requests can be made. Instead,
     * transfer the old requests again to produce the pending results.
     */
    bool areOldRequestsPending() const;

    /**
     * Receives results of the measurements configured by requests (see addRequest);
     * parses them and stores them in the Performance Data Base
     */
    Gather_Required_Info_Type getResults();

    /**
     * A block of methods for the application execution request handling
     */
    /** set the number of the phase iterations to be performed in the burst before the results are made available to the consumers*/
    void setIterationBurstLength( int length );


    ErrorType set_num_iterations_per_phase( int iterations );

    ErrorType stop_on_start_region( const Region* reg );

    ErrorType stop_on_end_region( const Region* reg );

    ErrorType app_finish();

    ErrorType wait();

    ApplicationState no_wait();

    bool test_need_restart();

    bool still_executing();

    void wait_for_ok( ApplProcess process );

    int getCurrentIterationNumber();
};

#endif /* DATAPROVIDER_H_ */
