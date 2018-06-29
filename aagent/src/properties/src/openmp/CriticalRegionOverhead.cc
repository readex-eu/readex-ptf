/**
   @file    CriticalRegionOverhead.cc
   @ingroup OpenMPProperties
   @brief   OpenMP Critical Section Overhead Property
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

#include "CriticalRegionOverhead.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"
#include <sstream>


PropertyID CriticalRegionOverheadProp::id() {
    PropertyID result;
    switch( metric ) {
    case PSC_CRITICAL_REGION_CYCLE:
        result = CRITICALREGIONOVERHEAD_OMPREGION;
        break;
    case PSC_OMP_ATOMIC_CYCLE:
        result = ATOMICREGIONOVERHEAD_OMPREGION;
        break;
    case PSC_FLUSH_CYCLES:
        result = FLUSHREGIONOVERHEAD_OMPREGION;
        break;
    default:
        result = CRITICALREGIONOVERHEAD_OMPREGION;
        break;
    }
    return result;
}

void CriticalRegionOverheadProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl;
}

bool CriticalRegionOverheadProp::condition() const {
    /*
       for (int i=0; i<appl->getOmpThreads(); i++){
       std::cout <<"waitCycles1  " << waitCycles1[i]<< "\t" <<"waitCycles2 "<< waitCycles2[i] << "\t\t";
       std::cout <<"waitCycles " << waitCycles[i] << "\n";
       }
     */
    return ( ( double )CriticalTime / ( double )phaseCycles ) * 100 > threshold;
}

double CriticalRegionOverheadProp::confidence() const {
    return 1.0;
}

double CriticalRegionOverheadProp::severity() const {
    if( ( ( double )CriticalTime / ( double )phaseCycles ) * 100 > 100 ) {
        return 100.0;
    }
    else {
        return ( ( double )CriticalTime / ( double )phaseCycles ) * 100;
    }
}

Context* CriticalRegionOverheadProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type CriticalRegionOverheadProp::request_metrics() {
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        pdb->request( ct, PSC_CRITICAL_REGION_CYCLE );
    }

    pdb->request( phaseContext, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string CriticalRegionOverheadProp::name() {
    std::string result;
    switch( metric ) {
    case PSC_CRITICAL_REGION_CYCLE:
        result = "Critical section overhead";
        break;
    case PSC_OMP_ATOMIC_CYCLE:
        result = "Atomic directive overhead";
        break;
    case PSC_FLUSH_CYCLES:
        result = "Flush directive overhead";
        break;
    default:
        result = "Critical section overhead";
        break;
    }
    return result;
}

void CriticalRegionOverheadProp::evaluate() {
    CriticalTime_eachthread.resize( appl->getOmpThreads(), 0 );
    CriticalTime = 0.0;

    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        //CriticalTime = CriticalTime + CriticalTime_eachthread[i];
        CriticalTime_eachthread[ i ] =     pdb->get( ct, metric );
        if( CriticalTime_eachthread[ i ] > CriticalTime ) {
            CriticalTime = CriticalTime_eachthread[ i ];
        }


        delete ct;
    }
    phaseCycles = pdb->get( phaseContext, PSC_EXECUTION_TIME );
}

std::string CriticalRegionOverheadProp::toXMLExtra() {
    std::stringstream stream;
    stream << "\t\t<ExecTime>" << CriticalTime << "</ExecTime>" << std::endl;
    return stream.str();
}
Property* CriticalRegionOverheadProp::clone() {
    CriticalRegionOverheadProp* prop = new CriticalRegionOverheadProp( context, phaseContext, metric );
    return prop;
}
