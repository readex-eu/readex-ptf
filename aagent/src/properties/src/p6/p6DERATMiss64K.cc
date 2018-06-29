/**
   @file    p6DERATMiss64K.cc
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

#include "p6DERATMiss64K.h"
#include "Property.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6DERATMiss64K::p6DERATMiss64K( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    misses       = 0;
    references   = 0;
    importance   = 0.0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6DERATMiss64K::~p6DERATMiss64K() {
}

PropertyID p6DERATMiss64K::id() {
    return P6DERATMISS64K;
}

void p6DERATMiss64K::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6DERATMiss64K::condition() const {
    psc_dbgmsg( 3, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return severity() > 1;
}

double p6DERATMiss64K::confidence() const {
    return 1.0;
}

double p6DERATMiss64K::severity() const {
    return importance * 100;
}

Context* p6DERATMiss64K::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6DERATMiss64K::request_metrics() {
    pdb->request( context, PSC_PM_DERAT_MISS_64K );
    pdb->request( context, PSC_PM_DERAT_REF_16M );
    pdb->request( context, PSC_PM_DERAT_REF_16G );
    pdb->request( context, PSC_PM_DERAT_REF_4K );
    pdb->request( context, PSC_PM_DERAT_REF_64K );
    return ALL_INFO_GATHERED;
}

std::string p6DERATMiss64K::name() {
    return "DERAT 64K pages miss rate";
}

void p6DERATMiss64K::evaluate() {
    misses     = pdb->get( context, PSC_PM_DERAT_MISS_64K );
    references = pdb->get( context, PSC_PM_DERAT_REF_4K ) + pdb->get( context, PSC_PM_DERAT_REF_64K )
                 + pdb->get( context, PSC_PM_DERAT_REF_16M ) + pdb->get( context, PSC_PM_DERAT_REF_16G );
    if( references > 0 ) {
        importance = ( double )misses / ( double )( references );
    }
    else {
        importance = 0.0;
    }
}

Property* p6DERATMiss64K::clone() {
    p6DERATMiss64K* prop = new p6DERATMiss64K( context, phaseContext );
    return prop;
}

std::string p6DERATMiss64K::info() {
    std::stringstream stream;

    stream << '\t' << " DERATMiss64K: " << importance;

    return stream.str();
}
;

std::string p6DERATMiss64K::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_DERAT_MISS_64K>" << misses << "</PM_DERAT_MISS_64K>" << std::endl;
    stream << "\t\t<references>" << references << "</references>" << std::endl;
    stream << "\t\t<DERATMiss64K>" << importance << "</DERATMiss64K>" << std::endl;

    return stream.str();
}
