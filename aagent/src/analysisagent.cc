/**
   @file    analysisagent.cc
   @ingroup AnalysisAgent
   @brief   Analysis agent
   @author  Edmond Kereku
   @verbatim
    Revision:       $Revision: 1.63 $
    Revision date:  $Date: 2011/08/08 09:15:32 $
    Committed by:   $Author: gudu $

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2013, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */
#include "config.h"
#include "global.h"
#include "DataProvider.h"
#include "application.h"
#include "experiment.h"
#include "psc_errmsg.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <list>
#include <cstdlib>
#include <cstring>

#include "xml_psc_tags.h"  // Contains the XML tags used in the properties file

#include "selective_debug.h"
#include "analysisagent.h"

/*
 * Strategies are included starting from here
 */
#include "ScorepMPIStrategy.h"
#include "OpenMPAnalysisScoreP.h"
#include "Importance.h"
#include "ConfigAnalysis.h"
#include "EnergyGranularityBF.h"
#include "Tuning.h"

#ifdef HAVE_TDA
#include "PerfDynamicsAnalysis.hpp"
#endif
/**
 * All strategies were disabled during the integration of the Score-P support.
 *
 * Before being re-enabled the strategies have to be adapted to the new measurement
 * request and retrieval mechanism of the DataProvider
 */
/*
   #include "MPIStrategy.h"
   #include "OpenMPAnalysis.h"
   #include "GPUTestingStrategy.h"
   #include "OCLStrategy.h"
   #include "OpenCLStrategy.h"
   #include "EnergyStrategy.h"
   #include "PipelineStrategy.h"
   #include "EnergyGranularity.h"
 */



bool first = true;

bool search_done;
bool calltree_sent;

int EventHandling::handle_timeout( const ACE_Time_Value& curr_time,
                                   const void*           arg ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "EventHandling::handle_timeout\n" );
    return nagent_->handle_step();
}


//
//Implementation of class NodeAgent
//


AnalysisAgent::AnalysisAgent( ACE_Reactor* r ) :
    PeriscopeAgent(r),
    iterations_per_experiment(1),
    doneappexit(false),
    current_experiment(NULL),
    started(false),
    leader(false),
    delay(0),
    strategy(NULL) {
}


AnalysisAgent::~AnalysisAgent() {
//    destroy_summary_table();
    delete strategy;
    delete parent_handler_;
    delete parent_stream_;
    delete own_info_.handler;
    delete own_info_.stream;
}


void AnalysisAgent::set_num_iterations( int iterations ) {
    if( iterations < 1 )
        psc_abort("Error: At least 1 iteration must be performed per experiment.");

    iterations_per_experiment = iterations;
}


//this does everything
void AnalysisAgent::stand_alone_search() {
    Region* cur_phase;

    cur_phase = appl->get_phase_region();

    if( cur_phase != appl->get_main_region() ) {
        dp->stop_on_start_region( cur_phase );
        dp->wait();
        if( agent->get_delay() ) {
            for( int i = 0; i < agent->get_delay(); i++ ) {
                psc_dbgmsg( 1, "Strategy delayed: %d. execution of phase.\n", i + 1 );
                dp->stop_on_start_region( cur_phase );
                dp->wait();
            }
        }
    }


    if( strategy->reqAndConfigureFirstExperiment( appl->get_phase_region() ) ) {
        current_experiment = new Experiment( appl );
        current_experiment->run();
        delete( current_experiment );
    }

    while( dp->getResults() != ALL_INFO_GATHERED || strategy->evaluateAndReqNextExperiment() ) {
        current_experiment = new Experiment( appl );
        dp->stop_on_start_region( cur_phase );
        strategy->configureNextExperiment();
        current_experiment->run();
        delete( current_experiment );
    }

    evaluated_properties_set = foundProperties;

    if( psc_get_debug_level() >= 2 ) {
        print_property_set( evaluated_properties_set, "SET OF FOUND PROPERTIES", true, true );
    }

    return;
}

