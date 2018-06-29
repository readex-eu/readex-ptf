/**
   @file    WM_L2MissRate.cc
   @ingroup WestmereProperties
   @brief   Westmere Generic L2 cache miss rate property header
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

#include "WM_L2MissRate.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"

WM_L2MissRate::WM_L2MissRate( Context* ct, Context* phaseCt, double threshold ) :
    Property( ct ), threshold( threshold ) {
    L2Access    = 0;
    L2Misses    = 0;
    phaseCycles = 0;
    cycles      = 0;

    importance    = 0.0;
    phaseContext  = phaseCt;
    availableProp = true;
}

WM_L2MissRate::~WM_L2MissRate() {
    free( context );
    free( phaseContext );
}

PropertyID WM_L2MissRate::id() {
    return WM_L2MISSRATE;
}

void WM_L2MissRate::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool WM_L2MissRate::condition() const {
    return availableProp && severity() > threshold;
}

double WM_L2MissRate::confidence() const {
    return 1.0;
}

double WM_L2MissRate::severity() const {
    if( !availableProp ) {
        return 0.0;
    }
    else {
        return importance * ( cycles / phaseCycles ) * 100;
    }
}

Context* WM_L2MissRate::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type WM_L2MissRate::request_metrics() {
    availableProp = checkMetricAvail( PSC_PAPI_L2_DCM ) & checkMetricAvail( PSC_PAPI_L2_DCA );

    if( !availableProp ) {
        abort();
    }
    else {
        pdb->request( context, PSC_PAPI_L2_DCM );
        pdb->request( context, PSC_PAPI_L2_DCA );
        pdb->request( context, PSC_PAPI_TOT_CYC );
        pdb->request( phaseContext, PSC_PAPI_TOT_CYC );

        return ALL_INFO_GATHERED;
    }
}

std::string WM_L2MissRate::name() {
    if( !availableProp ) {
        return "L2 cache misses rate property not supported";
    }
    ;
    return "L2 cache misses rate";
}

void WM_L2MissRate::evaluate() {
    if( !availableProp ) {
        return;
    }

    L2Access    = pdb->get( context, PSC_PAPI_L2_DCA );
    L2Misses    = pdb->get( context, PSC_PAPI_L2_DCM );
    phaseCycles = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );
    cycles      = pdb->get( context, PSC_PAPI_TOT_CYC );

    if( L2Access > 0.0 ) {
        importance = ( double )L2Misses / ( double )L2Access;
    }
    else {
        importance = 0.0;
    }
}

Property* WM_L2MissRate::clone() {
    WM_L2MissRate* prop = new WM_L2MissRate( context, phaseContext );
    return prop;
}

std::string WM_L2MissRate::info() {
    std::stringstream stream;

    stream << '\t' << " L2Misses: " << L2Misses << '\t' << " L2Access: " << L2Access;

    return stream.str();
}

std::string WM_L2MissRate::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<L2MissRate>" << L2Misses / L2Access << "</L2MissRate>" << std::endl;
    stream << "\t\t<L2Misses>" << L2Misses << "</L2Misses>" << std::endl;
    stream << "\t\t<L2Accesses>" << L2Access << "</L2Accesses>" << std::endl;

    return stream.str();
}
