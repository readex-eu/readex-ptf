/**
   @file    ConvIntFloatProp.cc
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

#include "ConvIntFloatProp.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

PropertyID ConvIntFloatProp::id() {
    return CONVERSIONINTFLOAT;
}

void ConvIntFloatProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool ConvIntFloatProp::condition() const {
    return convInstr > threshold;
}

double ConvIntFloatProp::confidence() const {
    return 1.0;
}

double ConvIntFloatProp::severity() const {
    //Can you estimate the percentage of the execution time
    //for the conversion on the time for the entire loop body?
    double percentConvInstr = ( double )convInstr / ( double )instrInBody;
    INT64  totalConvInstr   = totalInstructionsIssued * convInstr;
    return ( double )( totalConvInstr * 20 ) / phaseCycles * 100;
}

Context* ConvIntFloatProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type ConvIntFloatProp::request_metrics() {
    pdb->request( context, PSC_INT_FLOAT_CONVERSION );
    pdb->request( context, PSC_PAPI_TOT_INS );
    pdb->request( context, PSC_INSTRUCTIONS_IN_LOOP_BODY );
    pdb->request( phaseContext, PSC_PAPI_TOT_CYC );
    return ALL_INFO_GATHERED;
}

std::string ConvIntFloatProp::name() {
    return "Conversion of Integer to Float or vice versa";
}

void ConvIntFloatProp::evaluate() {
    convInstr               = pdb->get( context, PSC_INT_FLOAT_CONVERSION );
    totalInstructionsIssued = pdb->get( context, PSC_PAPI_TOT_INS );
    instrInBody             = pdb->get( context, PSC_INSTRUCTIONS_IN_LOOP_BODY );
    phaseCycles             = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );
}

Property* ConvIntFloatProp::clone() {
    ConvIntFloatProp* prop = new ConvIntFloatProp( context, phaseContext );
    return prop;
}

std::string ConvIntFloatProp::info() {
    std::stringstream stream;

    stream << '\t' << " conversion instructions in loop body: " << convInstr;

    return stream.str();
}
;

std::string ConvIntFloatProp::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<convInstr>" << convInstr << "</convInstr>" << std::endl;

    return stream.str();
}
