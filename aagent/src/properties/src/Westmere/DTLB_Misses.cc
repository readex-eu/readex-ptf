/**
   @file    DTLB_Misses.cc
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

#include "DTLB_Misses.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

PropertyID DTLB_Misses::id() {
    return DTLB_MISSES;
}

void DTLB_Misses::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool DTLB_Misses::condition() const {
    return cycles > 0 && severity() > threshold;
}

double DTLB_Misses::confidence() const {
    return 1.0;
}

double DTLB_Misses::severity() const {
    return evaluationResult * ( cycles / phaseCycles );
}

Context* DTLB_Misses::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type DTLB_Misses::request_metrics() {
    pdb->request( context, PSC_NP_DTLB_MISSES_ANY );
    pdb->request( context, PSC_NP_DTLB_LOAD_MISSES_ANY );
    pdb->request( context, PSC_NP_ITLB_MISSES_ANY );
    pdb->request( context, PSC_NP_DTLB_MISSES_WALK_COMPLETED );
    pdb->request( context, PSC_NP_THREAD_P );
    pdb->request( context, PSC_PAPI_TOT_CYC );
    pdb->request( phaseContext, PSC_PAPI_TOT_CYC );

    return ALL_INFO_GATHERED;
}

std::string DTLB_Misses::name() {
    return "DTLB Misses";
}

void DTLB_Misses::evaluate() {
    phaseCycles = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );
    cycles      = pdb->get( context, PSC_PAPI_TOT_CYC );

    DTLB_Misses_Any            = pdb->get( context, PSC_NP_DTLB_MISSES_ANY );
    DTLB_Load_Misses_Any       = pdb->get( context, PSC_NP_DTLB_LOAD_MISSES_ANY );
    ITLB_Misses_Any            = pdb->get( context, PSC_NP_ITLB_MISSES_ANY );
    DTLB_Misses_Walk_Completed = pdb->get( context, PSC_NP_DTLB_MISSES_WALK_COMPLETED );
    Thread_P                   = pdb->get( context, PSC_NP_THREAD_P );

    evaluationResult = ( double )( DTLB_Misses_Any + DTLB_Load_Misses_Any + ITLB_Misses_Any
                                   + DTLB_Misses_Walk_Completed * 30 ) / ( double )Thread_P;
}

Property* DTLB_Misses::clone() {
    DTLB_Misses* prop = new DTLB_Misses( context, phaseContext );
    return prop;
}

std::string DTLB_Misses::info() {
    std::stringstream stream;

    stream << '\t' << " DTLB_Misses_Any: " << DTLB_Misses_Any << '\t' << " DTLB_Load_Misses_Any: " << DTLB_Load_Misses_Any
           << '\t' << " ITLB_Misses_Any: " << ITLB_Misses_Any << '\t' << " DTLB_Misses_Walk_Completed: "
           << DTLB_Misses_Walk_Completed << '\t' << " Thread_P: " << Thread_P;

    return stream.str();
}

std::string DTLB_Misses::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<DTLB_Misses_Any>" << DTLB_Misses_Any << "</DTLB_Misses_Any>" << std::endl;
    stream << "\t\t<DTLB_Load_Misses_Any>" << DTLB_Load_Misses_Any
           << "</DTLB_Load_Misses_Any>" << std::endl;
    stream << "\t\t<ITLB_Misses_Any>" << ITLB_Misses_Any << "</ITLB_Misses_Any>" << std::endl;
    stream << "\t\t<DTLB_Misses_Walk_Completed>" << DTLB_Misses_Walk_Completed
           << "</DTLB_Misses_Walk_Completed>" << std::endl;
    stream << "\t\t<Thread_P>" << Thread_P << "</Thread_P>" << std::endl;

    stream << "\t\t<phaseCycles>" << phaseCycles << "</phaseCycles>" << std::endl;
    stream << "\t\t<cycles>" << cycles << "</cycles>" << std::endl;

    return stream.str();
}
