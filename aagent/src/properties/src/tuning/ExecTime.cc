/**
   @file    ExecTime.cc
   @ingroup TuningProperties
   @brief   Return the execution time of the region in thread 0 as severity.
   @author  Michael Gerndt
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

#include "ExecTime.h"
#include "global.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"
#include <sstream>

ExecTimeProp::ExecTimeProp( Context* ct, Context* phaseCt, double threshold ) : Property( ct ), threshold( threshold ) {
    phaseCycles  = 0;
    context      = ct;
    phaseContext = phaseCt;
}

ExecTimeProp::~ExecTimeProp() {
}

PropertyID ExecTimeProp::id() {
    return EXECTIME;
}

void ExecTimeProp::print() {
    std::cout << "Property:" << name() <<
    "  Process " << context->getRank() <<
    "  Thread " <<  context->getThread() << std::endl;
}

bool ExecTimeProp::condition() const {
    return ExecTime > 0.0 && severity() > threshold;
}

double ExecTimeProp::confidence() const {
    return 1.0;
}

double ExecTimeProp::severity() const {
    return ExecTime;
}

Context* ExecTimeProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type ExecTimeProp::request_metrics() {
    pdb->request( context, PSC_EXECUTION_TIME );

    return ALL_INFO_GATHERED;
}

std::string ExecTimeProp::name() {
    return "ExecTime";
}

void ExecTimeProp::evaluate() {
    INT64    cycles =  pdb->get( context, PSC_EXECUTION_TIME );



    if( cycles > ( INT64 )0 ) {
        ExecTime = cycles / NANOSEC_PER_SEC_DOUBLE;
    }
    else {
        ExecTime = 0.0;
    }
}

std::string ExecTimeProp::toXMLExtra() {
    std::stringstream stream;
    stream << "\t\t<ExecTime>" << ExecTime << "</ExecTime>" << std::endl;
    return stream.str();
}

Property* ExecTimeProp::clone() {
    ExecTimeProp* prop = new ExecTimeProp( context, phaseContext, threshold );
    return prop;
}

//BOOST_CLASS_EXPORT_IMPLEMENT(ExecTimeProp);
