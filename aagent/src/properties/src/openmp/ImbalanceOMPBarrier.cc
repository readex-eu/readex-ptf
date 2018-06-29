/**
   @file	ImbalanceOMPBarrier.cc
   @ingroup AnalysisAgent
   @brief   OpenMP Load Imbalance in Explicit Barrier property
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
#include "ImbalanceOMPBarrier.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"
#include <sstream>


PropertyID ImbalanceOMPBarrierProp::id() {
    return IMBALANCEOMPBARRIER;
}


void ImbalanceOMPBarrierProp::print() {
    std::cout << "Property:" << name() <<
    "  Process " << context->getRank() <<
    "  Thread " <<  context->getThread()  << std::endl;
}

bool ImbalanceOMPBarrierProp::condition() const {
/*
        for (int i=0; i<appl->getOmpThreads(); i++){
                std::cout << waitCycles[i] << "\t";
        }
        std::cout <<"phaseCycles " << phaseCycles << "\n";
 */


    return ( ( double )maxWaitTime - ( double )avg_threads ) / ( double )phaseCycles * 100 > threshold;
}

double ImbalanceOMPBarrierProp::confidence() const {
    return 1.0;
}

double ImbalanceOMPBarrierProp::severity() const {
    //if((((double)maxWaitTime-(double)minWaitTime)/(double)phaseCycles*100) > 100.0)
    //	return 100;
    //else
    return ( ( double )maxWaitTime - ( double )minWaitTime ) / ( double )phaseCycles * 100;
}

Context* ImbalanceOMPBarrierProp::get_phaseContext() {
    return phaseContext;
}


Gather_Required_Info_Type ImbalanceOMPBarrierProp::request_metrics() {
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        pdb->request( ct, PSC_OMP_BARRIER_CYCLE );
    }
    //pdb->request(phaseContext,CYCLES);
    pdb->request( phaseContext, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string ImbalanceOMPBarrierProp::name() {
    return "Imbalance in OMP explicit Barrier Region";
}

void ImbalanceOMPBarrierProp::evaluate() {
    waitCycles.resize( appl->getOmpThreads(), 0 );

    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        waitCycles[ i ] =  pdb->get( ct, PSC_OMP_BARRIER_CYCLE );

        delete ct;
    }
    //phaseCycles = pdb->get(phaseContext,CYCLES);
    phaseCycles = pdb->get( phaseContext, PSC_EXECUTION_TIME );

    maxWaitTime = waitCycles[ 0 ];
    minWaitTime = waitCycles[ 0 ];
    avg_threads = 0.0;

    for( int i = 1; i < appl->getOmpThreads(); i++ ) {
        if( waitCycles[ i ] > maxWaitTime ) {
            maxWaitTime = waitCycles[ i ];
        }
        if( waitCycles[ i ] < minWaitTime ) {
            minWaitTime = waitCycles[ i ];
        }
    }
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        if( waitCycles[ i ] != maxWaitTime ) {
            avg_threads = avg_threads + waitCycles[ i ];
        }
    }

    avg_threads = avg_threads / ( appl->getOmpThreads() - 1 );
}

std::string ImbalanceOMPBarrierProp::toXMLExtra() {
    std::stringstream stream;
    stream << "\t\t<ExecTime>" << maxWaitTime << "</ExecTime>" << std::endl;
    return stream.str();
}

Property* ImbalanceOMPBarrierProp::clone() {
    ImbalanceOMPBarrierProp* prop = new ImbalanceOMPBarrierProp( context, phaseContext );
    return prop;
}
