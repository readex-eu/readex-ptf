/**
   @file    AddressAliasing.cc
   @ingroup WestmereProperties
   @brief   Westmere specific property
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

#include "AddressAliasing.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

//Description : Tells you the fraction of execution that was wasted due to 4k false store forwarding
//Westmere EX Formula: (PARTIAL_ADDRESS_ALIAS * 3) / CPU_CLOCK_UNHALTED_THREAD_P
//Recommendation : The code is probably impacted by Set Associative Cache issues, 4k false Store Forwarding
//                 and may have Memory bank issues (all resulting in memory cache misses)

PropertyID AddressAliasing::id( void ) {
    return ADDRESS_ALIASING;
}

void AddressAliasing::print( void ) {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool AddressAliasing::condition( void ) const {
    return cycles > 0 && severity() > threshold;
}

double AddressAliasing::confidence( void ) const {
    return 1.0;
}

double AddressAliasing::severity( void ) const {
    return evaluationResult * ( cycles / phaseCycles );
}

Context* AddressAliasing::get_phaseContext( void ) {
    return phaseContext;
}

Gather_Required_Info_Type AddressAliasing::request_metrics( void ) {
    pdb->request( context, PSC_NP_PARTIAL_ADDRESS_ALIAS );
    pdb->request( context, PSC_NP_THREAD_P );
    pdb->request( context, PSC_PAPI_TOT_CYC );
    pdb->request( phaseContext, PSC_PAPI_TOT_CYC );

    return ALL_INFO_GATHERED;
}

std::string AddressAliasing::name( void ) {
    return "Address Aliasing";
}

void AddressAliasing::evaluate( void ) {
    phaseCycles           = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );
    cycles                = pdb->get( context, PSC_PAPI_TOT_CYC );
    Partial_Address_Alias = pdb->get( context, PSC_NP_PARTIAL_ADDRESS_ALIAS );
    Thread_P              = pdb->get( context, PSC_NP_THREAD_P );

    evaluationResult = ( double )Partial_Address_Alias * 3 / ( double )Thread_P;
}

Property* AddressAliasing::clone( void ) {
    AddressAliasing* prop = new AddressAliasing( context, phaseContext );
    return prop;
}

std::string AddressAliasing::info( void ) {
    std::stringstream stream;

    stream << '\t' << " Partial_Address_Alias: " << Partial_Address_Alias << '\t' << " Thread_P: " << Thread_P;

    return stream.str();
}

std::string AddressAliasing::toXMLExtra( void ) {
    std::stringstream stream;

    stream << "\t\t<Partial_Address_Alias>" << Partial_Address_Alias << "</Partial_Address_Alias>" << std::endl;
    stream << "\t\t<Thread_P>" << Thread_P << "</Thread_P>" << std::endl;

    stream << "\t\t<phaseCycles>" << phaseCycles << "</phaseCycles>" << std::endl;
    stream << "\t\t<cycles>" << cycles << "</cycles>" << std::endl;

    return stream.str();
}
