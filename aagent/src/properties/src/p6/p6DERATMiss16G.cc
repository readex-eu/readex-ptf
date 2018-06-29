/**
   @file    p6DERATMiss16G.cc
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

#include "p6DERATMiss16G.h"
#include "Property.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6DERATMiss16G::p6DERATMiss16G( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    misses       = 0;
    references   = 0;
    importance   = 0.0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6DERATMiss16G::~p6DERATMiss16G() {
}

PropertyID p6DERATMiss16G::id() {
    return P6DERATMISS16G;
}

void p6DERATMiss16G::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6DERATMiss16G::condition() const {
    psc_dbgmsg( 3, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return severity() > 1;
}

double p6DERATMiss16G::confidence() const {
    return 1.0;
}

double p6DERATMiss16G::severity() const {
    return importance * 100;
}

Context* p6DERATMiss16G::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6DERATMiss16G::request_metrics() {
    pdb->request( context, PSC_PM_DERAT_MISS_16G );
    pdb->request( context, PSC_PM_DERAT_REF_16M );
    pdb->request( context, PSC_PM_DERAT_REF_16G );
    pdb->request( context, PSC_PM_DERAT_REF_4K );
    pdb->request( context, PSC_PM_DERAT_REF_64K );
    return ALL_INFO_GATHERED;
}

std::string p6DERATMiss16G::name() {
    return "DERAT 16G pages miss rate";
}

void p6DERATMiss16G::evaluate() {
    misses     = pdb->get( context, PSC_PM_DERAT_MISS_16G );
    references = pdb->get( context, PSC_PM_DERAT_REF_4K ) + pdb->get( context, PSC_PM_DERAT_REF_64K )
                 + pdb->get( context, PSC_PM_DERAT_REF_16M ) + pdb->get( context, PSC_PM_DERAT_REF_16G );
    if( references > 0 ) {
        importance = ( double )misses / ( double )( references );
    }
    else {
        importance = 0.0;
    }
}

Property* p6DERATMiss16G::clone() {
    p6DERATMiss16G* prop = new p6DERATMiss16G( context, phaseContext );
    return prop;
}

std::string p6DERATMiss16G::info() {
    std::stringstream stream;

    stream << '\t' << " DERATMiss16G: " << importance;

    return stream.str();
}
;

std::string p6DERATMiss16G::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_DERAT_MISS_16G>" << misses << "</PM_DERAT_MISS_16G>" << std::endl;
    stream << "\t\t<references>" << references << "</references>" << std::endl;
    stream << "\t\t<DERATMiss16G>" << importance << "</DERATMiss16G>" << std::endl;

    return stream.str();
}
