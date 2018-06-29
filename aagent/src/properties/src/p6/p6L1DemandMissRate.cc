/**
   @file    p6L1DemandMissRate.cc
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


#include "p6L1DemandMissRate.h"
#include "Property.h"
#include <iostream>

#include "p6PrefL1.h"

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6L1DemandMissRate::p6L1DemandMissRate( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    pm_var1      = 0;
    pm_var2      = 0;
    importance   = 0.0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6L1DemandMissRate::~p6L1DemandMissRate() {
}

PropertyID p6L1DemandMissRate::id() {
    return P6L1DEMANDMISSRATE;
}

void p6L1DemandMissRate::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6L1DemandMissRate::condition() const {
    psc_dbgmsg( 3, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return severity() > 0;
}

double p6L1DemandMissRate::confidence() const {
    return 1.0;
}

double p6L1DemandMissRate::severity() const {
    return importance * 100;
}

Context* p6L1DemandMissRate::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6L1DemandMissRate::request_metrics() {
    pdb->request( context, PSC_PM_LD_MISS_L1 );
    pdb->request( context, PSC_PM_LD_REF_L1 );

    return ALL_INFO_GATHERED;
}

std::string p6L1DemandMissRate::name() {
    return "L1 demand miss rate";
}

void p6L1DemandMissRate::evaluate() {
    pm_var1 = pdb->get( context, PSC_PM_LD_MISS_L1 );
    pm_var2 = pdb->get( context, PSC_PM_LD_REF_L1 );

    if( pm_var2 > 0 ) {
        importance = ( double )pm_var1 / ( double )pm_var2;
    }
    else {
        importance = 0.0;
    }
}

Property* p6L1DemandMissRate::clone() {
    p6L1DemandMissRate* prop = new p6L1DemandMissRate( context, phaseContext );
    return prop;
}

Prop_List p6L1DemandMissRate::next() {
    Prop_List returnList;
    returnList.push_back( new p6PrefL1( context, phaseContext ) );
    return returnList;
}

std::string p6L1DemandMissRate::info() {
    std::stringstream stream;

    stream << '\t' << " L1DemandMissRate: " << importance;

    return stream.str();
}
;

std::string p6L1DemandMissRate::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_LD_MISS_L1>" << pm_var1 << "</PM_LD_MISS_L1>" << std::endl;
    stream << "\t\t<PM_LD_REF_L1>" << pm_var2 << "</PM_LD_REF_L1>" << std::endl;
    stream << "\t\t<L1DemandMissRate>" << importance << "</L1DemandMissRate>" << std::endl;

    return stream.str();
}
