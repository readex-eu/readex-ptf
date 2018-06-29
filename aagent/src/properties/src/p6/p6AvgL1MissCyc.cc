/**
   @file    p6AvgL1MissCyc.cc
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

#include "p6AvgL1MissCyc.h"
#include "Property.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6AvgL1MissCyc::p6AvgL1MissCyc( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    pm_var1      = 0;
    pm_var2      = 0;
    importance   = 0.0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6AvgL1MissCyc::~p6AvgL1MissCyc() {
}

PropertyID p6AvgL1MissCyc::id() {
    return P6AVGL1MISSCYC;
}

void p6AvgL1MissCyc::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6AvgL1MissCyc::condition() const {
    psc_dbgmsg( 3, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return importance > 1 && importance < 500;
}

double p6AvgL1MissCyc::confidence() const {
    return 1.0;
}

double p6AvgL1MissCyc::severity() const {
    return importance;
}

Context* p6AvgL1MissCyc::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6AvgL1MissCyc::request_metrics() {
    pdb->request( context, PSC_PM_LD_MISS_L1 );
    pdb->request( context, PSC_PM_LD_MISS_L1_CYC );

    return ALL_INFO_GATHERED;
}

std::string p6AvgL1MissCyc::name() {
    return "Average amount of cycles spent waiting for L1 miss";
}

void p6AvgL1MissCyc::evaluate() {
    pm_var1 = pdb->get( context, PSC_PM_LD_MISS_L1 );
    pm_var2 = pdb->get( context, PSC_PM_LD_MISS_L1_CYC );

    if( pm_var1 > 0 ) {
        importance = ( double )pm_var2 / ( double )pm_var1;
    }
    else {
        importance = 0.0;
    }
}

Property* p6AvgL1MissCyc::clone() {
    p6AvgL1MissCyc* prop = new p6AvgL1MissCyc( context, phaseContext );
    return prop;
}

std::string p6AvgL1MissCyc::info() {
    std::stringstream stream;

    stream << '\t' << " AvgL1MissCyc: " << importance;

    return stream.str();
}
;

std::string p6AvgL1MissCyc::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_LD_MISS_L1>" << pm_var1 << "</PM_LD_MISS_L1>" << std::endl;
    stream << "\t\t<PM_LD_MISS_L1_CYC>" << pm_var2 << "</PM_LD_MISS_L1_CYC>" << std::endl;
    stream << "\t\t<AvgL1MissCyc>" << importance << "</AvgL1MissCyc>" << std::endl;

    return stream.str();
}
