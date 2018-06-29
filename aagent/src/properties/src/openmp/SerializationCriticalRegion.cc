/**
   @file    SerializationCriticalRegion.cc
   @ingroup OpenMPProperties
   @brief   OpenMP Serialization in Critical region property
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

#include "SerializationCriticalRegion.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"
#include <sstream>

PropertyID SerializationCriticalRegionProp::id() {
    return SERIALIZATIONCRITICALREGIONOVERHEAD_OMPREGION;
}

void SerializationCriticalRegionProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl;
}

bool SerializationCriticalRegionProp::condition() const {
    return ( ( double )maxWaitTime / ( double )phaseCycles * 100.0 ) > threshold;
}

double SerializationCriticalRegionProp::confidence() const {
    return 1.0;
}

double SerializationCriticalRegionProp::severity() const {
    if( ( ( double )maxWaitTime / ( double )phaseCycles * 100 ) > 100.0 ) {
        return 100.0;
    }
    else {
        return ( double )maxWaitTime / ( double )phaseCycles * 100;
    }
}

Context* SerializationCriticalRegionProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type SerializationCriticalRegionProp::request_metrics() {
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );

        pdb->request( ct, PSC_CRITICAL_REGION_CYCLE );
    }

    pdb->request( phaseContext, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string SerializationCriticalRegionProp::name() {
    return "Serialization Critical Region Overhead";
}

void SerializationCriticalRegionProp::evaluate() {
    waitCycles.resize( appl->getOmpThreads(), 0 );

    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );

        waitCycles[ i ] = pdb->get( ct, PSC_CRITICAL_REGION_CYCLE );

        delete ct;
    }

    phaseCycles = pdb->get( phaseContext, PSC_EXECUTION_TIME );
    maxWaitTime = waitCycles[ 0 ];
    for( int i = 1; i < appl->getOmpThreads(); i++ ) {
        if( waitCycles[ i ] > maxWaitTime ) {
            maxWaitTime = waitCycles[ i ];
        }
    }
}

std::string SerializationCriticalRegionProp::toXMLExtra() {
    std::stringstream stream;
    stream << "\t\t<ExecTime>" << maxWaitTime << "</ExecTime>" << std::endl;
    return stream.str();
}

Property* SerializationCriticalRegionProp::clone() {
    SerializationCriticalRegionProp* prop = new SerializationCriticalRegionProp( context, phaseContext );
    return prop;
}
