/**
   @file    ImbalanceInParSections.cc
   @ingroup OpenMPProperties
   @brief   OpenMP Load Imbalance in Parallel Sections property
   @author  Shajulin Benedict
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim

   Refinements:
 *       i) Load imbalance due to not enough sections property
 *       ii) Load imbalance due to uneven sections property
 */
#include "ImbalanceInParSections.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"
#include <sstream>

PropertyID ImbalanceInParSectionsProp::id() {
    countNumSectionBodies();
    //std::cout<<"numOfParallelSectionBodies: " << numOfParallelSectionBodies << std::endl;
    if( appl->getOmpThreads() > numOfParallelSectionBodies ) {
        return NOT_ENOUGH_SECTION_OMP;
    }
    else if( appl->getOmpThreads() < numOfParallelSectionBodies ) {
        return UNEVEN_SECTION_OMP;
    }
    else {
        return LOADIMBALANCEPARALLELSECTION;
    }
}

void ImbalanceInParSectionsProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl;
}

bool ImbalanceInParSectionsProp::condition() const {
    return ( ( double )maxWaitTime - ( double )minWaitTime ) / ( double )phaseCycles * 100 > threshold;
}

double ImbalanceInParSectionsProp::confidence() const {
    return 1.0;
}

double ImbalanceInParSectionsProp::severity() const {
    if( ( ( ( double )maxWaitTime - ( double )minWaitTime ) / ( double )phaseCycles * 100 ) > 100.0 ) {
        return 100;
    }
    else {
        return ( ( double )maxWaitTime - ( double )minWaitTime ) / ( double )phaseCycles * 100;
    }
}

Context* ImbalanceInParSectionsProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type ImbalanceInParSectionsProp::request_metrics() {
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        pdb->request( ct, PSC_IMPLICIT_BARRIER_TIME );
    }
    pdb->request( phaseContext, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string ImbalanceInParSectionsProp::name() {
    countNumSectionBodies();
    if( appl->getOmpThreads() > numOfParallelSectionBodies ) {
        return "Load Imbalance due to not enough sections in SECTIONS region";
    }
    else if( appl->getOmpThreads() < numOfParallelSectionBodies ) {
        return "Load Imbalance due to uneven sections in SECTIONS region";
    }
    else {
        return "Load Imbalance in SECTIONS region";
    }
}

void ImbalanceInParSectionsProp::countNumSectionBodies() {
    std::list<Region*>           sub_regions;
    std::list<Region*>::iterator subr_it;
    Region*                      region = context->getRegion();

    sub_regions                = region->get_subregions();
    numOfParallelSectionBodies = 0;
    for( subr_it = sub_regions.begin(); subr_it != sub_regions.end(); subr_it++ ) {
        if( ( *subr_it )->get_type() == SECTION_REGION ) {
            numOfParallelSectionBodies++;
        }
    }
}

void ImbalanceInParSectionsProp::evaluate() {
    waitCycles.resize( appl->getOmpThreads(), 0 );

    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        waitCycles[ i ] = pdb->get( ct, PSC_IMPLICIT_BARRIER_TIME );

        delete ct;
    }

    phaseCycles = pdb->get( phaseContext, PSC_EXECUTION_TIME );
    maxWaitTime = waitCycles[ 0 ];
    minWaitTime = waitCycles[ 0 ];
    for( int i = 1; i < appl->getOmpThreads(); i++ ) {
        if( waitCycles[ i ] > maxWaitTime && waitCycles[ i ] != 0 ) {
            maxWaitTime = waitCycles[ i ];
        }
        if( waitCycles[ i ] < minWaitTime && waitCycles[ i ] != 0 ) {
            minWaitTime = waitCycles[ i ];
        }
    }
}

std::string ImbalanceInParSectionsProp::toXMLExtra() {
    std::stringstream stream;
    stream << "\t\t<ExecTime>" << maxWaitTime << "</ExecTime>" << std::endl;
    return stream.str();
}

Property* ImbalanceInParSectionsProp::clone() {
    ImbalanceInParSectionsProp* prop = new ImbalanceInParSectionsProp( context, phaseContext );
    return prop;
}
