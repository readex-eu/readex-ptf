/**
   @file    p6L2DemandMissRate.cc
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


#include "p6L2DemandMissRate.h"
#include "Property.h"
#include <iostream>

#include "p6PrefL2LD.h"

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6L2DemandMissRate::p6L2DemandMissRate( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles = 0;
    stallCycles = 0;
    pm_var1     = 0;
    pm_var2     = 0;
    pm_var3     = 0;

    importance   = 0.0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6L2DemandMissRate::~p6L2DemandMissRate() {
}

PropertyID p6L2DemandMissRate::id() {
    return P6L2DEMANDMISSRATE;
}

void p6L2DemandMissRate::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6L2DemandMissRate::condition() const {
    psc_dbgmsg( 3, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return severity() > 0;
}

double p6L2DemandMissRate::confidence() const {
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

double p6L2DemandMissRate::severity() const {
    return importance * 100;
}

Context* p6L2DemandMissRate::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6L2DemandMissRate::request_metrics() {
    pdb->request( context, PSC_PM_LD_MISS_L1 );
    pdb->request( context, PSC_PM_DATA_FROM_L2 );
    pdb->request( context, PSC_PM_DATA_FROM_L2MISS );

    return ALL_INFO_GATHERED;
}

std::string p6L2DemandMissRate::name() {
    return "L2 demand miss rate";
}

void p6L2DemandMissRate::evaluate() {
    pm_var1 = pdb->get( context, PSC_PM_LD_MISS_L1 );
    pm_var2 = pdb->get( context, PSC_PM_DATA_FROM_L2 );
    pm_var3 = pdb->get( context, PSC_PM_DATA_FROM_L2MISS );

    if( pm_var1 > 0 ) {
        importance = ( double )pm_var3 / ( double )pm_var1;
    }
    else {
        importance = 0;
    }
}

Property* p6L2DemandMissRate::clone() {
    p6L2DemandMissRate* prop = new p6L2DemandMissRate( context, phaseContext );
    return prop;
}

Prop_List p6L2DemandMissRate::next() {
    Prop_List returnList;
    returnList.push_back( new p6PrefL2LD( context, phaseContext ) );
    return returnList;
}

std::string p6L2DemandMissRate::info() {
    std::stringstream stream;

    stream << '\t' << " L2DemandMissRate: " << importance;

    return stream.str();
}
;

std::string p6L2DemandMissRate::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_LD_MISS_L1>" << pm_var1 << "</PM_LD_MISS_L1>" << std::endl;
    stream << "\t\t<PM_DATA_FROM_L2>" << pm_var2 << "</PM_DATA_FROM_L2>" << std::endl;
    stream << "\t\t<PM_DATA_FROM_L2MISS>" << pm_var3 << "</PM_DATA_FROM_L2MISS>" << std::endl;
    stream << "\t\t<L2DemandMissRate>" << importance << "</L2DemandMissRate>" << std::endl;

    return stream.str();
}
