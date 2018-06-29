/**
   @file    MPIlargeMessagesProp.cc
   @ingroup MPIProperties
   @brief   MPI specific property
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

#include "MPIlargeMessagesProp.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <sstream>
#include "psc_errmsg.h"

#define SIZE_THRESHOLD 1000000

using namespace std;

PropertyID MPIlargeMessagesProp::id() {
    return MPIPROP;
}

/// @todo SubId type might not be unique some day so it is not defined explicitly
std::string MPIlargeMessagesProp::subId() {
    std::stringstream sstr;
    sstr << PSC_MPI_AGGREGATE_MESSAGE_SIZE << "-L";
    return sstr.str();
}

void MPIlargeMessagesProp::print() {
    stringstream str;

    str << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
        << context->getRegion()->str_print();
    string help = str.str();
    psc_dbgmsg( 4, "%s\n", help.c_str() );
}

bool MPIlargeMessagesProp::condition() const {
    return severity() > threshold && agregateSize / callCount > SIZE_THRESHOLD;
}

double MPIlargeMessagesProp::confidence() const {
    return 1.0;
}

double MPIlargeMessagesProp::severity() const {
    //printf("Severity %f\n",((double) lateTime*100) / phaseTime);
    if( phaseTime ) {
        double severity = siteTime;
        severity = severity * 100 / phaseTime;
        return severity;
    }
    else {
        return .0;
    }
}

Gather_Required_Info_Type MPIlargeMessagesProp::request_metrics() {
    pdb->request( context, PSC_MPI );
    pdb->request( startCtx, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string MPIlargeMessagesProp::name() {
    return "Large messages";
}

void MPIlargeMessagesProp::evaluate() {
    callCount = pdb->get( context, PSC_MPI_CALL_COUNT );

    siteTime     = pdb->get( context, PSC_MPI_TIME_SPENT ); //Measured in microseconds
    phaseTime    = pdb->get( startCtx, PSC_EXECUTION_TIME ) / ( NANOSEC_PER_SEC_DOUBLE / 1000000 );
    agregateSize = pdb->get( context, PSC_MPI_AGGREGATE_MESSAGE_SIZE );
    msgSize      = agregateSize / callCount;
    //std::cout << "Property: " << m << " (" << name() << ")  Process " << context->getRank()
    //          << " lateTime " << lateTime << " phaseTime " << phaseTime << " totalCount " << totalCount << std::endl;
}

std::string MPIlargeMessagesProp::info() {
    std::stringstream stream;

    stream << '\t' << ">> Message Size: " << agregateSize << " Call count: " << callCount;

    return stream.str();
}

Property* MPIlargeMessagesProp::clone() {
    MPIlargeMessagesProp* prop = new MPIlargeMessagesProp( context, startCtx );
    return prop;
}

std::string MPIlargeMessagesProp::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<CallTime>" << siteTime << "</CallTime>" << std::endl;
    stream << "\t\t<PhaseTime>" << phaseTime << "</PhaseTime>" << std::endl;

    return stream.str();
}
