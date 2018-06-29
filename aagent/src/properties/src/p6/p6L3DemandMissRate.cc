/**
   @file    p6L3DemandMissRate.cc
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

#include "p6L3DemandMissRate.h"
#include "Property.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6L3DemandMissRate::p6L3DemandMissRate( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles = 0;
    stallCycles = 0;
    pm_var1     = 0;
    pm_var2     = 0;
    pm_var3     = 0;

    importance   = 0.0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6L3DemandMissRate::~p6L3DemandMissRate() {
}

PropertyID p6L3DemandMissRate::id() {
    return P6L3DEMANDMISSRATE;
}

void p6L3DemandMissRate::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6L3DemandMissRate::condition() const {
    psc_dbgmsg( 3, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return severity() > 0;
}

double p6L3DemandMissRate::confidence() const {
    evaluate();
    if( ( pm_var2 + pm_var3 - pm_var1 ) < 0 ) {
        return double( pm_var2 + pm_var3 ) / ( double )( pm_var1 );
    }
    if( ( pm_var2 + pm_var3 - pm_var1 ) > 0 ) {
        return double( pm_var1 ) / ( double )( pm_var2 + pm_var3 );
    }
    if( ( pm_var2 + pm_var3 - pm_var1 ) == 0 ) {
        return 0.0;
    }
}

double p6L3DemandMissRate::severity() const {
    return importance * 100;
}

Context* p6L3DemandMissRate::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6L3DemandMissRate::request_metrics() {
    pdb->request( context, PSC_PM_DATA_FROM_L2MISS );
    pdb->request( context, PSC_PM_DATA_FROM_L3 );
    pdb->request( context, PSC_PM_DATA_FROM_L3MISS );

    return ALL_INFO_GATHERED;
}

std::string p6L3DemandMissRate::name() {
    return "L3 demand miss rate";
}

void p6L3DemandMissRate::evaluate() {
    pm_var1 = pdb->get( context, PSC_PM_DATA_FROM_L2MISS );
    pm_var2 = pdb->get( context, PSC_PM_DATA_FROM_L3 );
    pm_var3 = pdb->get( context, PSC_PM_DATA_FROM_L3MISS );

    if( pm_var1 > 0 ) {
        importance = ( double )pm_var3 / ( double )pm_var1;
    }
    else {
        importance = 0;
    }
}

Property* p6L3DemandMissRate::clone() {
    p6L3DemandMissRate* prop = new p6L3DemandMissRate( context, phaseContext );
    return prop;
}

std::string p6L3DemandMissRate::info() {
    std::stringstream stream;

    stream << '\t' << " L3DemandMissRate: " << importance;

    return stream.str();
}
;

std::string p6L3DemandMissRate::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_DATA_FROM_L2MISS>" << pm_var1 << "</PM_DATA_FROM_L2MISS>" << std::endl;
    stream << "\t\t<PM_DATA_FROM_L3>" << pm_var2 << "</PM_DATA_FROM_L3>" << std::endl;
    stream << "\t\t<PM_DATA_FROM_L3MISS>" << pm_var3 << "</PM_DATA_FROM_L3MISS>" << std::endl;
    stream << "\t\t<L3DemandMissRate>" << importance << "</L3DemandMissRate>" << std::endl;

    return stream.str();
}
