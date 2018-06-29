/**
   @file    ia64/LC2DMissRate.cc
   @ingroup Itanium2Properites
   @brief   Itanium2 specific property
   @author  Edmond Kereku
   @verbatim
        Revision:       $Revision$
        Revision date:  $Date$
        Committed by:   $Author$

        This file is part of the Periscope performance measurement tool.
        See http://www.lrr.in.tum.de/periscope for details.

        Copyright (c) 2005-2011, Technische Universitaet Muenchen, Germany
        See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include <iostream>

#include "global.h"
#include "Metric.h"
#include "LC2DMissRate.h"
#include "Context.h"
#include "PropertyID.h"


//
//Implementation for class LC2DMissRate
//
PropertyID LC2DMissRateProp::id() {
    return LC2DMISSRATE;
}

bool LC2DMissRateProp::condition() const {
    if( TEST ) {
        return true;                                                           //Not selective at all, for testing only
    }
    return ( ( double )nr_lc2_cache_miss / ( double )nr_mem_ref ) > threshold; //greater than 5%
}

double LC2DMissRateProp::confidence() const {
    return 1.0;
}

double LC2DMissRateProp::severity() const {
//return (nr_mem_ref != 0)?((double)nr_lc2_cache_miss/(double)nr_mem_ref ):0.0;
    return ( nr_mem_ref != 0 ) ? ( ( double )nr_lc2_cache_miss / ( double )nr_mem_ref * ( double )execution_time ) : 0.0; //twice as severe as lc1
}

Gather_Required_Info_Type LC2DMissRateProp::request_metrics() {
    pdb->request( context, PSC_PAPI_LD_INS );
    pdb->request( context, PSC_PAPI_SR_INS );
    pdb->request( context, PSC_PAPI_L2_DCM );
    pdb->request( context, PSC_PAPI_TOT_CYC );
    return ALL_INFO_GATHERED;
}

std::string LC2DMissRateProp::name() {
    return "LC2DMissRate";
}

void LC2DMissRateProp::evaluate() {
    nr_mem_ref        = pdb->get( context, PSC_PAPI_LD_INS ) + pdb->get( context, PSC_PAPI_SR_INS );
    nr_lc2_cache_miss = pdb->get( context, PSC_PAPI_L2_DCM );
    execution_time    = pdb->get( context, PSC_PAPI_TOT_CYC );
    double rate = ( nr_mem_ref != 0 ) ? ( ( double )nr_lc2_cache_miss / ( double )nr_mem_ref ) : 0.0;
//cerr <<"Rate: =  "<<rate <<" ExecutionTime:  "<<execution_time ;
}

Property* LC2DMissRateProp::clone() {
    Property* prop = new LC2DMissRateProp();
    return prop;
}
