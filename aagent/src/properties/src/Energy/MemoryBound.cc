/**
   @file    MemoryBound.cc
   @ingroup EnergyProperties
   @brief   Return number of L3 cache misses per FLOP, indicating memory bound regions, as severity.
   @author  Robert Mijakovic
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

#include "MemoryBound.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include "psc_errmsg.h"
#include <sstream>

//Description :
//ENOPT Formula:  PSC_PAPI_L3_TCM/PSC_PAPI_FP_INS
//Recommendation :
//


MemoryBound::MemoryBound( Context* ct, Context* phaseCt, double threshold ) : Property( ct ), threshold( threshold ) {
    phaseCycles          = 0;
    context              = ct;
    phaseContext         = phaseCt;
    L3CacheMissesPerFLOP = 0.0;
}

MemoryBound::~MemoryBound() {
}

PropertyID MemoryBound::id() {
    return MEMORY_BOUND;
}

void MemoryBound::print() {
    std::cout << "Property:" << name()
              << "  Process " << context->getRank()
              << "  Thread " << context->getThread() << std::endl
              << "                  " << context->getRegion()->str_print() << std::endl;
}

bool MemoryBound::condition() const {
    return true;
}

double MemoryBound::confidence() const {
    return 1.0;
}

double MemoryBound::severity() const {
    return L3CacheMissesPerFLOP;
}

Context* MemoryBound::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type MemoryBound::request_metrics() {
    Context* ct = new Context( context->getRegion(), context->getRank(), 0 );
    pdb->request( ct, PSC_PAPI_L3_TCM );
    pdb->request( ct, PSC_PAPI_FP_INS );

    return ALL_INFO_GATHERED;
}

std::string MemoryBound::name() {
    return "Memory bound";
}

void MemoryBound::evaluate() {
    Context* ct = new Context( context->getRegion(), context->getRank(), 0 );
    L3CacheMisses = pdb->get( ct, PSC_PAPI_L3_TCM );
    FLOPS         = pdb->get( ct, PSC_PAPI_FP_INS );

    if( FLOPS != 0 ) {
        L3CacheMissesPerFLOP = L3CacheMisses / FLOPS;
    }
    else {
        L3CacheMissesPerFLOP = 0.0;
    }

    psc_dbgmsg( 1010, "Got number of L3 cache misses per FLOP: %f\n", L3CacheMissesPerFLOP );

    delete ct;
}

std::string MemoryBound::info() {
    std::stringstream stream;

    stream << '\t' << "L3 cache misses per Flop: " << L3CacheMissesPerFLOP;

    return stream.str();
}

std::string MemoryBound::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<L3CACHE_MISSES>" << L3CacheMisses << "</L3CACHE_MISSES>" << std::endl;
    stream << "\t\t<FLOPS>" << FLOPS << "</FLOPS>" << std::endl;
    stream << "\t\t<MEMORY_BOUND>" << L3CacheMissesPerFLOP << "</MEMORY_BOUND>" << std::endl;

    return stream.str();
}


Property* MemoryBound::clone() {
    MemoryBound* prop = new MemoryBound( context, phaseContext, threshold );
    return prop;
}
