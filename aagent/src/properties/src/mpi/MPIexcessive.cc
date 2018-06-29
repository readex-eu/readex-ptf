/**
   @file    MPIexcessive.cc
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

#include "MPIexcessive.h"
#include "global.h"
#include <iostream>
#include <sstream>
#include "psc_errmsg.h"

using namespace std;

PropertyID MPIexcessiveProp::id() {
    return MPITIME;
}

std::string MPIexcessiveProp::subId() {
    std::stringstream sstr;
    sstr << PSC_MPI_TIME_SPENT;
    return sstr.str();
}

void MPIexcessiveProp::print() {
    stringstream str;

    str << "Property:" << name() << "  Process " << context->getRank()
        << "  Thread " << context->getThread() << context->getRegion()->str_print();
    string help = str.str();
    psc_dbgmsg( 4, "%s\n", help.c_str() );
}

bool MPIexcessiveProp::condition() const {
    //  if (context->getRfl() == 528 && m == PSC_MPI_LATE_ALLREDUCE) {
    //    std::cout << "Property " << name() << " " << context->getRegion()->str_print(appl->get_file_name_maping());
    //    printf(" process=%d lateTime=%d phaseTime=%d totalCount=%d threshold=%f severity=%f\n", context->getRank(), lateTime, phaseTime, totalCount, threshold, severity());
    //  }
    return severity() > threshold;
}

double MPIexcessiveProp::confidence() const {
    return 1.0;
}

double MPIexcessiveProp::severity() const {
    //printf("Severity %f\n",((double) lateTime*100) / phaseTime);
    if( phaseTime ) {
        double severity = siteTime;
        severity = severity * 100 / phaseTime;
        psc_dbgmsg( 6, "SEVERITY: totalcount=%" PRId64 ", siteTime=%" PRId64 ", phaseTime=%" PRId64 ", severity=%f\n", totalCount, siteTime, phaseTime, severity );
        return severity;
    }
    else {
        return .0;
    }
}

Gather_Required_Info_Type MPIexcessiveProp::request_metrics() {
    pdb->request( context, PSC_MPI );
    pdb->request( startCtx, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string MPIexcessiveProp::name() {
    switch( late_metric ) {
    case PSC_MPI_EARLY_RECV:
        return "Excessive MPI communication time in MPI_RECV";
    case PSC_MPI_EARLY_BCAST:
        return "Excessive MPI communication time in MPI_BCAST";
    case PSC_MPI_EARLY_SCATTER:
        return "Excessive MPI communication time in MPI_SCATTER";
    case PSC_MPI_LATE_GATHER:
        return "Excessive MPI communication time in MPI_GATHER";
    case PSC_MPI_LATE_REDUCE:
        return "Excessive MPI communication time in MPI_REDUCE";

    case PSC_MPI_LATE_ALLREDUCE:
        return "Excessive MPI communication time in MPI_ALLREDUCE";
    case PSC_MPI_LATE_ALLGATHER:
        return "Excessive MPI communication time in MPI_ALLGATHER";
    case PSC_MPI_LATE_ALLTOALL:
        return "Excessive MPI communication time in MPI_ALLTOALL";

    case PSC_MPI_LATE_BARRIER:
        return "Excessive MPI communication time in MPI_BARRIER";
    default:
        std::stringstream name;
        name << "Excessive MPI communication time in " << context->getRegion()->get_name();
        return name.str();
    }
}

void MPIexcessiveProp::evaluate() {
    INT64 temp;

    totalCount = pdb->get( context, PSC_MPI_CALL_COUNT );

    siteTime  = pdb->get( context, PSC_MPI_TIME_SPENT );  //Measured in microseconds
    phaseTime = pdb->get( startCtx, PSC_EXECUTION_TIME );
    //std::cout << "Property: " << m << " (" << name() << ")  Process " << context->getRank()
    //          << " lateTime " << lateTime << " phaseTime " << phaseTime << " totalCount " << totalCount << std::endl;
}


std::string MPIexcessiveProp::info() {
    std::stringstream stream;

    stream << '\t' << ">>Exec Time this call: " << siteTime << '\t'  << " Phase Time: " << phaseTime;

    return stream.str();
}

Property* MPIexcessiveProp::clone() {
    MPIexcessiveProp* prop = new MPIexcessiveProp( context, startCtx );
    return prop;
}

std::string MPIexcessiveProp::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<CallTime>" << siteTime << "</CallTime>" << std::endl;
    stream << "\t\t<PhaseTime>" << phaseTime << "</PhaseTime>" << std::endl;

    return stream.str();
}
