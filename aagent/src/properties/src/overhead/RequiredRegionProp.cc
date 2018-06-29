/**
   @file    RequiredRegionProp.cc
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

#include "RequiredRegionProp.h"
#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "xml_psc_tags.h"

PropertyID RequiredRegionProp::id() {
    return REQUIREDREGION;
}

void RequiredRegionProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
    std::cout << "required regions" << requiredRegions << std::endl;
}

bool RequiredRegionProp::condition() const {
    //Is that the number you think is appropriate? You mentioned
    //a certain threshold.
    return true;
}

double RequiredRegionProp::confidence() const {
    return 0.1;
}

double RequiredRegionProp::severity() const {
    return 0;
}

Context* RequiredRegionProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type RequiredRegionProp::request_metrics() {
    return ALL_INFO_GATHERED;
}

std::string RequiredRegionProp::name() {
    return "Required regions in the next experiment";
}

void RequiredRegionProp::evaluate() {
}

Property* RequiredRegionProp::clone() {
    RequiredRegionProp* prop = new RequiredRegionProp( context, phaseContext );
    return prop;
}

std::string RequiredRegionProp::info() {
    std::stringstream stream;

    stream << '\t' << " Required regions " << requiredRegions;

    return stream.str();
}
;

std::string RequiredRegionProp::toXMLExtra() {
    return requiredRegions;
}

void RequiredRegionProp::add_region( std::string rgn ) {
    requiredRegions += rgn;
}
