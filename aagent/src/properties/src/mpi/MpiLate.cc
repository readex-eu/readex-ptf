/**
   @file    MpiLate.cc
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

#include "MpiLate.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <sstream>
#include "psc_errmsg.h"

using namespace std;

PropertyID MpiLateProp::id() {
    return MPIPROP;
}

std::string MpiLateProp::subId() {
    std::stringstream sstr;
    sstr << get_metric();
    return sstr.str();
}

void MpiLateProp::print() {
    stringstream str;

    str << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
        << context->getRegion()->str_print();
    string help = str.str();
    psc_dbgmsg( 4, "%s\n", help.c_str() );
}

bool MpiLateProp::condition() const {
    //  if ( context->getRfl() == 528 && m == PSC_MPI_LATE_ALLREDUCE )
    //  {
    //    std::cout << "Property " << name() << " " << context->getRegion()->str_print( appl->get_file_name_maping() );
    //    printf( " process=%d lateTime=%d totalTime=%d totalCount=%d threshold=%f severity=%f\n", context->getRank(), lateTime, totalTime, totalCount, threshold, severity() );
    //  }
    return severity() > threshold;
}

double MpiLateProp::confidence() const {
    return 1.0;
}

Metric MpiLateProp::get_metric() {
    return m;
}

double MpiLateProp::severity() const {
    //printf("Severity %f\n",((double) lateTime*100) / totalTime);
    if( totalTime ) {
        double severity = lateTime;
        if( siteTime < severity ) {
            severity = siteTime;
        }
        severity = severity * 100 / totalTime;
        psc_dbgmsg( 6, "SEVERITY: totalcount=%" PRId64 ", lateTime=%" PRId64 ", siteTime=%" PRId64 ", phaseTime=%" PRId64 ", severity=%f\n", totalCount, lateTime, siteTime, totalTime, severity );
        return severity;
    }
    else {
        return .0;
    }
}

Gather_Required_Info_Type MpiLateProp::request_metrics() {
    pdb->request( context, PSC_MPI );
    pdb->request( startCtx, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string MpiLateProp::name() {
    switch( m ) {
//case MPI_LATE_SEND:
//  return ("MPI Late Sender");
    case PSC_MPI_EARLY_RECV:
        return "Excessive MPI time in receive due to late sender";
    case PSC_MPI_LATE_RECV:
        return "Excessive MPI time in send due to late receive";
    //same as for MPI_LATE_REDUCE. You cannot be sure that the delay results from that. Depends on implementation.

    case PSC_MPI_EARLY_BCAST:
        return "Excessive MPI time due to late root in broadcast";
//	case PSC_MPI_LATE_BCAST:
//		return ("MPI process was late in Broadcast");
    case PSC_MPI_EARLY_SCATTER:
        return "Excessive MPI time due to late root in scatter";
//case PSC_MPI_LATE_SCATTER:
//  return ("MPI process was late in Scatter");

    case PSC_MPI_LATE_GATHER:
        return "Excessive MPI time in root due to late process in gather";
    case PSC_MPI_LATE_REDUCE:
        return "Excessive MPI time in root due to late process in reduce";
    //excessive in other process might happen for tree implementation but
    //should only be reported if end time was really larger than start time of last..

    case PSC_MPI_LATE_ALLREDUCE:
        return "Excessive MPI time due to late process in allreduce";
    case PSC_MPI_LATE_ALLGATHER:
        return "Excessive MPI time due to late process in allgather";
    case PSC_MPI_LATE_ALLTOALL:
        return "Excessive MPI time due to late process in alltoall";

    case PSC_MPI_LATE_BARRIER:
        return "Excessive MPI time due to late process in barrier";
    default:
        return "MPI Unknown Metric";
    }
}

void MpiLateProp::evaluate() {
    INT64 temp;

    if( m == PSC_MPI_LATE_SEND ) {
        temp = pdb->get( context, PSC_MPI_REMOTE_SITE_COUNT );
        if( temp ) {
            lateTime = pdb->get( context, m ) / temp;
        }
        else {
            lateTime = 0;
        }
        totalCount = pdb->get( context, PSC_MPI_CALL_COUNT_REMOTE );
    }
    else {
        lateTime   = pdb->get( context, m );
        totalCount = pdb->get( context, PSC_MPI_CALL_COUNT );
    }
    siteTime  = pdb->get( context, PSC_MPI_TIME_SPENT ); //Measured in microseconds
    totalTime = pdb->get( startCtx, PSC_EXECUTION_TIME );

    //std::cout << "Property: " << m << " (" << name() << ")  Process " << context->getRank()
    //          << " lateTime " << lateTime << " totalTime " << totalTime << " totalCount " << totalCount << std::endl;
}

std::string MpiLateProp::info() {
    std::stringstream stream;

    stream << '\t' << ">>Exec Time this call: " << ( double )siteTime / NANOSEC_PER_SEC_DOUBLE << '\t' << " Late Time: "
           << ( double )lateTime / NANOSEC_PER_SEC_DOUBLE << '\t' << " Phase Time: " << ( double )totalTime / NANOSEC_PER_SEC_DOUBLE;

    return stream.str();
}

Property* MpiLateProp::clone() {
    MpiLateProp* prop = new MpiLateProp( context, startCtx, m );
    return prop;
}

void MpiLateProp::set_metric( Metric m_p ) {
    m = m_p;
}

std::string MpiLateProp::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<LateTime>" << ( double )lateTime / NANOSEC_PER_SEC_DOUBLE << "</LateTime>" << std::endl;
    stream << "\t\t<CallTime>" << ( double )siteTime / NANOSEC_PER_SEC_DOUBLE << "</CallTime>" << std::endl;
    stream << "\t\t<PhaseTime>" << ( double )totalTime / NANOSEC_PER_SEC_DOUBLE << "</PhaseTime>" << std::endl;

    return stream.str();
}
