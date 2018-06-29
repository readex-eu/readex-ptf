/**
   @file    InterphaseProps.cc
   @ingroup InterphaseProperties
   @brief   Properties for phase features
   @author  Madhura Kumaraswamy
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2017, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include "InterphaseProps.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include "psc_errmsg.h"
#include <sstream>


const std::string InterphaseProps::name_constant = "Interphase Properties";


InterphaseProps::InterphaseProps( Context* ct, Context* phaseCt ) : Property( ct ), phaseCycles(0), phase_iter(0),
                 L3Accesses(0), L3Misses(0), totalInstr(0), branchInstr(0),instances(0),exectime(0),cpuEnergy(0){
    context           = ct;
    phaseContext      = phaseCt;
    energyConsumption = 0;
}


PropertyID InterphaseProps::id() {
    return INTERPHASE_PROPS;
}


void InterphaseProps::print() {
    std::cout << "Property:" << name()
              << "  Process " << context->getRank()
              << "  Thread " << context->getThread() << std::endl
              << "                  " << context->getRegion()->str_print() << std::endl;
}


bool InterphaseProps::condition() const {
    return energyConsumption >= 0;
}


double InterphaseProps::confidence() const {
    return 1.0;
}


double InterphaseProps::severity() const {
    return (double)energyConsumption;
}


Context* InterphaseProps::get_phaseContext() {
    return phaseContext;
}


Gather_Required_Info_Type InterphaseProps::request_metrics() {
    //Context* ct = context->copy();
    pdb->request( context, PSC_AVX_INSTS_CALC );
    pdb->request( context, PSC_PAPI_L3_TCA );
    pdb->request( context, PSC_PAPI_L3_TCM);
    pdb->request( context, PSC_PAPI_BR_CN);

    std::string newMetric;
    //Read the config tree to send proper measurement request
    try {
        newMetric = configTree.get < std::string > ("Configuration.periscope.metrics.node_energy");
        pdb->request( context, PSC_NODE_ENERGY );
    }
    catch (exception &e) {

    }
    try {
        newMetric = configTree.get < std::string > ("Configuration.periscope.metrics.cpu0_energy");
        pdb->request( context, PSC_CPU0_ENERGY );
    }
    catch (exception &e) {

    }
    try {
        newMetric = configTree.get < std::string > ("Configuration.periscope.metrics.cpu1_energy");
        pdb->request( context, PSC_CPU1_ENERGY );
    }
    catch (exception &e) {

    }

    pdb->request( context, PSC_EXECUTION_TIME);
    return ALL_INFO_GATHERED;
}


std::string InterphaseProps::name() {
    return name_constant;
}


void InterphaseProps::evaluate() {
//    Context* ct = context->copy();

    totalInstr = pdb->get( context, PSC_AVX_INSTS_CALC );
    L3Accesses = pdb->get( context, PSC_PAPI_L3_TCA );
    L3Misses   = pdb->get( context, PSC_PAPI_L3_TCM );
    branchInstr = pdb->get( context, PSC_PAPI_BR_CN);
    phase_iter = dp->getCurrentIterationNumber();

    energyConsumption = pdb->get( context, PSC_NODE_ENERGY );     // NOTE: HDEEM returns milli-Joules
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ), "Got energy consumption: %d\n", energyConsumption );

    cpuEnergy = pdb->get(context, PSC_CPU0_ENERGY) + pdb->get(context, PSC_CPU1_ENERGY);
    instances = pdb->get(context, PSC_INSTANCES);
    INT64 cycles = (pdb->get( context, PSC_EXECUTION_TIME ));
    if( cycles > ( INT64 )0 ) {
        exectime = static_cast<double>(cycles) / NANOSEC_PER_SEC_DOUBLE;
    }
    else {
        exectime = 0.0;
    }

//    delete ct;
}


std::string InterphaseProps::info() {
    std::stringstream stream;
    stream << '\t' << "TotalInstr: " << totalInstr ;
    stream << '\t' << "L3Accesses: " << L3Accesses ;
    stream << '\t' << "L3Misses: " << L3Misses ;
    stream << '\t' << "BranchInstr: " << branchInstr;
    stream << '\t' << "PhaseIter: " << phase_iter;

    stream << '\t' << "Node Energy: " << energyConsumption ;
    stream << '\t' << "CPU Energy: " << cpuEnergy ;
    stream << '\t' << "Instances: " << instances ;
    stream << '\t' << "Exec Time: " << exectime;
    return stream.str();
}

std::string InterphaseProps::toXMLExtra() {
    std::stringstream stream;
    stream << "\t\t<TotalInstr>" << totalInstr << "</TotalInstr>" << std::endl;
    stream << "\t\t<L3Accesses>" << L3Accesses << "</L3Accesses>" << std::endl;
    stream << "\t\t<L3Misses>" << L3Misses << "</L3Misses>" << std::endl;
    stream << "\t\t<BranchInstr>" << branchInstr << "</BranchInstr>" << std::endl;
    stream << "\t\t<PhaseIter>" << phase_iter << "</PhaseIter>" << std::endl;

    stream << "\t\t<NodeEnergy>" << energyConsumption << "</NodeEnergy>" << std::endl;
    stream << "\t\t<CPUEnergy>" << cpuEnergy << "</CPUEnergy>" << std::endl;
    stream << "\t\t<Instances>" << instances << "</Instances>" << std::endl;
    stream << "\t\t<ExecTime>" << exectime << "</ExecTime>" << std::endl;
    return stream.str();
}


Property* InterphaseProps::clone() {
    return new InterphaseProps( context, phaseContext );
}
