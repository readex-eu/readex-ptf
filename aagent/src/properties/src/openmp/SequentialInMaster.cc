/**
   @file    SequentialInMaster.cc
   @ingroup OpenMPProperties
   @brief   OpenMP Sequential Computation in Master Region property
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
 */

#include "SequentialInMaster.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"
#include <sstream>

PropertyID SequentialInMasterProp::id() {
    return SEQUENTIALINMASTER;
}

void SequentialInMasterProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl;
}

bool SequentialInMasterProp::condition() const {
    return ( ( double )waitCycles_MasterBody ) / ( double )phaseCycles * 100 > threshold;
}

double SequentialInMasterProp::confidence() const {
    return 1.0;
}

double SequentialInMasterProp::severity() const {
    if( ( ( double )waitCycles_MasterBody / ( double )phaseCycles * 100 ) > 100.0 ) {
        return 100;
    }
    else {
        return ( double )waitCycles_MasterBody / ( double )phaseCycles * 100;
    }
}

Context* SequentialInMasterProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type SequentialInMasterProp::request_metrics() {
    Context* ct = new Context( context->getRegion(), context->getRank(), 0 );
    pdb->request( ct, PSC_MASTER_BODY_CYCLE );
    pdb->request( phaseContext, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string SequentialInMasterProp::name() {
    return "Sequential Computation in MASTER region Overhead";
}

void SequentialInMasterProp::evaluate() {
    Context* ct = new Context( context->getRegion(), context->getRank(), 0 );
    waitCycles_MasterBody = pdb->get( ct, PSC_MASTER_BODY_CYCLE );
    delete ct;
    phaseCycles = pdb->get( phaseContext, PSC_EXECUTION_TIME );
}

std::string SequentialInMasterProp::toXMLExtra() {
    std::stringstream stream;
    stream << "\t\t<ExecTime>" << waitCycles_MasterBody << "</ExecTime>" << std::endl;
    return stream.str();
}

Property* SequentialInMasterProp::clone() {
    SequentialInMasterProp* prop = new SequentialInMasterProp( context, phaseContext );
    return prop;
}
