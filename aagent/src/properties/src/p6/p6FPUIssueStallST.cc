/**
   @file    p6FPUIssueStallST.cc
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

#include "p6FPUIssueStallST.h"
#include "Property.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6FPUIssueStallST::p6FPUIssueStallST( Context* ct, Context* phaseCt ) : Property( ct ) {
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

p6FPUIssueStallST::~p6FPUIssueStallST() {
}

PropertyID p6FPUIssueStallST::id() {
    return P6FPUISSUESTALLST;
}

void p6FPUIssueStallST::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6FPUIssueStallST::condition() const {
    psc_dbgmsg( 3, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return severity() > 10;
}

double p6FPUIssueStallST::confidence() const {
    return 1.0;
}

double p6FPUIssueStallST::severity() const {
    return importance * 100;
}

Context* p6FPUIssueStallST::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6FPUIssueStallST::request_metrics() {
    pdb->request( context, PSC_PM_FPU_ISSUE_STALL_ST );
    pdb->request( context, PSC_PM_FPU_ISSUE_0 );
    pdb->request( context, PSC_PM_FPU_ISSUE_1 );
    pdb->request( context, PSC_PM_FPU_ISSUE_2 );

    return ALL_INFO_GATHERED;
}

std::string p6FPUIssueStallST::name() {
    return "Floating point execution is inhibited by store instructions";
}

void p6FPUIssueStallST::evaluate() {
    pm_var1 = pdb->get( context, PSC_PM_FPU_ISSUE_STALL_ST );
    pm_var2 = pdb->get( context, PSC_PM_FPU_ISSUE_0 );
    pm_var3 = pdb->get( context, PSC_PM_FPU_ISSUE_1 );
    pm_var4 = pdb->get( context, PSC_PM_FPU_ISSUE_2 );

    if( pm_var2 > 0 ) {
        importance = ( double )pm_var1 / ( double )pm_var2;
    }
    else {
        importance = 0.0;
    }
}

Property* p6FPUIssueStallST::clone() {
    p6FPUIssueStallST* prop = new p6FPUIssueStallST( context, phaseContext );
    return prop;
}

std::string p6FPUIssueStallST::info() {
    std::stringstream stream;

    stream << '\t' << " FPUIssueStallST: " << importance;

    return stream.str();
}
;

std::string p6FPUIssueStallST::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_FPU_ISSUE_STALL_ST>" << pm_var1 << "</PM_FPU_ISSUE_STALL_ST>" << std::endl;
    stream << "\t\t<PM_FPU_ISSUE_0>" << pm_var2 << "</PM_FPU_ISSUE_0>" << std::endl;
    stream << "\t\t<PM_FPU_ISSUE_1>" << pm_var3 << "</PM_FPU_ISSUE_1>" << std::endl;
    stream << "\t\t<PM_FPU_ISSUE_2>" << pm_var4 << "</PM_FPU_ISSUE_2>" << std::endl;
    stream << "\t\t<FPUIssueStallST>" << importance << "</FPUIssueStallST>" << std::endl;

    return stream.str();
}
