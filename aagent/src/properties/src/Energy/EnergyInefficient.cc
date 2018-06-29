/**
   @file    EnergyInefficient.cc
   @ingroup EnergyProperties
   @brief   Return energy spent per floating point instruction, indicating energy inefficient regions, as severity.
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

#include "EnergyInefficient.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "psc_errmsg.h"
#include <sstream>

//Description :
//ENOPT Formula:  PSC_ENOPT_ALL_CORES/PSC_PAPI_FP_INS
//Recommendation :
//


EnergyInefficient::EnergyInefficient( Context* ct, Context* phaseCt, double threshold ) : Property( ct ), threshold( threshold ) {
    evaluationResult = 0.0;
    phaseContext     = phaseCt;
}

EnergyInefficient::~EnergyInefficient() {
}

PropertyID EnergyInefficient::id() {
    return ENERGY_INEFFICIENT;
}

void EnergyInefficient::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread "
              << context->getThread() << std::endl << "                  "
              << context->getRegion()->str_print() << std::endl;
}

bool EnergyInefficient::condition() const {
    return severity() > threshold;
}

double EnergyInefficient::confidence() const {
    return 1.0;
}

double EnergyInefficient::severity() const {
    return evaluationResult;
}

Context* EnergyInefficient::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type EnergyInefficient::request_metrics() {
    pdb->request( context, PSC_ENOPT_ALL_CORES );
    pdb->request( context, PSC_PAPI_FP_INS );

    return ALL_INFO_GATHERED;
}

std::string EnergyInefficient::name() {
    return "Energy Inefficiency";
}

void EnergyInefficient::evaluate() {
    ConsumedEnergy = pdb->get( context, PSC_ENOPT_ALL_CORES );
    FLOPS          = pdb->get( context, PSC_PAPI_FP_INS );

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ), "EnergyInefficient: Got energy consumption: %ld\n", ConsumedEnergy );

    if( ConsumedEnergy > 0 ) {
        FlopsPerJoule    = ( INT64 )( ( FLOPS * 1. ) / ( ConsumedEnergy * 1. ) );
        evaluationResult = ( double )FLOPS / ( double )ConsumedEnergy;
    }
    else {
        FlopsPerJoule    = 0;
        evaluationResult = 0.0;
    }
}

Property* EnergyInefficient::clone() {
    EnergyInefficient* prop = new EnergyInefficient( context, phaseContext, threshold );
    return prop;
}

std::string EnergyInefficient::info() {
    std::stringstream stream;

    stream << '\t' << " Flops per joule: " << FlopsPerJoule;

    return stream.str();
}

std::string EnergyInefficient::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<ENERGY_CONSUMPTION>" << ConsumedEnergy << "</ENERGY_CONSUMPTION>" << std::endl;
    stream << "\t\t<FLOPS>" << FLOPS << "</FLOPS>" << std::endl;
    stream << "\t\t<ENERGY_INEFFICIENCY>" << FlopsPerJoule << "</ENERGY_INEFFICIENCY>" << std::endl;

    return stream.str();
}
