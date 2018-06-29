/**
   @file    p6DPUHeldSMT.cc
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

#include "p6DPUHeldSMT.h"
#include "Property.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6DPUHeldSMT::p6DPUHeldSMT( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    importance   = 0.0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6DPUHeldSMT::~p6DPUHeldSMT() {
}

PropertyID p6DPUHeldSMT::id() {
    return P6DPUHELDSMT;
}

void p6DPUHeldSMT::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6DPUHeldSMT::condition() const {
    psc_dbgmsg( 10, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return severity() > 1;
}

double p6DPUHeldSMT::confidence() const {
    return 1.0;
}

double p6DPUHeldSMT::severity() const {
    return ( double )stallCycles / ( double )phaseCycles * 100;
}

Context* p6DPUHeldSMT::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6DPUHeldSMT::request_metrics() {
    pdb->request( context, PSC_PM_DPU_HELD_SMT );
    pdb->request( phaseContext, PSC_PM_RUN_CYC );
    return ALL_INFO_GATHERED;
}

std::string p6DPUHeldSMT::name() {
    return "Cycles lost due to other thread active in SMT mode";
}

void p6DPUHeldSMT::evaluate() {
    phaseCycles = pdb->get( phaseContext, PSC_PM_RUN_CYC );
    stallCycles = pdb->get( context, PSC_PM_DPU_HELD_SMT );
    importance  = ( double )( stallCycles );
}

Property* p6DPUHeldSMT::clone() {
    p6DPUHeldSMT* prop = new p6DPUHeldSMT( context, phaseContext );
    return prop;
}

std::string p6DPUHeldSMT::info() {
    std::stringstream stream;

    stream << '\t' << " DPUHeldSMT: " << stallCycles;

    return stream.str();
}
;

std::string p6DPUHeldSMT::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_DPU_HELD_SMT>" << stallCycles << "</PM_DPU_HELD_SMT>" << std::endl;

    return stream.str();
}
