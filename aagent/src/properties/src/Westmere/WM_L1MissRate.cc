/**
   @file    WM_L1MissRate.cc
   @ingroup WestmereProperties
   @brief   Westmere L1 cache miss rate property
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

#include "WM_L1MissRate.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"

WM_L1MissRate::WM_L1MissRate( Context* ct, Context* phaseCt, double threshold ) : Property( ct ), threshold( threshold ) {
    L1Access    = 0;
    L1Misses    = 0;
    phaseCycles = 0;
    cycles      = 0;

    importance    = 0.0;
    phaseContext  = phaseCt;
    availableProp = true;
}

WM_L1MissRate::~WM_L1MissRate() {
    free( context );
    free( phaseContext );
}

PropertyID WM_L1MissRate::id() {
    return WM_L1MISSRATE;
}

void WM_L1MissRate::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool WM_L1MissRate::condition() const {
    return availableProp && severity() > threshold;
}

double WM_L1MissRate::confidence() const {
    return 1.0;
}

double WM_L1MissRate::severity() const {
    if( !availableProp ) {
        return 0.0;
    }
    else {
        return importance * ( cycles / phaseCycles ) * 100;
    }
}

Context* WM_L1MissRate::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type WM_L1MissRate::request_metrics() {
    availableProp = checkMetricAvail( PSC_PAPI_L1_DCM ) & checkMetricAvail( PSC_PAPI_L1_DCA );

    if( !availableProp ) {
        abort();
    }
    else {
        pdb->request( context, PSC_PAPI_L1_DCM );
        pdb->request( context, PSC_PAPI_L1_DCA );
        pdb->request( context, PSC_PAPI_TOT_CYC );
        pdb->request( phaseContext, PSC_PAPI_TOT_CYC );
        return ALL_INFO_GATHERED;
    }
}

std::string WM_L1MissRate::name() {
    if( !availableProp ) {
        return "L1 cache misses rate property not supported";
    }
    ;
    return "L1 cache misses rate";
}

void WM_L1MissRate::evaluate() {
    if( !availableProp ) {
        return;
    }

    L1Access    = pdb->get( context, PSC_PAPI_L1_DCA );
    L1Misses    = pdb->get( context, PSC_PAPI_L1_DCM );
    phaseCycles = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );
    cycles      = pdb->get( context, PSC_PAPI_TOT_CYC );

    if( L1Access > 0.0 ) {
        importance = ( double )L1Misses / ( double )L1Access;
    }
    else {
        importance = 0.0;
    }
}

Property* WM_L1MissRate::clone() {
    WM_L1MissRate* prop = new WM_L1MissRate( context, phaseContext );
    return prop;
}

std::string WM_L1MissRate::info() {
    std::stringstream stream;

    stream << '\t' << " L1Misses: " << L1Misses << '\t' << " L1Access: " << L1Access;

    return stream.str();
}

std::string WM_L1MissRate::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<L1MissRate>" << L1Misses / L1Access << "</L1MissRate>" << std::endl;
    stream << "\t\t<L1Misses>" << L1Misses << "</L1Misses>" << std::endl;
    stream << "\t\t<L1Accesses>" << L1Access << "</L1Accesses>" << std::endl;

    return stream.str();
}
