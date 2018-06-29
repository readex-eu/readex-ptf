/**
   @file    PowerUsed.cc
   @ingroup EnergyProperties
   @brief   Power Used property
   @author  Carmen Navarrete
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

#include "PowerUsed.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include "psc_errmsg.h"
#include <sstream>

//Description :  Tells you the power used by the code region
//ENOPT Formula:  PSC_ENOPT_ALL_CORES / PSC_EXECUTION_TIME
//Recommendation :
//


PowerUsed::PowerUsed( Context* ct, Context* phaseCt, double threshold ) : Property( ct ), threshold( threshold ) {
    phaseCycles  = 0;
    context      = ct;
    phaseContext = phaseCt;
    powerUsed    = 0.0;
}

PowerUsed::~PowerUsed() {
}

PropertyID PowerUsed::id() {
    return POWER_USED;
}

void PowerUsed::print() {
    std::cout << "Property:" << name()
              << "  Process " << context->getRank()
              << "  Thread " << context->getThread() << std::endl
              << "                  " << context->getRegion()->str_print() << std::endl;
}

bool PowerUsed::condition() const {
    return true;
}

double PowerUsed::confidence() const {
    return 1.0;
}

double PowerUsed::severity() const {
    return powerUsed;
}

Context* PowerUsed::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type PowerUsed::request_metrics() {
    Context* ct = new Context( context->getRegion(), context->getRank(), 0 );
    pdb->request( ct, PSC_ENOPT_ALL_CORES );
    pdb->request( ct, PSC_EXECUTION_TIME );

    return ALL_INFO_GATHERED;
}

std::string PowerUsed::name() {
    return "Power Used";
}

void PowerUsed::evaluate() {
    Context* ct = new Context( context->getRegion(), context->getRank(), 0 );

    double energyConsumption = pdb->get( ct, PSC_ENOPT_ALL_CORES );
    double runTime           =  pdb->get( ct, PSC_EXECUTION_TIME );

    powerUsed = energyConsumption / runTime;

    psc_dbgmsg( 1010, "Got energy consumption: %f\n", powerUsed );

    delete ct;
}

std::string PowerUsed::info() {
    std::stringstream stream;
    stream << '\t' << " POWER_USED: " << powerUsed;
    return stream.str();
}

Property* PowerUsed::clone() {
    PowerUsed* prop = new PowerUsed( context, phaseContext, threshold );
    return prop;
}
