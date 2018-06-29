/**
   @file    PipelineStageBufWaitTime.cc
   @ingroup PipelineProperties
   @brief   Buffer wait time of pipeline stage for Vienna pattern property header
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


#include "PipelineStageBufWaitTime.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include "psc_errmsg.h"
#include <sstream>

PipelineStageBufWaitTime::PipelineStageBufWaitTime( Context* ct, Context* phaseCt, double threshold ) : Property( ct ), threshold( threshold ) {
    phaseCycles  = 0;
    context      = ct;
    phaseContext = phaseCt;
}

PipelineStageBufWaitTime::~PipelineStageBufWaitTime() {
}

PropertyID PipelineStageBufWaitTime::id() {
    return PIPESTAGEBUFWAITTIME;
}


void PipelineStageBufWaitTime::print() {
    std::cout << "Property:" << name() <<
    "  Process " << context->getRank() <<
    "  Thread " <<  context->getThread() << std::endl;
}

bool PipelineStageBufWaitTime::condition() const {
    return PipeStageBufWaitTime > 0 && severity() > threshold;
}

double PipelineStageBufWaitTime::confidence() const {
    return 1.0;
}

double PipelineStageBufWaitTime::severity() const {
    return PipeStageBufWaitTime;
}

Context* PipelineStageBufWaitTime::get_phaseContext() {
    return phaseContext;
}


Gather_Required_Info_Type PipelineStageBufWaitTime::request_metrics() {
    Context* ct = new Context( context->getRegion(), context->getRank(), 0 );
    pdb->request( ct, PSC_PIPE_BUFOUTP_TIME );

    return ALL_INFO_GATHERED;
}

std::string PipelineStageBufWaitTime::name() {
    return "PipelineStageBufWaitTime";
}

void PipelineStageBufWaitTime::evaluate() {
    Context* ct = new Context( context->getRegion(), context->getRank(), 0 );
    PipeStageBufWaitTime = pdb->get( ct, PSC_PIPE_BUFOUTP_TIME );
    PipeStageBufWaitTime = PipeStageBufWaitTime / NANOSEC_PER_SEC_DOUBLE;
    psc_dbgmsg( 1010, "Got execution time: %f\n", PipeStageBufWaitTime );
    delete ct;
}

std::string PipelineStageBufWaitTime::info() {
    std::stringstream stream;

    stream << '\t' << " PIPE_STAGEBUFWAIT_TIME: " << PipeStageBufWaitTime;

    return stream.str();
}

Property* PipelineStageBufWaitTime::clone() {
    PipelineStageBufWaitTime* prop = new PipelineStageBufWaitTime( context, phaseContext );
    return prop;
}
