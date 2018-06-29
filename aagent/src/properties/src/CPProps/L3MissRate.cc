/**
   @file    L3MissRate.cc
   @ingroup GenericProperties
   @brief   Generic L3 cache miss rate property
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

#include "L3MissRate.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"

L3MissRate::~L3MissRate() {
    free( context );
    free( phaseContext );
}

PropertyID L3MissRate::id() {
    return CPPL3MISS;
}

void L3MissRate::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool L3MissRate::condition() const {
    return availableProp && severity() > threshold;
}

double L3MissRate::confidence() const {
    return 1.0;
}

double L3MissRate::severity() const {
    if( !availableProp ) {
        return 0.0;
    }
    else {
        return importance * 100.0;
    }
}

Context* L3MissRate::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type L3MissRate::request_metrics() {
    availableProp = checkMetricAvail( PSC_PAPI_L3_TCM ) & checkMetricAvail( PSC_PAPI_L3_TCA );

    if( !availableProp ) {
        abort();
    }
    else {
        pdb->request( context, PSC_PAPI_L3_TCM );
        pdb->request( context, PSC_PAPI_L3_TCA );
        return ALL_INFO_GATHERED;
    }
}

std::string L3MissRate::name() {
    if( !availableProp ) {
        return "L3 cache misses rate property not supported";
    }
    ;
    return "L3 cache misses rate";
}

void L3MissRate::evaluate() {
    if( !availableProp ) {
        return;
    }

    L3Access = pdb->get( context, PSC_PAPI_L3_TCA );
    L3Misses = pdb->get( context, PSC_PAPI_L3_TCM );

    if( L3Access > 0.0 ) {
        importance = ( double )L3Misses / ( double )L3Access;
    }
    else {
        importance = 0.0;
    }
}

Property* L3MissRate::clone() {
    L3MissRate* prop = new L3MissRate( context, phaseContext );
    return prop;
}

std::string L3MissRate::info() {
    std::stringstream stream;

    stream << '\t' << " L3Misses: " << L3Misses << '\t' << " L3Access: " << L3Access;

    return stream.str();
}

std::string L3MissRate::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<L3Misses>" << L3Misses << "</L3Misses>" << std::endl;
    stream << "\t\t<L3Access>" << L3Access << "</L3Access>" << std::endl;

    return stream.str();
}
