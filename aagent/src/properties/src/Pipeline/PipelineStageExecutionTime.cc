/**
   @file    PipelineStageExecutionTime.cc
   @ingroup PipelineProperties
   @brief   Execution time of pipeline stage for Vienna pattern property header
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


#include "PipelineStageExecutionTime.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include "psc_errmsg.h"
#include <sstream>

PipelineStageExecutionTime::PipelineStageExecutionTime( Context* ct, Context* phaseCt, double threshold ) : Property( ct ), threshold( threshold ) {
    phaseCycles  = 0;
    context      = ct;
    phaseContext = phaseCt;
}

PipelineStageExecutionTime::~PipelineStageExecutionTime() {
}

PropertyID PipelineStageExecutionTime::id() {
    return PIPESTAGEEXECTIME;
}


void PipelineStageExecutionTime::print() {
    std::cout << "Property:" << name() <<
    "  Process " << context->getRank() <<
    "  Thread " <<  context->getThread() << std::endl;
}

bool PipelineStageExecutionTime::condition() const {
    return PipelineStageExecTime > 0 && severity() > threshold;
}

double PipelineStageExecutionTime::confidence() const {
    return 1.0;
}

double PipelineStageExecutionTime::severity() const {
    return PipelineStageExecTime;
}

Context* PipelineStageExecutionTime::get_phaseContext() {
    return phaseContext;
}


Gather_Required_Info_Type PipelineStageExecutionTime::request_metrics() {
    Context* ct = new Context( context->getRegion(), context->getRank(), 0 );
    pdb->request( ct, PSC_PIPE_STAGEEXEC_TIME );

    return ALL_INFO_GATHERED;
}

std::string PipelineStageExecutionTime::name() {
    return "PipelineStageExecutionTime";
}

void PipelineStageExecutionTime::evaluate() {
    Context* ct = new Context( context->getRegion(), context->getRank(), 0 );
    PipelineStageExecTime = pdb->get( ct, PSC_PIPE_STAGEEXEC_TIME );
    PipelineStageExecTime = PipelineStageExecTime / NANOSEC_PER_SEC_DOUBLE;
    psc_dbgmsg( 1010, "Got execution time: %f\n", PipelineStageExecTime );
    delete ct;
}

std::string PipelineStageExecutionTime::info() {
    std::stringstream stream;

    stream << '\t' << " PIPE_STAGEEXEC_TIME: " << PipelineStageExecTime;

    return stream.str();
}

Property* PipelineStageExecutionTime::clone() {
    PipelineStageExecutionTime* prop = new PipelineStageExecutionTime( context, phaseContext );
    return prop;
}