void AnalysisAgent::start_experiment() {
    current_experiment = new Experiment( appl );
    current_experiment->begin();

    started = true;
    return;
}

/**
 * @brief This function is called at the end of experiment and is intended to decide whether to request a new experiment or a restart
 */
void AnalysisAgent::appropriate_request() {
    /**
     * Here is decided whether application has to be restarted for the new experiment or not.
     */
    if( !dp->test_need_restart() ) {
        psc_dbgmsg( 6, "requesting next experiment\n" );
        parent_handler_->reqexperiment( this->get_local_tag() );
    }
    else {
        psc_dbgmsg( 6, "requesting restart\n" );
        parent_handler_->needrestart( this->get_local_tag() );
    }
}


bool AnalysisAgent::evaluate_experiment() {
    psc_dbgmsg( 5, "in evaluate experiment\n" );
    if( !started || dp->still_executing() ) {
        return false;
    }

    started = false;
    delete( current_experiment );
    current_experiment = NULL;
    if( appl->get_phase_region() != appl->get_main_region() &&
        ( dp->no_wait() == APP_SUSPENDED_AT_END || dp->no_wait() == APP_TERMINATED ) ) {
        psc_dbgmsg( 6, "in evaluate experiment: requesting restart\n" );
        parent_handler_->needrestart( this->get_local_tag() );
        return true;
    }
    psc_dbgmsg( 6, "in evaluate experiment: done\n" );

    /*
     * Ask DataProvider to receive measurements
     */
    Gather_Required_Info_Type results_status = dp->getResults();

    /*
     * If the applications has not terminated and not at the exit point
     * request it to run to the beginning of the phase region.
     */
    if( !dp->test_need_restart() ) {
        dp->stop_on_start_region( appl->get_phase_region() );
        dp->wait();
    }

    // NOTE: evaluateAndReqNextExperiment() should only be called, if ALL_INFO_GATHERED!
    if( results_status != ALL_INFO_GATHERED || strategy->evaluateAndReqNextExperiment() ) {
        /*
         * if strategy returned true for evaluate experiment, then request new experiment or
         * request restart depending on the application state.
         */
        appropriate_request();
    }
    else {
        /*
         * If analysis strategy returned false for evaluate experiment, it means that search is finished,
         * send the corresponding message upwards
         */

        psc_dbgmsg( 6, "AnalysisAgent::evaluate_experiment: Search finished\n" );
        parent_handler_->searchfinished( this->get_local_tag() );
    }

    return true; //experiment stopped
}


std::list< Property* > AnalysisAgent::get_results() {
    return evaluated_properties_set;
}


RegistryService* AnalysisAgent::get_registry() {
    return regsrv_;
}


int AnalysisAgent::register_self() {
    if( regsrv_ == 0 ) {
        psc_errmsg( "register_self(): registry not set\n" );
        return -1;
    }

    int pid, port;
    pid  = getpid();
    port = get_local_port();

    char hostname[ 200 ];
    gethostname( hostname, 200 );

    EntryData data;
    data.id   = -1;
    data.app  = appname();
    data.site = sitename();
    data.mach = machinename();
    data.node = hostname;
    data.port = port;
    data.pid  = pid;
    data.comp = "PeriscopeNode-Agent";
    data.tag  = get_local_tag();

    regid_ = regsrv_->add_entry( data );

    return regid_;
}


ACCL_Handler* AnalysisAgent::create_protocol_handler( ACE_SOCK_Stream& peer ) {
    return new ACCL_MRINodeagent_Handler( this, peer );
}


void AnalysisAgent::set_ready() {
}


void AnalysisAgent::on_search_finished() {
    if( !doneappexit ) {
        doneappexit = true;
        psc_dbgmsg( 5, "on_appl_finished\n" );

        parent_handler_->searchfinished( own_info_.tag );
        started     = false;
        doneappexit = false;
    }
}


