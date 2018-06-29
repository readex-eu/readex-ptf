/**
   @file    OpenMPAnalysisScoreP.cc
   @ingroup OpenMPAnalysisScorePStrategy
   @brief   OpenMP search strategy
   @author  Shajulin Benedict
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
#include "OpenMPAnalysisScoreP.h"
#include "application.h"
#include "Metric.h"
#include "strategy.h"
#include "PropertyID.h"
#include "psc_errmsg.h"
#include <iostream>
#include <string.h>
#include <analysisagent.h>

#include "ImbalanceOMPBarrier.h"
#include "CriticalRegionOverhead.h"
#include "SerializationCriticalRegion.h"
#include "SequentialInSingle.h"
#include "SequentialInMaster.h"
#include "StartupShutdownOverhead.h"
#include "SequentialInOrderedLoop.h"
#include "ImbalanceInOrderedLoop.h"
#include "ImbalanceInParSections.h"
#include "LoadImbalanceOMPRegion.h"
#include "HotRegionExecTime.h"
#include "OverheadDueToSmallTask.h"
#include "OverheadDueToTaskCreation.h"
#include "TooFineGranularTasks.h"
#include "EmptyTaskinTaskRegion.h"
#include "ImbalancedTaskRegion.h"
#include "HighInclOverheadProp.h"
#include "HighExclOverheadProp.h"


using namespace std;

bool OpenMPAnalysisScoreP::reqAndConfigureFirstExperiment( Region* r ) {
    /* Store phase region. Even though it is supposed to be NULL, since it is not known beforehand */
    phaseRegion = r;

    /* Add global request for EXECUTION_TIME */
    dp->addMeasurementRequest( NULL, PSC_EXECUTION_TIME );
    /* Subscribe for metric EXUCTION_TIME */
    dp->globalMetricSubscribe( this, PSC_EXECUTION_TIME );
    /* Subscribe for metric group GROUP_OMP */
    dp->metricGroupSubscribe( this, GROUP_OMP );


    return true;
}

