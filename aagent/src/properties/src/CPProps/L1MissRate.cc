/**
   @file    L1MissRate.cc
   @ingroup GenericProperties
   @brief   Generic L1 cache miss rate property
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

#include "L1MissRate.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"

L1MissRate::~L1MissRate() {
    free( context );
    free( phaseContext );
}

PropertyID L1MissRate::id() {
    return CPPL1MISS;
}

void L1MissRate::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool L1MissRate::condition() const {
    return availableProp && severity() > threshold;
}

double L1MissRate::confidence() const {
    return 1.0;
}

double L1MissRate::severity() const {
    if( !availableProp ) {
        return 0.0;
    }
    else {
        return importance * 100.0;
    }
}

Context* L1MissRate::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type L1MissRate::request_metrics() {
    availableProp = checkMetricAvail( PSC_PAPI_L1_DCM ) & checkMetricAvail( PSC_PAPI_L1_DCA );

    if( !availableProp ) {
        abort();
    }
    else {
        pdb->request( context, PSC_PAPI_L1_DCM );
        pdb->request( context, PSC_PAPI_L1_DCA );
        return ALL_INFO_GATHERED;
    }
}

std::string L1MissRate::name() {
    if( !availableProp ) {
        return "L1 cache misses rate property not supported";
    }
    ;
    return "L1 cache misses rate";
}

void L1MissRate::evaluate() {
    if( !availableProp ) {
        return;
    }

    L1Access = pdb->get( context, PSC_PAPI_L1_DCA );
    L1Misses = pdb->get( context, PSC_PAPI_L1_DCM );

    if( L1Access > 0.0 ) {
        importance = ( double )L1Misses / ( double )L1Access;
    }
    else {
        importance = 0.0;
    }
}

Property* L1MissRate::clone() {
    L1MissRate* prop = new L1MissRate( context, phaseContext );
    return prop;
}

std::string L1MissRate::info() {
    std::stringstream stream;

    stream << '\t' << " L1Misses: " << L1Misses << '\t' << " L1Access: " << L1Access;

    return stream.str();
}

std::string L1MissRate::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<L1Misses>" << L1Misses << "</L1Misses>" << std::endl;
    stream << "\t\t<L1Access>" << L1Access << "</L1Access>" << std::endl;

    return stream.str();
}
