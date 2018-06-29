/**
   @file    L2MissRate.cc
   @ingroup GenericProperties
   @brief   Generic L2 cache miss rate property
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

#include "L2MissRate.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"

L2MissRate::~L2MissRate() {
    free( context );
    free( phaseContext );
}

PropertyID L2MissRate::id() {
    return CPPL2MISS;
}

void L2MissRate::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool L2MissRate::condition() const {
    return availableProp && severity() > threshold;
}

double L2MissRate::confidence() const {
    return 1.0;
}

double L2MissRate::severity() const {
    if( !availableProp ) {
        return 0.0;
    }
    else {
        return importance * 100.0;
    }
}

Context* L2MissRate::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type L2MissRate::request_metrics() {
    availableProp = checkMetricAvail( PSC_PAPI_L2_DCM ) & checkMetricAvail( PSC_PAPI_L2_DCA );

    if( !availableProp ) {
        return NO_INFO_GATHERED;
    }
    else {
        pdb->request( context, PSC_PAPI_L2_DCM );
        pdb->request( context, PSC_PAPI_L2_DCA );
        return ALL_INFO_GATHERED;
    }
}

std::string L2MissRate::name() {
    if( !availableProp ) {
        return "L2 cache misses rate property not supported";
    }
    ;
    return "L2 cache misses rate";
}

void L2MissRate::evaluate() {
    if( !availableProp ) {
        return;
    }

    L2Access = pdb->get( context, PSC_PAPI_L2_DCA );
    L2Misses = pdb->get( context, PSC_PAPI_L2_DCM );

    if( L2Access > 0.0 ) {
        importance = ( double )L2Misses / ( double )L2Access;
    }
    else {
        importance = 0.0;
    }
}

Property* L2MissRate::clone() {
    L2MissRate* prop = new L2MissRate( context, phaseContext );
    return prop;
}

std::string L2MissRate::info() {
    std::stringstream stream;

    stream << '\t' << " L2Misses: " << L2Misses << '\t' << " L2Access: " << L2Access;

    return stream.str();
}

std::string L2MissRate::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<L2Misses>" << L2Misses << "</L2Misses>" << std::endl;
    stream << "\t\t<L2Access>" << L2Access << "</L2Access>" << std::endl;

    return stream.str();
}
