/**
   @file    L2MissesProp.cc
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

#include "L2MissesProp.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

PropertyID L2MissProp::id() {
    return L2MISS;
}

void L2MissProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool L2MissProp::condition() const {
    return importance > 0 && severity() > threshold;
}

double L2MissProp::confidence() const {
    return 1.0;
}

double L2MissProp::severity() const {
    return ( double )L2StallCycles / ( double )phaseCycles * 100;
}

Context* L2MissProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type L2MissProp::request_metrics() {
    pdb->request( context, PSC_BE_EXE_BUBBLE_GRALL );
    pdb->request( context, PSC_BE_EXE_BUBBLE_GRGR );
    pdb->request( context, PSC_BE_EXE_BUBBLE_FRALL );
    pdb->request( context, PSC_PAPI_L1_DCM );
    pdb->request( context, PSC_PAPI_L2_DCM );
    pdb->request( context, PSC_PAPI_L3_TCM );
    pdb->request( phaseContext, PSC_PAPI_TOT_CYC );
    return ALL_INFO_GATHERED;
}

std::string L2MissProp::name() {
    return "L2 misses";
}

void L2MissProp::evaluate() {
    stallCycles = pdb->get( context, PSC_BE_EXE_BUBBLE_GRALL ) - pdb->get( context, PSC_BE_EXE_BUBBLE_GRGR )
                  + pdb->get( context, PSC_BE_EXE_BUBBLE_FRALL );
    phaseCycles   = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );
    L1Misses      = pdb->get( context, PSC_PAPI_L1_DCM );
    L2Misses      = pdb->get( context, PSC_PAPI_L2_DCM );
    L3Misses      = pdb->get( context, PSC_PAPI_L3_TCM );
    importance    = ( double )( L2Misses * 14 ) / ( double )( L1Misses * 7 + L2Misses * 14 + L3Misses * 200 );
    L2StallCycles = ( INT64 )( ( double )stallCycles * importance );
}

Property* L2MissProp::clone() {
    L2MissProp* prop = new L2MissProp( context, phaseContext );
    return prop;
}

std::string L2MissProp::info() {
    std::stringstream stream;

    stream << '\t' << " L1Misses: " << L1Misses << '\t' << " L2Misses: " << L2Misses << '\t' << " L3Misses: " << L3Misses;

    return stream.str();
}

std::string L2MissProp::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<L1Misses>" << L1Misses << "</L1Misses>" << std::endl;
    stream << "\t\t<L2Misses>" << L2Misses << "</L2Misses>" << std::endl;
    stream << "\t\t<L3Misses>" << L3Misses << "</L3Misses>" << std::endl;

    return stream.str();
}
