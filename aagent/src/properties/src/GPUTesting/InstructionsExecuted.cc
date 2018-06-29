/**
   @file    InstructionsExecuted.cc
   @ingroup CUDAProperties
   @brief   GPU Number of instructions executed property
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

#include "InstructionsExecuted.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

//Description :  Tells you the total number of instructions executed on GPU
//CUDA Formula:  PSC_CUPTI_DD_INST_EXECUTED
//Recommendation :
//


InstructionsExecuted::InstructionsExecuted( Context* ct, Context* phaseCt ) : Property( ct ) {
    evaluationResult = 0.0;
    phaseContext     = phaseCt;
}

InstructionsExecuted::~InstructionsExecuted() {
}

PropertyID InstructionsExecuted::id() {
    return CUDA_INSTRUCTIONSEXECUTED;
}

void InstructionsExecuted::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread "
              << context->getThread() << std::endl << "                  "
              << context->getRegion()->str_print() << std::endl;
}

bool InstructionsExecuted::condition() const {
    return Instructions > 0 && severity() > 0;
}

double InstructionsExecuted::confidence() const {
    return 1.0;
}

double InstructionsExecuted::severity() const {
    return evaluationResult;
}

Context* InstructionsExecuted::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type InstructionsExecuted::request_metrics() {
    pdb->request( context, PSC_CUPTI_DD_INST_EXECUTED );

    return ALL_INFO_GATHERED;
}

std::string InstructionsExecuted::name() {
    return "Instructions executed";
}

void InstructionsExecuted::evaluate() {
    Instructions = pdb->get( context, PSC_CUPTI_DD_INST_EXECUTED );

    evaluationResult = ( double )( Instructions );
}

Property* InstructionsExecuted::clone() {
    InstructionsExecuted* prop = new InstructionsExecuted( context, phaseContext );
    return prop;
}

std::string InstructionsExecuted::info() {
    std::stringstream stream;

    stream << '\t' << " Instructions: " << Instructions;

    return stream.str();
}

std::string InstructionsExecuted::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<Instructions>" << Instructions << "</Instructions>" << std::endl;

    return stream.str();
}
