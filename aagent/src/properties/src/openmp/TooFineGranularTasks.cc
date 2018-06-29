/**
   @file    TooFineGranularTasks.cc
   @ingroup OpenMPProperties
   @brief   Task Granularity description
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

#include "TooFineGranularTasks.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"
#include <sstream>

PropertyID TooFineGranularTasksProp::id() {
    return TOOFINEGRANULARTASKS;
}

void TooFineGranularTasksProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl;
}

bool TooFineGranularTasksProp::condition() const {
    //TODO: fine_granular_tasks should be considered w.r.t the avg of task body time
    return ( ( ( ( double )Max_TaskRegion - ( double )fine_granular_tasks ) / ( double )phaseCycles ) * 100 ) > threshold;
}

double TooFineGranularTasksProp::confidence() const {
    return 0.99;
}

double TooFineGranularTasksProp::severity() const {
    return ( ( double )Max_TaskRegion / ( double )phaseCycles ) * 100;
}

Context* TooFineGranularTasksProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type TooFineGranularTasksProp::request_metrics() {
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        pdb->request( ct, PSC_TASK_REGION_CYCLE );
        pdb->request( ct, PSC_TASK_REGION_BODY_CYCLE );
    }
    pdb->request( phaseContext, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string TooFineGranularTasksProp::name() {
    return " Too fine granularity in Tasks ";
}

void TooFineGranularTasksProp::evaluate() {
    TaskRegion_eachthread.resize( appl->getOmpThreads(), 0 );
    TaskBody_eachthread.resize( appl->getOmpThreads(), 0 );
    Tasks_Created_eachthread.resize( appl->getOmpThreads(), 0 );
    fine_granular_tasks = 0.0;
    ExecTime_TaskRegion = 0.0;
    ExecTime_TaskBody   = 0.0;
    Tasks_Created       = 0;
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        TaskRegion_eachthread[ i ]    = ( double )( pdb->get( ct, PSC_TASK_REGION_CYCLE ) );
        TaskBody_eachthread[ i ]      = ( double )( pdb->get( ct, PSC_TASK_REGION_BODY_CYCLE ) );
        Tasks_Created_eachthread[ i ] = ( double )( pdb->get( ct, PSC_TASKS_CREATED ) );

        //find taskregion time
        if( TaskRegion_eachthread[ i ] > 0 ) {
            ExecTime_TaskRegion += TaskRegion_eachthread[ i ];
        }
        else {
            ;
        }
        //find the total tasks created
        Tasks_Created += Tasks_Created_eachthread[ i ];
        //find the sum of taskbody w.r.t tasks created
        if( TaskBody_eachthread[ i ] > 0 ) {
            ExecTime_TaskBody += TaskBody_eachthread[ i ];
        }
        else {
            ;
        }
        //std::cout << " Tasks_Created " << (double) pdb->get(ct, TASKS_CREATED) << " ExecTime_TaskBody "<< ExecTime_TaskBody << std::endl;
        delete ct;
    }

    //fine_granular_tasks = ExecTime_TaskBody /  Tasks_Created;
    phaseCycles = ( INT64 )( double )( pdb->get( phaseContext, PSC_EXECUTION_TIME ) );

    //Take the maximum task region time among all the active threads
    Max_TaskRegion = 0.0;
    Max_TaskRegion = TaskRegion_eachthread[ 0 ];
    for( int i = 1; i < appl->getOmpThreads(); i++ ) {
        if( TaskRegion_eachthread[ i ] > Max_TaskRegion ) {
            Max_TaskRegion = TaskRegion_eachthread[ i ];
        }
    }
}
Property* TooFineGranularTasksProp::clone() {
    TooFineGranularTasksProp* prop = new TooFineGranularTasksProp( context, phaseContext );
    return prop;
}

std::string TooFineGranularTasksProp::toXMLExtra() {
    std::stringstream stream;
    stream << "\t\t<ExecTime>" << ExecTime_TaskRegion << "</ExecTime>" << std::endl;
    return stream.str();
}

INT64 TooFineGranularTasksProp::exec_time() {
    return ExecTime_TaskRegion;
}
