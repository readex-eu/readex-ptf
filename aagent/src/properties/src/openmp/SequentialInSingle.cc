/**
   @file    SequentialInSingle.cc
   @ingroup OpenMPProperties
   @brief   OpenMP Sequential Computation in Single Region property
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

#include "SequentialInSingle.h"
#include "global.h"
#include "PropertyID.h"
#include "Property.h" //sss
#include <iostream>
#include <vector>
#include "psc_errmsg.h"
#include <sstream>

PropertyID SequentialInSingleProp::id() {
    return SEQUENTIALINSINGLE;
}

void SequentialInSingleProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl;
}

bool SequentialInSingleProp::condition() const {
    return ( double )singleRegionCycles / ( double )phaseCycles * 100 > threshold;
}

double SequentialInSingleProp::confidence() const {
    return 1.0;
}

double SequentialInSingleProp::severity() const {
    if( ( singleRegionCycles / ( double )phaseCycles * 100 ) > 100.0 ) {
        return 100;
    }
    else {
        return singleRegionCycles / ( double )phaseCycles * 100;
    }
}

Context* SequentialInSingleProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type SequentialInSingleProp::request_metrics() {
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        pdb->request( ct, PSC_SINGLE_REGION_CYCLE );
    }
    //pdb->request(phaseContext, PSC_PAPI_TOT_CYC);
    pdb->request( phaseContext, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string SequentialInSingleProp::name() {
    return "Sequential computation in SINGLE region Overhead";
}

void SequentialInSingleProp::evaluate() {
    waitCycles.resize( appl->getOmpThreads(), 0 );
    singleRegionCycles = 0.0;
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        waitCycles[ i ] = pdb->get( ct, PSC_SINGLE_REGION_CYCLE );
        delete ct;
    }
    phaseCycles = pdb->get( phaseContext, PSC_EXECUTION_TIME );

    singleRegionCycles = waitCycles[ 0 ];
    for( int i = 1; i < appl->getOmpThreads(); i++ ) {
        if( ( double )waitCycles[ i ] > ( double )singleRegionCycles ) {
            singleRegionCycles = waitCycles[ i ];
        }
    }
}

std::string SequentialInSingleProp::toXMLExtra() {
    std::stringstream stream;
    stream << "\t\t<ExecTime>" << singleRegionCycles << "</ExecTime>" << std::endl;
    return stream.str();
}

Property* SequentialInSingleProp::clone() {
    SequentialInSingleProp* prop = new SequentialInSingleProp( context, phaseContext );
    return prop;
}
