/**
   @file    OverheadDueToTaskCreation.cc
   @ingroup OpenMPProperties
   @brief   OpenMP Task Creation overhead property
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

#include "OverheadDueToTaskCreation.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"
#include <sstream>

PropertyID OverheadDueToTaskCreationProp::id() {
    return TASKREGIONOVERHEAD;
}

void OverheadDueToTaskCreationProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl;
}

bool OverheadDueToTaskCreationProp::condition() const {
    return ( ( ( double )Max_TaskRegion / ( double )phaseCycles ) * 100 ) > threshold;
}

double OverheadDueToTaskCreationProp::confidence() const {
    return 1.0;
}

double OverheadDueToTaskCreationProp::severity() const {
    return ( ( double )Max_TaskRegion / ( double )phaseCycles ) * 100;
}

Context* OverheadDueToTaskCreationProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type OverheadDueToTaskCreationProp::request_metrics() {
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        pdb->request( ct, PSC_TASK_REGION_CYCLE );
    }
    pdb->request( phaseContext, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string OverheadDueToTaskCreationProp::name() {
    return " Overhead due to task creation ";
}

void OverheadDueToTaskCreationProp::evaluate() {
    TaskRegion_eachthread.resize( appl->getOmpThreads(), 0 );
    ExecTime_TaskRegion = 0.0;
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        TaskRegion_eachthread[ i ] = ( double )( pdb->get( ct, PSC_TASK_REGION_CYCLE ) );
        if( TaskRegion_eachthread[ i ] > 0 ) {
            ExecTime_TaskRegion += TaskRegion_eachthread[ i ];
        }
        else {
            ;
        }
        delete ct;
    }
    phaseCycles = ( INT64 )( double )( pdb->get( phaseContext, PSC_EXECUTION_TIME ) );

    Max_TaskRegion = 0.0;
    Max_TaskRegion = TaskRegion_eachthread[ 0 ];
    for( int i = 1; i < appl->getOmpThreads(); i++ ) {
        if( TaskRegion_eachthread[ i ] > Max_TaskRegion ) {
            Max_TaskRegion = TaskRegion_eachthread[ i ];
        }
    }
}
Property* OverheadDueToTaskCreationProp::clone() {
    OverheadDueToTaskCreationProp* prop = new OverheadDueToTaskCreationProp( context, phaseContext );
    return prop;
}

std::string OverheadDueToTaskCreationProp::toXMLExtra() {
    std::stringstream stream;
    stream << "\t\t<ExecTime>" << ExecTime_TaskRegion << "</ExecTime>" << std::endl;
    return stream.str();
}

INT64 OverheadDueToTaskCreationProp::exec_time() {
    return ExecTime_TaskRegion;
}
