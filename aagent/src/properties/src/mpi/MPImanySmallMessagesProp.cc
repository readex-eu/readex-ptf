/**
   @file    MPImanySmallMessagesProp.cc
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

#include "MPImanySmallMessagesProp.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <sstream>
#include "psc_errmsg.h"

#define SIZE_THRESHOLD 1000

using namespace std;

PropertyID MPImanySmallMessagesProp::id() {
    return MPIPROP;
}

std::string MPImanySmallMessagesProp::subId() {
    std::stringstream sstr;
    sstr << PSC_MPI_AGGREGATE_MESSAGE_SIZE << "-S";
    return sstr.str();
}

void MPImanySmallMessagesProp::print() {
    stringstream str;

    str << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
        << context->getRegion()->str_print();
    string help = str.str();
    psc_dbgmsg( 4, "%s\n", help.c_str() );
}

bool MPImanySmallMessagesProp::condition() const {
    return severity() > threshold && agregateSize / callCount < SIZE_THRESHOLD;
}

double MPImanySmallMessagesProp::confidence() const {
    return 1.0;
}

double MPImanySmallMessagesProp::severity() const {
    //printf("Severity %f\n",((double) siteTime*100) / phaseTime);
    if( phaseTime ) {
        double severity = siteTime;
        severity = severity * 100 / phaseTime;
        return severity;
    }
    else {
        return .0;
    }
}

Gather_Required_Info_Type MPImanySmallMessagesProp::request_metrics() {
    pdb->request( context, PSC_MPI );
    pdb->request( startCtx, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string MPImanySmallMessagesProp::name() {
    return "Many small messages";
}

void MPImanySmallMessagesProp::evaluate() {
    callCount = pdb->get( context, PSC_MPI_CALL_COUNT );

    siteTime     = pdb->get( context, PSC_MPI_TIME_SPENT ); //Measured in microseconds
    phaseTime    = pdb->get( startCtx, PSC_EXECUTION_TIME ) / ( NANOSEC_PER_SEC / 1000000 );
    agregateSize = pdb->get( context, PSC_MPI_AGGREGATE_MESSAGE_SIZE );
    msgSize      = agregateSize / callCount;

    //std::cout << "Property: " << m << " (" << name() << ")  Process " << context->getRank()
    //          << " lateTime " << lateTime << " phaseTime " << phaseTime << " totalCount " << totalCount << std::endl;
}

std::string MPImanySmallMessagesProp::info() {
    std::stringstream stream;

    stream << '\t' << ">> Message Size: " << agregateSize << " Call count: " << callCount;

    return stream.str();
}

Property* MPImanySmallMessagesProp::clone() {
    MPImanySmallMessagesProp* prop = new MPImanySmallMessagesProp( context, startCtx );
    return prop;
}

std::string MPImanySmallMessagesProp::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<CallTime>" << siteTime << "</CallTime>" << std::endl;
    stream << "\t\t<PhaseTime>" << phaseTime << "</PhaseTime>" << std::endl;

    return stream.str();
}
