/**
   @file    BytesTransfered.cc
   @ingroup CUDAProperties
   @brief   GPU Amount of bytes transferred by between a host and a device Property
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


#include "BytesTransfered.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

//Description :  Tells you amount of bytes transfered between host and device
//Formula:       PSC_CUDA_BYTES_TRANSFERRED
//Recommendation:
//


BytesTransfered::BytesTransfered( Context* ct, Context* phaseCt ) : Property( ct ) {
    evaluationResult = 0.0;
    phaseContext     = phaseCt;
}

BytesTransfered::~BytesTransfered() {
}

PropertyID BytesTransfered::id() {
    return CUDA_BYTESTRANSFERED;
}

void BytesTransfered::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread "
              << context->getThread() << std::endl << "                  "
              << context->getRegion()->str_print() << std::endl;
}

bool BytesTransfered::condition() const {
    return Bytes_Transfered > 0 && severity() > 0;
}

double BytesTransfered::confidence() const {
    return 1.0;
}

double BytesTransfered::severity() const {
    return evaluationResult;
}

Context* BytesTransfered::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type BytesTransfered::request_metrics() {
    pdb->request( context, PSC_CUDA_BYTES_TRANSFERRED );

    return ALL_INFO_GATHERED;
}

std::string BytesTransfered::name() {
    return "Bytes Transfered";
}

void BytesTransfered::evaluate() {
    Bytes_Transfered = pdb->get( context, PSC_CUDA_BYTES_TRANSFERRED );

    evaluationResult = ( double )( Bytes_Transfered );
}

Property* BytesTransfered::clone() {
    BytesTransfered* prop = new BytesTransfered( context, phaseContext );
    return prop;
}

std::string BytesTransfered::info() {
    std::stringstream stream;

    stream << '\t' << " Bytes_Transfered: " << Bytes_Transfered;

    return stream.str();
}

std::string BytesTransfered::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<Bytes_Transfered>" << Bytes_Transfered << "</Bytes_Transfered>" << std::endl;

    return stream.str();
}
