/**
   @file    Misprediction.cc
   @ingroup WestmereProperties
   @brief   Westmere specific property
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

#include "Misprediction.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

PropertyID Misprediction::id() {
    return MISPREDICTION;
}

void Misprediction::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool Misprediction::condition() const {
    return cycles > 0 && severity() > threshold;
}

double Misprediction::confidence() const {
    return 1.0;
}

double Misprediction::severity() const {
    return evaluationResult * ( cycles / phaseCycles );
}

Context* Misprediction::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type Misprediction::request_metrics() {
    pdb->request( context, PSC_NP_UOPS_ISSUED_FUSED );
    pdb->request( context, PSC_NP_UOPS_ISSUED_ANY );
    pdb->request( context, PSC_NP_UOPS_RETIRED_ANY );

    pdb->request( context, PSC_PAPI_TOT_CYC );
    pdb->request( phaseContext, PSC_PAPI_TOT_CYC );

    return ALL_INFO_GATHERED;
}

std::string Misprediction::name() {
    return "Misprediction";
}

void Misprediction::evaluate() {
    phaseCycles = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );

    cycles = pdb->get( context, PSC_PAPI_TOT_CYC );

    uopsIssuedFused = pdb->get( context, PSC_NP_UOPS_ISSUED_FUSED );
    uopsIssuedAny   = pdb->get( context, PSC_NP_UOPS_ISSUED_ANY );
    uopsRetiredAny  = pdb->get( context, PSC_NP_UOPS_RETIRED_ANY );

    evaluationResult = ( ( double )uopsIssuedFused + ( double )uopsIssuedAny - ( double )uopsRetiredAny )
                       / ( ( double )uopsIssuedFused + ( double )uopsIssuedAny );
}

Property* Misprediction::clone() {
    Misprediction* prop = new Misprediction( context, phaseContext );
    return prop;
}

std::string Misprediction::info() {
    std::stringstream stream;

    stream << '\t' << " UOPS_ISSUED_FUSED: " << uopsIssuedFused << '\t' << " UOPS_ISSUED_ANY: " << uopsIssuedAny << '\t'
           << " UOPS_RETIRED_ANY: " << uopsRetiredAny;

    return stream.str();
}

std::string Misprediction::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<UOPS_ISSUED_FUSED>" << uopsIssuedFused << "</UOPS_ISSUED_FUSED>" << std::endl;
    stream << "\t\t<UOPS_ISSUED_ANY>" << uopsIssuedAny << "</UOPS_ISSUED_ANY>" << std::endl;
    stream << "\t\t<UOPS_RETIRED_ANY>" << uopsRetiredAny << "</UOPS_RETIRED_ANY>" << std::endl;

    stream << "\t\t<phaseCycles>" << phaseCycles << "</phaseCycles>" << std::endl;
    stream << "\t\t<cycles>" << cycles << "</cycles>" << std::endl;

    return stream.str();
}
