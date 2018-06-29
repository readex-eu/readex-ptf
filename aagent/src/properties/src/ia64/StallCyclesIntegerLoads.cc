/**
   @file    StallCyclesIntegerLoads.cc
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

#include "StallCyclesIntegerLoads.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>

PropertyID StallCyclesIntegerLoadsProp::id() {
    return STALLCYCLESINTEGERLOADS;
}

void StallCyclesIntegerLoadsProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool StallCyclesIntegerLoadsProp::condition() const {
    return ( ( double )stallCyclesGRALL - stallCyclesGRGR ) / ( double )phaseCycles * 100 > threshold;
}

double StallCyclesIntegerLoadsProp::confidence() const {
    return 1.0;
}

double StallCyclesIntegerLoadsProp::severity() const {
    return ( double )( stallCyclesGRALL - stallCyclesGRGR ) / ( double )phaseCycles * 100;
}

Context* StallCyclesIntegerLoadsProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type StallCyclesIntegerLoadsProp::request_metrics() {
    pdb->request( context, PSC_BE_EXE_BUBBLE_GRALL );
    pdb->request( context, PSC_BE_EXE_BUBBLE_GRGR );
    pdb->request( phaseContext, PSC_PAPI_TOT_CYC );
    return ALL_INFO_GATHERED;
}

std::string StallCyclesIntegerLoadsProp::name() {
    return "Stalls due to waiting for integer loads";
}

void StallCyclesIntegerLoadsProp::evaluate() {
    stallCyclesGRALL = pdb->get( context, PSC_BE_EXE_BUBBLE_GRALL );
    stallCyclesGRGR  = pdb->get( context, PSC_BE_EXE_BUBBLE_GRGR );
    phaseCycles      = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );
}

Property* StallCyclesIntegerLoadsProp::clone() {
    StallCyclesIntegerLoadsProp* prop = new StallCyclesIntegerLoadsProp( context, phaseContext );
    return prop;
}
