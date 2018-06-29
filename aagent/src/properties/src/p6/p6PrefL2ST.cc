/**
   @file    p6PrefL2ST.cc
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

#include "p6PrefL2ST.h"
#include "Property.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6PrefL2ST::p6PrefL2ST( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    misses       = 0;
    references   = 0;
    importance   = 0.0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6PrefL2ST::~p6PrefL2ST() {
}

PropertyID p6PrefL2ST::id() {
    return P6PREFL2ST;
}

void p6PrefL2ST::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6PrefL2ST::condition() const {
    psc_dbgmsg( 10, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return true;
}

double p6PrefL2ST::confidence() const {
    return 1;
}

double p6PrefL2ST::severity() const {
    return importance * 100;
}

Context* p6PrefL2ST::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6PrefL2ST::request_metrics() {
    pdb->request( context, PSC_PM_L2_PREF_ST );
    pdb->request( context, PSC_PM_ST_REF_L1 );
    return ALL_INFO_GATHERED;
}

std::string p6PrefL2ST::name() {
    return "Bytes prefetched for store to L2 rate (per store operation)";
}

void p6PrefL2ST::evaluate() {
    misses     = 128 * pdb->get( context, PSC_PM_L2_PREF_ST );
    references = 8 * pdb->get( context, PSC_PM_ST_REF_L1 );
    if( references > 0 ) {
        importance = ( double )misses / ( double )( references );
    }
    else {
        importance = 1.0;
    }
}

Property* p6PrefL2ST::clone() {
    p6PrefL2ST* prop = new p6PrefL2ST( context, phaseContext );
    return prop;
}

std::string p6PrefL2ST::info() {
    std::stringstream stream;

    stream << '\t' << " PrefL2ST: " << importance;

    return stream.str();
}
;

std::string p6PrefL2ST::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_L2_PREF_ST>" << misses << "</PM_L2_PREF_ST>" << std::endl;
    stream << "\t\t<PM_ST_REF_L1>" << references << "</PM_ST_REF_L1>" << std::endl;
    stream << "\t\t<PrefL2ST>" << importance << "</PrefL2ST>" << std::endl;

    return stream.str();
}
