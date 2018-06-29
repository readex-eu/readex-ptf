/**
   @file    LongLatencyInstructionException.cc
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

#include "LongLatencyInstructionException.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

// Percentage of instructions that are translated into uops by the sequencer

PropertyID LongLatencyInstructionException::id() {
    return LONG_LATENCY_INSTRUCTION_EXCEPTION;
}

void LongLatencyInstructionException::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool LongLatencyInstructionException::condition() const {
    return cycles > 0 && severity() > threshold;
}

double LongLatencyInstructionException::confidence() const {
    return 1.0;
}

double LongLatencyInstructionException::severity() const {
    return evaluationResult * ( cycles / phaseCycles );
}

Context* LongLatencyInstructionException::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type LongLatencyInstructionException::request_metrics() {
    pdb->request( context, PSC_NP_UOPS_DECODED_MS );
    pdb->request( context, PSC_NP_UOPS_RETIRED_ANY );
    pdb->request( context, PSC_PAPI_TOT_CYC );
    pdb->request( phaseContext, PSC_PAPI_TOT_CYC );

    return ALL_INFO_GATHERED;
}

std::string LongLatencyInstructionException::name() {
    return "Long Latency Instruction Exception";
}

void LongLatencyInstructionException::evaluate() {
    phaseCycles = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );
    cycles      = pdb->get( context, PSC_PAPI_TOT_CYC );

    uopsDecodedMS  = pdb->get( context, PSC_NP_UOPS_DECODED_MS );
    uopsRetiredAny = pdb->get( context, PSC_NP_UOPS_RETIRED_ANY );

    evaluationResult = ( double )uopsRetiredAny / ( double )uopsDecodedMS;
}

Property* LongLatencyInstructionException::clone() {
    LongLatencyInstructionException* prop = new LongLatencyInstructionException( context, phaseContext );
    return prop;
}

std::string LongLatencyInstructionException::info() {
    std::stringstream stream;

    stream << '\t' << " UOPS_DECODED_MS: " << uopsDecodedMS << '\t' << " UOPS_RETIRED_ANY: " << uopsRetiredAny;

    return stream.str();
}

std::string LongLatencyInstructionException::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<UOPS_DECODED_MS>" << uopsDecodedMS << "</UOPS_DECODED_MS>" << std::endl;
    stream << "\t\t<UOPS_RETIRED_ANY>" << uopsRetiredAny << "</UOPS_RETIRED_ANY>" << std::endl;

    stream << "\t\t<phaseCycles>" << phaseCycles << "</phaseCycles>" << std::endl;
    stream << "\t\t<cycles>" << cycles << "</cycles>" << std::endl;

    return stream.str();
}
