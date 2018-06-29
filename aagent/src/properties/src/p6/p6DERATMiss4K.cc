/**
   @file    p6DERATMiss4K.cc
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

#include "p6DERATMiss4K.h"
#include "Property.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6DERATMiss4K::p6DERATMiss4K( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    misses       = 0;
    references   = 0;
    importance   = 0.0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6DERATMiss4K::~p6DERATMiss4K() {
}

PropertyID p6DERATMiss4K::id() {
    return P6DERATMISS4K;
}

void p6DERATMiss4K::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6DERATMiss4K::condition() const {
    psc_dbgmsg( 3, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return severity() > 1;
}

double p6DERATMiss4K::confidence() const {
    return 1.0;
}

double p6DERATMiss4K::severity() const {
    return importance * 100;
}

Context* p6DERATMiss4K::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6DERATMiss4K::request_metrics() {
    pdb->request( context, PSC_PM_DERAT_MISS_4K );
    pdb->request( context, PSC_PM_DERAT_REF_16M );
    pdb->request( context, PSC_PM_DERAT_REF_16G );
    pdb->request( context, PSC_PM_DERAT_REF_4K );
    pdb->request( context, PSC_PM_DERAT_REF_64K );
    return ALL_INFO_GATHERED;
}

std::string p6DERATMiss4K::name() {
    return "DERAT 4K pages miss rate";
}

void p6DERATMiss4K::evaluate() {
    misses     = pdb->get( context, PSC_PM_DERAT_MISS_4K );
    references = pdb->get( context, PSC_PM_DERAT_REF_4K ) + pdb->get( context, PSC_PM_DERAT_REF_64K )
                 + pdb->get( context, PSC_PM_DERAT_REF_16M ) + pdb->get( context, PSC_PM_DERAT_REF_16G );
    if( references > 0 ) {
        importance = ( double )misses / ( double )( references );
    }
    else {
        importance = 0.0;
    }
}

Property* p6DERATMiss4K::clone() {
    p6DERATMiss4K* prop = new p6DERATMiss4K( context, phaseContext );
    return prop;
}

std::string p6DERATMiss4K::info() {
    std::stringstream stream;

    stream << '\t' << " DERATMiss4K: " << importance;

    return stream.str();
}
;

std::string p6DERATMiss4K::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_DERAT_MISS_4K>" << misses << "</PM_DERAT_MISS_4K>" << std::endl;
    stream << "\t\t<references>" << references << "</references>" << std::endl;
    stream << "\t\t<DERATMiss4K>" << importance << "</DERATMiss4K>" << std::endl;

    return stream.str();
}
