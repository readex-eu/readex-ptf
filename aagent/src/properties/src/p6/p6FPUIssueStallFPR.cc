/**
   @file    p6FPUIssueStallFPR.cc
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

#include "p6FPUIssueStallFPR.h"
#include "Property.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6FPUIssueStallFPR::p6FPUIssueStallFPR( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles = 0;
    stallCycles = 0;
    pm_var1     = 0;
    pm_var2     = 0;
    pm_var3     = 0;
    pm_var4     = 0;

    importance   = 0.0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6FPUIssueStallFPR::~p6FPUIssueStallFPR() {
}

PropertyID p6FPUIssueStallFPR::id() {
    return P6FPUISSUESTALLFPR;
}

void p6FPUIssueStallFPR::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6FPUIssueStallFPR::condition() const {
    psc_dbgmsg( 10, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return severity() > 10;
}

double p6FPUIssueStallFPR::confidence() const {
    return 1.0;
}

double p6FPUIssueStallFPR::severity() const {
    return importance * 100;
}

Context* p6FPUIssueStallFPR::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6FPUIssueStallFPR::request_metrics() {
    pdb->request( context, PSC_PM_FPU_ISSUE_STALL_FPR );
    pdb->request( context, PSC_PM_FPU_ISSUE_1 );
    pdb->request( context, PSC_PM_FPU_ISSUE_2 );
    pdb->request( context, PSC_PM_FPU_ISSUE_0 );

    return ALL_INFO_GATHERED;
}

std::string p6FPUIssueStallFPR::name() {
    return "Floating point instructions execution slowed down due to register conflicts";
}

void p6FPUIssueStallFPR::evaluate() {
    pm_var1 = pdb->get( context, PSC_PM_FPU_ISSUE_STALL_FPR );
    pm_var2 = pdb->get( context, PSC_PM_FPU_ISSUE_1 );
    pm_var3 = pdb->get( context, PSC_PM_FPU_ISSUE_2 );
    pm_var4 = pdb->get( context, PSC_PM_FPU_ISSUE_0 );

    if( pm_var4 > 0 ) {
        importance = ( double )pm_var1 / ( double )pm_var4;
    }
    else {
        importance = 0.0;
    }
}

Property* p6FPUIssueStallFPR::clone() {
    p6FPUIssueStallFPR* prop = new p6FPUIssueStallFPR( context, phaseContext );
    return prop;
}

std::string p6FPUIssueStallFPR::info() {
    std::stringstream stream;

    stream << '\t' << " FPUIssueStallFPR: " << importance;

    return stream.str();
}
;

std::string p6FPUIssueStallFPR::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_FPU_ISSUE_STALL_FPR>" << pm_var1 << "</PM_FPU_ISSUE_STALL_FPR>" << std::endl;
    stream << "\t\t<PM_FPU_ISSUE_1>" << pm_var2 << "</PM_FPU_ISSUE_1>" << std::endl;
    stream << "\t\t<PM_FPU_ISSUE_2>" << pm_var3 << "</PM_FPU_ISSUE_2>" << std::endl;
    stream << "\t\t<PM_FPU_ISSUE_0>" << pm_var4 << "</PM_FPU_ISSUE_0>" << std::endl;
    stream << "\t\t<FPUIssueStallFPR>" << importance << "</FPUIssueStallFPR>" << std::endl;

    return stream.str();
}
