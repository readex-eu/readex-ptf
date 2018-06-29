/**
   @file    p6StoreChained.cc
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

#include "p6StoreChained.h"
#include "Property.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6StoreChained::p6StoreChained( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    misses       = 0;
    references   = 0;
    importance   = 0.0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6StoreChained::~p6StoreChained() {
}

PropertyID p6StoreChained::id() {
    return P6STORECHAINED;
}

void p6StoreChained::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6StoreChained::condition() const {
    psc_dbgmsg( 3, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return severity() > 1;
}

double p6StoreChained::confidence() const {
    return 1.0;
}

double p6StoreChained::severity() const {
    return importance * 100;
}

Context* p6StoreChained::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6StoreChained::request_metrics() {
    pdb->request( context, PSC_PM_LSU_ST_CHAINED );
    pdb->request( context, PSC_PM_ST_REF_L1 );
    return ALL_INFO_GATHERED;
}

std::string p6StoreChained::name() {
    return "Percentage of not chained stores";
}

void p6StoreChained::evaluate() {
    misses     = pdb->get( context, PSC_PM_LSU_ST_CHAINED );
    references = pdb->get( context, PSC_PM_ST_REF_L1 );
    if( references > 0 ) {
        importance = 1 - 2 * ( double )misses / ( double )( references );
    }
    else {
        importance = 0.0;
    }
}

Property* p6StoreChained::clone() {
    p6StoreChained* prop = new p6StoreChained( context, phaseContext );
    return prop;
}

std::string p6StoreChained::info() {
    std::stringstream stream;

    stream << '\t' << " StoreChained: " << importance;

    return stream.str();
}
;

std::string p6StoreChained::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_LSU_ST_CHAINED>" << misses << "</PM_LSU_ST_CHAINED>" << std::endl;
    stream << "\t\t<PM_ST_REF_L1>" << references << "</PM_ST_REF_L1>" << std::endl;
    stream << "\t\t<StoreChained>" << importance << "</StoreChained>" << std::endl;

    return stream.str();
}
