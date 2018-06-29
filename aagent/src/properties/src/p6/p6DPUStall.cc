/**
   @file    p6DPUStall.cc
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

#include "p6DPUStall.h"
#include "p6DPUStallICache.h"
#include "p6DPUStallOther.h"
#include "Property.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6DPUStall::p6DPUStall( Context* ct, Context* phaseCt ) :
    Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    dpu_stall    = 0;
    cycles       = 0;
    importance   = 0.0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6DPUStall::~p6DPUStall() {
}

PropertyID p6DPUStall::id() {
    return P6DPUSTALL;
}

void p6DPUStall::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6DPUStall::condition() const {
    psc_dbgmsg( 10, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return importance > 10;
}

double p6DPUStall::confidence() const {
    return 1.0;
}

double p6DPUStall::severity() const {
    return ( double )stallCycles / ( double )phaseCycles * 100;
}

Context* p6DPUStall::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6DPUStall::request_metrics() {
    pdb->request( context, PSC_PM_DPU_WT );
    pdb->request( context, PSC_PM_RUN_CYC );
    pdb->request( phaseContext, PSC_PM_RUN_CYC );
    return ALL_INFO_GATHERED;
}

std::string p6DPUStall::name() {
    return "Cycles lost waiting for instructions";
}

void p6DPUStall::evaluate() {
    phaseCycles = pdb->get( phaseContext, PSC_PM_RUN_CYC );
    dpu_stall   = pdb->get( context, PSC_PM_DPU_WT );
    cycles      = pdb->get( context, PSC_PM_RUN_CYC );
    importance  = ( double )( dpu_stall ) / ( double )( phaseCycles ) * 100.0;
    stallCycles = dpu_stall;
}

Property* p6DPUStall::clone() {
    p6DPUStall* prop = new p6DPUStall( context, phaseContext );
    return prop;
}

Prop_List p6DPUStall::next() {
    Prop_List returnList;
    /*
       Property         *prop;
       prop=new p6DPUStallOther(get_context(),get_phaseContext());
       returnList.push_back(prop);

       prop=new p6DPUStallICache(get_context(),get_phaseContext());
       returnList.push_back(prop);
     */

    return returnList;
}

std::string p6DPUStall::info() {
    std::stringstream stream;

    stream << '\t' << " DPUStall: " << dpu_stall;

    return stream.str();
}
;

std::string p6DPUStall::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_DPU_WT>" << dpu_stall << "</PM_DPU_WT>" << std::endl;

    return stream.str();
}
