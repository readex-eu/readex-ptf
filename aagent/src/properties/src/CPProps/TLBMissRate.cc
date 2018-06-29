/**
   @file    TLBMissRate.cc
   @ingroup GenericProperties
   @brief   Generic TLB miss rate property
   @author  Yury Oleynik
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

#include "TLBMissRate.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"

TLBMissRate::~TLBMissRate() {
    free( context );
    free( phaseContext );
}

PropertyID TLBMissRate::id() {
    return CPPTLBMISS;
}

void TLBMissRate::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool TLBMissRate::condition() const {
    return availableProp && severity() > threshold;
}

double TLBMissRate::confidence() const {
    return 1.0;
}

double TLBMissRate::severity() const {
    if( !availableProp ) {
        return 0.0;
    }
    else {
        return importance * 100.0;
    }
}

Context* TLBMissRate::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type TLBMissRate::request_metrics() {
    availableProp = checkMetricAvail( PSC_PAPI_LST_INS ) & checkMetricAvail( PSC_PAPI_TLB_DM );

    if( !availableProp ) {
        abort();
    }
    else {
        pdb->request( context, PSC_PAPI_TLB_DM );
        pdb->request( context, PSC_PAPI_LST_INS );
        return ALL_INFO_GATHERED;
    }
}

std::string TLBMissRate::name() {
    if( !availableProp ) {
        return "TLB misses rate property not supported";
    }
    ;
    return "TLB misses rate";
}

void TLBMissRate::evaluate() {
    if( !availableProp ) {
        return;
    }

    TLBAccess = pdb->get( context, PSC_PAPI_LST_INS );
    TLBMisses = pdb->get( context, PSC_PAPI_TLB_DM );

    if( TLBAccess > 0.0 ) {
        importance = ( double )TLBMisses / ( double )TLBAccess;
    }
    else {
        importance = 0.0;
    }
}

Property* TLBMissRate::clone() {
    TLBMissRate* prop = new TLBMissRate( context, phaseContext );
    return prop;
}

std::string TLBMissRate::info() {
    std::stringstream stream;

    stream << '\t' << " TLBMisses: " << TLBMisses << '\t' << " TLBAccess: " << TLBAccess;

    return stream.str();
}

std::string TLBMissRate::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<TLBMisses>" << TLBMisses << "</TLBMisses>" << std::endl;
    stream << "\t\t<TLBAccess>" << TLBAccess << "</TLBAccess>" << std::endl;

    return stream.str();
}
