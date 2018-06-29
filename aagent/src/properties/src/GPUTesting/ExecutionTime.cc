/**
   @file    ExecutionTime.cc
   @ingroup CUDAProperties
   @brief   GPU kernel execution time property
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


#include "ExecutionTime.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

//Description :  Tells you time spent executing CUDA functions
//CUDA Formula:  PSC_CUDA_KERNEL_EXECUTION_TIME
//Recommendation :
//


ExecutionTime::ExecutionTime( Context* ct, Context* phaseCt ) : Property( ct ) {
    evaluationResult = 0.0;
    phaseContext     = phaseCt;
}

ExecutionTime::~ExecutionTime() {
}

PropertyID ExecutionTime::id() {
    return CUDA_EXECUTIONTIME;
}

void ExecutionTime::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread "
              << context->getThread() << std::endl << "                  "
              << context->getRegion()->str_print() << std::endl;
}

bool ExecutionTime::condition() const {
    return Execution_Time > 0 && severity() > 0;
}

double ExecutionTime::confidence() const {
    return 1.0;
}

double ExecutionTime::severity() const {
    return evaluationResult;
}

Context* ExecutionTime::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type ExecutionTime::request_metrics() {
    pdb->request( context, PSC_CUDA_KERNEL_EXECUTION_TIME );

    return ALL_INFO_GATHERED;
}

std::string ExecutionTime::name() {
    return "Execution time";
}

void ExecutionTime::evaluate() {
    Execution_Time = pdb->get( context, PSC_CUDA_KERNEL_EXECUTION_TIME );

    evaluationResult = ( double )( Execution_Time );
}

Property* ExecutionTime::clone() {
    ExecutionTime* prop = new ExecutionTime( context, phaseContext );
    return prop;
}

std::string ExecutionTime::info() {
    std::stringstream stream;

    stream << '\t' << " Execution_Time: " << Execution_Time;

    return stream.str();
}

std::string ExecutionTime::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<Execution_Time>" << Execution_Time << "</Execution_Time>" << std::endl;

    return stream.str();
}
