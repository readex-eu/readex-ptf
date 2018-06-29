/**
   @file    accl_mrinodeagent_handler.cc
   @ingroup Communication
   @brief   Analysis agent communication layer
   @author  Karl Fuerlinger
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

#include "accl_mrinodeagent_handler.h"
#include <string>
#include <list>
#include <poll.h>

// Boost serialization
#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/serialization/vector.hpp>

#include "global.h"
#include "Property.h"
#include "application.h"
#include "analysisagent.h"
#include "selective_debug.h"
#include "StrategyRequest.h"
#include "strategy.h"



/**
 * @brief Handle a start request
 *
 * When a start request is received, the monitored application and
 * the communication between the agents are started.
 */
int ACCL_MRINodeagent_Handler::on_start( start_req_t& req, start_reply_t& reply ) {
    //psc_dbgmsg(5, "ACCL_MRINodeagent_Handler::on_start(%d) \n",
    //           req.serialized_strategy_request_container.size);
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_MRINodeagent_Handler::on_start(%d)\n",
                req.serialized_strategy_request_container.size );
    /// Set the agent state to ready.
    agent_->set_ready();

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ),
                "Serialized strategy request received on AA side: strategy request size %d\n",
                req.serialized_strategy_request_container.size );

    if( req.serialized_strategy_request_container.size != 0 ) {
        namespace io = boost::iostreams;
        io::basic_array_source<char> source( ( char* )req.serialized_strategy_request_container.payload,
                                             req.serialized_strategy_request_container.size );
        io::stream < io::basic_array_source<char> > input_stream( source );

        boost::archive::binary_iarchive ia( input_stream );

        StrategyRequest*            strategyRequest;
        StrategyRequestGeneralInfo* srgi;

        ia >> strategyRequest;
        free( req.serialized_strategy_request_container.payload );
        /*
         * TODO: solve the issue below:
         * Region Serialization Issue:
         * The de-serialized strategy request contains a de-serialized list of regions that were sent from the FE. These instances
         * differ from the corresponding region instances stored in the Application of the AA. Therefore, it is not safe to use the received regions in the AA!
         *
         * Use Application::substitueRemoteInstanceWithLocal and Application::substitueRemoteInstanceWithLocalInTheList methods to substitute the received instances with the local one
         *
         */
        if( !strategyRequest ) {
            psc_errmsg( "StrategyRequest is NULL!" );
        }

        srgi = strategyRequest->getGeneralInfo();
        StrategyRequest* analysisRequest = strategyRequest->getSubStrategyRequest();

        if( !analysisRequest ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ),
                        "AA: %s strategy with analysis duration = %d, %d delay phases and %d delay seconds\n",
                        srgi->strategy_name.c_str(), srgi->analysis_duration, srgi->delay_phases,
                        srgi->delay_seconds );
        }
        else {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ),
                        "AA: %s strategy with %s analysis strategy, analysis duration = %d, %d delay phases and %d delay seconds\n",
                        srgi->strategy_name.c_str(),
                        analysisRequest->getGeneralInfo()->strategy_name.c_str(), srgi->analysis_duration,
                        srgi->delay_phases,
                        srgi->delay_seconds );
        }

        // set the strategy
        agent_->set_strategy( strategyRequest );

        if( srgi->delay_phases != 0 ) {
            // analysis delay of srgi->delay_phases is requested. Skip first srgi->delay_phases executions of phase
            agent_->set_delay( srgi->delay_phases );
            for( int i = 0; i < srgi->delay_phases; i++ ) {
                if( agent->get_leader() ) {
                    psc_dbgmsg( 1, "Strategy delayed: %d. execution of phase.\n", i + 1 );
                }
                dp->stop_on_start_region( appl->get_phase_region() );
                dp->wait();
            }
        }

        // check whether the application is at the exit. request restart or cancel the search if the delay could not be met.
        if( dp->test_need_restart() ) {
            if( srgi->delay_phases != 0 || srgi->delay_seconds != 0 ) {
                psc_dbgmsg( 5, "Analysis delay was requested but the application quited earlier... aborting search!\n" );
                ( agent_->get_parent_handler() )->searchfinished( agent_->get_local_tag() );

                delete strategyRequest;

                return 0;
            }
            ( agent_->get_parent_handler() )->needrestart( agent_->get_local_tag() );

            delete strategyRequest;

            return 0;
        }

        delete strategyRequest;

        ///Create initial properties set



        /**
         * If instrumentation strategy not used, Analysis strategy is called to create initial candidate
         * properties and submit appropriate requests
         */
        //psc_dbgmsg(5, "Using analysis strategy for the first run\n");
        Region*   phaseReg = appl->get_phase_region();
        Strategy* strat    = agent_->get_strategy();

        //The strategy to configure its candidate properties
        agent_->get_strategy()->reqAndConfigureFirstExperiment( phaseReg );

        //Here the strategy is supposed to call the DataProvider to transfer requests to processes
        agent_->get_strategy()->configureNextExperiment();


        agent_->start_experiment();

        psc_dbgmsg( 5, "ACCL_MRINodeagent_Handler::on_start() done!\n" );
        return 0;
    }
    else {
        psc_errmsg( "Received empty strategy request container!\n" );
    }
}