void AnalysisAgent::run() {
    psc_dbgmsg( 7, "AnalysisAgent::run(): sending heartbeat and run event loop\n" );
    if( iterations_per_experiment != 1 )
        dp->set_num_iterations_per_phase( iterations_per_experiment );

    if( parent_handler_ ) {
        if( !get_fastmode() ) {       // the heartbeats are sent in another location on fast mode (allows the application to start afterwards)
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ), "sending a  heart-beat in regular mode...\n" );
            parent_handler_->heartbeat( own_info_.hostname, own_info_.port, own_info_.tag,
                                        OWN_HEARTBEAT, dp->get_total_num_processes() );
#ifdef WITH_MRI_OVER_ACE
            startup_mode_on();
#endif
        }
    }
    else {
        psc_errmsg( " Parent handler not set at AnalysisAgent::run()\n" );
    }

    if( get_fastmode() ) {  // fast timer-less ACE comm.
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Timer-less ACE comm. in the AA \n" );
        search_done = false;
        while( !search_done ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "handling a communication step (no timers) in the analysis agent...\n" );
            fflush( 0 );
            handle_step();
            reactor_->handle_events();
        }
    }
    else {       // timer-based ACE comm.
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ), "AA agent running WITH timers\n" );
        int runret = reactor_->run_reactor_event_loop();
    }
}

int AnalysisAgent::handle_step() {
#ifdef WITH_MRI_OVER_ACE
    if( get_startup_mode() ) {
        if( get_parent_handler() ) {
            int new_procs = dp->get_new_appl_procs();
            if( new_procs > 0 ) {
                get_parent_handler()->heartbeat( get_own_info().hostname,
                                                 get_own_info().port,
                                                 get_own_info().tag,
                                                 FORWARDED_HEARTBEAT,
                                                 new_procs );
            }
        }
        else {
            psc_errmsg( " Parent handler not set on AnalysisAgent::handle_step()\n" );
        }
        return 0;
    }
#endif

    if( !get_started() ) {
        return 0;
    }

    // actual work is done in this routine
    if( evaluate_experiment() ) {
//    psc_dbgmsg( 5, "EventHandling::experiment finished\n" );
//    reactor()->cancel_timer( this );
//    psc_dbgmsg( 5, "EventHandling::finished!\n" );
    }
    else {
        // continue to wait for application to stop
//    psc_dbgmsg(5, "EventHandling::not finished...\n" );
    }
    return 0;
}

void AnalysisAgent::reinit_providers( int maplen,
                                      int idmap_from[ 8192 ],
                                      int idmap_to[ 8192 ] ) {
    dp->reinit( get_registry(), maplen, idmap_from, idmap_to );
    if( parent_handler_ ) {
        psc_dbgmsg( 5, "[reinit] Sending heartbeat: %s %d %s\n",
                    own_info_.hostname.c_str(),
                    own_info_.port,
                    own_info_.tag.c_str() );

        parent_handler_->heartbeat( own_info_.hostname,
                                    own_info_.port,
                                    own_info_.tag, OWN_HEARTBEAT, dp->get_total_num_processes() );
    }
    else {
        psc_errmsg( "NA:Parent handler not set\n" );
    }
}

