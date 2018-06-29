/**
   @file    Task_StartupAndShutdown_Overhead.cc
   @ingroup OpenMPProperties
   @brief   OpenMP Task startup and Shutdown overhead property
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

#include "Task_StartupAndShutdown_Overhead.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"
#include <sstream>

PropertyID Task_StartupAndShutdown_OverheadProp::id() {
    return TASKREGIONOVERHEAD;
}

void Task_StartupAndShutdown_OverheadProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl;
}

bool Task_StartupAndShutdown_OverheadProp::condition() const {
    /* //Max_TaskBody = 0.0;
       //ExecTime_TaskRegion=0.0;
       //Max_TaskBody = TaskBody_eachthread[0];
       //ExecTime_TaskRegion = TaskRegion_eachthread[0];
       for (int i=1;i<appl->getOmpThreads();i++) {
       if (TaskBody_eachthread[i] > Max_TaskBody)
       Max_TaskBody = TaskBody_eachthread[i];
       if (TaskRegion_eachthread[i] > ExecTime_TaskRegion)
       ExecTime_TaskRegion = TaskRegion_eachthread[i];
       } */

    //std::cout << " Max_TaskBody " << Max_TaskBody << " ExecTime_TaskRegion " <<ExecTime_TaskRegion<< std::endl;
    //for (int i = 0; i < appl->getOmpThreads(); i++) {
    //  std::cout << " TaskBody_eachthread " << TaskBody_eachthread[i]
    //            << " TaskRegion_eachthread " << TaskRegion_eachthread[i] << std::endl;
    //}
    Startup_Time = 0.0;
    Startup_Time = ExecTime_TaskRegion - ExecTime_TaskBody_forMaster;

    return ( ( ( double )Startup_Time / ( double )phaseCycles ) * 100 ) > threshold; //sss redefined here
}

double Task_StartupAndShutdown_OverheadProp::confidence() const {
    return 1.0;
}

double Task_StartupAndShutdown_OverheadProp::severity() const {
    //INT64 severity_val = (((double)ExecTime_TaskRegion -(double) Max_TaskBody)/(double)phaseCycles) * 100;
    //std::cout <<" severity_val " << severity_val << std::endl;
    return ( ( double )Startup_Time / ( double )phaseCycles ) * 100;
}

Context* Task_StartupAndShutdown_OverheadProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type Task_StartupAndShutdown_OverheadProp::request_metrics() {
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        pdb->request( ct, PSC_TASK_REGION_CYCLE );
        pdb->request( ct, PSC_TASK_REGION_BODY_CYCLE );
    }
    pdb->request( phaseContext, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string Task_StartupAndShutdown_OverheadProp::name() {
    return "Task Region Startup and Shutdown overhead";
}

void Task_StartupAndShutdown_OverheadProp::evaluate() {
    TaskBody_eachthread.resize( appl->getOmpThreads(), 0 );
    TaskRegion_eachthread.resize( appl->getOmpThreads(), 0 );
    ExecTime_TaskRegion         = 0.0;
    ExecTime_TaskBody_forMaster = 0.0;
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        TaskRegion_eachthread[ i ] = ( double )( pdb->get( ct, PSC_TASK_REGION_CYCLE ) );
        TaskBody_eachthread[ i ]   = ( double )( pdb->get( ct, PSC_TASK_REGION_BODY_CYCLE ) );
        if( TaskRegion_eachthread[ i ] > 0 ) {
            ExecTime_TaskRegion += TaskRegion_eachthread[ i ];
        }
        else {
            ;
        }
        if( i == 0 && TaskBody_eachthread[ i ] > 0 ) {
            ExecTime_TaskBody_forMaster = TaskBody_eachthread[ i ];
        }
        else {
            ;
        }
        //std::cout << " TaskBody_eachthread " << TaskBody_eachthread[i] << " TaskRegion " << TaskRegion_eachthread[i] << "\n";
        delete ct;
    }
    //std::cout << " ExecTime_TaskRegion " << ExecTime_TaskRegion << std::endl;
    phaseCycles = ( INT64 )( double )( pdb->get( phaseContext, PSC_EXECUTION_TIME ) );
}
Property* Task_StartupAndShutdown_OverheadProp::clone() {
    Task_StartupAndShutdown_OverheadProp* prop = new Task_StartupAndShutdown_OverheadProp( context, phaseContext );
    return prop;
}

std::string Task_StartupAndShutdown_OverheadProp::toXMLExtra() {
    std::stringstream stream;
    stream << "\t\t<ExecTime>" << ExecTime_TaskRegion << "</ExecTime>" << std::endl;
    return stream.str();
}

INT64 Task_StartupAndShutdown_OverheadProp::exec_time() {
    return ExecTime_TaskRegion;
}
