/**
   @file    HPCConditional.cc
   @ingroup EnergyProperties
   @brief   Returns important metrics used as criterium on the plugin selection process in adaptive-sequence plugin
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

#include "HPCConditional.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"
#include <sstream>


HPCConditional::HPCConditional( Context* ct, Context* phaseCt, double threshold ) : Property( ct ), threshold( threshold ) {
    phaseCycles         = 0;
    context             = ct;
    phaseContext        = phaseCt;
    ExecutionTime       = 0.0;
    RegionExecutedCount = 0;
    TimePerInstance     = 0.0;
    MPITime             = 0.0;
    TotalInstructions   = 0;
    EnergyCores         = 0;
    EnergyDRAM          = 0;
}

HPCConditional::~HPCConditional() {
}

PropertyID HPCConditional::id() {
    return HPCCONDITIONAL;
}

void HPCConditional::print() {
    std::cout << "Property:" << name() <<
    "  Process " << context->getRank() <<
    "  Thread " <<  context->getThread() << std::endl;
}

bool HPCConditional::condition() const {
    return TimePerInstance > 0 && severity() > threshold;
}

double HPCConditional::confidence() const {
    return 1.0;
}

double HPCConditional::severity() const {
    return TimePerInstance;
}

Context* HPCConditional::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type HPCConditional::request_metrics() {
    Context* ct = new Context( context->getRegion(), context->getRank(), 0 );
    pdb->request( ct, PSC_EXECUTION_TIME );
    pdb->request( ct, PSC_INSTANCES );
    pdb->request( ct, PSC_MPI );
    pdb->request( ct, PSC_PAPI_TOT_INS );
    pdb->request( ct, PSC_ENOPT_ALL_CORES );
    pdb->request( ct, PSC_ENOPT_ALL_DRAMS );

    return ALL_INFO_GATHERED;
}

std::string HPCConditional::name() {
    return "HPCConditional";
}

void HPCConditional::evaluate() {
    Context* ct = new Context( context->getRegion(), context->getRank(), 0 );

    ExecutionTime       = ( double )pdb->get( ct, PSC_EXECUTION_TIME ) / NANOSEC_PER_SEC_DOUBLE;
    RegionExecutedCount = pdb->get( ct, PSC_INSTANCES );
    TimePerInstance     = ExecutionTime / ( double )RegionExecutedCount;

    MPITime           = ( double )pdb->get( ct, PSC_MPI_TIME_SPENT ) / NANOSEC_PER_SEC_DOUBLE;
    TotalInstructions = pdb->get( ct, PSC_PAPI_TOT_INS );
    EnergyCores       = pdb->get( ct, PSC_ENOPT_ALL_CORES );
    EnergyDRAM        = pdb->get( ct, PSC_ENOPT_ALL_DRAMS );

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AnalysisProperty ), "Evaluation of SuitedForEnergyTuning:\n" );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AnalysisProperty ), "  Execution time: %f\n", ExecutionTime );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AnalysisProperty ), "  Region executed: %f times\n", RegionExecutedCount );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AnalysisProperty ), "  Time per instance: %f\n", TimePerInstance );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AnalysisProperty ), "  MPI Time: %ld\n", MPITime );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AnalysisProperty ), "  Energy for Cores: %ld\n", EnergyCores );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AnalysisProperty ), "  Energy for DRAM: %ld\n", EnergyDRAM );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AnalysisProperty ), "  Total Instructions: %ld\n\n", TotalInstructions );
    delete ct;
}

std::string HPCConditional::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<ExecutionTime>" << ExecutionTime << "</ExecutionTime>" << std::endl;
    stream << "\t\t<RegionExecutedCount>" << RegionExecutedCount << "</RegionExecutedCount>" << std::endl;
    stream << "\t\t<TimePerInstance>" << TimePerInstance << "</TimePerInstance>" << std::endl;
    stream << "\t\t<MPITime>" << MPITime << "</MPITime>" << std::endl;
    stream << "\t\t<EnergyCores>" << EnergyCores << "</EnergyCores>" << std::endl;
    stream << "\t\t<EnergyDRAM>" << EnergyDRAM << "</EnergyDRAM>" << std::endl;
    stream << "\t\t<TotalInstructions>" << TotalInstructions << "</TotalInstructions>" << std::endl;

    // Add info metrics for model...

    return stream.str();
}

Property* HPCConditional::clone() {
    HPCConditional* prop = new HPCConditional( context, phaseContext, threshold );
    return prop;
}

std::string HPCConditional::info() {
    std::stringstream stream;

    stream << "Execution time: " << ExecutionTime;
    stream << ", Number of region executions: " << RegionExecutedCount;
    stream << ", Time per instance: " << TimePerInstance;
    stream << ", MPITime: " << MPITime;
    stream << ", Cores Energy: " << EnergyCores;
    stream << ", DRAM Energy: " << EnergyDRAM;
    stream << ", Total Instructions: " << TotalInstructions;

    return stream.str();
}
