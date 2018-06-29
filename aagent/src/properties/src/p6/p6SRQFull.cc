/**
   @file    p6SRQFull.cc
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

#include "p6SRQFull.h"
#include "Property.h"
#include <iostream>

#include "p6StoreChained.h"
#include "p6StoreMiss.h"
#include "p6PrefL2ST.h"

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6SRQFull::p6SRQFull( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    misses       = 0;
    references   = 0;
    importance   = 0.0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6SRQFull::~p6SRQFull() {
}

PropertyID p6SRQFull::id() {
    return P6SRQFULL;
}

void p6SRQFull::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6SRQFull::condition() const {
    psc_dbgmsg( 10, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return severity() > 1 && severity() <= 100;
}

double p6SRQFull::confidence() const {
    return 1.0;
}

double p6SRQFull::severity() const {
    return importance * 100;
}

Context* p6SRQFull::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6SRQFull::request_metrics() {
    pdb->request( context, PSC_PM_LSU_REJECT_STQ_FULL );
    pdb->request( context, PSC_PM_ST_REF_L1 );
    return ALL_INFO_GATHERED;
}

std::string p6SRQFull::name() {
    return "Store Request Queue (SRQ) full rejects per store operation";
}

void p6SRQFull::evaluate() {
    misses     = pdb->get( context, PSC_PM_LSU_REJECT_STQ_FULL );
    references = pdb->get( context, PSC_PM_ST_REF_L1 );
    importance = ( double )misses / ( double )( references );
}

Property* p6SRQFull::clone() {
    p6SRQFull* prop = new p6SRQFull( context, phaseContext );
    return prop;
}

Prop_List p6SRQFull::next() {
    Prop_List returnList;
    returnList.push_back( new p6StoreChained( context, phaseContext ) );
    returnList.push_back( new p6StoreMiss( context, phaseContext ) );
    //returnList.push_back(new p6L2StoreMissAligned(context, phaseContext));
    returnList.push_back( new p6PrefL2ST( context, phaseContext ) );

    return returnList;
}

std::string p6SRQFull::info() {
    std::stringstream stream;

    stream << '\t' << " SRQFull: " << stallCycles;

    return stream.str();
}
;

std::string p6SRQFull::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_LSU_REJECT_STQ_FULL>" << misses << "</PM_LSU_REJECT_STQ_FULL>" << std::endl;
    stream << "\t\t<PM_ST_REF_L1>" << references << "</PM_ST_REF_L1>" << std::endl;
    stream << "\t\t<SRQFull>" << importance << "</SRQFull>" << std::endl;

    return stream.str();
}
