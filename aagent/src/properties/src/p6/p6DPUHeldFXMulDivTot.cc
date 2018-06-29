/**
   @file    p6DPUHeldFXMulDivTot.cc
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

#include "p6DPUHeldFXMulDivTot.h"
#include "Property.h"
#include <iostream>

#include "p6DPUHeldFXMultGPR.h"

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6DPUHeldFXMulDivTot::p6DPUHeldFXMulDivTot( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    importance   = 0.0;
    cycles       = 0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6DPUHeldFXMulDivTot::~p6DPUHeldFXMulDivTot() {
}

PropertyID p6DPUHeldFXMulDivTot::id() {
    return P6DPUHELDFXMULDIVTOT;
}

void p6DPUHeldFXMulDivTot::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6DPUHeldFXMulDivTot::condition() const {
    psc_dbgmsg( 10, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return importance > 1;
}

double p6DPUHeldFXMulDivTot::confidence() const {
    return 1.0;
}

double p6DPUHeldFXMulDivTot::severity() const {
    return ( double )stallCycles / ( double )phaseCycles * 100;
}

Context* p6DPUHeldFXMulDivTot::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6DPUHeldFXMulDivTot::request_metrics() {
    pdb->request( context, PSC_PM_DPU_HELD_FP_FX_MULT );
    pdb->request( phaseContext, PSC_PM_RUN_CYC );
    pdb->request( context, PSC_PM_RUN_CYC );
    return ALL_INFO_GATHERED;
}

std::string p6DPUHeldFXMulDivTot::name() {
    return "Cycles lost due to fixed point multiply/divide";
}

void p6DPUHeldFXMulDivTot::evaluate() {
    phaseCycles = pdb->get( phaseContext, PSC_PM_RUN_CYC );
    cycles      = pdb->get( context, PSC_PM_RUN_CYC );
    stallCycles = pdb->get( context, PSC_PM_DPU_HELD_FP_FX_MULT );
    importance  = ( double )( stallCycles ) / ( double )( phaseCycles );
}

Property* p6DPUHeldFXMulDivTot::clone() {
    p6DPUHeldFXMulDivTot* prop = new p6DPUHeldFXMulDivTot( context, phaseContext );
    return prop;
}

Prop_List p6DPUHeldFXMulDivTot::next() {
    Prop_List returnList;
    returnList.push_back( new p6DPUHeldFXMultGPR( context, phaseContext ) );
    return returnList;
}

std::string p6DPUHeldFXMulDivTot::info() {
    std::stringstream stream;

    stream << '\t' << " DPUHeldFXMulDivTot: " << stallCycles;

    return stream.str();
}
;

std::string p6DPUHeldFXMulDivTot::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_DPU_HELD_FP_FX_MULT>" << stallCycles << "</PM_DPU_HELD_FP_FX_MULT>" << std::endl;

    return stream.str();
}
