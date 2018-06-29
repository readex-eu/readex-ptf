/**
   @file    p6LSULHSReject.cc
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

#include "p6LSULHSReject.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6LSULHSReject::p6LSULHSReject( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    rejects      = 0;
    conv_ops     = 0;
    instructions = 0;
    LHSrejects   = 0;
    importance   = 0.0;
    threshold    = 1.0;
    phaseContext = phaseCt;
}

p6LSULHSReject::~p6LSULHSReject() {
}

PropertyID p6LSULHSReject::id() {
    return P6LSULHSREJ;
}

void p6LSULHSReject::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6LSULHSReject::condition() const {
    //psc_dbgmsg(3,"Rank %d: %s :%s: condition=%f, severity=%f\n",phaseContext->getRank(),name().c_str(),get_region()->str_print().c_str(),importance,severity());
    //return ( conv_ops > 10 || LHSrejects > 1000);
    return false;
}

double p6LSULHSReject::confidence() const {
    return 1.0;
}

double p6LSULHSReject::severity() const {
    if( !phaseCycles ) {
        return 0;
    }
    return ( double )LHSrejects / ( double )phaseCycles * 100.0;
}

Context* p6LSULHSReject::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6LSULHSReject::request_metrics() {
    pdb->request( context, PSC_PM_LSU_REJECT_LHS );
    pdb->request( context, PSC_PM_LD_REF_L1 );
    pdb->request( context, PSC_PM_ST_REF_L1 );
    pdb->request( context, PSC_PM_FPU_FCONV );
    pdb->request( context, PSC_PM_RUN_INST_CMPL );
    pdb->request( context, PSC_PM_RUN_CYC );
    return ALL_INFO_GATHERED;
}

std::string p6LSULHSReject::name() {
    return "LHS rejects per load(load operation is dependent on the not yet completed store)";
}

void p6LSULHSReject::evaluate() {
    l1ref        = pdb->get( context, PSC_PM_LD_REF_L1 );
    stref        = pdb->get( context, PSC_PM_ST_REF_L1 );
    LHSrejects   = pdb->get( context, PSC_PM_LSU_REJECT_LHS );
    conv_ops     = pdb->get( context, PSC_PM_FPU_FCONV );
    instructions = pdb->get( context, PSC_PM_RUN_INST_CMPL );
    phaseCycles  = pdb->get( context, PSC_PM_RUN_CYC );
    st_reject    = pdb->get( context, PSC_PM_LSU_REJECT_STQ_FULL );
    importance   = ( double )( LHSrejects );
    stallCycles  = LHSrejects;
}

Property* p6LSULHSReject::clone() {
    p6LSULHSReject* prop = new p6LSULHSReject( context, phaseContext );
    return prop;
}

std::string p6LSULHSReject::info() {
    std::stringstream stream;

    stream << '\t' << " LHSrejects: " << LHSrejects;

    return stream.str();
}
;

std::string p6LSULHSReject::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PM_LSU_REJECT_LHS>" << LHSrejects << "</PM_LSU_REJECT_LHS>" << std::endl;
    stream << "\t\t<PM_LD_REF_L1>" << l1ref << "</PM_LD_REF_L1>" << std::endl;
    stream << "\t\t<PM_LSU_REJECT_STQ_FULL>" << st_reject << "</PM_LSU_REJECT_STQ_FULL>" << std::endl;
    stream << "\t\t<PM_ST_REF_L1>" << stref << "</PM_ST_REF_L1>" << std::endl;
    stream << "\t\t<PM_FPU_FCONV>" << conv_ops << "</PM_FPU_FCONV>" << std::endl;
    stream << "\t\t<PM_RUN_INST_CMPL>" << instructions << "</PM_RUN_INST_CMPL>" << std::endl;
    return stream.str();
}