/**
 * @brief Handle a startexperiment request
 *
 * When a start request is received, the monitored application and
 * the communication between the agents are started.
 */
int ACCL_MRINodeagent_Handler::on_startexperiment( startexperiment_req_t& req, startexperiment_reply_t& reply ) {
    std::list <ApplProcess>           controlled_processes = dp->get_controlled_processes();
    std::list <ApplProcess>::iterator process;
    char                              str[ 2000 ];


    //psc_dbgmsg( 5, "ACCL_MRINodeagent_Handler::on_startexperiment()\n" );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_MRINodeagent_Handler::on_startexperiment()\n" );

    if( dp->test_need_restart() ) {
        ( agent_->get_parent_handler() )->needrestart( agent_->get_local_tag() );
        return 0;
    }

    agent_->get_strategy()->configureNextExperiment();

    agent_->start_experiment();

    return 0;
}

/**
 * @brief Handle a terminate request
 *
 * When a start request is received, the monitored application and
 * the communication between the agents are started.
 */
int ACCL_MRINodeagent_Handler::on_terminate( terminate_req_t& req, terminate_reply_t& reply ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_MRINodeagent_Handler::on_terminate()\n" );
    dp->app_finish();

    if( !agent->get_fastmode() ) {
        sleep( 10 );
    }

    ( agent_->get_parent_handler() )->terminated( agent_->get_local_tag() );

    return 0;
}



/**
 * @brief Handle a quit request
 *
 * When a quit request is received, the monitored application and
 * the communication between the agents are terminated.
 */
int ACCL_MRINodeagent_Handler::on_quit( quit_req_t& req, quit_reply_t& reply ) {
    //psc_dbgmsg(5, "ACCL_MRINodeagent_Handler::on_quit()\n");
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_MRINodeagent_Handler::on_quit()\n" );

    /// Stop the application and the experiment.
#ifdef _BGP_PORT
    dp->app_finish();
#endif
    /// Stop the communication routine.
    agent_->stop();


    psc_dbgmsg( 5, "ACCL_MRINodeagent_Handler::on_quit() done\n" );

    return ACCL_Handler::on_quit( req, reply );
}

/**
 * @brief Handle a check request
 *
 * Convert each property to string, send it to the master agent,
 * and delete it from the local list.
 */
int ACCL_MRINodeagent_Handler::on_check( check_req_t& req, check_reply_t& reply ) {
    //psc_dbgmsg( 5, "on_check()\n" );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_MRINodeagent_Handler::on_check()\n" );
    Prop_List           resultsanalys;
    Prop_List::iterator pi;

    /**
     * Ask Analysis strategy for the found properties and send them hierarchy-upwards.
     * Clear list of found properties afterwards.
     */
    resultsanalys = foundProperties;
    psc_dbgmsg( 3, "AA: sending %d properties to HL from analysis strategy\n", resultsanalys.size() );


    // Marking of properties with ScenarioId
    std::list<ApplProcess>               controlled_processes = dp->get_controlled_processes();
    std::list<ApplProcess>::iterator     process;
    std::list<Scenario*>::const_iterator scenario;
    int                                  scenarioNum = 0;
    int                                  rank;

    for( pi = resultsanalys.begin(); pi != resultsanalys.end(); pi++ ) {
        rank = ( *pi )->get_rank();
        for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
            if( rank == process->rank ) {
                break;
            }
        }
        if( ( *pi )->get_Purpose() == PSC_PROPERTY_PURPOSE_ANALYSIS ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "Per-experiment property, all scenario ids for experiment are added to a property.\n" );

            for( scenario = process->getScenariosPerPropertyRequest()->begin(), scenarioNum = 0;
                 scenario != process->getScenariosPerPropertyRequest()->end(); scenario++, scenarioNum++ ) {
                ( *pi )->add_ScenarioId( ( *scenario )->getID() );
            }
        }
        else if( ( *pi )->get_Purpose() != PSC_PROPERTY_PURPOSE_TUNING ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ), "Warning: Property purpose not marked\n" );
        }
    }

    for( pi = resultsanalys.begin(); pi != resultsanalys.end(); pi++ ) {
        std::string propData = ( *pi )->toXMLSerialize();
        //psc_dbgmsg(5, "Sending %s\n",propData.c_str());
        ( agent_->get_parent_handler() )->foundprop( propData );
    }

    clear_found_properties();

    //RM: Addition related to deserialization
