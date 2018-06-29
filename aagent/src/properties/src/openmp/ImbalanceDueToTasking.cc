/**
   @file    ImbalanceDueToTasking.cc
   @ingroup OpenMPProperties
   @brief   OpenMP Imbalance in threads due to tasking property
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

#include "ImbalanceDueToTasking.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"

PropertyID ImbalanceDueToTaskingProp::id() {
    return IMBALANCEDUETOTASKING;
}

void ImbalanceDueToTaskingProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl;
}

bool ImbalanceDueToTaskingProp::condition() const {
    //Initialize at the beginning
    Max_ExecTime              = 0.0;
    Average_Differential_Time = 0.0;
    // Calculate the maximum execution time of all the threads due to tasks
    Max_ExecTime = ExecTime_eachthread[ 0 ];
    for( int i = 1; i < appl->getOmpThreads(); i++ ) {
        if( ( double )ExecTime_eachthread[ i ] > ( double )Max_ExecTime ) {
            Max_ExecTime = ExecTime_eachthread[ i ];
        }
    }
    // Take the avg. of difference between Max_ExecTime and ExecTime_eachthread[i] w.r.t Max_ExecTime
    // By this, 0 implies all the threads are equally loaded and any increase in the percentage reflects the load imbalance.
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Average_Differential_Time = ( ( ( double )Average_Differential_Time
                                        + ( ( double )Max_ExecTime - ( double )ExecTime_eachthread[ i ] ) ) / ( double )Max_ExecTime );
    }
    return Average_Differential_Time * 100 > threshold;
}

double ImbalanceDueToTaskingProp::confidence() const {
    return 1.0;
}

double ImbalanceDueToTaskingProp::severity() const {
    return Average_Differential_Time * 100;
}

Context* ImbalanceDueToTaskingProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type ImbalanceDueToTaskingProp::request_metrics() {
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        pdb->request( ct, PSC_TASK_REGION_BODY_CYCLE );
        pdb->request( ct, PSC_TASK_REGION_CYCLE );
    }
    pdb->request( phaseContext, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string ImbalanceDueToTaskingProp::name() {
    return "Imbalance in threads due to tasks";
}

void ImbalanceDueToTaskingProp::evaluate() {
    TaskRegion_eachthread.resize( appl->getOmpThreads(), 0 );
    TaskBody_eachthread.resize( appl->getOmpThreads(), 0 );

    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        TaskRegion_eachthread[ i ] = ( INT64 )( double )( pdb->get( ct, PSC_TASK_REGION_CYCLE ) );
        TaskBody_eachthread[ i ]   = ( INT64 )( double )( pdb->get( ct, PSC_TASK_REGION_BODY_CYCLE ) );
        //std::cout << "TaskRegion_eachthread " << TaskRegion_eachthread[i] << "TaskBody_eachthread " << TaskBody_eachthread[i] << "\n";
        // Here, we calculate the exec time for each thread due to tasks
        if( TaskRegion_eachthread[ i ] > 0 ) {
            ExecTime_eachthread[ i ] = TaskRegion_eachthread[ i ]; //may be we have to subtract the implicit barrier time here
        }
        else {
            ExecTime_eachthread[ i ] = TaskBody_eachthread[ i ]; //Do we have more task body in one thread? if so, what happens with body cycle?
        }
        delete ct;
    }
    phaseCycles = ( INT64 )( double )( pdb->get( phaseContext, PSC_EXECUTION_TIME ) ); //sss
    //std::cout<<"In evaluation: ExecTime_TaskRegion "<<ExecTime_TaskRegion<<"ExecTime_TaskBody "<<ExecTime_TaskBody <<std::endl;
}

Property* ImbalanceDueToTaskingProp::clone() {
    ImbalanceDueToTaskingProp* prop = new ImbalanceDueToTaskingProp( context, phaseContext );
    return prop;
}
