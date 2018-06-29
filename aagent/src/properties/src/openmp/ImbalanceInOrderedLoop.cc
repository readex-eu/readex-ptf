/**
   @file    ImbalanceInOrderedLoop.cc
   @ingroup OpenMPProperties
   @brief   OpenMP Load Imbalance in Ordered Loop property
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

#include "ImbalanceInOrderedLoop.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"
#include <sstream>

PropertyID ImbalanceInOrderedLoopProp::id() {
    return IMBALANCE_IN_ORDERED_LOOP_OMP;
}

void ImbalanceInOrderedLoopProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl;
}

bool ImbalanceInOrderedLoopProp::condition() const {
    return ( maxWaitTime - minWaitTime ) / ( double )phaseCycles * 100 > threshold;
}

double ImbalanceInOrderedLoopProp::confidence() const {
    return 1.0;
}

double ImbalanceInOrderedLoopProp::severity() const {
    if( ( ( maxWaitTime - minWaitTime ) / ( double )phaseCycles * 100 ) > 100.0 ) {
        return 100;
    }
    else {
        return ( maxWaitTime - minWaitTime ) / ( double )phaseCycles * 100;
    }
}

Context* ImbalanceInOrderedLoopProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type ImbalanceInOrderedLoopProp::request_metrics() {
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        pdb->request( ct, PSC_ORDERED_REGION_CYCLE );
    }
    //pdb->request(phaseContext, PSC_PAPI_TOT_CYC);
    pdb->request( phaseContext, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string ImbalanceInOrderedLoopProp::name() {
    return "Imbalance in ORDERED loop";
}

void ImbalanceInOrderedLoopProp::evaluate() {
    waitCycles.resize( appl->getOmpThreads(), 0 );

    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );

        waitCycles[ i ] = pdb->get( ct, PSC_ORDERED_REGION_CYCLE );
        delete ct;
    }

    phaseCycles = pdb->get( phaseContext, PSC_EXECUTION_TIME );
    minWaitTime = waitCycles[ 0 ];
    maxWaitTime = waitCycles[ 0 ];
    for( int i = 1; i < appl->getOmpThreads(); i++ ) {
        if( waitCycles[ i ] > maxWaitTime ) {
            maxWaitTime = waitCycles[ i ];
        }
        else if( waitCycles[ i ] < minWaitTime ) { //there is a possibility for 0 execution time?
            minWaitTime = waitCycles[ i ];
        }
    }
}

Property* ImbalanceInOrderedLoopProp::clone() {
    ImbalanceInOrderedLoopProp* prop = new ImbalanceInOrderedLoopProp( context, phaseContext );
    return prop;
}

std::string ImbalanceInOrderedLoopProp::toXMLExtra() {
    std::stringstream stream;
    stream << "\t\t<ExecTime>" << maxWaitTime << "</ExecTime>" << std::endl;
    return stream.str();
}
