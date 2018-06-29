/**
   @file    ImbalancedTaskRegion.cc
   @ingroup OpenMPProperties
   @brief   Task region with imbalanced distribution
   @author  Shajulin Benedict
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include "ImbalancedTaskRegion.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"

PropertyID ImbalancedTaskRegionProp::id() {
    switch( propertyName ) {
    case Imbalanced_Task_Region:
        return IMBALANCEDTASKREGION;
        break;
    case Imbalace_Due_To_Uneven_Distribution_of_Tasks:
        return IMBALANCEDUETOUNEVENDISTRIBUTIONOFTASKS;
        break;
    case Numberof_Tasks_Smaller_than_Numberof_Threads:
        return NUMBEROFTASKSSMALLERTHANNUMBEROFTHREADS;
        break;
    }
    return IMBALANCEDTASKREGION;
}

void ImbalancedTaskRegionProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl;
}

bool ImbalancedTaskRegionProp::condition() const {
    switch( propertyName ) {
    case Imbalanced_Task_Region:
        //For Debug
        //	std::cout << "TotalTasks " << Max_Tasks << " Total_CreatedTasks " << Total_CreatedTasks
        //			  << std::endl;

        return ( ( Average_Differential_Time / phaseCycles ) ) * 100 > threshold;
        break;
    case Imbalace_Due_To_Uneven_Distribution_of_Tasks:
        return ( ( Average_Differential_Tasks / phaseCycles ) * 100 ) > threshold;
        break;
    case Numberof_Tasks_Smaller_than_Numberof_Threads:
        if( Max_Tasks < appl->getOmpThreads() ) {
            return ( ( Average_Differential_Time / phaseCycles ) * 100 ) > threshold;
        }
        else {
            return false;
        }
        break;
    }
    return true;
}

double ImbalancedTaskRegionProp::confidence() const {
    return 0.99;
}

double ImbalancedTaskRegionProp::severity() const {
    switch( propertyName ) {
    case Imbalanced_Task_Region:
        return ( Average_Differential_Time / phaseCycles ) * 100;
        break;
    case Imbalace_Due_To_Uneven_Distribution_of_Tasks:
        return ( Average_Differential_Tasks / phaseCycles ) * 100;
        break;
    case Numberof_Tasks_Smaller_than_Numberof_Threads:
        if( Max_Tasks < appl->getOmpThreads() ) {
            return ( Average_Differential_Time / phaseCycles ) * 100;
        }
        else {
            return 0.0;
        }
        break;
    }
    return 0.0;
}

Context* ImbalancedTaskRegionProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type ImbalancedTaskRegionProp::request_metrics() {
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        pdb->request( ct, PSC_TASK_REGION_BODY_CYCLE );
        pdb->request( ct, PSC_TASK_REGION_CYCLE );
    }
    pdb->request( phaseContext, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string ImbalancedTaskRegionProp::name() {
    switch( propertyName ) {
    case Imbalanced_Task_Region:
        return "Imbalance in task region";
        break;
    case Imbalace_Due_To_Uneven_Distribution_of_Tasks:
        return "Imbalance due to uneven distribution of tasks";
        break;
    case Numberof_Tasks_Smaller_than_Numberof_Threads:
        return "Number of tasks are smaller than the number of threads";
        break;
    }
    return "Imbalance in task region";
}

void ImbalancedTaskRegionProp::evaluate() {
    TaskRegion_eachthread.resize( appl->getOmpThreads(), 0 );
    TaskBody_eachthread.resize( appl->getOmpThreads(), 0 );
    ExecTime_eachthread.resize( appl->getOmpThreads(), 0 );
    nooftasks_eachthread.resize( appl->getOmpThreads(), 0 );
    nooftasks_created.resize( appl->getOmpThreads(), 0 );

    //std::cout << std::endl;

    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        TaskRegion_eachthread[ i ] = ( INT64 )( double )( pdb->get( ct, PSC_TASK_REGION_CYCLE ) );
        TaskBody_eachthread[ i ]   = ( INT64 )( double )( pdb->get( ct, PSC_TASK_REGION_BODY_CYCLE ) );

        ExecTime_eachthread[ i ]  = TaskBody_eachthread[ i ];
        nooftasks_eachthread[ i ] = ( double )pdb->get( ct, PSC_TASKS_EXECUTED );
        nooftasks_created[ i ]    = ( double )pdb->get( ct, PSC_TASKS_CREATED );
        //	std::cout << " Noof tasks executed " << nooftasks_eachthread[i] << " Noof tasks created "
        //			<< (INT64) pdb->get(ct,TASKS_CREATED) << std::endl;

        //	std::cout << " TaskBody " << TaskBody_eachthread[i] << " Task " << TaskRegion_eachthread[i] << std::endl;

        delete ct;
    }
    phaseCycles = ( INT64 )( double )( pdb->get( phaseContext, PSC_EXECUTION_TIME ) );

    //Initialize at the beginning
    Max_ExecTime               = 0.0;
    Max_ExecTasks              = 0;
    Max_Tasks                  = 0;
    Total_CreatedTasks         = 0;
    Average_Differential_Time  = 0.0;
    Average_Differential_Tasks = 0.0;
    Mean_Task_Body             = 0.0;

    // Calculate the maximum execution time of all the threads due to tasks &
    // maximum no.of tasks
    Max_ExecTime  = ExecTime_eachthread[ 0 ];
    Max_ExecTasks = nooftasks_eachthread[ 0 ];

    for( int i = 1; i < appl->getOmpThreads(); i++ ) {
        if( ( double )ExecTime_eachthread[ i ] > Max_ExecTime ) {
            Max_ExecTime = ( double )ExecTime_eachthread[ i ];
        }
        else {
            ;
        }
        if( nooftasks_eachthread[ i ] > Max_ExecTasks ) {
            Max_ExecTasks += nooftasks_eachthread[ i ];
        }
        else {
            ;
        }
    }
    // Take the avg. of the difference between Max_ExecTime and ExecTime_eachthread[i] w.r.t Max_ExecTime
    // By this, 0 implies all the threads are equally loaded and any increase in the percentage reflects the load imbalance.
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Average_Differential_Time = ( ( Average_Differential_Time + ( Max_ExecTime - ( double )ExecTime_eachthread[ i ] ) ) );
        //Take the avg. w.r.t no.of tasks
        Average_Differential_Tasks = ( ( Average_Differential_Tasks + ( Max_ExecTasks - nooftasks_eachthread[ i ] ) ) );
        //Calculate the total executed and created tasks on all threads
        Max_Tasks          += nooftasks_eachthread[ i ];
        Total_CreatedTasks += nooftasks_created[ i ];
    }
    Average_Differential_Time  = ( double )Average_Differential_Time / ( double )appl->getOmpThreads();
    Average_Differential_Tasks = ( double )Average_Differential_Tasks / ( double )appl->getOmpThreads();
}

Property* ImbalancedTaskRegionProp::clone() {
    ImbalancedTaskRegionProp* prop = new ImbalancedTaskRegionProp( context, phaseContext );
    return prop;
}
