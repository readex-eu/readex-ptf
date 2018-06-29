/**
   @file    ActiveCycles.cc
   @ingroup CUDAProperties
   @brief   GPU Active Cycles property header
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


#include "ActiveCycles.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

//Description :  Tells you the total number of cycles a multiprocessor has at least one active warp
//CUDA Formula:  PSC_CUPTI_DD_ACTIVE_CYCLES
//Recommendation :
//


ActiveCycles::ActiveCycles( Context* ct, Context* phaseCt ) : Property( ct ) {
    evaluationResult = 0.0;
    phaseContext     = phaseCt;
}

ActiveCycles::~ActiveCycles() {
}

PropertyID ActiveCycles::id() {
    return CUDA_ACTIVECYCLES;
}

void ActiveCycles::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread "
              << context->getThread() << std::endl << "                  "
              << context->getRegion()->str_print() << std::endl;
}

bool ActiveCycles::condition() const {
    return Active_Cycles > 0 && severity() > 0;
}

double ActiveCycles::confidence() const {
    return 1.0;
}

double ActiveCycles::severity() const {
    return evaluationResult;
}

Context* ActiveCycles::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type ActiveCycles::request_metrics() {
    pdb->request( context, PSC_CUPTI_DD_ACTIVE_CYCLES );

    return ALL_INFO_GATHERED;
}

std::string ActiveCycles::name() {
    return "Active cycles";
}

void ActiveCycles::evaluate() {
    Active_Cycles = pdb->get( context, PSC_CUPTI_DD_ACTIVE_CYCLES );

    evaluationResult = ( double )( Active_Cycles );
}

Property* ActiveCycles::clone() {
    ActiveCycles* prop = new ActiveCycles( context, phaseContext );
    return prop;
}

std::string ActiveCycles::info() {
    std::stringstream stream;

    stream << '\t' << " Active_Cycles: " << Active_Cycles;

    return stream.str();
}

std::string ActiveCycles::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<Active_Cycles>" << Active_Cycles << "</Active_Cycles>" << std::endl;

    return stream.str();
}
