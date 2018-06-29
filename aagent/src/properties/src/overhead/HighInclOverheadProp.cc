/**
   @file    HighInclOverheadProp.cc
   @ingroup OverheadProperties
   @brief   Property identifies a region with high MRI overhead
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

#include "HighInclOverheadProp.h"
#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "xml_psc_tags.h"

PropertyID HighInclOverheadProp::id() {
    return HIGHINCLOVERHEAD;
}

void HighInclOverheadProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
    std::cout << "overhead:" << ovhdCycles - exclOvhdCycles << "region cycles:" << cycles << std::endl;
}

bool HighInclOverheadProp::condition() const {
    //Is that the number you think is appropriate? You mentioned
    //a certain threshold.
    return ovhdPercent > threshold;
}

double HighInclOverheadProp::confidence() const {
    return 1.0;
}

double HighInclOverheadProp::severity() const {
    return ovhdPercent;
}

Context* HighInclOverheadProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type HighInclOverheadProp::request_metrics() {
    pdb->request( context, PSC_MRI_OVERHEAD );
    pdb->request( context, PSC_MRI_EXCL_OVERHEAD );
    pdb->request( context, PSC_PAPI_TOT_CYC );

    return ALL_INFO_GATHERED;
}

std::string HighInclOverheadProp::name() {
    return "high inclusive overhead";
}

void HighInclOverheadProp::evaluate() {
    exclOvhdCycles = pdb->get( context, PSC_MRI_EXCL_OVERHEAD );
    ovhdCycles     = pdb->get( context, PSC_MRI_OVERHEAD );

    cycles      = pdb->get( context, PSC_PAPI_TOT_CYC );
    phaseCycles = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );

    ovhdPercent = ( double )( ovhdCycles - exclOvhdCycles ) / ( double )cycles * 100;
}

Property* HighInclOverheadProp::clone() {
    HighInclOverheadProp* prop = new HighInclOverheadProp( context, phaseContext );
    return prop;
}

std::string HighInclOverheadProp::info() {
    std::stringstream stream;

    stream << '\t' << " High inclusive overhead " << ovhdPercent << "%";

    return stream.str();
}
;

std::string HighInclOverheadProp::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<ExclOverhead>" << exclOvhdCycles << "</ExclOverhead>" << std::endl;
    stream << "\t\t<InclOverhead>" << ovhdCycles - exclOvhdCycles << "</InclOverhead>" << std::endl;
    stream << "\t\t<Cycles>" << cycles << "</Cycles>" << std::endl;
    stream << "\t\t<PercentPhaseCycles>" << ( double )( ovhdCycles - exclOvhdCycles ) * 100 / ( double )phaseCycles
           << "</PercentPhaseCycles>" << std::endl;

    return stream.str();
}
