/**
   @file    StartupShutdownOverhead.cc
   @ingroup OpenMPProperties
   @brief   OpenMP Parallel Region Start up and Shutdown overhead property
   @author  Shajulin Benedict
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

#include "StartupShutdownOverhead.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"
#include <sstream>

PropertyID StartupShutdownOverheadProp::id() {
    return STARTUPSHUTDOWNOVERHEAD;
}

std::string StartupShutdownOverheadProp::info() {
    std::string parallel_time;

    parallel_time = masterWaitCycles;

    return parallel_time;
}

void StartupShutdownOverheadProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl;
}

bool StartupShutdownOverheadProp::condition() const {
    /*
       std::cout << "\nmasterWaitCycles: " << masterWaitCycles ;
       std::cout << " slaveWaitCycles: "  << slaveWaitCycles << "\n";

       for(int i = 0; i < appl->getOmpThreads(); i++)
       {
       std::cout << waitCycles2[i]  << "\t";
       }
     */
    return ( ( ( double )masterWaitCycles - ( double )slaveWaitCycles ) / ( double )masterWaitCycles ) * 100 > threshold;
}

double StartupShutdownOverheadProp::confidence() const {
    return 1.0;
}

double StartupShutdownOverheadProp::severity() const {
    if( ( double )( masterWaitCycles > slaveWaitCycles ) ) {
        return ( ( double )( masterWaitCycles - slaveWaitCycles ) / ( double )phaseCycles ) * 100;
    }
    else {
        return 0.0;
    }
}

Context* StartupShutdownOverheadProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type StartupShutdownOverheadProp::request_metrics() {
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        pdb->request( ct, PSC_PARALLEL_REGION_BODY_CYCLE );
        if( i == 0 ) {
            pdb->request( ct, PSC_PARALLEL_REGION_CYCLE );
        }
    }
    pdb->request( phaseContext, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string StartupShutdownOverheadProp::name() {
    return "Parallel Region Startup and Shutdown overhead";
}

void StartupShutdownOverheadProp::evaluate() {
    waitCycles2.resize( appl->getOmpThreads(), 0 );
    masterWaitCycles = 0.0;
    slaveWaitCycles  = 0.0;

    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        waitCycles2[ i ] = ( INT64 )( double )( pdb->get( ct, PSC_PARALLEL_REGION_BODY_CYCLE ) );
        if( i == 0 ) {
            masterWaitCycles = ( INT64 )( double )( pdb->get( ct, PSC_PARALLEL_REGION_CYCLE ) );
        }
        delete ct;
    }
    slaveWaitCycles = waitCycles2[ 0 ];
    phaseCycles     = ( INT64 )( double )( pdb->get( phaseContext, PSC_EXECUTION_TIME ) );
}

Property* StartupShutdownOverheadProp::clone() {
    StartupShutdownOverheadProp* prop = new StartupShutdownOverheadProp( context, phaseContext );
    return prop;
}

std::string StartupShutdownOverheadProp::toXMLExtra() {
    std::stringstream stream;
    stream << "\t\t<ExecTime>" << masterWaitCycles << "</ExecTime>" << std::endl;
    return stream.str();
}
