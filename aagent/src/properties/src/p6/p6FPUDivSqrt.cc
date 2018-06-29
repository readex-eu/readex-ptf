/**
   @file    p6FPUDivSqrt.cc
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

#include "p6FPUDivSqrt.h"
#include "Property.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6FPUDivSqrt::p6FPUDivSqrt( Context* ct, Context* phaseCt ) :
    Property( ct ) {
    phaseCycles = 0;
    stallCycles = 0;
    pm_var1     = 0;
    pm_var2     = 0;
    pm_var3     = 0;

    importance   = 0.0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6FPUDivSqrt::~p6FPUDivSqrt() {
}

PropertyID p6FPUDivSqrt::id() {
    return P6FPUDIVSQRT;
}

void p6FPUDivSqrt::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6FPUDivSqrt::condition() const {
    psc_dbgmsg( 10, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return severity() > 5;
}

double p6FPUDivSqrt::confidence() const {
    return 1.0;
}

double p6FPUDivSqrt::severity() const {
    return importance * 100;
}

Context* p6FPUDivSqrt::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6FPUDivSqrt::request_metrics() {
    pdb->request( context, PSC_PM_FPU_FSQRT_FDIV );
    pdb->request( context, PSC_PM_FPU_FMA );
    pdb->request( context, PSC_PM_FPU_FLOP );

    return ALL_INFO_GATHERED;
}

std::string p6FPUDivSqrt::name() {
    return "Many expensive floating point divisions and sqrt instructions";
}

void p6FPUDivSqrt::evaluate() {
    pm_var1 = pdb->get( context, PSC_PM_FPU_FSQRT_FDIV );
    pm_var2 = pdb->get( context, PSC_PM_FPU_FMA );
    pm_var3 = pdb->get( context, PSC_PM_FPU_FLOP );
    if( ( pm_var2 + pm_var3 ) > 0 ) {
        importance = ( double )pm_var1 / ( double )( pm_var2 + pm_var3 );
    }
    else {
        importance = 0;
    }
}

Property* p6FPUDivSqrt::clone() {
    p6FPUDivSqrt* prop = new p6FPUDivSqrt( context, phaseContext );
    return prop;
}

std::string p6FPUDivSqrt::info() {
    std::stringstream stream;

    stream << '\t' << " FPUDivSqrt: " << importance;

    return stream.str();
}
;

std::string p6FPUDivSqrt::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_FPU_FSQRT_FDIV>" << pm_var1 << "</PM_FPU_FSQRT_FDIV>" << std::endl;
    stream << "\t\t<PM_FPU_FMA>" << pm_var2 << "</PM_FPU_FMA>" << std::endl;
    stream << "\t\t<PM_FPU_FLOP>" << pm_var3 << "</PM_FPU_FLOP>" << std::endl;
    stream << "\t\t<FPUDivSqrt>" << importance << "</FPUDivSqrt>" << std::endl;

    return stream.str();
}
