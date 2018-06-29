/**
   @file    KernelExecutionTime.cc
   @ingroup OpenCLProperties
   @brief   Execution time of OpenCL kernel property header
   @author  Robert Mijakovic
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


#include "KernelExecutionTime.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

//Description :  Tells you the total amount of time spent on kernel execution
//CUDA Formula:  PSC_OPENCL_KERNEL_EXECUTION_TIME
//Recommendation :
//


KernelExecutionTime::KernelExecutionTime( Context* ct, Context* phaseCt,
                                          string kName ) : Property( ct ) {
    evaluationResult = 0.0;
    phaseContext     = phaseCt;
    kernelName       = kName;
}

KernelExecutionTime::~KernelExecutionTime() {
}

PropertyID KernelExecutionTime::id() {
    return OPENCL_KERNELEXECTIME;
}

void KernelExecutionTime::print() {
    std::cout << "Property:" << name()
              << "  Process " << context->getRank()
              << "  Thread " << context->getThread() << std::endl
              << "                  " << context->getRegion()->str_print() << std::endl;
}

bool KernelExecutionTime::condition() const {
    return Execution_Time > 0 && severity() > 0;
}

double KernelExecutionTime::confidence() const {
    return 1.0;
}

double KernelExecutionTime::severity() const {
    return evaluationResult;
}

Context* KernelExecutionTime::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type KernelExecutionTime::request_metrics() {
    pdb->request( context, PSC_OPENCL_KERNEL_EXECUTION_TIME );

    return ALL_INFO_GATHERED;
}

std::string KernelExecutionTime::name() {
    return "Kernel execution time";
}

void KernelExecutionTime::evaluate() {
    Execution_Time = pdb->get( context, PSC_OPENCL_KERNEL_EXECUTION_TIME );

    evaluationResult = ( double )( Execution_Time );
}

Property* KernelExecutionTime::clone() {
    KernelExecutionTime* prop = new KernelExecutionTime( context, phaseContext, kernelName );
    return prop;
}

std::string KernelExecutionTime::info() {
    std::stringstream stream;

    stream << '\t' << " Execution_Time: " << Execution_Time;
    stream << '\t' << " Kernel_Name: " << kernelName;

    return stream.str();
}

std::string KernelExecutionTime::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<Execution_Time>" << Execution_Time << "</Execution_Time>" << endl;
    stream << "\t\t<Kernel_Name>" << kernelName << "</Kernel_Name>" << endl;
    return stream.str();
}
