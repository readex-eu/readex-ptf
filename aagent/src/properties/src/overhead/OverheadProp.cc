/**
   @file    OverheadProp.cc
   @ingroup OverheadProperties
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

#include "OverheadProp.h"
#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "xml_psc_tags.h"

PropertyID OverheadProp::id() {
    return OVERHEAD;
}

void OverheadProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
    std::cout << "required regions" << requiredRegions << std::endl;
}

bool OverheadProp::condition() const {
    //Is that the number you think is appropriate? You mentioned
    //a certain threshold.
    return Overhead > threshold;
}

double OverheadProp::confidence() const {
    return 0.1;
}

double OverheadProp::severity() const {
    //issues=R1*theor
    //cycles/r1=best
    //cycles-best=overhead
    //cycles-cycles/R1=cycles*(1-1/R1)=overhead
    return Overhead;
}

Context* OverheadProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type OverheadProp::request_metrics() {
    return ALL_INFO_GATHERED;
}

std::string OverheadProp::name() {
    return "high Instrumentation overhead";
}

void OverheadProp::evaluate() {
    /*issues = pdb->get(context, ISSUES);
       theoreticalIssues = pdb->get(context, THEORETICAL_ISSUE_BOUND);
       cycles = pdb->get(context, PSC_PAPI_TOT_CYC);
       phaseCycles = pdb->get(phaseContext, PSC_PAPI_TOT_CYC);
       if (theoreticalIssues > 0 && issues > 0)
       r1 = (double)issues / (double)theoreticalIssues;
       else
       r1 = -1;
     */
}

Property* OverheadProp::clone() {
    OverheadProp* prop = new OverheadProp( context, phaseContext, Overhead );
    return prop;
}

std::string OverheadProp::info() {
    std::stringstream stream;

    stream << '\t' << " Instrumentation Overhead" << Overhead;

    return stream.str();
}
;

std::string OverheadProp::toXMLExtra() {
    return requiredRegions;
}
