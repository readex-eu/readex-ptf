/**
   @file    p6L2TotalMissRate.cc
   @ingroup Power6Properties
   @brief   Power6 specific property
   @author  Yury Oleynik
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

#include "p6L2TotalMissRate.h"
#include "Property.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6L2TotalMissRate::p6L2TotalMissRate( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    pm_var1      = 0;
    pm_var2      = 0;
    importance   = 0.0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6L2TotalMissRate::~p6L2TotalMissRate() {
}

PropertyID p6L2TotalMissRate::id() {
    return P6L2TOTALMISSRATE;
}

void p6L2TotalMissRate::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6L2TotalMissRate::condition() const {
    psc_dbgmsg( 3, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return severity() > 0;
}

double p6L2TotalMissRate::confidence() const {
    return 1.0;
}

double p6L2TotalMissRate::severity() const {
    return importance * 100;
}

Context* p6L2TotalMissRate::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6L2TotalMissRate::request_metrics() {
    pdb->request( context, PSC_PM_L2_LD_REQ_DATA );
    pdb->request( context, PSC_PM_L2_LD_MISS_DATA );

    return ALL_INFO_GATHERED;
}

std::string p6L2TotalMissRate::name() {
    return "L2 total load miss rate (includes speculative, prefetched and demand requests)";
}

void p6L2TotalMissRate::evaluate() {
    pm_var1 = pdb->get( context, PSC_PM_L2_LD_REQ_DATA );
    pm_var2 = pdb->get( context, PSC_PM_L2_LD_MISS_DATA );

    if( pm_var1 > 0 ) {
        importance = ( double )pm_var2 / ( double )pm_var1;
    }
}

Property* p6L2TotalMissRate::clone() {
    p6L2TotalMissRate* prop = new p6L2TotalMissRate( context, phaseContext );
    return prop;
}

std::string p6L2TotalMissRate::info() {
    std::stringstream stream;

    stream << '\t' << " L2TotalMissRate: " << importance;

    return stream.str();
}
;

std::string p6L2TotalMissRate::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_L2_LD_REQ_DATA>" << pm_var1 << "</PM_L2_LD_REQ_DATA>" << std::endl;
    stream << "\t\t<PM_L2_LD_MISS_DATA>" << pm_var2 << "</PM_L2_LD_MISS_DATA>" << std::endl;
    stream << "\t\t<L2TotalMissRate>" << importance << "</L2TotalMissRate>" << std::endl;

    return stream.str();
}
