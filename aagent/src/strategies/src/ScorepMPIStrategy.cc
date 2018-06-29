/**
   @file    ScorepMPIStrategy.cc
   @ingroup ScorepMPIStrategy
   @brief   ScoreP MPI search strategy
   @author  Michael Ott
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
#include "global.h"
#include "ScorepMPIStrategy.h"
#include "MpiLate.h"
#include "MPIexcessive.h"
#include "application.h"
#include "psc_errmsg.h"
#include "DataProvider.h"
#include <analysisagent.h>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

ScorepMPIStrategy::~ScorepMPIStrategy() {
    clear_found_properties();
    pdb->clean();
}

Metric ScorepMPIStrategy::interpret_MPI_LATE_SEND_OfRegion( std::string region_name ) {
    //psc_dbgmsg( 6, "TRANSLATING MPI METRIC of the region %s\n", region_name.c_str() );
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

bool ScorepMPIStrategy::reqAndConfigureFirstExperiment( Region* r ) {
    // TRUE can start; FALSE not ready
    psc_dbgmsg( 1, "SCOREP MPI Strategy is initiated\n", strategy_steps );

    /* Store phase region. Even though it is supposed to be NULL, since it is not known beforehand */
    phaseRegion = r;

    /* Add global request for EXECUTION_TIME */
    dp->addMeasurementRequest( NULL, PSC_EXECUTION_TIME );
    /* Add global request for EXECUTION_TIME */
    dp->addMeasurementRequest( NULL, PSC_MPI );
    /* Subscribe for metric group GROUP_OMP */
    dp->metricGroupSubscribe( this, GROUP_MPI );
    return true;
}

void ScorepMPIStrategy::metric_found_callback( Metric m, Context ct_in ) {
    /* Ask application for phase region. If not phase region registered analysis can not be done. */

    //psc_dbgmsg( 6, "Notification: m=%s, file=%d rfl=%d thread=%d region_type=%d\n", EventList[ m ].EventName, ct_in.getFileId(), ct_in.getRfl(), ct_in.getThread(), ct_in.getRegionType() );

    if( phaseRegion == NULL ) {
        return;
    }

    /* Only thread 0 is of interest */
    if( ct_in.getThread() != 0 ) {
        return;
    }

    /* Create the contexts */
    Context* pCt = new Context( phaseRegion, ct_in.getRank(), 0 );
    Context* ct  = new Context( ct_in.getRegion(), ct_in.getRank(), 0 );

    Property* candidate = NULL;

    switch( m ) {
    case    PSC_MPI_EARLY_RECV:
    case    PSC_MPI_EARLY_BCAST:
    case    PSC_MPI_EARLY_SCATTER:
    case    PSC_MPI_LATE_GATHER:
    case    PSC_MPI_LATE_REDUCE:
    case    PSC_MPI_LATE_ALLREDUCE:
    case    PSC_MPI_LATE_ALLGATHER:
    case    PSC_MPI_LATE_ALLTOALL:
    case    PSC_MPI_LATE_BARRIER:
    {
        candidate = new MpiLateProp( ct, pCt, m, pedanticSearch ? 0.0 : MPI_PROPS_THRESHOLD::MpiLateProp );
        //psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
        candProperties.push_back( candidate );

        break;
    }
    case    PSC_MPI_TIME_SPENT:
    {
        candidate = new MPIexcessiveProp( ct, pCt, interpret_MPI_LATE_SEND_OfRegion( ct_in.getRegion()->get_name() ),
                                          pedanticSearch ? 0.0 : MPI_PROPS_THRESHOLD::MPIexcessiveProp );
        //psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
        candProperties.push_back( candidate );
        //prop->print();
        break;
    }
    default:
        break;
    }
    if( candidate == NULL ) {
        /* No property was created, then contexts should be deleted */
        delete ct;
        delete pCt;
        //psc_dbgmsg( 6, "Property was NOT created!\n" );
    }
}

void ScorepMPIStrategy::configureNextExperiment() {
    psc_dbgmsg( 1, "Calling dp->transfer_requests_to_processes();\n", strategy_steps );
    dp->transfer_requests_to_processes();
}


bool ScorepMPIStrategy::evaluateAndReqNextExperiment() {
    /* Print candidate properties, instantiated following the notification from DataProvider */
    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( candProperties, "SET OF CANDIDATE PROPERTIES", false, false );
    }

    /* Properties instantiated following the notifications from DataProvider to be evaluated here */
    for( const auto& property : candProperties ) {
        property->evaluate();
        if( property->condition() ) {
            foundProperties.push_back( property );
        }
        else {
            delete property;
        }
    }

    /* Print found properties */
    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( foundProperties, "SET OF FOUND PROPERTIES", true, false );
    }

    /* Determine new candidate properties */
    candProperties.clear();

    /* This is a one-step strategy. Return false to terminate this analysis */
    return false;
}



std::list< Property* > ScorepMPIStrategy::create_initial_candidate_properties_set( Region* initial_region ) {
}

std::list< Property* > ScorepMPIStrategy::create_next_candidate_properties_set( std::list< Property* > ev_set ) {
    Prop_List n;
    return n; //empty
}

std::string ScorepMPIStrategy::name() {
    return "ScorepMPIStrategy";
}
