/**
   @file    OverheadDueToSmallTask.cc
   @ingroup OpenMPProperties
   @brief   OpenMP 'Overhead due to less workload in task' property
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

#include "OverheadDueToSmallTask.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"
#include <sstream>

PropertyID OverheadDueToSmallTaskProp::id() {
    return OVERHEADDUETOSMALLTASK;
}

void OverheadDueToSmallTaskProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl;
}

bool OverheadDueToSmallTaskProp::condition() const {
    /* for (int i = 0;i < appl->getOmpThreads(); i++) {
       std::cout << " TaskRegion " << TaskRegion_eachthread[i] << " TaskBody " << TaskBody_eachthread[i] << std::endl;
       }
       /*	ExecTime_TaskBody = 0.0;
       ExecTime_singleTaskBody =0.0;
       ExecTime_TaskBody = TaskBody_eachthread[0];
       for (int i = 1; i < appl->getOmpThreads(); i++) {
       if(TaskBody_eachthread[i] > ExecTime_TaskBody)
       ExecTime_TaskBody = TaskBody_eachthread[i];
       }
       ExecTime_singleTaskBody = (double) ExecTime_TaskBody / (double)numberofTaskbody;
       return (((((double)phaseCycles - (double)ExecTime_singleTaskBody) / (double)phaseCycles))* 100 > threshold);
     */
    if( ExecTime_TaskRegion >= ExecTime_TaskBody ) {
        //It means that that the workload is less
        return ( ( ( double )ExecTime_TaskRegion - ( double )ExecTime_TaskBody ) * 100 ) > threshold;
    }
    else {
        return 0.0;
    }
}

double OverheadDueToSmallTaskProp::confidence() const {
    return 1.0;
}

double OverheadDueToSmallTaskProp::severity() const {
    //return ((((double)phaseCycles -(double) ExecTime_singleTaskBody)/(double)phaseCycles)* 100);
    if( ExecTime_TaskRegion >= ExecTime_TaskBody ) {
        //It means that that the workload is less
        return ( ( ( double )ExecTime_TaskRegion - ( double )ExecTime_TaskBody ) / ( ( double )phaseCycles ) ) * 100;
    }
    else {
        return 0.0;
    }
}

Context* OverheadDueToSmallTaskProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type OverheadDueToSmallTaskProp::request_metrics() {
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        //pdb->request(ct, PSC_INSTANCES);
        pdb->request( ct, PSC_TASK_REGION_CYCLE );
    }
    pdb->request( phaseContext, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string OverheadDueToSmallTaskProp::name() {
    return "Overhead due to less workload in tasks";
}

void OverheadDueToSmallTaskProp::evaluate() {
    TaskRegion_eachthread.resize( appl->getOmpThreads(), 0 );
    TaskBody_eachthread.resize( appl->getOmpThreads(), 0 );
    ExecTime_TaskRegion     = 0.0;
    ExecTime_TaskBody       = 0.0;
    ExecTime_singleTaskBody = 0.0;
    numberofTaskbody        = 0.0;
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        TaskRegion_eachthread[ i ] = ( INT64 )( double )( pdb->get( ct, PSC_TASK_REGION_CYCLE ) );
        TaskBody_eachthread[ i ]   = ( INT64 )( double )( pdb->get( ct, PSC_TASK_REGION_BODY_CYCLE ) );
        numberofTaskbody           = numberofTaskbody + ( pdb->get( ct, PSC_INSTANCES ) );
        if( TaskRegion_eachthread[ i ] > 0 ) {
            ExecTime_TaskRegion += TaskRegion_eachthread[ i ];
        }
        else {
            ;
        }
        if( TaskBody_eachthread[ i ] > 0 ) {
            ExecTime_TaskBody += TaskBody_eachthread[ i ];
        }
        else {
            ;
        }

        delete ct;
    }

    phaseCycles = ( INT64 )( double )( pdb->get( phaseContext, PSC_EXECUTION_TIME ) ); //sss
}

INT64 OverheadDueToSmallTaskProp::exec_time() {
    return 0;
}

std::string OverheadDueToSmallTaskProp::toXMLExtra() {
    std::stringstream stream;
    stream << "\t\t<ExecTime>" << ExecTime_singleTaskBody << "</ExecTime>" << std::endl;
    return stream.str();
}

Property* OverheadDueToSmallTaskProp::clone() {
    OverheadDueToSmallTaskProp* prop = new OverheadDueToSmallTaskProp( context, phaseContext );
    return prop;
}
