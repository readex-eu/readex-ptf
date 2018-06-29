/**
   @file    MissingPrefetchProp.cc
   @ingroup Itanium2Properites
   @brief   Itanium2 specific property
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

#include "MissingPrefetchProp.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

PropertyID MissingPrefetchProp::id() {
    return MISSINGPREFETCH;
}

void MissingPrefetchProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool MissingPrefetchProp::condition() const {
    //currently you give the number of prefetch instructions.
    //Are these prefetches also for FP values?
    //Can you give the number of LDs that are not prefetched?
    return prefetch > threshold;
}

double MissingPrefetchProp::confidence() const {
    return 1.0;
}

double MissingPrefetchProp::severity() const {
    //Here we can probably give back the cycles from
    //PSC_BE_EXE_BUBBLE_ALL which are for data transfers
    //to registers
    return ( double )stalls / ( double )phaseCycles * 100;
}

Context* MissingPrefetchProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type MissingPrefetchProp::request_metrics() {
    pdb->request( context, PSC_PREFETCH );
    pdb->request( context, PSC_BE_EXE_BUBBLE_ALL );
    pdb->request( phaseContext, PSC_PAPI_TOT_CYC );
    return ALL_INFO_GATHERED;
}

std::string MissingPrefetchProp::name() {
    return "Missing prefetch instructions";
}

void MissingPrefetchProp::evaluate() {
    prefetch    = pdb->get( context, PSC_PREFETCH );
    stalls      = pdb->get( context, PSC_BE_EXE_BUBBLE_ALL );
    phaseCycles = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );
}

Property* MissingPrefetchProp::clone() {
    MissingPrefetchProp* prop = new MissingPrefetchProp( context, phaseContext );
    return prop;
}

std::string MissingPrefetchProp::info() {
    std::stringstream stream;

    stream << '\t' << " non-prefetched load instructions: " << prefetch;

    return stream.str();
}
;

std::string MissingPrefetchProp::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<nonprefetch>" << prefetch << "</nonprefetch>" << std::endl;

    return stream.str();
}
