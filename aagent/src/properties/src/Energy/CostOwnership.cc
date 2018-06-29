/**
   @file    CostOwnership.cc
   @ingroup EnergyProperties
   @brief   Cost of Ownership property
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

#include "CostOwnership.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include "psc_errmsg.h"
#include <sstream>

//Description :  Tells you the cost of ownership of a code region
//Formula:  ((60e6 EUR / 5 years (s)) / #Cores) * PSC_EXECUTION_TIME
//Recommendation :
//

CostOwnership::CostOwnership( Context* ct, Context* phaseCt, double threshold ) : Property( ct ), threshold( threshold ) {
    phaseCycles  = 0;
    context      = ct;
    phaseContext = phaseCt;
    cost         = 0.0;
}

CostOwnership::~CostOwnership() {
}

PropertyID CostOwnership::id() {
    return ENERGY_CONSUMPTION;
}

void CostOwnership::print() {
    std::cout << "Property:" << name()
              << "  Process " << context->getRank()
              << "  Thread " << context->getThread() << std::endl
              << "                  " << context->getRegion()->str_print() << std::endl;
}

bool CostOwnership::condition() const {
    return true;
}

double CostOwnership::confidence() const {
    return 1.0;
}

double CostOwnership::severity() const {
    return cost;
}

Context* CostOwnership::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type CostOwnership::request_metrics() {
    Context* ct = new Context( context->getRegion(), context->getRank(), 0 );
    pdb->request( ct, PSC_EXECUTION_TIME );
    pdb->request( ct, PSC_TASKS_CREATED );

    return ALL_INFO_GATHERED;
}

std::string CostOwnership::name() {
    return "Cost of ownership";
}

void CostOwnership::evaluate() {
    Context* ct = new Context( context->getRegion(), context->getRank(), 0 );
    cost = pdb->get( ct, PSC_EXECUTION_TIME );

    int tasks = pdb->get( ct, PSC_TASKS_CREATED );
    cost *= ALFA / tasks;

    psc_dbgmsg( 1010, "Got cost of ownership: %f\n", cost );
    delete ct;
}

std::string CostOwnership::info() {
    std::stringstream stream;
    stream << '\t' << " ENERGY_CONSUMPTION: " << cost;
    return stream.str();
}

Property* CostOwnership::clone() {
    CostOwnership* prop = new CostOwnership( context, phaseContext, threshold );
    return prop;
}
