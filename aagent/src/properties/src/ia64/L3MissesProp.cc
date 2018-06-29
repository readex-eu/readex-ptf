/**
   @file    L3MissesProp.cc
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

#include "L3MissesProp.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

PropertyID L3MissProp::id() {
    return L3MISS;
}

void L3MissProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool L3MissProp::condition() const {
    return importance > 0 && severity() > threshold;
}

double L3MissProp::confidence() const {
    return 1.0;
}

double L3MissProp::severity() const {
    return ( double )L3StallCycles / ( double )phaseCycles * 100;
}

Context* L3MissProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type L3MissProp::request_metrics() {
    pdb->request( context, PSC_BE_EXE_BUBBLE_GRALL );
    pdb->request( context, PSC_BE_EXE_BUBBLE_GRGR );
    pdb->request( context, PSC_BE_EXE_BUBBLE_FRALL );
    pdb->request( context, PSC_PAPI_L1_DCM );
    pdb->request( context, PSC_PAPI_L2_DCM );
    pdb->request( context, PSC_PAPI_L3_TCM );
    pdb->request( phaseContext, PSC_PAPI_TOT_CYC );
    return ALL_INFO_GATHERED;
}

std::string L3MissProp::name() {
    return "L3 misses";
}

void L3MissProp::evaluate() {
    stallCycles = pdb->get( context, PSC_BE_EXE_BUBBLE_GRALL ) - pdb->get( context, PSC_BE_EXE_BUBBLE_GRGR )
                  + pdb->get( context, PSC_BE_EXE_BUBBLE_FRALL );
    phaseCycles   = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );
    L1Misses      = pdb->get( context, PSC_PAPI_L1_DCM );
    L2Misses      = pdb->get( context, PSC_PAPI_L2_DCM );
    L3Misses      = pdb->get( context, PSC_PAPI_L3_TCM );
    importance    = ( double )( L3Misses * 200 ) / ( double )( L1Misses * 7 + L2Misses * 14 + L3Misses * 200 );
    L3StallCycles = ( INT64 )( ( double )stallCycles * importance );
}

Property* L3MissProp::clone() {
    L3MissProp* prop = new L3MissProp( context, phaseContext );
    return prop;
}

std::string L3MissProp::info() {
    std::stringstream stream;

    stream << '\t' << " L1Misses: " << L1Misses << '\t' << " L2Misses: " << L2Misses << '\t' << " L3Misses: " << L3Misses;

    return stream.str();
}
;

std::string L3MissProp::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<L1Misses>" << L1Misses << "</L1Misses>" << std::endl;
    stream << "\t\t<L2Misses>" << L2Misses << "</L2Misses>" << std::endl;
    stream << "\t\t<L3Misses>" << L3Misses << "</L3Misses>" << std::endl;

    return stream.str();
}
