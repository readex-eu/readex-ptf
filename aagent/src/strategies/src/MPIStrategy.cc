/**
   @file    MPIStrategy.cc
   @ingroup MPIStrategy
   @brief   MPI search strategy
   @author  Michael Ott
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
#include "global.h"
#include "MPIStrategy.h"
#include "MpiLate.h"
#include "MPIexcessive.h"
#include "MpiMessageSizeProp.h"
#include "EagerLimitDependent.h"
#include <iostream>
#include "psc_errmsg.h"
#include <analysisagent.h>
#include "return_summary_data.h"
#ifdef __p575
#include <strings.h>
#endif

#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include "config.h"


MPIStrategy::~MPIStrategy() {
    clear_found_properties();
    pdb->clean();
}

void MPIStrategy::createCandidateProperties() {
    //1. Receive measurements
    std::list<ApplProcess>           controlled_processes = dp->get_controlled_processes();
    std::list<ApplProcess>::iterator process;
    SummaryDataType                  summary;
    int                              number = 4, length;
    char                             str[ 2000 ];
    Gather_Required_Info_Type        gather_info = ALL_INFO_GATHERED;

    psc_dbgmsg( 5, "Creating MPI candidate properties\n" );

    std::list<PerfEntry*>           MPIperfList = dp->getMPIperfList();
    std::list<PerfEntry*>::iterator entry;

    for( entry = MPIperfList.begin(); entry != MPIperfList.end(); entry++ ) {
        switch( ( *entry )->getMetric() ) {
        case PSC_MPI_MSG_P2P_THR:
            Context*             ct, *startCtx;
            EagerLimitDependent* prop;

            ct       = new Context( ( *entry )->getFileID(), ( *entry )->getRfl(), ( *entry )->getRank(), ( *entry )->getThread() );
            startCtx = new Context( phaseRegion, ( *entry )->getRank(), ( *entry )->getThread() );
            prop     = new EagerLimitDependent( ct, startCtx, ( *entry )->getMetric(),
                                                pedanticSearch ? 0.0 : MPI_PROPS_THRESHOLD::EagerLimitDependent );
            candProperties.push_back( prop );
            break;
        //case PSC_MPI_LATE_SEND:
        case PSC_MPI_EARLY_RECV:     ///< Excessive MPI time in receive due to late sender
        //case PSC_MPI_LATE_RECV: ///<@TODO decide on how to handle late_receive cases, currently commented out
        case PSC_MPI_EARLY_BCAST:
        //case PSC_MPI_LATE_BCAST:
        case PSC_MPI_EARLY_SCATTER:
        //case PSC_MPI_LATE_SCATTER:
        case PSC_MPI_LATE_GATHER:
        case PSC_MPI_LATE_REDUCE:
        case PSC_MPI_LATE_ALLREDUCE:
        case PSC_MPI_LATE_ALLGATHER:
        case PSC_MPI_LATE_ALLTOALL:
        case PSC_MPI_LATE_BARRIER:
        {
            Context*     ct, * startCtx;
            MpiLateProp* prop;

            ct       = new Context( ( *entry )->getFileID(), ( *entry )->getRfl(), ( *entry )->getRank(), ( *entry )->getThread() );
            startCtx = new Context( phaseRegion, ( *entry )->getRank(), ( *entry )->getThread() );
            prop     = new MpiLateProp( ct, startCtx, ( *entry )->getMetric(),
                                        pedanticSearch ? 0.0 : MPI_PROPS_THRESHOLD::MpiLateProp );
            candProperties.push_back( prop );
            break;
        }
        case PSC_MPI_AGGREGATE_MESSAGE_SIZE:
        {
            Context*            ct, * startCtx;
            MpiMessageSizeProp* msgSizeProp;

            ct       = new Context( ( *entry )->getFileID(), ( *entry )->getRfl(), ( *entry )->getRank(), ( *entry )->getThread() );
            startCtx = new Context( phaseRegion, ( *entry )->getRank(), ( *entry )->getThread() );

            msgSizeProp = new MpiMessageSizeProp( ct, startCtx,
                                                  pedanticSearch ? 0.0 : MPI_PROPS_THRESHOLD::MpiMessageSizeProp );
            candProperties.push_back( msgSizeProp );
            break;
        }
        case PSC_MPI_TIME_SPENT:
        {
            Context*          ct, * startCtx;
            MPIexcessiveProp* prop;

            ct       = new Context( ( *entry )->getFileID(), ( *entry )->getRfl(), ( *entry )->getRank(), ( *entry )->getThread() );
            startCtx = new Context( phaseRegion, ( *entry )->getRank(), ( *entry )->getThread() );
            prop     = new MPIexcessiveProp( ct, startCtx, PSC_UNDEFINED_METRIC,
                                             pedanticSearch ? 0.0 : MPI_PROPS_THRESHOLD::MPIexcessiveProp );
            candProperties.push_back( prop );
            break;
        }
        default:
            break;
        }
    }

    while( !MPIperfList.empty() ) {
        delete MPIperfList.front(), MPIperfList.pop_front();
    }
    MPIperfList.clear();
}

bool MPIStrategy::reqAndConfigureFirstExperiment( Region* r ) {
// TRUE can start; FALSE not ready
    psc_dbgmsg( 1, "MPI Strategy is initiated\n" );
    phaseRegion = r;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAll ), "on reqAndConfigureFirstExperiment:  phaseRegion address: %x\n", phaseRegion );

    if( agent->get_leader() ) {
        psc_dbgmsg( 1, "MPI Strategy Step %d\n", strategy_steps );
    }

    return true;
}

void MPIStrategy::configureNextExperiment() {
    std::list<Request*>::iterator    req;
    std::list<ApplProcess>::iterator process;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAll ), "on configureNextExperiment:  before the loop: phaseRegion address: %x\n", phaseRegion );
    std::list<ApplProcess> controlled_processes = dp->get_controlled_processes();
    psc_dbgmsg( 5, "Requesting MPI measurements...\n" );

    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        char str[ 2000 ];
        sprintf( str, "request[0] global MPI;\n" );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "requesting: %s\n", str );
        dp->write_line( &( *process ), str );
        dp->wait_for_ok( *process );

        for( int i = 0; i < appl->getOmpThreads(); i++ ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAll ), "on configureNextExperiment:  in the loop: phaseRegion address: %x\n", phaseRegion );
            pdb->erase( phaseRegion->get_ident().file_id, phaseRegion->get_ident().rfl, process->rank, i, PSC_PAPI_TOT_CYC );
            sprintf( str, "request[0] local (%d, %s, %d) = %d;\n", phaseRegion->get_ident().file_id,
                     dp->regionType2reqSpec( phaseRegion->get_type(), PSC_PAPI_TOT_CYC ).c_str(), phaseRegion->get_ident().rfl,
                     PSC_EXECUTION_TIME );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "requesting: %s\n", str );
            dp->write_line( &( *process ), str );
            dp->wait_for_ok( *process );
        }
    }
}

bool MPIStrategy::evaluateAndReqNextExperiment() {
    // TRUE requires next step; FALSE if done
    Prop_List::iterator prop_it;
    double              max_severity = 0;

    //Evaluate candidate properties
    psc_dbgmsg( 2, "%s: analyzing results...\n", name().c_str(), candProperties.size() );

    createCandidateProperties();
    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( candProperties, "SET OF CANDIDATE PROPERTIES", false, false );
    }

    foundPropertiesLastStep.clear();

    for( prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
        ( *prop_it )->evaluate();

        //compute maximum of the severities
        if( ( *prop_it )->severity() > max_severity ) {
            max_severity = ( *prop_it )->severity();
        }

        if( ( *prop_it )->condition() ) {
            foundPropertiesLastStep.push_back( *prop_it );
            foundProperties.push_back( *prop_it );
        }
    }
    psc_dbgmsg( 2, "%s: size of candidate properties %d\n", name().c_str(), candProperties.size() );
    psc_dbgmsg( 2, "%s: size of found list this step %d\n", name().c_str(), foundPropertiesLastStep.size() );
    psc_dbgmsg( 2, "%s: size of total found list %d\n", name().c_str(), foundProperties.size() );
    psc_dbgmsg( 2, "%s: maximum detected severity was %f\n", name().c_str(), max_severity );

    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( foundPropertiesLastStep, "SET OF FOUND PROPERTIES", true, true );
    }

    return false;
}

std::list<Property*> MPIStrategy::create_initial_candidate_properties_set( Region* initial_region ) {
    Prop_List n;
    return n; //empty
}

std::list<Property*> MPIStrategy::create_next_candidate_properties_set( std::list<Property*> ev_set ) {
    Prop_List n;
    return n; //empty
}

std::string MPIStrategy::name() {
    return "MPIStrategy";
}
