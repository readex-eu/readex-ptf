/**
   @file    HotRegionExecTime.cc
   @ingroup OpenMPProperties
   @brief   Hot OpenMP region execution time in the application
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

#include "HotRegionExecTime.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"
#include <sstream>

PropertyID HotRegionExecTimeProp::id() {
    switch( regionType ) {
    case USER_REGION_TYPE:
        return USERREGIONEXECTIME;
        break;
    case MAIN_REGION_TYPE:
        return MAINREGIONEXECTIME;
        break;
    case CALL_REGION_TYPE:
        return CALLREGIONEXECTIME;
        break;
    case SUB_REGION_TYPE:
        return SUBREGIONEXECTIME;
        break;
    case OMP_FOR_REGION_TYPE:
        return OMPFORREGIONEXECTIME;
        break;
    case PARALLEL_REGION_TYPE:
        return PARALLELREGIONEXECTIME;
        break;
    case DO_REGION_TYPE:
        return PARALLELREGIONEXECTIME;
        break;
    default:
        break;
    }
}

void HotRegionExecTimeProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl;
}

bool HotRegionExecTimeProp::condition() const {
    return true;
}

double HotRegionExecTimeProp::confidence() const {
    return 1.0;
}

double HotRegionExecTimeProp::severity() const {
    return ( ( double )ExecTime / ( double )phaseCycles ) * 100;
}

Context* HotRegionExecTimeProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type HotRegionExecTimeProp::request_metrics() {
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        switch( regionType ) {
        case OMP_FOR_REGION_TYPE:
            pdb->request( ct, PSC_OMP_DO_REGION_CYCLE );
            break;
        case PARALLEL_REGION_TYPE:
            if( i == 0 ) {
                pdb->request( ct, PSC_PARALLEL_REGION_CYCLE );
            }
            break;
        case DO_REGION_TYPE:
            if( i == 0 ) {
                pdb->request( ct, PSC_PARALLEL_REGION_CYCLE );
            }
            break;
        case USER_REGION_TYPE:
            if( i == 0 ) {
                pdb->request( phaseContext, PSC_EXECUTION_TIME );
            }
            break;
        case MAIN_REGION_TYPE:
            if( i == 0 ) {
                pdb->request( phaseContext, PSC_EXECUTION_TIME );
            }
            break;
        case CALL_REGION_TYPE:
            pdb->request( ct, PSC_EXECUTION_TIME );
            break;
        case SUB_REGION_TYPE:
            if( i == 0 ) {
                pdb->request( ct, PSC_EXECUTION_TIME );
            }
            break;
        default:
            break;
        }
    }

    return ALL_INFO_GATHERED;
}

std::string HotRegionExecTimeProp::name() {
    switch( regionType ) {
    case OMP_FOR_REGION_TYPE:
        return " ExecTime ::::: OmpFor region ";
        break;
    case PARALLEL_REGION_TYPE:
        return " ExecTime ::::: Parallel region ";
        break;
    case DO_REGION_TYPE:
        return " ExecTime ::::: OmpParallelFor region ";
        break;
    case USER_REGION_TYPE:
        return " ExecTime ::::: Main-User region ";
        break;
    case MAIN_REGION_TYPE:
        return " ExecTime ::::: Main-User region ";
        break;
    case CALL_REGION_TYPE:
        return " ExecTime ::::: Call region ";
        break;
    case SUB_REGION_TYPE:
        return " ExecTime ::::: Sub region ";
        break;
    default:
        break;
    }
}

void HotRegionExecTimeProp::evaluate() {
    Context* ct = new Context( context->getRegion(), context->getRank(), 0 );
    switch( regionType ) {
    case USER_REGION_TYPE:
        ExecTime = 0;
        ExecTime = pdb->get( phaseContext, PSC_EXECUTION_TIME );
        break;
    case MAIN_REGION_TYPE:
        ExecTime = 0;
        ExecTime = pdb->get( phaseContext, PSC_EXECUTION_TIME );
        break;
    case OMP_FOR_REGION_TYPE:
        ExecTime = 0;
        threads_ExecTime.resize( appl->getOmpThreads(), 0 );
        for( int i = 0; i < appl->getOmpThreads(); i++ ) {
            Context* ct1 = new Context( context->getRegion(), context->getRank(), i );
            threads_ExecTime[ i ] = pdb->get( ct1, PSC_OMP_DO_REGION_CYCLE );
            if( threads_ExecTime[ i ] > ExecTime ) {
                ExecTime = threads_ExecTime[ i ];
            }
            delete ct1;
        }
        break;
    case CALL_REGION_TYPE:
        ExecTime = 0;
        threads_ExecTime.resize( appl->getOmpThreads(), 0 );
        for( int i = 0; i < appl->getOmpThreads(); i++ ) {
            Context* ct2 = new Context( context->getRegion(), context->getRank(), i );
            threads_ExecTime[ i ] = pdb->get( ct2, PSC_EXECUTION_TIME );
            if( threads_ExecTime[ i ] > ExecTime ) {
                ExecTime = threads_ExecTime[ i ];
            }
            delete ct2;
        }
        break;
    case SUB_REGION_TYPE:
        ExecTime = 0;
        ExecTime = pdb->get( ct, PSC_EXECUTION_TIME );
        break;
    case PARALLEL_REGION_TYPE:
        ExecTime = 0;
        ExecTime = pdb->get( ct, PSC_PARALLEL_REGION_CYCLE );
        break;
    case DO_REGION_TYPE:
        ExecTime = 0;
        ExecTime = pdb->get( ct, PSC_PARALLEL_REGION_CYCLE );
        break;
    default:
        break;
    }
    phaseCycles = pdb->get( phaseContext, PSC_EXECUTION_TIME );
    delete ct;
}

std::string HotRegionExecTimeProp::toXMLExtra() {
    std::stringstream stream;
    stream << "\t\t<ExecTime>" << ExecTime << "</ExecTime>" << std::endl;
    return stream.str();
}

Property* HotRegionExecTimeProp::clone() {
    HotRegionExecTimeProp* prop = new HotRegionExecTimeProp( context, phaseContext );
    return prop;
}