void AnalysisAgent::export_property_set( char* fileName ) {
    Prop_List::iterator property;
    std::ofstream       xmlfile( fileName );

    if( xmlfile ) {
        psc_dbgmsg( 5, "Exporting results to %s\n", fileName );

        time_t rawtime;
        tm*    timeinfo;
        time( &rawtime );
        timeinfo = localtime( &rawtime );


        xmlfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
        xmlfile << "<" << XML_PSC_EXPERIMENT_TAG
                << " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\""
                << XML_PSC_NAMESPACE << "\" xsi:schemaLocation=\""
                << XML_PSC_SCHEMA_LOC << "\">" << std::endl << std::endl;

        xmlfile << std::setfill( '0' );
        xmlfile << "  <" << XML_PSC_DATE_TAG << ">" << std::setw( 4 )
                << ( timeinfo->tm_year + 1900 ) << "-" << std::setw( 2 )
                << timeinfo->tm_mon + 1 << "-" << std::setw( 2 )
                << timeinfo->tm_mday << "</" << XML_PSC_DATE_TAG
                << ">" << std::endl;
        xmlfile << "  <" << XML_PSC_TIME_TAG << ">" << std::setw( 2 )
                << timeinfo->tm_hour << ":" << std::setw( 2 )
                << timeinfo->tm_min << ":" << std::setw( 2 ) << timeinfo->tm_sec
                << "</" << XML_PSC_TIME_TAG << ">" << std::endl;


        // Wording directory for the experiment: can be used to find the data file(s)
        char* cwd = getenv( "PWD" );
        xmlfile << "  <" << XML_PSC_DIR_TAG << ">" << cwd << "</" << XML_PSC_DIR_TAG << ">" << std::endl;
        // Source revision
        if( opts.has_srcrev ) {
            // Source revision
            xmlfile << "  <" << XML_PSC_REVISION_TAG << ">" << opts.srcrev << "</" << XML_PSC_REVISION_TAG << ">" << std::endl;
        }
        // Total number of properties that will be stored in the file
        xmlfile << "  <" << XML_PSC_NUM_PROPS_TAG << ">" << evaluated_properties_set.size()
                << "</" << XML_PSC_NUM_PROPS_TAG << ">" << std::endl;
        // Empty line before the properties
        xmlfile << std::endl;

        for( const auto& property : evaluated_properties_set ) {
            xmlfile << property->toXML();
        }

        xmlfile << "</" << XML_PSC_EXPERIMENT_TAG << ">" << std::endl;
    }
    xmlfile.close();
}


void AnalysisAgent::print_property( Property* p,
                                    bool      with_severity,
                                    bool      with_add_info ) {
    Context* ct = p->get_context();
    std::cout << "P" << ct->getRank() << "; \t" << ct->getThread() << "; \t";

    if (ct->isRtsBased()){
       std::cout << ct->getCallpath() << "; \t";
    } else {
    if( ct->getRegionType() == SUB_REGION ) {
        std::cout << std::setw( 12 ) << ct->getRegion()->get_name() << "; \t";
    }
    else if( ct->getRegionType() == CALL_REGION ) {
        std::cout << "Call " << std::setw( 12 ) << ct->getRegion()->get_name() << "; \t";
    }
    else {
        std::cout << std::setw( 12 ) << region_type_to_name( ct->getRegionType() ) << "; \t";
    }
    std::cout << std::setw( 10 ) << ct->getFileName() << ":" << ct->getStartPosition() << ";\t";
    }
    if( with_severity ) {
        std::cout << std::setprecision( 3 ) << std::setw( 8 ) << std::fixed << p->severity() << ";\t";
    }
    std::cout << p->name();
    if( psc_get_debug_level() >= 3 && with_add_info ) {
        std::cout << p->info();
    }
    std::cout << std::endl;
}

/**
 * Comparator for sorting properties according to the severity value.
 *
 * @return true if p1 should come before p2
 */
bool propSevComparator( Property* p1,
                        Property* p2 ) {
    return p1->severity() > p2->severity();
}


void AnalysisAgent::print_property_set( std::list <Property*> property_set,
                                        const char*           str,
                                        bool                  with_severity,
                                        bool                  with_add_info ) {
    std::list< Property* >::iterator property;
    if( get_leader() ) {
        std::cout << std::endl << "\n(" << psc_get_msg_prefix() << ") PROPERTIES in " << str << ":" << std::endl;
        std::cout << "---------------------------------------------------------------------\n";

        // with_severity is true for "SET OF FOUND PROPERTIES
        if( !with_severity ) {
            for( const auto& property : property_set ) {
                print_property( property, false, with_add_info );
            }
        }
        else {
            // Sort the properties set according to the severity
            property_set.sort( propSevComparator );
            // Print the sorted set
            for( const auto& property : property_set ) {
                print_property( property, true, with_add_info );
            }
        }
        std::cout << "----------------------------------------------------------\n";
        std::cout << std::endl;
    }
}

