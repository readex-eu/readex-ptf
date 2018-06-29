/**
   @file    L1D.cc
   @ingroup BenchmarkingProperties
   @brief   L1 Data Cache Misses Ratio Property
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

#include "L1D.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

PropertyID L1D::id() {
    return L1DMETRIC;
}

void L1D::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool L1D::condition() const {
    if( PAPI_L1 == -1 ) {
        return false;
    }
    else {
        return true;
    }
}

double L1D::confidence() const {
    return 1.0;
}

double L1D::severity() const {
    return evaluationResult * ( cycles / phaseCycles );
}

Context* L1D::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type L1D::request_metrics() {
    pdb->request( context, PSC_PAPI_L1_DCM );
    pdb->request( context, PSC_PAPI_L1_DCH );
    pdb->request( context, PSC_PAPI_TOT_CYC );
    pdb->request( phaseContext, PSC_PAPI_TOT_CYC );

    return ALL_INFO_GATHERED;
}

std::string L1D::name() {
    return "L1D";
}

void L1D::evaluate() {
    phaseCycles = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );
    cycles      = pdb->get( context, PSC_PAPI_TOT_CYC );

    PAPI_L1   = pdb->get( context, PSC_PAPI_L1_DCM );
    PAPI_L1_H = pdb->get( context, PSC_PAPI_L1_DCH );

    evaluationResult = ( double )PAPI_L1 / double( PAPI_L1 + PAPI_L1_H );
}

Property* L1D::clone() {
    L1D* prop = new L1D( context, phaseContext );
    return prop;
}

std::string L1D::info() {
    std::stringstream stream;

    stream << '\t' << " PAPI_L1_DCM: " << PAPI_L1;

    return stream.str();
}

std::string L1D::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<PAPI_L1_DCM>" << PAPI_L1 << "</PAPI_L1_DCM>" << std::endl;
    stream << "\t\t<PAPI_L1_DCH>" << PAPI_L1_H << "</PAPI_L1_DCH>" << std::endl;

    stream << "\t\t<phaseCycles>" << phaseCycles << "</phaseCycles>" << std::endl;
    stream << "\t\t<cycles>" << cycles << "</cycles>" << std::endl;

    return stream.str();
}
