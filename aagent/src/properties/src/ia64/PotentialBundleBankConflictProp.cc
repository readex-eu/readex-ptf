/**
   @file    PotentialBundleBankConflictProp.cc
   @ingroup Itanium2Properites
   @brief   Itanium2 specific property
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

#include "PotentialBundleBankConflictProp.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>


PropertyID PotentialBundleBankConflictProp::id() {
    return POTENTIALBUNDLEBANKCONFLICT;
}

void PotentialBundleBankConflictProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool PotentialBundleBankConflictProp::condition() const {
    //If there exist pairs of bundles that might lead to
    //bank conflicts.
    return loadBundles > threshold;
}

double PotentialBundleBankConflictProp::confidence() const {
    return 1.0;
}

double PotentialBundleBankConflictProp::severity() const {
    double percentLoads;
    INT64  totalLoadBundles, conflLoads;

    conflLoads       = 4 * loadBundles;
    percentLoads     = ( double )conflLoads / ( double )instrInBody;
    totalLoadBundles = totalInstructionsIssued * ( ( INT64 )percentLoads / 4 );

    //Assuming conflict costs 2 cycles
    return ( totalLoadBundles * 2 ) / phaseCycles * 100;
}

Context* PotentialBundleBankConflictProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type PotentialBundleBankConflictProp::request_metrics() {
    pdb->request( context, PSC_FOUR_LOADS_IN_SUBSEQUENT_BUNDLES );
    pdb->request( context, PSC_PAPI_TOT_INS );
    pdb->request( context, PSC_INSTRUCTIONS_IN_LOOP_BODY );
    pdb->request( phaseContext, PSC_PAPI_TOT_CYC );
    return ALL_INFO_GATHERED;
}

std::string PotentialBundleBankConflictProp::name() {
    return "Potential bank conflict due to 4 loads in bundles";
}

void PotentialBundleBankConflictProp::evaluate() {
    loadBundles             = pdb->get( context, PSC_FOUR_LOADS_IN_SUBSEQUENT_BUNDLES );
    totalInstructionsIssued = pdb->get( context, PSC_PAPI_TOT_INS );
    instrInBody             = pdb->get( context, PSC_INSTRUCTIONS_IN_LOOP_BODY );
    phaseCycles             = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );
}

Property* PotentialBundleBankConflictProp::clone() {
    PotentialBundleBankConflictProp* prop = new PotentialBundleBankConflictProp( context, phaseContext );
    return prop;
}

std::string PotentialBundleBankConflictProp::info() {
    std::stringstream stream;

    stream << '\t' << " Bundles with 4 loads in loop body: " << loadBundles;

    return stream.str();
}
;

std::string PotentialBundleBankConflictProp::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<loadBundles>" << loadBundles << "</loadBundles>" << std::endl;

    return stream.str();
}