void AnalysisAgent::set_strategy( StrategyRequest* strategyRequest ) {
    //
    // Remove any previously-existing strategy.
    //
    if( get_strategy() ) {
        psc_dbgmsg( 5, "Deactivating strategy, cleaning up...\n" );
        delete get_strategy();
        pdb->clean();
    }

    Strategy*                        strategy            = NULL;
    std::list<Scenario*>*            scenarioList        = NULL;
    std::list<int>*                  propID_list         = NULL;
    std::list<TuningSpecification*>* preconfigureTS_list = NULL;

    const std::string strategyName      = strategyRequest->getGeneralInfo()->strategy_name;
    const bool        pedantic          = strategyRequest->getGeneralInfo()->pedantic;
    const int         analysis_duration = strategyRequest->getGeneralInfo()->analysis_duration;

    //
    // Processing different types of strategy requests
    //
    // TUNE - tuning strategy was requested
    // PERSYST - persyst strategy with a set of predefined properties (specified by prop. IDs) is requested
    // PRECONFIGURATION - TODO please, clarify here what this type of request is for
    // ANALYSIS - an analysis strategy was requested
    // CONFIG - TODO please, clarify here what this type of request is for
    //
    switch( strategyRequest->getTypeOfConfiguration() ) {
    case strategy_configuration_type( TUNE ):
        scenarioList = strategyRequest->getConfiguration().configuration_union.TuneScenario_list;
        break;
    case strategy_configuration_type( PERSYST ):
        propID_list = strategyRequest->getConfiguration().configuration_union.PersystPropertyID_list;
        break;
    case strategy_configuration_type( PRECONFIGURATION ):
        preconfigureTS_list = strategyRequest->getConfiguration().configuration_union.TuningSpecification_list;
        break;
    case strategy_configuration_type( ANALYSIS ):
        break;
    case strategy_configuration_type( CONFIG ):
        break;
    default:
        psc_abort( "Error: Strategy request configuration type is in undefined state." );
    }

    //
    // Matching strategy name to the strategy class
    //
    if( strategyName == "MPI" ) {
        strategy = new ScorepMPIStrategy( pedantic );
    }
    else if( strategyName == "OMP" ) {
        strategy = new OpenMPAnalysisScoreP( pedantic );
    }
    else if( strategyName == "Importance" ) {
        strategy = new Importance( pedantic );
    }
    else if( strategyName == "ConfigAnalysis" ) {
        strategy = new ConfigAnalysis( strategyRequest->getConfiguration().configuration_union.PropertyRequest_list, pedantic );
    }
    else if( strategyName == "EnergyGranularityBF" ) {
        if( !preconfigureTS_list || preconfigureTS_list->size() != 1 ) {
            psc_abort( "The tuning specification list must contain exactly one specification." );
        }

        strategy = new EnergyGranularityBF( preconfigureTS_list->front(), pedantic );
    }
    else if( strategyName == "Autotune" ) {

        if( scenarioList->empty() )
        {
            if( !scenarioList->front()->isRtsBased() && !scenarioList->front()->getRegion() ) {
                psc_abort( "Scenario in strategy request has no tuning region. Terminating.\n" );
            }
            else if( scenarioList->front()->isRtsBased() && !scenarioList->front()->getRts() )
            {
                psc_abort( "Scenario in strategy request has no tuning region. Terminating.\n" );
            }
        }
        else
        {
            psc_dbgmsg( 1, "Analysis Agent: Setting strategy as TuningStrategy in set_strategy(). \n" );

            if( scenarioList->front()->isRtsBased()  )
            {
                std::string entityID = scenarioList->front()->getRts()->getCallPath();

                if ( entityID.empty() )
                    psc_abort( "Analysis Agent: NO Phase rts callpath." );

                strategy = new TuningStrategy( scenarioList, strategyRequest->getSubStrategyRequest(),
                                               true, pedantic );
                psc_dbgmsg( 1, "Analysis Agent: rts based TuningStrategy is created. \n" );
            }

            else
            {
                strategy = new TuningStrategy( scenarioList, strategyRequest->getSubStrategyRequest(),
                                               false, pedantic );
                psc_dbgmsg( 1, "Analysis Agent: Region based TuningStrategy is created. \n" );

            }


        }


    }
#ifdef HAVE_TDA
    else if( strategyName == "PerfDyn-MPI" ) {
        //TODO: add duration to the strategy request
        strategy = new PerfDynamicsStrategy( "MPI", appl, analysis_duration, pedantic );
    }
    else if( strategyName == "PerfDyn-OMP" ) {
        strategy = new PerfDynamicsStrategy( "OMP", appl, analysis_duration, pedantic );
    }
    else if( strategyName == "PerfDyn-Time" ) {
        strategy = new PerfDynamicsStrategy( "Importance", appl, analysis_duration, pedantic );
    }
#endif

    else {
        psc_errmsg( "Error: An invalid strategy has been specified (%s)! Aborting.", strategyName.c_str() );
        abort();
    }

    psc_dbgmsg( 1, "Analyzing application with strategy %s (pedantic: %s)\n", strategy->name().c_str(), pedantic ? "yes" : "no" );
    agent->set_strategy( strategy );


    /**
     * All strategies were disabled during the integration of the Score-P support.
     *
     * Before being re-enabled the strategies have to be adapted to the new measurement
     * request and retrieval mechanism of the DataProvider
     */

//    if ( strategy == NULL ) {
//        psc_infomsg( "Warning: A valid strategy (%s) was not chosen, default strategy will be applied\n",
//                     strategyName.c_str() );
//        strategy = new MPIStrategy( pedantic );
//        strategy->set_max_strategy_steps( 40 );
//    }
//    psc_dbgmsg( 1, "Analyzing application with strategy %s (pedantic: %s)\n",
//                strategy->name().c_str(), pedantic ? "yes" : "no" );
//    agent->set_strategy( strategy );
//
//    if ( strategyName == "MPI" ) {
//        strategy = new MPIStrategy( pedantic );
//        strategy->set_max_strategy_steps( 40 );
//    }
//    if ( strategyName == "scalability_MPI" ) {
//        psc_dbgmsg( 1, "Analyzing application for scalability issues\n" );
//        strategy = new MPIStrategy( pedantic );
//        strategy->set_max_strategy_steps( 40 );
//    }
//    if ( strategyName == "OMP" ) {
//        psc_dbgmsg( 1, "Analyzing application for OpenMP-based issues\n" );
//        strategy = new OpenMPAnalysis( pedantic );
//        strategy->set_max_strategy_steps( 40 );
//    }
//    if ( strategyName == "scalability_OMP" ) {
//        psc_dbgmsg( 1, "Analyzing application for scalability issues \n" );
//        strategy = new OpenMPAnalysis( pedantic );
//        strategy->set_max_strategy_steps( 40 );
//    }
////  if ( ( strategyName == "RegionNestingStrategy" ) ||
////       ( strategyName == "RN" ) ) {
////      strategy = new RegionNestingStrategy();
////      strategy->set_max_strategy_steps( 40 );
////  }
//
//    if ( strategyName == "SCPS_BF" ) {
//#ifdef _WITH_CROSS_PLATFORM_PROPS
//        strategy = new SCP_StrategyBF( pedantic );
//        strategy->set_max_strategy_steps( 100 );
//#else
//        psc_infomsg( "Warning: %s strategy requested but not enabled during compile time!\n",
//                     strategyName.c_str() );
//#endif /* _WITH_CROSS_PLATFORM_PROPS */
//    }
//
//    if ( strategyName == "BGP_Cache" ) {
//#ifdef _WITH_BGP_STRATEGY
//        strategy = new BGPStrategyDF( pedantic );
//        strategy->set_max_strategy_steps( 100 );
//#else
//        psc_infomsg( "Warning: %s strategy requested but not enabled during compile time!\n",
//                     strategyName.c_str() );
//#endif /* _WITH_BGP_STRATEGY */
//    }
//
//    if ( strategyName == "OCL" ) {
//#ifdef _WITH_OCL
//        strategy = new OCLStrategy( pedantic );
//        strategy->set_max_strategy_steps( 140 );
//#else
//        psc_infomsg( "Warning: %s strategy requested but not enabled during compile time!\n",
//                     strategyName.c_str() );
//#endif /* _WITH_OCL */
//    }
//
//    if ( strategyName == "OpenCL" ) {
//#ifdef _WITH_OCL
//        strategy = new OpenCLStrategy( pedantic );
//        strategy->set_max_strategy_steps( 140 );
//#else
//        psc_infomsg( "Warning: %s strategy requested but not enabled during compile time!\n",
//                     strategyName.c_str() );
//#endif /* _WITH_OCL */
//    }
//    if ( strategyName == "GPU" ) {
//#ifdef _WITH_CUPTI
//        strategy = new GPUTestingStrategy( pedantic );
//        strategy->set_max_strategy_steps( 140 );
//#else
//        psc_infomsg( "Warning: %s strategy requested but not enabled during compile time!\n",
//                     strategyName.c_str() );
//#endif /* _WITH_CUPTI */
//    }
//
//    if ( strategyName == "Energy" )
//    {
//        strategy = new EnergyStrategy( pedantic );
//        strategy->set_max_strategy_steps( 140 );
//        psc_infomsg( "Warning: %s strategy requested but not enabled during compile time!\n",
//                     strategyName.c_str() );
//#endif /* _WITH_ENOPT */
//    }
//
//    if ( strategyName == "Pipeline" ) {
//#ifdef _WITH_VPATTERN
//        strategy = new PipelineStrategy( pedantic );
//        strategy->set_max_strategy_steps( 140 );
//#else
//        psc_infomsg( "Warning: %s strategy requested but not enabled during compile time!\n",
//                     strategyName.c_str() );
//#endif /* _WITH_VPATTERN */
//    }
//
//    if ( strategyName == "EnergyGranularity" )
//    {
//        strategy = new EnergyGranularity( pedantic, preconfigureTS_list );
//        strategy->set_max_strategy_steps( 40 );
//    }
//    if ( strategy == NULL )
//    {
//        psc_infomsg( "Warning: A valid strategy (%s) was not chosen , default strategy will be applied\n",
//                strategyName.c_str() );
//        strategy = new MPIStrategy( pedantic );
//        strategy->set_max_strategy_steps( 40 );
//    }
}

