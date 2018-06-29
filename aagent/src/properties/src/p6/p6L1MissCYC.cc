/**
   @file    p6L1MissCYC.cc
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

#include "p6L1MissCYC.h"
#include "Property.h"
#include <iostream>

#include "p6AvgL1MissCyc.h"
#include "p6L1DemandMissRate.h"
#include "p6L2DemandMissRate.h"
#include "p6L3DemandMissRate.h"
#include "p6L2TotalMissRate.h"

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6L1MissCYC::p6L1MissCYC( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    importance   = 0.0;
    threshold    = 1.0;
    cycles       = 0;
    phaseContext = phaseCt;
}

p6L1MissCYC::~p6L1MissCYC() {
}

PropertyID p6L1MissCYC::id() {
    return P6L1MISSCYC;
}

void p6L1MissCYC::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6L1MissCYC::condition() const {
    psc_dbgmsg( 10, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return importance > 1;
}

double p6L1MissCYC::confidence() const {
    return 1.0;
}

double p6L1MissCYC::severity() const {
    return ( double )stallCycles / ( double )phaseCycles * 100;
}

Context* p6L1MissCYC::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6L1MissCYC::request_metrics() {
    pdb->request( context, PSC_PM_LD_MISS_L1_CYC );
    pdb->request( context, PSC_PM_RUN_CYC );
    pdb->request( phaseContext, PSC_PM_RUN_CYC );
    return ALL_INFO_GATHERED;
}

std::string p6L1MissCYC::name() {
    return "Cycles lost waiting for cache misses";
}

void p6L1MissCYC::evaluate() {
    cycles      = pdb->get( context, PSC_PM_RUN_CYC );
    phaseCycles = pdb->get( phaseContext, PSC_PM_RUN_CYC );
    stallCycles = pdb->get( context, PSC_PM_LD_MISS_L1_CYC );
    importance  = ( double )( stallCycles ) / ( double )( phaseCycles ) * 100;
}

Property* p6L1MissCYC::clone() {
    p6L1MissCYC* prop = new p6L1MissCYC( context, phaseContext );
    return prop;
}

Prop_List p6L1MissCYC::next() {
    Prop_List returnList;
    returnList.push_back( new p6AvgL1MissCyc( context, phaseContext ) );
    returnList.push_back( new p6L1DemandMissRate( context, phaseContext ) );
    returnList.push_back( new p6L2DemandMissRate( context, phaseContext ) );
    returnList.push_back( new p6L3DemandMissRate( context, phaseContext ) );
    returnList.push_back( new p6L2TotalMissRate( context, phaseContext ) );

    return returnList;
}

std::string p6L1MissCYC::info() {
    std::stringstream stream;

    stream << '\t' << " L1MissCYC: " << stallCycles;

    return stream.str();
}
;

std::string p6L1MissCYC::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_LD_MISS_L1_CYC>" << stallCycles << "</PM_LD_MISS_L1_CYC>" << std::endl;
    stream << "\t\t<cycles>" << cycles << "</cycles>" << std::endl;
    stream << "\t\t<phaseCycles>" << phaseCycles << "</phaseCycles>" << std::endl;

    return stream.str();
}
