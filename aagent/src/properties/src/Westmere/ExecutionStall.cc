/**
   @file    ExecutionStall.cc
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

#include "ExecutionStall.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

PropertyID ExecutionStall::id() {
    return EXECUTIONSTALL;
}

void ExecutionStall::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool ExecutionStall::condition() const {
    return cycles > 0 && severity() > threshold;
}

double ExecutionStall::confidence() const {
    return 1.0;
}

double ExecutionStall::severity() const {
    return evaluationResult * ( cycles / phaseCycles );
}

Context* ExecutionStall::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type ExecutionStall::request_metrics() {
    pdb->request( context, PSC_NP_UOPS_EXECUTED_PORT015 );
    pdb->request( context, PSC_NP_THREAD_P );
    pdb->request( context, PSC_PAPI_TOT_CYC );
    pdb->request( phaseContext, PSC_PAPI_TOT_CYC );

    return ALL_INFO_GATHERED;
}

std::string ExecutionStall::name() {
    return "Execution Stall";
}

void ExecutionStall::evaluate() {
    phaseCycles = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );
    cycles      = pdb->get( context, PSC_PAPI_TOT_CYC );

    uops_Executed_Port015 = pdb->get( context, PSC_NP_UOPS_EXECUTED_PORT015 );
    Thread_P              = pdb->get( context, PSC_NP_THREAD_P );

    evaluationResult = ( double )uops_Executed_Port015 / ( double )Thread_P;
}

Property* ExecutionStall::clone() {
    ExecutionStall* prop = new ExecutionStall( context, phaseContext );
    return prop;
}

std::string ExecutionStall::info() {
    std::stringstream stream;

    stream << '\t' << " uops_Executed_Port015: " << uops_Executed_Port015 << '\t' << " Thread_P: " << Thread_P;

    return stream.str();
}

std::string ExecutionStall::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<uops_Executed_Port015>" << uops_Executed_Port015 << "</uops_Executed_Port015>" << std::endl;
    stream << "\t\t<Thread_P>" << Thread_P << "</Thread_P>" << std::endl;

    stream << "\t\t<phaseCycles>" << phaseCycles << "</phaseCycles>" << std::endl;
    stream << "\t\t<cycles>" << cycles << "</cycles>" << std::endl;

    return stream.str();
}
