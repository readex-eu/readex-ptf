/**
   @file    p6StoreMiss.cc
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

#include "p6StoreMiss.h"
#include "Property.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6StoreMiss::p6StoreMiss( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    misses       = 0;
    references   = 0;
    importance   = 0.0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6StoreMiss::~p6StoreMiss() {
}

PropertyID p6StoreMiss::id() {
    return P6STOREMISS;
}

void p6StoreMiss::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6StoreMiss::condition() const {
    psc_dbgmsg( 3, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return severity() > 1;
}

double p6StoreMiss::confidence() const {
    return 1.0;
}

double p6StoreMiss::severity() const {
    return importance * 100;
}

Context* p6StoreMiss::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6StoreMiss::request_metrics() {
    pdb->request( context, PSC_PM_L2_ST_REQ_DATA );
    pdb->request( context, PSC_PM_L2_ST_MISS_DATA );
    return ALL_INFO_GATHERED;
}

std::string p6StoreMiss::name() {
    return "L2 total store miss rate (includes speculative, prefetched and demand requests)";
}

void p6StoreMiss::evaluate() {
    misses     = pdb->get( context, PSC_PM_L2_ST_MISS_DATA );
    references = pdb->get( context, PSC_PM_L2_ST_REQ_DATA );
    if( references > 0 ) {
        importance = ( double )misses / ( double )( references );
    }
    else {
        importance = 0.0;
    }
}

Property* p6StoreMiss::clone() {
    p6StoreMiss* prop = new p6StoreMiss( context, phaseContext );
    return prop;
}

std::string p6StoreMiss::info() {
    std::stringstream stream;

    stream << '\t' << " StoreMiss: " << importance;

    return stream.str();
}
;

std::string p6StoreMiss::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_L2_ST_MISS_DATA>" << misses << "</PM_L2_ST_MISS_DATA>" << std::endl;
    stream << "\t\t<PM_L2_ST_REQ_DATA>" << references << "</PM_L2_ST_REQ_DATA>" << std::endl;
    stream << "\t\t<StoreMiss>" << importance << "</StoreMiss>" << std::endl;

    return stream.str();
}
