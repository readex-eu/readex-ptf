/**
   @file    BranchEfficiency.cc
   @ingroup CUDAProperties
   @brief   GPU Ratio of non-divergent branches to total branches property
   @author  Robert Mijakovic
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

#include "BranchEfficiency.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

//Description :  Tells you the fraction of non-divergent branches to total branches
//CUDA Formula:  (PSC_CUPTI_DD_BRANCH - PSC_CUPTI_DD_DIVERGENT_BRANCH) / PSC_CUPTI_DD_BRANCH
//Recommendation : The code is probably impacted by non-divergent that serializes warp execution
//


BranchEfficiency::BranchEfficiency( Context* ct, Context* phaseCt ) : Property( ct ) {
    evaluationResult = 0.0;
    phaseContext     = phaseCt;
}

BranchEfficiency::~BranchEfficiency() {
}

PropertyID BranchEfficiency::id() {
    return CUDA_BRANCHEFFICIENCY;
}

void BranchEfficiency::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread "
              << context->getThread() << std::endl << "                  "
              << context->getRegion()->str_print() << std::endl;
}

bool BranchEfficiency::condition() const {
    return Branch > 0 && severity() > 0;
}

double BranchEfficiency::confidence() const {
    return 1.0;
}

double BranchEfficiency::severity() const {
    return evaluationResult;
}

Context* BranchEfficiency::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type BranchEfficiency::request_metrics() {
    pdb->request( context, PSC_CUPTI_DD_DIVERGENT_BRANCH );
    pdb->request( context, PSC_CUPTI_DD_BRANCH );

    return ALL_INFO_GATHERED;
}

std::string BranchEfficiency::name() {
    return "Branch Efficiency";
}

void BranchEfficiency::evaluate() {
    Branch           = pdb->get( context, PSC_CUPTI_DD_BRANCH );
    Divergent_Branch = pdb->get( context, PSC_CUPTI_DD_DIVERGENT_BRANCH );

    evaluationResult = ( double )( Branch - Divergent_Branch ) / ( double )Branch;
}

Property* BranchEfficiency::clone() {
    BranchEfficiency* prop = new BranchEfficiency( context, phaseContext );
    return prop;
}

std::string BranchEfficiency::info() {
    std::stringstream stream;

    stream << '\t' << " Branch: " << Branch << '\t' << " Divergent_Branch: "
           << Divergent_Branch;

    return stream.str();
}

std::string BranchEfficiency::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<Branch>" << Branch << "</Branch>" << std::endl;
    stream << "\t\t<Divergent_Branch>" << Divergent_Branch << "</Divergent_Branch>" << std::endl;

    return stream.str();
}
