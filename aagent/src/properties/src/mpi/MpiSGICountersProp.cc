/**
   @file    MpiSGICountersProp.cc
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

#include "MpiSGICountersProp.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <string.h>
#include <sstream>
#include "psc_errmsg.h"

using namespace std;

PropertyID MpiSGICountersProp::id() {
    return MPIPROP;
}

std::string MpiSGICountersProp::subId() {
    std::stringstream sstr;
    sstr << PSC_MPI_SGI_COUNTERS;
    return sstr.str();
}

void MpiSGICountersProp::print() {
    stringstream str;

    str << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
        << context->getRegion()->str_print();
    string help = str.str();
    psc_dbgmsg( 4, "%s\n", help.c_str() );
}

bool MpiSGICountersProp::condition() const {
    return true;
}

double MpiSGICountersProp::confidence() const {
    return 1.0;
}

double MpiSGICountersProp::severity() const {
    if( phaseTime ) {
        double severity = siteTime;
        severity = severity * 100 / phaseTime;
        return severity;
    }
    else {
        return .0;
    }
}

Gather_Required_Info_Type MpiSGICountersProp::request_metrics() {
    pdb->request( context, PSC_MPI );
    pdb->request( startCtx, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string MpiSGICountersProp::name() {
    char name[] = "SGI retries found at";
    if( counters[ 0 ] > 0 ) {
        strcat( name, " 1" );
    }
    if( counters[ 1 ] > 0 ) {
        strcat( name, " 2" );
    }
    if( counters[ 2 ] > 0 ) {
        strcat( name, " 3" );
    }
    if( counters[ 3 ] > 0 ) {
        strcat( name, " 4" );
    }
    if( counters[ 4 ] > 0 ) {
        strcat( name, " 5" );
    }
    if( counters[ 5 ] > 0 ) {
        strcat( name, " 6" );
    }
    if( counters[ 6 ] > 0 ) {
        strcat( name, " 7" );
    }
    if( counters[ 7 ] > 0 ) {
        strcat( name, " 8" );
    }
    strcat( name, " counters." );
    return name;
}

void MpiSGICountersProp::evaluate() {
    callCount = pdb->get( context, PSC_MPI_CALL_COUNT );

    siteTime   = pdb->get( context, PSC_MPI_TIME_SPENT );                                  //Measured in microseconds
    phaseTime  = pdb->get( startCtx, PSC_EXECUTION_TIME ) / ( NANOSEC_PER_SEC / 1000000 ); //Cycles to microseconds
    compressed = pdb->get( context, PSC_MPI_SGI_COUNTERS );
    printf( "\ncounters %lu ***********8888", compressed );
    unsigned int tmp;

    for( int i = 0; i < 8; i++ ) {
        tmp           = compressed >> i * 4;
        counters[ i ] = tmp & 15;
    }
    //std::cout << "Property: " << m << " (" << name() << ")  Process " << context->getRank()
    //          << " lateTime " << lateTime << " phaseTime " << phaseTime << " totalCount " << totalCount << std::endl;
}

std::string MpiSGICountersProp::info() {
    std::stringstream stream;

    stream << '\t' << ">>Exec Time this call: " << siteTime << '\t' << " Phase Time: " << phaseTime;

    return stream.str();
}

Property* MpiSGICountersProp::clone() {
    MpiSGICountersProp* prop = new MpiSGICountersProp( context, startCtx );
    return prop;
}

std::string MpiSGICountersProp::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<CallTime>" << siteTime << "</CallTime>" << std::endl;
    stream << "\t\t<PhaseTime>" << phaseTime << "</PhaseTime>" << std::endl;

    return stream.str();
}
