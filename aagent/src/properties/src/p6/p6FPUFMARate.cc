/**
   @file    p6FPUFMARate.cc
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

#include "p6FPUFMARate.h"
#include "Property.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6FPUFMARate::p6FPUFMARate( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles = 0;
    stallCycles = 0;
    pm_var1     = 0;
    pm_var2     = 0;

    importance   = 0.0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6FPUFMARate::~p6FPUFMARate() {
}

PropertyID p6FPUFMARate::id() {
    return P6FPUFMARATE;
}

void p6FPUFMARate::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6FPUFMARate::condition() const {
    psc_dbgmsg( 10, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return severity() < 40;
}

double p6FPUFMARate::confidence() const {
    return 1.0;
}

double p6FPUFMARate::severity() const {
    return importance * 100;
}

Context* p6FPUFMARate::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6FPUFMARate::request_metrics() {
    pdb->request( context, PSC_PM_FPU_FMA );
    pdb->request( context, PSC_PM_FPU_FLOP );

    return ALL_INFO_GATHERED;
}

std::string p6FPUFMARate::name() {
    return "Poor utilization of floating point Multiply-Add instructions";
}

void p6FPUFMARate::evaluate() {
    pm_var1 = pdb->get( context, PSC_PM_FPU_FMA );
    pm_var2 = pdb->get( context, PSC_PM_FPU_FLOP );

    if( pm_var2 > 0 ) {
        importance = ( double )pm_var1 / ( double )pm_var2;
    }
    else {
        importance = 0.0;
    }
}

Property* p6FPUFMARate::clone() {
    p6FPUFMARate* prop = new p6FPUFMARate( context, phaseContext );
    return prop;
}

std::string p6FPUFMARate::info() {
    std::stringstream stream;

    stream << '\t' << " FPUFMARate: " << importance;

    return stream.str();
}
;

std::string p6FPUFMARate::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_FPU_FMA>" << pm_var1 << "</PM_FPU_FMA>" << std::endl;
    stream << "\t\t<PM_FPU_FLOP>" << pm_var2 << "</PM_FPU_FLOP>" << std::endl;
    stream << "\t\t<FPUFMARate>" << importance << "</FPUFMARate>" << std::endl;

    return stream.str();
}
