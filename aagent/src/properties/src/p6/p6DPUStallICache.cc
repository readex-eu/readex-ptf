/**
   @file    p6DPUStallICache.cc
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

#include "p6DPUStallICache.h"
#include "Property.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6DPUStallICache::p6DPUStallICache( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    dpu_stall    = 0;
    importance   = 0.0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6DPUStallICache::~p6DPUStallICache() {
}

PropertyID p6DPUStallICache::id() {
    return P6DPUSTALLICACHE;
}

void p6DPUStallICache::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6DPUStallICache::condition() const {
    psc_dbgmsg( 10, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return importance > 1;
}

double p6DPUStallICache::confidence() const {
    return 1.0;
}

double p6DPUStallICache::severity() const {
    return ( double )stallCycles / ( double )phaseCycles * 100;
}

Context* p6DPUStallICache::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6DPUStallICache::request_metrics() {
    pdb->request( context, PSC_PM_DPU_WT_IC_MISS );
    pdb->request( phaseContext, PSC_PM_RUN_CYC );
    pdb->request( context, PSC_PM_RUN_CYC );
    return ALL_INFO_GATHERED;
}

std::string p6DPUStallICache::name() {
    return "Cycles lost due to instruction cache misses";
}

void p6DPUStallICache::evaluate() {
    phaseCycles = pdb->get( phaseContext, PSC_PM_RUN_CYC );
    dpu_stall   = pdb->get( context, PSC_PM_DPU_WT_IC_MISS );
    cycles      = pdb->get( context, PSC_PM_RUN_CYC );
    importance  = ( double )( dpu_stall ) / ( double )( phaseCycles ) * 100;
    stallCycles = dpu_stall;
}

Property* p6DPUStallICache::clone() {
    p6DPUStallICache* prop = new p6DPUStallICache( context, phaseContext );
    return prop;
}

std::string p6DPUStallICache::info() {
    std::stringstream stream;

    stream << '\t' << " DPUStallICache: " << dpu_stall;

    return stream.str();
}
;

std::string p6DPUStallICache::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_DPU_WT_IC_MISS>" << dpu_stall << "</PM_DPU_WT_IC_MISS>" << std::endl;

    return stream.str();
}
