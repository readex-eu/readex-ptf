/**
   @file    ActiveCyclesObjective.cc
   @ingroup TuningProperties
   @brief   Active Cycles Objective
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

//Description :  Tells you the total number of cycles a multiprocessor has at least one active warp
//CUDA Formula:  PSC_CUPTI_DD_ACTIVE_CYCLES
//Recommendation :
//


#include "ActiveCyclesObjective.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"
#include <sstream>

ActiveCyclesObjective::ActiveCyclesObjective( Context* ct,
                                              Context* phaseCt,
                                              double   threshold ) :
    Property( ct ),
    threshold( threshold ) {
    evaluationResult = 0.0;
    phaseContext     = phaseCt;
}

ActiveCyclesObjective::~ActiveCyclesObjective() {
}

PropertyID ActiveCyclesObjective::id() {
    return ACTIVECYCLESOBJECTIVE;
}

void ActiveCyclesObjective::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread "
              << context->getThread() << std::endl << "                  "
              << context->getRegion()->str_print() << std::endl;
}

bool ActiveCyclesObjective::condition() const {
    return activeCycles > 0 && severity() > threshold;
}

double ActiveCyclesObjective::confidence() const {
    return 1.0;
}

double ActiveCyclesObjective::severity() const {
    return evaluationResult;
}

Context* ActiveCyclesObjective::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type ActiveCyclesObjective::request_metrics() {
    pdb->request( context, PSC_CUPTI_DD_ACTIVE_CYCLES );

    return ALL_INFO_GATHERED;
}

std::string ActiveCyclesObjective::name() {
    return "Active cycles objective";
}

void ActiveCyclesObjective::evaluate() {
    activeCycles = pdb->get( context, PSC_CUPTI_DD_ACTIVE_CYCLES );

    evaluationResult = ( double )( activeCycles );
}

Property* ActiveCyclesObjective::clone() {
    ActiveCyclesObjective* prop = new ActiveCyclesObjective( context, phaseContext, threshold );
    return prop;
}

std::string ActiveCyclesObjective::info() {
    std::stringstream stream;

    stream << '\t' << " ACTIVE_CYCLES: " << activeCycles;

    return stream.str();
}

std::string ActiveCyclesObjective::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<ACTIVE_CYCLES>" << activeCycles << "</ACTIVE_CYCLES>" << std::endl;

    return stream.str();
}
