/**
   @file    AdvanceSpeculativeLoadProp.cc
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

#include "AdvanceSpeculativeLoadProp.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

PropertyID AdvanceSpeculativeLoadProp::id() {
    return ADVANCESPECULATIVELOAD;
}

void AdvanceSpeculativeLoadProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool AdvanceSpeculativeLoadProp::condition() const {
    return storesALAT > threshold;
}

double AdvanceSpeculativeLoadProp::confidence() const {
    return 1.0;
}

double AdvanceSpeculativeLoadProp::severity() const {
    double percentStoresALAT = ( double )storesALAT / ( double )instrInBody;
    totalStoresALAT = ( INT64 )totalInstructionsIssued * percentStoresALAT;
    return ( double )( totalStoresALAT * 2 ) / ( double )phaseCycles * 100;
    return 100.0;
}

Context* AdvanceSpeculativeLoadProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type AdvanceSpeculativeLoadProp::request_metrics() {
    pdb->request( context, PSC_ADVANCE_LOAD );
    pdb->request( context, PSC_PAPI_TOT_INS );
    pdb->request( context, PSC_INSTRUCTIONS_IN_LOOP_BODY );
    pdb->request( phaseContext, PSC_PAPI_TOT_CYC );
    return ALL_INFO_GATHERED;
}

std::string AdvanceSpeculativeLoadProp::name() {
    return "Advance speculative load instructions";
}

void AdvanceSpeculativeLoadProp::evaluate() {
    storesALAT              = pdb->get( context, PSC_ADVANCE_LOAD );
    totalInstructionsIssued = pdb->get( context, PSC_PAPI_TOT_INS );
    instrInBody             = pdb->get( context, PSC_INSTRUCTIONS_IN_LOOP_BODY );
    phaseCycles             = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );
}

Property* AdvanceSpeculativeLoadProp::clone() {
    AdvanceSpeculativeLoadProp* prop = new AdvanceSpeculativeLoadProp( context, phaseContext );
    return prop;
}

std::string AdvanceSpeculativeLoadProp::info() {
    std::stringstream stream;

    stream << '\t' << " Stores going to ALAT: " << storesALAT << "  Total: " << totalStoresALAT;

    return stream.str();
}
;

std::string AdvanceSpeculativeLoadProp::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<storesALAT>" << storesALAT << "</storesALAT>" << std::endl;
    stream << "\t\t<totalStoresALAT>" << totalStoresALAT << "</totalStoresALAT>" << std::endl;

    return stream.str();
}
