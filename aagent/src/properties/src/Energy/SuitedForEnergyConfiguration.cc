/**
   @file    SuitedForEnergyConfiguration.cc
   @ingroup EnergyProperties
   @brief   Return the execution time of the region in thread 0 as severity.
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

#include "SuitedForEnergyConfiguration.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"
#include <sstream>


SuitedForEnergyConfiguration::SuitedForEnergyConfiguration( Context* ct, Context* phaseCt, double threshold ):
        Property(ct), cycles(0), phaseCycles(0), threshold(threshold), nanojoules(0), nanojoules_dram(0) {
    context       = ct;
    phaseContext  = phaseCt;
    l3CacheMisses = 0;
    l2CacheMisses = 0;
    instr         = 0;
}

PropertyID SuitedForEnergyConfiguration::id() {
    return ENERGY_SUITED;
}

void SuitedForEnergyConfiguration::print() {
    std::cout << "Property:" << name() <<
    "  Process " << context->getRank() <<
    "  Thread " <<  context->getThread() << std::endl;
}

bool SuitedForEnergyConfiguration::condition() const {
    return TimePerInstance > 0 && severity() > threshold;
}

double SuitedForEnergyConfiguration::confidence() const {
    return 1.0;
}

double SuitedForEnergyConfiguration::severity() const {
    return TimePerInstance;
}

Context* SuitedForEnergyConfiguration::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type SuitedForEnergyConfiguration::request_metrics() {
    // creating a new context here, because we really need to get the data from the first core
    // TODO: verify this or remove temporary context
    Context* ct = new Context( context->getRegion(), context->getRank(), 0 );
    pdb->request( ct, PSC_EXECUTION_TIME );
    pdb->request( ct, PSC_PAPI_L3_TCM );
    pdb->request( ct, PSC_PAPI_L2_TCM );
    pdb->request( ct, PSC_PAPI_TOT_CYC );
    pdb->request( ct, PSC_PAPI_TOT_INS );
    pdb->request( ct, PSC_ENOPT_ALL_CORES );
    pdb->request( ct, PSC_ENOPT_ALL_DRAMS );

    // Add metrics for model

    return ALL_INFO_GATHERED;
}

std::string SuitedForEnergyConfiguration::name() {
    return "SuitedForEnergyConfiguration";
}

void SuitedForEnergyConfiguration::evaluate() {
    // creating a new context here, because we really need to get the data from the first core
    // TODO: verify this or remove temporary context
    Context* ct = new Context( context->getRegion(), context->getRank(), 0 );

    ExecutionTime       = pdb->get( ct, PSC_EXECUTION_TIME ) / NANOSEC_PER_SEC_DOUBLE;
    RegionExecutedCount = pdb->get( ct, PSC_INSTANCES );
    TimePerInstance     = ExecutionTime / ( double )RegionExecutedCount;

    l3CacheMisses   = pdb->get( ct, PSC_PAPI_L3_TCM );
    l2CacheMisses   = pdb->get( ct, PSC_PAPI_L2_TCM );
    cycles          = pdb->get( ct, PSC_PAPI_TOT_CYC );
    instr           = pdb->get( ct, PSC_PAPI_TOT_INS );
    nanojoules      = pdb->get( ct, PSC_ENOPT_ALL_CORES );
    nanojoules_dram = pdb->get( ct, PSC_ENOPT_ALL_DRAMS );

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AnalysisProperty ), "Evaluation of SuitedForEnergyConfiguration:\n" );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AnalysisProperty ), "    Region name: %s\n", ct->getRegionId().c_str() );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AnalysisProperty ), "    Execution time: %f\n", ExecutionTime );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AnalysisProperty ), "    Region executed: %ld times\n", RegionExecutedCount );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AnalysisProperty ), "    Time per instance: %f\n", TimePerInstance );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AnalysisProperty ), "    L2 cache misses: %ld\n", l2CacheMisses );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AnalysisProperty ), "    L3 cache misses: %ld\n", l3CacheMisses );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AnalysisProperty ), "    Cycles: %ld\n", cycles );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AnalysisProperty ), "    Nanojoules: %ld\n", nanojoules );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AnalysisProperty ), "    Nanojoules (DRAM): %ld\n", nanojoules_dram );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AnalysisProperty ), "    Instructions: %ld\n\n", instr );
    delete ct;
}

std::string SuitedForEnergyConfiguration::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<TimePerInstance>" << TimePerInstance << "</TimePerInstance>" << std::endl;
    stream << "\t\t<ExecutionTime>" << ExecutionTime << "</ExecutionTime>" << std::endl;
    stream << "\t\t<RegionExecutedCount>" << RegionExecutedCount << "</RegionExecutedCount>" << std::endl;
    stream << "\t\t<L3CacheMisses>" << l3CacheMisses << "</L3CacheMisses>" << std::endl;
    stream << "\t\t<L2CacheMisses>" << l2CacheMisses << "</L2CacheMisses>" << std::endl;
    stream << "\t\t<Instructions>" << instr << "</Instructions>" << std::endl;
    stream << "\t\t<Cycles>" << cycles << "</Cycles>" << std::endl;
    stream << "\t\t<Nanojoules>" << nanojoules << "</Nanojoules>" << std::endl;
    stream << "\t\t<DRAM>" << nanojoules_dram << "</DRAM>" << std::endl;

    // Add info metrics for model...

    return stream.str();
}

Property* SuitedForEnergyConfiguration::clone() {
    return new SuitedForEnergyConfiguration( context, phaseContext, threshold );
}

std::string SuitedForEnergyConfiguration::info() {
    std::stringstream stream;

    stream << " Time per inst: " << TimePerInstance;
    stream << ", #inst: " << RegionExecutedCount;
    stream << ", L3 misses: " << l3CacheMisses;
    stream << ", L2 misses: " << l2CacheMisses;
    stream << ", Instr: " << instr;
    stream << ", Cycles: " << cycles;
    stream << ", Nanojoules: " << nanojoules;
    stream << ", DRAM: " << nanojoules_dram;

    return stream.str();
}
