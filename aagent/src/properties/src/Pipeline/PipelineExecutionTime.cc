/**
   @file    PipelineExecutionTime.cc
   @ingroup PipelineProperties
   @brief   Execution time of pipeline for Vienna pattern property header
   @author  Research Group Scientific Computing, University of Vienna
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


#include "PipelineExecutionTime.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include "psc_errmsg.h"
#include <sstream>

PipelineExecutionTime::PipelineExecutionTime( Context* ct, Context* phaseCt, double threshold ) : Property( ct ), threshold( threshold ) {
    phaseCycles  = 0;
    context      = ct;
    phaseContext = phaseCt;
}

PipelineExecutionTime::~PipelineExecutionTime() {
}

PropertyID PipelineExecutionTime::id() {
    return PIPEEXECTIME;
}


void PipelineExecutionTime::print() {
    std::cout << "Property:" << name() <<
    "  Process " << context->getRank() <<
    "  Thread " <<  context->getThread() << std::endl;
}

bool PipelineExecutionTime::condition() const {
    return PipelineExecTime > 0 && severity() > threshold;
}

double PipelineExecutionTime::confidence() const {
    return 1.0;
}

double PipelineExecutionTime::severity() const {
    return PipelineExecTime;
}

Context* PipelineExecutionTime::get_phaseContext() {
    return phaseContext;
}


Gather_Required_Info_Type PipelineExecutionTime::request_metrics() {
    Context* ct = new Context( context->getRegion(), context->getRank(), 0 );
    pdb->request( ct, PSC_PIPE_EXEC_TIME );

    return ALL_INFO_GATHERED;
}

std::string PipelineExecutionTime::name() {
    return "PipelineExecutionTime";
}

void PipelineExecutionTime::evaluate() {
    Context* ct = new Context( context->getRegion(), context->getRank(), 0 );
    PipelineExecTime = pdb->get( ct, PSC_PIPE_EXEC_TIME );
    PipelineExecTime = PipelineExecTime / NANOSEC_PER_SEC_DOUBLE;
    psc_dbgmsg( 1010, "Got execution time: %f\n", PipelineExecTime );
    delete ct;
}

std::string PipelineExecutionTime::info() {
    std::stringstream stream;

    stream << '\t' << " PIPE_EXEC_TIME: " << PipelineExecTime;

    return stream.str();
}

Property* PipelineExecutionTime::clone() {
    PipelineExecutionTime* prop = new PipelineExecutionTime( context, phaseContext );
    return prop;
}
