/**
   @file    p6DERATMissCYC.cc
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

#include "p6DERATMissCYC.h"
#include "Property.h"
#include <iostream>

#include "p6DERATMiss4K.h"
#include "p6DERATMiss64K.h"
#include "p6DERATMiss16M.h"
#include "p6DERATMiss16G.h"

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6DERATMissCYC::p6DERATMissCYC( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    cycles       = 0;
    importance   = 0.0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6DERATMissCYC::~p6DERATMissCYC() {
}

PropertyID p6DERATMissCYC::id() {
    return P6DERATMISSCYC;
}

void p6DERATMissCYC::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6DERATMissCYC::condition() const {
    psc_dbgmsg( 3, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return importance > 1;
}

double p6DERATMissCYC::confidence() const {
    return 1.0;
}

double p6DERATMissCYC::severity() const {
    return ( double )stallCycles / ( double )phaseCycles * 100;
}

Context* p6DERATMissCYC::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6DERATMissCYC::request_metrics() {
    pdb->request( context, PSC_PM_LSU_DERAT_MISS_CYC );
    pdb->request( context, PSC_PM_RUN_CYC );
    pdb->request( phaseContext, PSC_PM_RUN_CYC );
    return ALL_INFO_GATHERED;
}

std::string p6DERATMissCYC::name() {
    return "Cycles lost due to DERAT miss";
}

void p6DERATMissCYC::evaluate() {
    cycles      = pdb->get( context, PSC_PM_RUN_CYC );
    phaseCycles = pdb->get( phaseContext, PSC_PM_RUN_CYC );
    stallCycles = pdb->get( context, PSC_PM_LSU_DERAT_MISS_CYC );
    importance  = ( double )( stallCycles ) / ( double )( phaseCycles ) * 100.0;
}

Property* p6DERATMissCYC::clone() {
    p6DERATMissCYC* prop = new p6DERATMissCYC( context, phaseContext );
    return prop;
}

Prop_List p6DERATMissCYC::next() {
    Prop_List returnList;
    returnList.push_back( new p6DERATMiss4K( context, phaseContext ) );
    returnList.push_back( new p6DERATMiss64K( context, phaseContext ) );
    returnList.push_back( new p6DERATMiss16M( context, phaseContext ) );
    returnList.push_back( new p6DERATMiss16G( context, phaseContext ) );
    return returnList;
}

std::string p6DERATMissCYC::info() {
    std::stringstream stream;

    stream << '\t' << " DERATMissCYC: " << stallCycles;

    return stream.str();
}
;

std::string p6DERATMissCYC::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_LSU_DERAT_MISS_CYC>" << stallCycles << "</PM_LSU_DERAT_MISS_CYC>" << std::endl;
    stream << "\t\t<cycles>" << cycles << "</cycles>" << std::endl;
    stream << "\t\t<phaseCycles>" << phaseCycles << "</phaseCycles>" << std::endl;

    return stream.str();
}
