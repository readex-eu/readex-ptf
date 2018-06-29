/**
   @file    InstructionStarvation.cc
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

#include "InstructionStarvation.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

PropertyID InstructionStarvation::id() {
    return INSTRUCTION_STARVATION;
}

void InstructionStarvation::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool InstructionStarvation::condition() const {
    return cycles > 0 && severity() > threshold;
}

double InstructionStarvation::confidence() const {
    return 1.0;
}

double InstructionStarvation::severity() const {
    //printf("Severity: Cycles=%ld   Phase cycles=%ld  evaluation result: %f  severity: %f\n\n\n", cycles, phaseCycles, evaluationResult, evaluationResult*(cycles/phaseCycles));
    return evaluationResult * ( cycles / phaseCycles );
}

Context* InstructionStarvation::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type InstructionStarvation::request_metrics() {
    pdb->request( context, PSC_NP_UOPS_EXECUTED_PORT015 );
    pdb->request( context, PSC_NP_THREAD_P );
    pdb->request( context, PSC_PAPI_TOT_CYC );
    pdb->request( phaseContext, PSC_PAPI_TOT_CYC );
    pdb->request( context, PSC_NP_STALL_CYCLES );
    pdb->request( context, PSC_NP_RESOURCE_STALLS_ANY );

    return ALL_INFO_GATHERED;
}

std::string InstructionStarvation::name() {
    return "Instruction Starvation";
}

void InstructionStarvation::evaluate() {
    phaseCycles = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );
    cycles      = pdb->get( context, PSC_PAPI_TOT_CYC );

    stallCycles      = pdb->get( context, PSC_NP_STALL_CYCLES );
    resourceStallAny = pdb->get( context, PSC_NP_RESOURCE_STALLS_ANY );
    uopsIssuedAny    = pdb->get( context, PSC_NP_UOPS_ISSUED_ANY );

    //printf("Evaluate: Cycles=%ld   Phase cycles=%ld   stall cylces= %ld    Resource stalls: %ld   uops_issued: %ld   result: %f \n\n\n", cycles, phaseCycles, stallCycles, resourceStallAny, uopsIssuedAny, ((double)((double)stallCycles - (double)resourceStallAny) /  (double) uopsIssuedAny));
    evaluationResult = ( ( double )stallCycles - ( double )resourceStallAny ) / ( double )uopsIssuedAny;
}

Property* InstructionStarvation::clone() {
    InstructionStarvation* prop = new InstructionStarvation( context, phaseContext );
    return prop;
}

std::string InstructionStarvation::info() {
    std::stringstream stream;

    stream << '\t' << " STALL_CYCLES: " << stallCycles << '\t' << " RESOURCE_STALLS_ANY: " << resourceStallAny << '\t'
           << " UOPS_ISSUED_ANY: " << uopsIssuedAny;

    return stream.str();
}

std::string InstructionStarvation::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<STALL_CYCLES>" << stallCycles << "</STALL_CYCLES>" << std::endl;
    stream << "\t\t<RESOURCE_STALLS_ANY>" << resourceStallAny << "</RESOURCE_STALLS_ANY>" << std::endl;
    stream << "\t\t<UOPS_ISSUED_ANY>" << uopsIssuedAny << "</UOPS_ISSUED_ANY>" << std::endl;

    stream << "\t\t<phaseCycles>" << phaseCycles << "</phaseCycles>" << std::endl;
    stream << "\t\t<cycles>" << cycles << "</cycles>" << std::endl;

    return stream.str();
}
