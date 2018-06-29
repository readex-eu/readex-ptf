/**
   @file    p6DPUHeldFXMultGPR.cc
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

#include "p6DPUHeldFXMultGPR.h"
#include "Property.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6DPUHeldFXMultGPR::p6DPUHeldFXMultGPR( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    importance   = 0.0;
    cycles       = 0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6DPUHeldFXMultGPR::~p6DPUHeldFXMultGPR() {
}

PropertyID p6DPUHeldFXMultGPR::id() {
    return P6DPUHELDFXMULTGPR;
}

void p6DPUHeldFXMultGPR::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6DPUHeldFXMultGPR::condition() const {
    psc_dbgmsg( 10, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return importance > 1;
}

double p6DPUHeldFXMultGPR::confidence() const {
    return 1.0;
}

double p6DPUHeldFXMultGPR::severity() const {
    return ( double )stallCycles / ( double )phaseCycles * 100;
}

Context* p6DPUHeldFXMultGPR::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6DPUHeldFXMultGPR::request_metrics() {
    pdb->request( context, PSC_PM_DPU_HELD_MULT_GPR );
    pdb->request( phaseContext, PSC_PM_RUN_CYC );
    pdb->request( context, PSC_PM_RUN_CYC );
    return ALL_INFO_GATHERED;
}

std::string p6DPUHeldFXMultGPR::name() {
    return "Cycles lost due to FX mult/div has a GPR dependency on another FX mult/div";
}

void p6DPUHeldFXMultGPR::evaluate() {
    cycles      = pdb->get( context, PSC_PM_RUN_CYC );
    phaseCycles = pdb->get( phaseContext, PSC_PM_RUN_CYC );
    stallCycles = pdb->get( context, PSC_PM_DPU_HELD_MULT_GPR );
    importance  = ( double )( stallCycles ) / ( double )( phaseCycles ) * 100.0;
}

Property* p6DPUHeldFXMultGPR::clone() {
    p6DPUHeldFXMultGPR* prop = new p6DPUHeldFXMultGPR( context, phaseContext );
    return prop;
}

std::string p6DPUHeldFXMultGPR::info() {
    std::stringstream stream;

    stream << '\t' << " DPUHeldFXMultGPR: " << stallCycles;

    return stream.str();
}
;

std::string p6DPUHeldFXMultGPR::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_DPU_HELD_MULT_GPR>" << stallCycles << "</PM_DPU_HELD_MULT_GPR>" << std::endl;

    return stream.str();
}