void AnalysisAgent::compareResults( Prop_List newResults,
                                    Prop_List prevResults,
                                    int       run ) {
    Prop_List additionalProperties;
    for( const auto& new_prop : newResults ) {
        bool found = false;
        for( const auto& prev_prop : prevResults ) {
            if( new_prop->get_region() == prev_prop->get_region() &&
                new_prop->get_rank() == prev_prop->get_rank() &&
                new_prop->get_thread() == prev_prop->get_thread() &&
                new_prop->name() == prev_prop->name() ) {
                found = true;
                break;
            }
        }
        if( !found ) {
            additionalProperties.push_back( new_prop );
        }
    }

    Prop_List missingProperties;
    for( const auto& prev_prop : prevResults ) {
        bool found = false;
        for( const auto& new_prop : newResults ) {
            if( prev_prop->get_region() == new_prop->get_region() &&
                prev_prop->get_rank() == new_prop->get_rank() &&
                prev_prop->get_thread() == new_prop->get_thread() &&
                prev_prop->name() == new_prop->name() ) {
                found = true;
                break;
            }
        }
        if( !found ) {
            missingProperties.push_back( prev_prop );
        }
    }

    char str[ 2000 ];
    sprintf( str, "SET OF MISSING PROPERTIES Search %d", run );
    if( !missingProperties.empty() ) {
        print_property_set( missingProperties, str, true, true );
    }

    sprintf( str, "SET OF ADDITIONAL PROPERTIES Search %d", run );
    if( !additionalProperties.empty() ) {
        print_property_set( additionalProperties, str, true, true );
    }
}
