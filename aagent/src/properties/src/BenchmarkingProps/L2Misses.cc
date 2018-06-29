/**
   @file    L2Misses.cc
   @ingroup BenchmarkingProperties
   @brief   L2 Cache Miss Ratio Property
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

#include "L2Misses.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

PropertyID L2Misses::id( void ) {
    return L2MISSES;
}

void L2Misses::print( void ) {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool L2Misses::condition( void ) const {
    if( PAPI_L2_TM == -1 || PAPI_L2_TH == -1 ) {
        return false;
    }
    else {
        return true;
    }
}

double L2Misses::confidence( void ) const {
    return 1.0;
}

double L2Misses::severity( void ) const {
    return evaluationResult * ( cycles / phaseCycles );
}

Context* L2Misses::get_phaseContext( void ) {
    return phaseContext;
}

Gather_Required_Info_Type L2Misses::request_metrics( void ) {
    pdb->request( context, PSC_PAPI_L2_TCM );
    pdb->request( context, PSC_PAPI_L2_TCH );

    pdb->request( context, PSC_PAPI_TOT_CYC );
    pdb->request( phaseContext, PSC_PAPI_TOT_CYC );

    return ALL_INFO_GATHERED;
}

std::string L2Misses::name( void ) {
    return "L2MISSES";
}

void L2Misses::evaluate( void ) {
    phaseCycles = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );
    cycles      = pdb->get( context, PSC_PAPI_TOT_CYC );
    PAPI_L2_TM  = pdb->get( context, PSC_PAPI_L2_TCM );
    PAPI_L2_TH  = pdb->get( context, PSC_PAPI_L2_TCH );

    evaluationResult = ( double )PAPI_L2_TM / double( PAPI_L2_TM + PAPI_L2_TH );
}

Property* L2Misses::clone( void ) {
    L2Misses* prop = new L2Misses( context, phaseContext );
    return prop;
}

std::string L2Misses::info( void ) {
    std::stringstream stream;

    stream << '\t' << " PAPI_L2_TCM: " << PAPI_L2_TM << '\t' << " PAPI_L2_TCH: " << PAPI_L2_TH;

    return stream.str();
}

std::string L2Misses::toXMLExtra( void ) {
    std::stringstream stream;

    stream << "\t\t<PAPI_L2_TCM>" << PAPI_L2_TM << "</PAPI_L2_TCM>" << std::endl;
    stream << "\t\t<PAPI_L2_TCH>" << PAPI_L2_TH << "</PAPI_L2_TCH>" << std::endl;

    stream << "\t\t<phaseCycles>" << phaseCycles << "</phaseCycles>" << std::endl;
    stream << "\t\t<cycles>" << cycles << "</cycles>" << std::endl;

    return stream.str();
}
