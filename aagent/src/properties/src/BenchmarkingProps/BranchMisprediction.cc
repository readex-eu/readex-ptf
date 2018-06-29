/**
   @file    BranchMisprediction.cc
   @ingroup BenchmarkingProperties
   @brief   Branch Misprediction Property
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


#include "BranchMisprediction.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

PropertyID BranchMisprediction::id() {
    return BRANCH_MISPREDICTION;
}

void BranchMisprediction::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool BranchMisprediction::condition() const {
    if( PAPI_BR_M == -1 || PAPI_BR_C == -1 ) {
        return false;
    }
    else {
        return true;
    }
}

double BranchMisprediction::confidence() const {
    return 1.0;
}

double BranchMisprediction::severity() const {
    return evaluationResult * ( cycles / phaseCycles );
}

Context* BranchMisprediction::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type BranchMisprediction::request_metrics() {
    pdb->request( context, PSC_PAPI_BR_MSP );
    pdb->request( context, PSC_PAPI_BR_CN );

    pdb->request( context, PSC_PAPI_TOT_CYC );
    pdb->request( phaseContext, PSC_PAPI_TOT_CYC );

    return ALL_INFO_GATHERED;
}

std::string BranchMisprediction::name() {
    return "Branch Misprediction";
}

void BranchMisprediction::evaluate() {
    phaseCycles = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );
    cycles      = pdb->get( context, PSC_PAPI_TOT_CYC );

    PAPI_BR_M = pdb->get( context, PSC_PAPI_BR_MSP );
    PAPI_BR_C = pdb->get( context, PSC_PAPI_BR_CN );

    evaluationResult = ( double )PAPI_BR_M / ( double )PAPI_BR_C;
}

Property* BranchMisprediction::clone() {
    BranchMisprediction* prop = new BranchMisprediction( context, phaseContext );
    return prop;
}

std::string BranchMisprediction::info() {
    std::stringstream stream;

    stream << '\t' << " PAPI_BR_MSP: " << PAPI_BR_M << '\t' << " PAPI_BR_CN: " << PAPI_BR_C;

    return stream.str();
}

std::string BranchMisprediction::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PAPI_BR_MSP>" << PAPI_BR_M << "</PAPI_BR_MSP>" << std::endl;
    stream << "\t\t<PAPI_BR_CN>" << PAPI_BR_C << "</PAPI_BR_CN>" << std::endl;

    stream << "\t\t<phaseCycles>" << phaseCycles << "</phaseCycles>" << std::endl;
    stream << "\t\t<cycles>" << cycles << "</cycles>" << std::endl;

    return stream.str();
}
