/**
   @file    EmptyTaskinTaskRegion.cc
   @ingroup OpenMPProperties
   @brief   OpenMP 'Empty task in tasks' Property
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

#include "EmptyTaskinTaskRegion.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"
#include <sstream>

PropertyID EmptyTasksinTaskRegionProp::id() {
    return EMPTYTASKINTASKREGION;
}

void EmptyTasksinTaskRegionProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl;
}

bool EmptyTasksinTaskRegionProp::condition() const {
//This property expresses an objective answer - either it is empty or not so
/*	std::cout<<" ExecTime_Body "<<ExecTime_Body
   <<" ExecTime_Region " << ExecTime_Region
   <<" phaseCycles "<<phaseCycles<<"  "<<std::endl; */
//Check if the ExecTime_Body value is < (No.ofThreads)*1800
    if( empty_task_for_all_threads == true ) {
        return ( ( ( ( double )ExecTime_Body ) / ( double )phaseCycles ) * 100 ) > threshold;
    }
    else {
        return false;
    }
}

double EmptyTasksinTaskRegionProp::confidence() const {
    return 1.0;
}

double EmptyTasksinTaskRegionProp::severity() const {
    //severity is proportional to task creation; Only an empty task region is considered here.
    if( empty_task_for_all_threads == true ) {
        return ( ( ( double )ExecTime_Region - ( double )ExecTime_Body ) / ( double )phaseCycles ) * 100;
    }
    else {
        return 0.0;
    }
}

Context* EmptyTasksinTaskRegionProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type EmptyTasksinTaskRegionProp::request_metrics() {
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        pdb->request( ct, PSC_TASK_REGION_BODY_CYCLE );
        pdb->request( ct, PSC_TASK_REGION_CYCLE );
    }
    pdb->request( phaseContext, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string EmptyTasksinTaskRegionProp::name() {
    return "Empty task in Task region";
}

void EmptyTasksinTaskRegionProp::evaluate() {
    TaskRegion_thread.resize( appl->getOmpThreads(), 0 );
    TaskBody_thread.resize( appl->getOmpThreads(), 0 );
    ExecTime_Region            = 0.0;
    ExecTime_Body              = 0.0;
    ExecTime_singleTask        = 0.0;
    numberofCreatedTasks       = 0;
    numberofExecutedTasks      = 0;
    empty_count                = 0;
    empty_task_for_all_threads = false;
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        TaskRegion_thread[ i ] = pdb->get( ct, PSC_TASK_REGION_CYCLE );
        TaskBody_thread[ i ]   = pdb->get( ct, PSC_TASK_REGION_BODY_CYCLE );
        if( TaskRegion_thread[ i ] > 0 ) {
            ExecTime_Region = ExecTime_Region + TaskRegion_thread[ i ];
        }
        else {
            ;
        }
        if( TaskBody_thread[ i ] <= 1800 ) {
            empty_count++;
        }
        else {
            ;
        }
        delete ct;
    }
    /*
       for (int i = 0; i < appl->getOmpThreads(); i++) {
       std::cout << " TaskRegion " << TaskRegion_thread[i] << " TaskBody " << TaskBody_thread[i] << std::endl;
       }
       std::cout << " Empty count " << empty_count << std::endl;
     */
    if( empty_count >= ( appl->getOmpThreads() - 1 ) ) {
        empty_task_for_all_threads = true;
    }
    else {
        empty_task_for_all_threads = false;
    }
    //ExecTime_singleTask =  (double)ExecTime_Body / numberofTask;
    phaseCycles = pdb->get( phaseContext, PSC_EXECUTION_TIME ); //sss
}

std::string EmptyTasksinTaskRegionProp::toXMLExtra() {
    std::stringstream stream;
    stream << "\t\t<ExecTime>" << ExecTime_Body << "</ExecTime>" << std::endl;
    return stream.str();
}

Property* EmptyTasksinTaskRegionProp::clone() {
    EmptyTasksinTaskRegionProp* prop = new EmptyTasksinTaskRegionProp( context, phaseContext );
    return prop;
}
