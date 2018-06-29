/**
   @file    OverloadedMasterProp.cc
   @ingroup OverloadedMasterProp
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

#include "OverloadedMasterProp.h"
#include "global.h"
#include <iostream>
#include <sstream>
#include "psc_errmsg.h"

using namespace std;

PropertyID OverloadedMasterProp::id() {
    return OVERLOADEDMASTER;
}

std::string OverloadedMasterProp::subId() {
    std::stringstream sstr;
    sstr << PSC_MPI_TIME_SPENT;
    return sstr.str();
}

void OverloadedMasterProp::print() {
    stringstream str;

    str << "Property:" << name() << "  Process " << context->getRank()
        << "  Thread " << context->getThread() << context->getRegion()->str_print();
    string help = str.str();
    psc_dbgmsg( 4, "%s\n", help.c_str() );
}

bool OverloadedMasterProp::condition() const {
    return severity() > threshold;
}

double OverloadedMasterProp::confidence() const {
    return 1.0;
}

double OverloadedMasterProp::severity() const {
    if( phaseTime ) {
        double severity = lateTime;
        severity = severity * 100 / phaseTime;
        psc_dbgmsg( 9, "Phasetime= %d, sitetime=%d, lateTime=%d, msgSize=%d, severity=%f\n", phaseTime, siteTime, lateTime, msgSize, severity );
        return severity;
    }
    else {
        return .0;
    }
}

Gather_Required_Info_Type OverloadedMasterProp::request_metrics() {
    pdb->request( context, PSC_MPI );
    pdb->request( startCtx, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string OverloadedMasterProp::name() {
    return "Overloaded Master";
}

void OverloadedMasterProp::evaluate() {
    INT64 temp;

    totalCount = pdb->get( context, PSC_MPI_CALL_COUNT );

    siteTime = pdb->get( context, PSC_MPI_TIME_SPENT ); //Measured in microseconds
    lateTime = pdb->get( context, PSC_MPI_LATE_SEND );
    if( lateTime <= 0 ) {
        lateTime == 0;
    }
    phaseTime = pdb->get( startCtx, PSC_EXECUTION_TIME );
    msgSize   = pdb->get( context, PSC_MPI_AGGREGATE_MESSAGE_SIZE );
    if( msgSize <= 0 ) {
        msgSize = 0;
    }
    //std::cout << "Property: " << m << " (" << name() << ")  Process " << context->getRank()
    //          << " lateTime " << lateTime << " phaseTime " << phaseTime << " totalCount " << totalCount << std::endl;
}

std::string OverloadedMasterProp::info() {
    std::stringstream stream;

    stream << '\t' << ">>Exec Time this call: " << ( double )siteTime / NANOSEC_PER_SEC_DOUBLE << '\t' << " Phase Time: "
           << ( double )phaseTime / NANOSEC_PER_SEC_DOUBLE << '\t' << " msgSize: " << msgSize << '\t' << " lateTime: " << ( double )lateTime / NANOSEC_PER_SEC_DOUBLE << '\t' << " #Inst: " << totalCount;

    return stream.str();
}

Property* OverloadedMasterProp::clone() {
    OverloadedMasterProp* prop = new OverloadedMasterProp( context, startCtx );
    return prop;
}

std::string OverloadedMasterProp::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<CallTime>" << ( double )siteTime / NANOSEC_PER_SEC_DOUBLE << "</CallTime>" << std::endl;
    stream << "\t\t<PhaseTime>" << ( double )phaseTime / NANOSEC_PER_SEC_DOUBLE << "</PhaseTime>" << std::endl;
    stream << "\t\t<LateTime>" << ( double )lateTime / NANOSEC_PER_SEC_DOUBLE << "</lateTime>" << std::endl;
    if( totalCount > 0 ) {
        stream << "\t\t<MeanMsgSize>" << msgSize / totalCount << "</MeanMsgSize>" << std::endl;
    }
    else {
        stream << "\t\t<MeanMsgSize>" << 0 << "</MeanMsgSize>" << std::endl;
    }
    stream << "\t\t<TotalCount>" << totalCount << "</TotalCount>" << std::endl;
    return stream.str();
}
