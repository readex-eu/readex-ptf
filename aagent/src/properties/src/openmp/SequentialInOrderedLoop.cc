/**
   @file    SequentialInOrderedLoop.cc
   @ingroup OpenMPProperties
   @brief   OpenMP Sequential Computation in Ordered Loop property
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

#include "SequentialInOrderedLoop.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"
#include <sstream>

PropertyID SequentialInOrderedLoopProp::id() {
    return SEQUENTIALINORDEREDLOOP;
}

void SequentialInOrderedLoopProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl;
}

bool SequentialInOrderedLoopProp::condition() const {
    /*
       std::cout << "phaseCycles: " << phaseCycles << "\n";
       for (int i = 0; i<appl->getOmpThreads(); i++) {
       std::cout << waitCycles[i] << "\t" ;
       }
       std::cout <<std::endl;

     */
    return orderedRegionCycles / ( double )phaseCycles * 100 > threshold;
}

double SequentialInOrderedLoopProp::confidence() const {
    return 1.0;
}

double SequentialInOrderedLoopProp::severity() const {
    if( ( orderedRegionCycles / ( double )phaseCycles * 100 ) > 100.0 ) {
        return 100;
    }
    else {
        return orderedRegionCycles / ( double )phaseCycles * 100;
    }
}

Context* SequentialInOrderedLoopProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type SequentialInOrderedLoopProp::request_metrics() {
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        pdb->request( ct, PSC_ORDERED_REGION_CYCLE );
    }
    //pdb->request(phaseContext, PSC_PAPI_TOT_CYC);
    pdb->request( phaseContext, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string SequentialInOrderedLoopProp::name() {
    return "Sequential Computation in ORDERED loop overhead";
}

void SequentialInOrderedLoopProp::evaluate() {
    waitCycles.resize( appl->getOmpThreads(), 0 );
    orderedRegionCycles = 0.0;
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );

        waitCycles[ i ]     = pdb->get( ct, PSC_ORDERED_REGION_CYCLE );
        orderedRegionCycles = orderedRegionCycles + waitCycles[ i ]; //Total the ordered region time
        delete ct;
    }
    //phaseCycles = pdb->get(phaseContext, PSC_PAPI_TOT_CYC);
    phaseCycles = pdb->get( phaseContext, PSC_EXECUTION_TIME );
}

std::string SequentialInOrderedLoopProp::toXMLExtra() {
    std::stringstream stream;
    stream << "\t\t<ExecTime>" << orderedRegionCycles << "</ExecTime>" << std::endl;
    return stream.str();
}

Property* SequentialInOrderedLoopProp::clone() {
    SequentialInOrderedLoopProp* prop = new SequentialInOrderedLoopProp( context, phaseContext );
    return prop;
}
