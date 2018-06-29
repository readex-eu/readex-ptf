/**
   @file    p6DPUHeldFPUCR.cc
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

#include "p6DPUHeldFPUCR.h"
#include "Property.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6DPUHeldFPUCR::p6DPUHeldFPUCR( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    importance   = 0.0;
    cycles       = 0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6DPUHeldFPUCR::~p6DPUHeldFPUCR() {
}

PropertyID p6DPUHeldFPUCR::id() {
    return P6DPUHELDFPUCR;
}

void p6DPUHeldFPUCR::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6DPUHeldFPUCR::condition() const {
    psc_dbgmsg( 10, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return importance > 10;
}

double p6DPUHeldFPUCR::confidence() const {
    return 1.0;
}

double p6DPUHeldFPUCR::severity() const {
    return ( double )stallCycles / ( double )phaseCycles * 100;
}

Context* p6DPUHeldFPUCR::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6DPUHeldFPUCR::request_metrics() {
    pdb->request( context, PSC_PM_RUN_CYC );
    pdb->request( context, PSC_PM_DPU_HELD_FPU_CR );
    pdb->request( phaseContext, PSC_PM_RUN_CYC );
    return ALL_INFO_GATHERED;
}

std::string p6DPUHeldFPUCR::name() {
    return "Cycles lost due to floating point comparisons";
}

void p6DPUHeldFPUCR::evaluate() {
    cycles      = pdb->get( context, PSC_PM_RUN_CYC );
    phaseCycles = pdb->get( phaseContext, PSC_PM_RUN_CYC );
    stallCycles = pdb->get( context, PSC_PM_DPU_HELD_FPU_CR );
    importance  = ( double )( stallCycles ) / ( double )( phaseCycles ) * 100.0;
}

Property* p6DPUHeldFPUCR::clone() {
    p6DPUHeldFPUCR* prop = new p6DPUHeldFPUCR( context, phaseContext );
    return prop;
}

std::string p6DPUHeldFPUCR::info() {
    std::stringstream stream;

    stream << '\t' << " DPUHeldFPUCR: " << stallCycles;

    return stream.str();
}
;

std::string p6DPUHeldFPUCR::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_DPU_HELD_FPU_CR>" << stallCycles << "</PM_DPU_HELD_FPU_CR>" << std::endl;

    return stream.str();
}