/******************************************************************************/
//  Prop_List* pl;
//  foundProperties = agent_->deserializePropertyList();

//  clear_found_properties();
/******************************************************************************/


    /**
     * Send message, that all properties are submitted
     */
    ( agent_->get_parent_handler() )->propertiessent( agent_->get_local_tag() );
    return 0;
}


/**
 * @brief Handle a serialize call-tree request
 *
 * Convert each call-tree node to XML string and send it to the master agent
 */
int ACCL_MRINodeagent_Handler::on_serializecalltree( calltreeserial_req_t&   req,
                                                     calltreeserial_reply_t& reply ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_MRINodeagent_Handler::on_serializecalltree()\n" );

    //psc_dbgmsg( 3, "AA: sending call-tree node to HL\n" );

    //Serializing the call-tree to XML
    std::vector<Rts*> rts_list = rtstree::get_rts_to_serialize();
    std::vector<Rts*>::iterator rts_list_it;

    for( rts_list_it = rts_list.begin(); rts_list_it != rts_list.end(); rts_list_it++ ) {
        std::string calltreeData = ( *rts_list_it )->toXMLdata();
        //Send the call-tree node upwards in the hierarchy
        ( agent_->get_parent_handler() )->sendcalltree( calltreeData );
    }

    //Send message that the call-tree is sent by one/more aagents

    ( agent_->get_parent_handler() )->calltreesent( agent_->get_local_tag() );

    //Now, we don't need the aagent call-tree anymore
    //rtstree::delete_rts_tree();
    rtstree::clear_rts_to_serialize();
    return 0;
}



/**
 * @brief Handle a setparent request
 *
 * Convert each property to string, send it to the master agent,
 * and delete it from the local list.
 */
int ACCL_MRINodeagent_Handler::on_setparent( setparent_req_t& req, setparent_reply_t& reply ) {
    //psc_dbgmsg( 5, "ACCL_MRINodeagent_Handler::on_setparent()\n" );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_MRINodeagent_Handler::on_setparent()\n" );
    //agent_->connect_to_parent( req.hostname, req.port );
    return 0;
}

/**
 * @brief Handle a reinit request
 *
 *
 */
int ACCL_MRINodeagent_Handler::on_reinit( reinit_req_t& req, reinit_reply_t& reply ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ACECommunication ), "ACCL_MRINodeagent_Handler::on_reinit, rsize %d\n", req.maplen );

    int mapfrom[ 8192 ];
    int mapto[ 8192 ];

    dp->app_finish();

    for( int i = 0; i < req.maplen; i++ ) {
        mapfrom[ i ] = req.idmap[ i ].idf;
        mapto[ i ]   = req.idmap[ i ].idt;
    }

    psc_init_start_time();
    agent_->reinit_providers( req.maplen, mapfrom, mapto );



    //if analysis is delayed, we skip the first iterations MG
    if( agent->get_delay() ) {
        for( int i = 0; i < agent->get_delay(); i++ ) {
            if( agent->get_leader() ) {
                psc_dbgmsg( 1, "Strategy delayed: %d. execution of phase.\n", i + 1 );
            }
            dp->stop_on_start_region( appl->get_phase_region() );
            dp->wait();
        }
    }


    return 0;
}
