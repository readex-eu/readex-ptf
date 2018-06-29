/**
   @file    TransferTime.cc
   @ingroup CUDAProperties
   @brief   GPU Time to transfer data from host to device property
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

#include "TransferTime.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

//Description :  Tells you time spent executing CUDA functions
//CUDA Formula:  PSC_CUDA_TRANSFER_EXECUTION_TIME
//Recommendation :
//


TransferTime::TransferTime( Context* ct, Context* phaseCt ) : Property( ct ) {
    evaluationResult = 0.0;
    phaseContext     = phaseCt;
}

TransferTime::~TransferTime() {
}

PropertyID TransferTime::id() {
    return CUDA_TRANSFERTIME;
}

void TransferTime::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread "
              << context->getThread() << std::endl << "                  "
              << context->getRegion()->str_print() << std::endl;
}

bool TransferTime::condition() const {
    return Transfer_Time > 0 && severity() > 0;
}

double TransferTime::confidence() const {
    return 1.0;
}

double TransferTime::severity() const {
    return evaluationResult;
}

Context* TransferTime::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type TransferTime::request_metrics() {
    pdb->request( context, PSC_CUDA_TRANSFER_EXECUTION_TIME );

    return ALL_INFO_GATHERED;
}

std::string TransferTime::name() {
    return "Transfer time";
}

void TransferTime::evaluate() {
    Transfer_Time = pdb->get( context, PSC_CUDA_TRANSFER_EXECUTION_TIME );

    evaluationResult = ( double )( Transfer_Time );
}

Property* TransferTime::clone() {
    TransferTime* prop = new TransferTime( context, phaseContext );
    return prop;
}

std::string TransferTime::info() {
    std::stringstream stream;

    stream << '\t' << " Transfer_Time: " << Transfer_Time;

    return stream.str();
}

std::string TransferTime::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<Transfer_Time>" << Transfer_Time << "</Transfer_Time>" << std::endl;

    return stream.str();
}