void OpenMPAnalysisScoreP::metric_found_callback( Metric m, Context ct_in ) {
    /* Ask application for phase region. If not phase region registered analysis can not be done. */

    psc_dbgmsg( 6, "Notification: m=%s, file=%d rfl=%d thread=%d region_type=%d\n", EventList[ m ].EventName, ct_in.getFileId(), ct_in.getRfl(), ct_in.getThread(), ct_in.getRegionType() );

    if( phaseRegion == NULL ) {
        return;
    }

    /* Only thread 0 is of interest */
    if( ct_in.getThread() != 0 ) {
        return;
    }


    /* Filter out some metrics in order to prevent property duplication */
    if( m == PSC_TASKS_CREATED || m == PSC_TASKS_EXECUTED ) {
        return;
    }

    /* Create the phase context */
    Context*  pCt       = new Context( phaseRegion, ct_in.getRank(), 0 );
    Context*  ct        = new Context( ct_in.getRegion(), ct_in.getRank(), 0 );
    Property* candidate = NULL;

    /* Note: In ScoreP one OMP region may result in multiple region definitions. e.g. "omp for" will result in
     * "for" and "implicit_barrier" regions. Nevertheless they will have the same file name and rfl, but diferent
     * names. This will result in multiple Region objects stored in Application object. However, since the Performance
     * data base stores measurements using only file name and rfl, measurements of these multiple regions will be stored
     * under the same context.  */

    /* Here we instantiate candidate properties based on the metric type and the type of the region they were
     * measured for*/
    if( m == PSC_EXECUTION_TIME ) {
        switch( ct_in.getRegionType() ) {
        case USER_REGION:
            candidate = new HotRegionExecTimeProp( ct, pCt, USER_REGION_TYPE );
            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
            break;
        }
    }
    else if( EventList[ m ].EventGroup == GROUP_OMP ) {
        /* Else could be only an OpenMP metric since we subscribed only for execution time and OMP metric group */
        switch( ct_in.getRegionType() ) {
        case DO_REGION:
            /* This property is not supported, since ScoreP doesn't provide PSC_PARALLEL_REGION_BODY_CYCLE metric */
            //candidate = new StartupShutdownOverheadProp( ct, pCt );
            //candidates.push_back( candidate );
            candidate = new LoadImbalanceOMPRegionProp( ct, pCt, PARALLEL_LOOP, pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::LoadImbalanceOMPRegionProp );
            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
            break;
        case TASK_REGION:
            candidate = new OverheadDueToTaskCreationProp( ct, pCt, pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::OverheadDueToTaskCreationProp );
            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
            candidate = new OverheadDueToSmallTaskProp( ct, pCt, pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::OverheadDueToSmallTaskProp );
            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
            candidate = new EmptyTasksinTaskRegionProp( ct, pCt, pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::EmptyTasksinTaskRegionProp );
            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
            candidate = new TooFineGranularTasksProp( ct, pCt, pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::TooFineGranularTasksProp );
            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
            candidate = new ImbalancedTaskRegionProp( ct, pCt, Imbalanced_Task_Region, pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::ImbalancedTaskRegionProp );
            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
            candidate = new ImbalancedTaskRegionProp( ct, pCt, Imbalace_Due_To_Uneven_Distribution_of_Tasks, pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::ImbalancedTaskRegionProp );
            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
            candidate = new ImbalancedTaskRegionProp( ct, pCt, Numberof_Tasks_Smaller_than_Numberof_Threads, pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::ImbalancedTaskRegionProp );
            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
            break;
        case PARALLEL_REGION:
            /* This property is not supported, since ScoreP doesn't provide PSC_PARALLEL_REGION_BODY_CYCLE metric */
            //candidate = new StartupShutdownOverheadProp( ct, pCt );
            //candProperties.push_back( candidate );
            candidate = new LoadImbalanceOMPRegionProp( ct, pCt, PARALLEL_OVERALL_REGION, pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::LoadImbalanceOMPRegionProp );
            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
            break;
        case WORKSHARE_DO:
            candidate = new LoadImbalanceOMPRegionProp( ct, pCt, PARALLEL_LOOP, pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::LoadImbalanceOMPRegionProp );
            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
            break;
        case WORKSHARE:
            candidate = new LoadImbalanceOMPRegionProp( ct, pCt, PARALLEL_WORKSHARE_REGION, pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::LoadImbalanceOMPRegionProp );
            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
            break;
        case CRITICAL_REGION:
            candidate = new CriticalRegionOverheadProp( ct, pCt, PSC_CRITICAL_REGION_CYCLE, pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::CriticalRegionOverheadProp );
            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
//            candidate = new SerializationCriticalRegionProp( ct, pCt,PSC_CRITICAL_REGION_CYCLE );
//            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
//            candProperties.push_back( candidate);
            break;
        case ATOMIC_REGION:
            candidate = new CriticalRegionOverheadProp( ct, pCt, PSC_OMP_ATOMIC_CYCLE, pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::CriticalRegionOverheadProp );
            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
            break;
        case OMP_FLASH_REGION:
            candidate = new CriticalRegionOverheadProp( ct, pCt, PSC_FLUSH_CYCLES, pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::CriticalRegionOverheadProp );
            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
            break;
        case SINGLE_REGION:
            candidate = new SequentialInSingleProp( ct, pCt, pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::SequentialInSingleProp );
            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
            break;

        case MASTER_REGION_BODY:
            candidate = new SequentialInMasterProp( ct, pCt, pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::SequentialInMasterProp );
            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
            break;

        case BARRIER_REGION:
            candidate = new ImbalanceOMPBarrierProp( ct, pCt, pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::ImbalanceOMPBarrierProp );
            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
            break;

        case ORDERED_REGION:
            candidate = new SequentialInOrderedLoopProp( ct, pCt, pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::SequentialInOrderedLoopProp );
            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
            candidate = new ImbalanceInOrderedLoopProp( ct, pCt, pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::ImbalanceInOrderedLoopProp );
            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
            break;

        case SECTIONS_REGION:
            candidate = new ImbalanceInParSectionsProp( ct, pCt, pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::ImbalanceInParSectionsProp );
            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
            break;

        case WORKSHARE_SECTIONS:
            candidate = new ImbalanceInParSectionsProp( ct, pCt, pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::ImbalanceInParSectionsProp );
            psc_dbgmsg( 6, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
            break;
        }
    }
    if( candidate == NULL ) {
        /* No property was created, then contexts should be deleted */
        delete ct;
        delete pCt;
        psc_dbgmsg( 6, "Property was NOT created!\n" );
    }
}

std::list <Property*> OpenMPAnalysisScoreP::create_initial_candidate_properties_set( Region* r ) {
    /* Nothing to be done here */
    std::list <Property*> empty_list;
    return empty_list;
}

void OpenMPAnalysisScoreP::configureNextExperiment() {
    dp->transfer_requests_to_processes();
}



bool OpenMPAnalysisScoreP::evaluateAndReqNextExperiment() {
    /* Tell DataProvider to collect measurements. And check whether all requests are completed */
    /*  Note: during getResult DataProvider will notify this strategy about measured metrics using
     *  metric_found_callback function. This function will instantiate candidate properties based on the metric type
     *  and other conditions. */
    if( dp->getResults() == ALL_INFO_GATHERED ) {
        /* Information was measured. Now the properties can be evaluated. */
    }
    else {
        /* Information is missing */
        return true;
    }

    /* Print candidate properties, instantiated following the notification from DataProvider */
    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( candProperties, "SET OF CANDIDATE PROPERTIES", false, false );
    }

    /* Properties instantiated following the notifications from DataProvider to be evaluated here */
    for( const auto& property : candProperties ) {
        property->evaluate();
        property->condition();
    }
    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( candProperties, "SET OF EVALUATED PROPERTIES", true, false );
    }
    /* Properties evaluated above, to be checked for condition here */
    for( const auto& property : candProperties ) {
        if( property->condition() ) {
            foundPropertiesLastStep.push_back( property );
            foundProperties.push_back( property );
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

std::list < Property* > OpenMPAnalysisScoreP::create_next_candidate_properties_set( std::list< Property* > foundProperties ) {
    std::list <Property*> candidates;
    /* Empty candidate property set will be returned, since this strategy is one step-strategy */
    return candidates;
};

std::string OpenMPAnalysisScoreP::name() {
    return "OpenMP inefficiencies analysis strategy (using ScoreP)";
};
