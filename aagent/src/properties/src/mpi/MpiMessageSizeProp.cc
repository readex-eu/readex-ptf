/**
   @file    MpiMessageSizeProp.cc
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

#include "MpiMessageSizeProp.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <sstream>
#include "psc_errmsg.h"

#define MIN_SIZE_THRESHOLD 1000
#define MAX_SIZE_THRESHOLD 1000000


using namespace std;

PropertyID MpiMessageSizeProp::id() {
    return MPIPROP;
}

/// @todo SubId type might not be unique some day so it is not defined explicitly
std::string MpiMessageSizeProp::subId() {
    std::stringstream sstr;
    sstr << PSC_MPI_AGGREGATE_MESSAGE_SIZE;
    if( messageSize > MAX_SIZE_THRESHOLD ) {
        sstr << "-L";
    }
    else {
        sstr << "-S";
    }
    return sstr.str();
}

void MpiMessageSizeProp::print() {
    stringstream str;

    str << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
        << context->getRegion()->str_print();
    string help = str.str();
    psc_dbgmsg( 4, "%s\n", help.c_str() );
}

bool MpiMessageSizeProp::condition() const {
    return false; ///<message size property is disabled for further testing
    //return ( ( (severity() > threshold) && (messageSize > MAX_SIZE_THRESHOLD) ) ||
    // ( (severity() > threshold) && (messageSize > 0) && (messageSize < MIN_SIZE_THRESHOLD) ) );
}

double MpiMessageSizeProp::confidence() const {
    return 1.0;
}

double MpiMessageSizeProp::severity() const {
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

Gather_Required_Info_Type MpiMessageSizeProp::request_metrics() {
    pdb->request( context, PSC_MPI );
    pdb->request( startCtx, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string MpiMessageSizeProp::name() {
    if( messageSize > MAX_SIZE_THRESHOLD ) {
        return "Excessive MPI time due to large messages";
    }
    else if( messageSize < MIN_SIZE_THRESHOLD ) {
        return "Excessive MPI time due to many small messages";
    }
    else {
        return "Undef";
    }
}

void MpiMessageSizeProp::evaluate() {
    callCount = pdb->get( context, PSC_MPI_CALL_COUNT );
    siteTime  = pdb->get( context, PSC_MPI_TIME_SPENT );                                  //Measured in microseconds
    phaseTime = pdb->get( startCtx, PSC_EXECUTION_TIME ) / ( NANOSEC_PER_SEC / 1000000 ); //Cycles to microseconds

    messageSize = pdb->get( context, PSC_MPI_AGGREGATE_MESSAGE_SIZE ) / callCount;
}

std::string MpiMessageSizeProp::info() {
    std::stringstream stream;

    stream << '\t' << ">>Message Size: " << messageSize;

    return stream.str();
}

Property* MpiMessageSizeProp::clone() {
    MpiMessageSizeProp* prop = new MpiMessageSizeProp( context, startCtx );
    return prop;
}

std::string MpiMessageSizeProp::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<Size>" << messageSize << "</Size>" << std::endl;

    return stream.str();
}
