/**
   @file    p6PrefL1.cc
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

#include "p6PrefL1.h"
#include "Property.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6PrefL1::p6PrefL1( Context* ct, Context* phaseCt ) :
    Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    misses       = 0;
    references   = 0;
    importance   = 0.0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6PrefL1::~p6PrefL1() {
}

PropertyID p6PrefL1::id() {
    return P6PREFL1;
}

void p6PrefL1::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6PrefL1::condition() const {
    psc_dbgmsg( 10, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return true;
}

double p6PrefL1::confidence() const {
    return 1.0;
}

double p6PrefL1::severity() const {
    return importance * 100;
}

Context* p6PrefL1::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6PrefL1::request_metrics() {
    pdb->request( context, PSC_PM_L1_PREF );
    pdb->request( context, PSC_PM_LD_REF_L1 );
    return ALL_INFO_GATHERED;
}

std::string p6PrefL1::name() {
    return "Prefetched bytes to L1 rate (per L1 load)";
}

void p6PrefL1::evaluate() {
    misses     = 128 * pdb->get( context, PSC_PM_L1_PREF );
    references = 8 * pdb->get( context, PSC_PM_LD_REF_L1 );
    if( references > 0 ) {
        importance = ( double )misses / ( double )( references );
    }
    else {
        importance = 1.0;
    }
}

Property* p6PrefL1::clone() {
    p6PrefL1* prop = new p6PrefL1( context, phaseContext );
    return prop;
}

std::string p6PrefL1::info() {
    std::stringstream stream;

    stream << '\t' << " PrefL1: " << importance;

    return stream.str();
}
;

std::string p6PrefL1::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_L1_PREF>" << misses << "</PM_L1_PREF>" << std::endl;
    stream << "\t\t<PM_LD_REF_L1>" << references << "</PM_LD_REF_L1>" << std::endl;
    stream << "\t\t<PrefL1>" << importance << "</PrefL1>" << std::endl;

    return stream.str();
}
