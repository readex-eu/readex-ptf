/**
   @file    p6DPUHeldFPQ.cc
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

#include "p6DPUHeldFPQ.h"
#include "Property.h"
#include <iostream>

#include "p6FPUIssueStallFPR.h"
#include "p6FPUIssueStallST.h"
#include "p6FPUDivSqrt.h"
#include "p6FPUFMARate.h"

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6DPUHeldFPQ::p6DPUHeldFPQ( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    importance   = 0.0;
    cycles       = 0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6DPUHeldFPQ::~p6DPUHeldFPQ() {
}

PropertyID p6DPUHeldFPQ::id() {
    return P6DPUHELDFPQ;
}

void p6DPUHeldFPQ::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6DPUHeldFPQ::condition() const {
    psc_dbgmsg( 10, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return importance > 10;
}

double p6DPUHeldFPQ::confidence() const {
    return 1.0;
}

double p6DPUHeldFPQ::severity() const {
    return ( double )stallCycles / ( double )phaseCycles * 100.0;
}

Context* p6DPUHeldFPQ::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6DPUHeldFPQ::request_metrics() {
    pdb->request( context, PSC_PM_RUN_CYC );
    pdb->request( context, PSC_PM_DPU_HELD_FPQ );
    pdb->request( phaseContext, PSC_PM_RUN_CYC );
    return ALL_INFO_GATHERED;
}

std::string p6DPUHeldFPQ::name() {
    return "Cycles lost due to floating point instructions execution inefficiencies";
}

void p6DPUHeldFPQ::evaluate() {
    cycles      = pdb->get( context, PSC_PM_RUN_CYC );
    phaseCycles = pdb->get( phaseContext, PSC_PM_RUN_CYC );
    stallCycles = pdb->get( context, PSC_PM_DPU_HELD_FPQ );
    importance  = ( double )( stallCycles ) / ( double )( phaseCycles ) * 100.0;
}

Property* p6DPUHeldFPQ::clone() {
    p6DPUHeldFPQ* prop = new p6DPUHeldFPQ( context, phaseContext );
    return prop;
}

Prop_List p6DPUHeldFPQ::next() {
    Prop_List returnList;
    returnList.push_back( new p6FPUIssueStallFPR( context, phaseContext ) );
    returnList.push_back( new p6FPUIssueStallST( context, phaseContext ) );
    returnList.push_back( new p6FPUDivSqrt( context, phaseContext ) );
    //returnList.push_back(new p6FPUIssueDivOVLP(context,phaseContext)); Change to OOO
    returnList.push_back( new p6FPUFMARate( context, phaseContext ) );

    return returnList;
}

std::string p6DPUHeldFPQ::info() {
    std::stringstream stream;

    stream << '\t' << " DPUHeldFPQ: " << stallCycles;

    return stream.str();
}
;

std::string p6DPUHeldFPQ::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_DPU_HELD_FPQ>" << stallCycles << "</PM_DPU_HELD_FPQ>" << std::endl;

    return stream.str();
}
