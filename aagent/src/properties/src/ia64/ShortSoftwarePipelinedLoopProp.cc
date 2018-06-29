/**
   @file    ShortSoftwarePipelinedLoopProp.cc
   @ingroup Itanium2Properites
   @brief   Itanium2 specific property
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

#include "ShortSoftwarePipelinedLoopProp.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

PropertyID ShortSoftwarePipelinedLoopProp::id() {
    return SHORTSWPLOOP;
}

void ShortSoftwarePipelinedLoopProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool ShortSoftwarePipelinedLoopProp::condition() const {
    //ar.ec (count here) determines how many iterations will be
    //needed for getting to full speed. Correct? The
    //smaller the value the better?
    //A*N+(A*arec-1) where A is issues and arec is count.
    INT64 cyclesSingleInstance;

    if( issues == 0 || instances == 0 ) {
        return false;
    }

    cyclesSingleInstance = cycles / instances;
    iterations           = ( cyclesSingleInstance - issues * count ) / issues;
    if( iterations < 10 * count ) {
        return true;
    }
    else {
        return false;
    }
}

double ShortSoftwarePipelinedLoopProp::confidence() const {
    return 1.0;
}

double ShortSoftwarePipelinedLoopProp::severity() const {
    //The overhead is the prolog and epilog. Best
    //we can do is to get rid of it.
    return ( ( double )( issues * count ) / ( double )phaseCycles ) * 100;
}

Context* ShortSoftwarePipelinedLoopProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type ShortSoftwarePipelinedLoopProp::request_metrics() {
    pdb->request( context, PSC_ITERATIONS_IN_SOFTWARE_PIPELINE );
    pdb->request( context, PSC_PAPI_TOT_CYC );
    pdb->request( context, PSC_ISSUES );
    pdb->request( phaseContext, PSC_PAPI_TOT_CYC );
    return ALL_INFO_GATHERED;
}

std::string ShortSoftwarePipelinedLoopProp::name() {
    return "SWP overhead dominates execution";
}

void ShortSoftwarePipelinedLoopProp::evaluate() {
    count       = pdb->get( context, PSC_ITERATIONS_IN_SOFTWARE_PIPELINE );
    cycles      = pdb->get( context, PSC_PAPI_TOT_CYC );
    issues      = pdb->get( context, PSC_ISSUES );
    phaseCycles = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );
}

Property* ShortSoftwarePipelinedLoopProp::clone() {
    ShortSoftwarePipelinedLoopProp* prop = new ShortSoftwarePipelinedLoopProp( context, phaseContext );
    return prop;
}

std::string ShortSoftwarePipelinedLoopProp::info() {
    std::stringstream stream;

    stream << '\t' << " Iterations in prolog/epilog: " << count;

    return stream.str();
}
;

std::string ShortSoftwarePipelinedLoopProp::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<iters>" << count << "</iters>" << std::endl;

    return stream.str();
}
