/**
   @file    AchievedOccupancy.cc
   @ingroup CUDAProperties
   @brief   GPU Achieved occupancy property
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


#include "AchievedOccupancy.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

//Description :  Tells you the total number of cycles a multiprocessor has at least one active warp
//CUDA Formula:  100*(PSC_CUPTI_DD_ACTIVE_WARPS/PSC_CUPTI_DD_ACTIVE_CYCLES)/48
//Recommendation :
//


AchievedOccupancy::AchievedOccupancy( Context* ct, Context* phaseCt ) : Property( ct ) {
    evaluationResult = 0.0;
    phaseContext     = phaseCt;
}

AchievedOccupancy::~AchievedOccupancy() {
}

PropertyID AchievedOccupancy::id() {
    return CUDA_ACHIEVEDOCCUPANCY;
}

void AchievedOccupancy::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread "
              << context->getThread() << std::endl << "                  "
              << context->getRegion()->str_print() << std::endl;
}

bool AchievedOccupancy::condition() const {
    return Active_Cycles > 0 && severity() > 0;
}

double AchievedOccupancy::confidence() const {
    return 1.0;
}

double AchievedOccupancy::severity() const {
    return evaluationResult;
}

Context* AchievedOccupancy::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type AchievedOccupancy::request_metrics() {
    pdb->request( context, PSC_CUPTI_DD_ACTIVE_CYCLES );
    pdb->request( context, PSC_CUPTI_DD_ACTIVE_WARPS );

    return ALL_INFO_GATHERED;
}

std::string AchievedOccupancy::name() {
    return "Active cycles";
}

void AchievedOccupancy::evaluate() {
    Active_Warps     = pdb->get( context, PSC_CUPTI_DD_ACTIVE_WARPS );
    Active_Cycles    = pdb->get( context, PSC_CUPTI_DD_ACTIVE_CYCLES );
    evaluationResult = ( double )( 100 * ( Active_Warps / Active_Cycles ) / 48 );
}

Property* AchievedOccupancy::clone() {
    AchievedOccupancy* prop = new AchievedOccupancy( context, phaseContext );
    return prop;
}

std::string AchievedOccupancy::info() {
    std::stringstream stream;

    stream << '\t' << " Active_Warps: " << Active_Warps  << '\t' << " Active_Cyles: " << Active_Cycles;

    return stream.str();
}

std::string AchievedOccupancy::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<Active_Warps>" << Active_Warps << "</Active_Warps>" << std::endl;
    stream << "\t\t<Active_Cycles>" << Active_Cycles << "</Active_Cycles>" << std::endl;

    return stream.str();
}
