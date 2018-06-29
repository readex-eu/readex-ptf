/**
   @file	LoadImbalanceOMPRegion.cc
   @ingroup AnalysisAgent
   @brief   OpenMP Load Imbalance in Parallel region, Parallel loop, workshare region and parallel section property
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
#include "LoadImbalanceOMPRegion.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"
#include <sstream>

PropertyID LoadImbalanceOMPRegionProp::id() {
    switch( regionType ) {
    case PARALLEL_OVERALL_REGION:
        return LOADIMBALANCEPARALLELREGION;
        break;
    case PARALLEL_LOOP:
        return LOADIMBALANCEPARALLELLOOP;
        break;
    case PARALLEL_WORKSHARE_REGION:
        return LOADIMBALANCEWORKSHAREREGION;

    case PARALLEL_SECTION:
        return LOADIMBALANCEPARALLELSECTION;
        break;
    }
    return LOADIMBALANCEOMPREGION;
}


void LoadImbalanceOMPRegionProp::print() {
    std::cout << "Property:" << name() <<
    "  Process " << context->getRank() <<
    "  Thread " <<  context->getThread() << std::endl;
}

bool LoadImbalanceOMPRegionProp::condition() const {
    return ( ( ( double )maxWaitTime - ( double )minWaitTime ) / ( double )phaseCycles * 100 ) > threshold;
}

double LoadImbalanceOMPRegionProp::confidence() const {
    return 1.0;
}

double LoadImbalanceOMPRegionProp::severity() const {
    if( ( ( ( double )maxWaitTime - ( double )avg_imb_threads ) / ( double )phaseCycles * 100 ) > 100.0 ) {
        return 100;
    }
    else {
        return ( ( double )maxWaitTime - ( double )minWaitTime ) / ( double )phaseCycles * 100;
    }
}

Context* LoadImbalanceOMPRegionProp::get_phaseContext() {
    return phaseContext;
}


Gather_Required_Info_Type LoadImbalanceOMPRegionProp::request_metrics() {
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        pdb->request( ct, PSC_IMPLICIT_BARRIER_TIME );
    }
    //pdb->request(phaseContext,CYCLES);
    pdb->request( phaseContext, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string LoadImbalanceOMPRegionProp::name() {
    switch( regionType ) {
    case PARALLEL_OVERALL_REGION:
        return "Load Imbalance in parallel region";
        break;
    case PARALLEL_LOOP:
        return "Load Imbalance in parallel loop";
        break;
    case PARALLEL_WORKSHARE_REGION:
        return "Load Imbalance in workshare region";
    case PARALLEL_SECTION:
        return "Load Imbalance in Parallel Section";
    }
    return "Unknown OMP property";
}

void LoadImbalanceOMPRegionProp::evaluate() {
    waitCycles.resize( appl->getOmpThreads(), 0 );

    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        waitCycles[ i ] =  pdb->get( ct, PSC_IMPLICIT_BARRIER_TIME );
        delete ct;
    }
    phaseCycles = pdb->get( phaseContext, PSC_EXECUTION_TIME );

    maxWaitTime     = waitCycles[ 0 ];
    minWaitTime     = waitCycles[ 0 ];
    nonzero_count   = 0;
    avg_imb_threads = 0.0;
/*
        std::cout << " Implicit Barrier time: in LoadImbalanceOMPRegion.cc " << std::endl;
        for (int i=0; i<appl->getOmpThreads(); i++){
                std::cout << waitCycles[i] << "\t";
        }
 */
    for( int i = 1; i < appl->getOmpThreads(); i++ ) {
        if( waitCycles[ i ] > maxWaitTime ) {
            maxWaitTime = waitCycles[ i ];
        }
        if( waitCycles[ i ] < minWaitTime ) {
            minWaitTime = waitCycles[ i ];
        }
        avg_imb_threads = avg_imb_threads + waitCycles[ i ];
    }
    for( int j = 0; j < appl->getOmpThreads(); j++ ) {
        if( waitCycles[ j ] != 0 ) {
            nonzero_count++;
        }
    }
    if( nonzero_count != 0 ) {
        avg_imb_threads = avg_imb_threads / ( nonzero_count );
    }

    //std::cout << "maxWaitTime: " << maxWaitTime << " minWaitTime : " << minWaitTime << std::endl;
}

std::string LoadImbalanceOMPRegionProp::toXMLExtra() {
    std::stringstream stream;
    stream << "\t\t<ExecTime>" << maxWaitTime << "</ExecTime>" << std::endl;
    return stream.str();
}

Property* LoadImbalanceOMPRegionProp::clone() {
    LoadImbalanceOMPRegionProp* prop = new LoadImbalanceOMPRegionProp( context, phaseContext );
    return prop;
}
