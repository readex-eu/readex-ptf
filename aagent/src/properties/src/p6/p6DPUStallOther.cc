/**
   @file    p6DPUStallOther.cc
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

#include "p6DPUStallOther.h"
#include "Property.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6DPUStallOther::p6DPUStallOther( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    importance   = 0.0;
    threshold    = 1.0;
    br_pred      = 0;
    br_mpred     = 0;
    phaseContext = phaseCt;
}

p6DPUStallOther::~p6DPUStallOther() {
}

PropertyID p6DPUStallOther::id() {
    return P6DPUSTALLOTHER;
}

void p6DPUStallOther::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6DPUStallOther::condition() const {
    psc_dbgmsg( 10, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return importance > 10;
}

double p6DPUStallOther::confidence() const {
    return 1.0;
}

double p6DPUStallOther::severity() const {
    return importance;
}

Context* p6DPUStallOther::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6DPUStallOther::request_metrics() {
    pdb->request( context, PSC_PM_BR_MPRED );
    pdb->request( context, PSC_PM_BR_PRED );
    return ALL_INFO_GATHERED;
}

std::string p6DPUStallOther::name() {
    return "Branch misprediction rate is too high";
}

void p6DPUStallOther::evaluate() {
    phaseCycles = pdb->get( phaseContext, PSC_PM_RUN_CYC );
    br_pred     = pdb->get( context, PSC_PM_BR_PRED );
    br_mpred    = pdb->get( context, PSC_PM_BR_MPRED );

    importance = ( double )( br_mpred ) / ( double )( br_pred ) * 100.0;
}

Property* p6DPUStallOther::clone() {
    p6DPUStallOther* prop = new p6DPUStallOther( context, phaseContext );
    return prop;
}

std::string p6DPUStallOther::info() {
    std::stringstream stream;

    stream << '\t' << " BR_MPRED: " << br_mpred;

    return stream.str();
}
;

std::string p6DPUStallOther::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<BR_MPRED>" << br_mpred << "</BR_MPRED>" << std::endl;
    stream << "\t\t<BR_PRED>" << br_pred << "</BR_PRED>" << std::endl;

    return stream.str();
}
