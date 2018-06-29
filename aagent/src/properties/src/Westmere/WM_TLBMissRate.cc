/**
   @file    WM_TLBMissRate.cc
   @ingroup WestmereProperties
   @brief   Westmere TLB miss rate property
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

#include "WM_TLBMissRate.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"

WM_TLBMissRate::WM_TLBMissRate( Context* ct, Context* phaseCt, double threshold ) : Property( ct ), threshold( threshold ) {
    TLBAccess   = 0;
    TLBMisses   = 0;
    phaseCycles = 0;
    cycles      = 0;

    importance    = 0.0;
    phaseContext  = phaseCt;
    availableProp = true;
}

WM_TLBMissRate::~WM_TLBMissRate() {
    free( context );
    free( phaseContext );
}

PropertyID WM_TLBMissRate::id() {
    return WM_TLBMISSRATE;
}

void WM_TLBMissRate::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool WM_TLBMissRate::condition() const {
    return availableProp && severity() > threshold;
}

double WM_TLBMissRate::confidence() const {
    return 1.0;
}

double WM_TLBMissRate::severity() const {
    if( !availableProp ) {
        return 0.0;
    }
    else {
        return importance * ( cycles / phaseCycles ) * 100;
    }
}

Context* WM_TLBMissRate::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type WM_TLBMissRate::request_metrics() {
    availableProp = checkMetricAvail( PSC_PAPI_LST_INS ) & checkMetricAvail( PSC_PAPI_TLB_DM );

    if( !availableProp ) {
        abort();
    }
    else {
        pdb->request( context, PSC_PAPI_TLB_DM );
        pdb->request( context, PSC_PAPI_LST_INS );
        pdb->request( context, PSC_PAPI_TOT_CYC );
        pdb->request( phaseContext, PSC_PAPI_TOT_CYC );
        return ALL_INFO_GATHERED;
    }
}

std::string WM_TLBMissRate::name() {
    if( !availableProp ) {
        return "TLB misses rate property not supported";
    }
    ;
    return "TLB misses rate";
}

void WM_TLBMissRate::evaluate() {
    if( !availableProp ) {
        return;
    }

    TLBAccess   = pdb->get( context, PSC_PAPI_LST_INS );
    TLBMisses   = pdb->get( context, PSC_PAPI_TLB_DM );
    phaseCycles = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );
    cycles      = pdb->get( context, PSC_PAPI_TOT_CYC );

    if( TLBAccess > 0.0 ) {
        importance = ( double )TLBMisses / ( double )TLBAccess;
    }
    else {
        importance = 0.0;
    }
}

Property* WM_TLBMissRate::clone() {
    WM_TLBMissRate* prop = new WM_TLBMissRate( context, phaseContext );
    return prop;
}

std::string WM_TLBMissRate::info() {
    std::stringstream stream;

    stream << '\t' << " TLBMisses: " << TLBMisses << '\t' << " TLBAccess: " << TLBAccess;

    return stream.str();
}

std::string WM_TLBMissRate::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<TLBMissRate>" << TLBMisses / TLBAccess << "</TLBMissRate>" << std::endl;
    stream << "\t\t<TLBMisses>" << TLBMisses << "</TLBMisses>" << std::endl;
    stream << "\t\t<TLBAccesses>" << TLBAccess << "</TLBAccesses>" << std::endl;

    return stream.str();
}
