/**
   @file    EnergyConsumption.cc
   @ingroup EnergyProperties
   @brief   Energy consumption property
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

#include "EnergyConsumption.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include "psc_errmsg.h"
#include <sstream>

//Description :  Tells you the energy consumption of code region
//ENOPT Formula:  PSC_ENOPT_ALL_CORES
//Recommendation :
//


const std::string EnergyConsumption::name_constant = "Energy Consumption";


EnergyConsumption::EnergyConsumption( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles       = 0;
    context           = ct;
    phaseContext      = phaseCt;
    energyConsumption = 0;
    totalInstr        = 0;
}

PropertyID EnergyConsumption::id() {
    return ENERGY_CONSUMPTION;
}


void EnergyConsumption::print() {
    std::cout << "Property:" << name()
              << "  Process " << context->getRank()
              << "  Thread " << context->getThread() << std::endl
              << "                  " << context->getRegion()->str_print() << std::endl;
}


bool EnergyConsumption::condition() const {
    return energyConsumption >= 0;
}


double EnergyConsumption::confidence() const {
    return 1.0;
}


double EnergyConsumption::severity() const {
    return energyConsumption;
}


Context* EnergyConsumption::get_phaseContext() {
    return phaseContext;
}


Gather_Required_Info_Type EnergyConsumption::request_metrics() {

    std::string newMetric;
    //Read the config tree to send proper measurement request
    try {
        newMetric = configTree.get < std::string > ("Configuration.periscope.metrics.node_energy");
        pdb->request( context, PSC_NODE_ENERGY );
    }
    catch ( exception &e ) {

    }
    try {
        newMetric = configTree.get < std::string > ("Configuration.periscope.metrics.cpu0_energy");
        pdb->request( context, PSC_CPU0_ENERGY );
    }
    catch ( exception &e ) {

    }
    try {
        newMetric = configTree.get < std::string > ("Configuration.periscope.metrics.cpu1_energy");
        pdb->request( context, PSC_CPU1_ENERGY );
    }
    catch ( exception &e ) {

    }

    pdb->request( context, PSC_EXECUTION_TIME );
//    pdb->request( ct, PSC_PAPI_TOT_INS );
    //pdb->request( ct, PSC_INSTANCES );

    return ALL_INFO_GATHERED;
}


std::string EnergyConsumption::name() {
    return name_constant;
}


void EnergyConsumption::evaluate() {
    INT64    raw = pdb->get( context, PSC_NODE_ENERGY );
    //energyConsumption = static_cast<double>( raw ) / 1e9;      // NOTE: Score-P returns nano-Joules
    energyConsumption = static_cast<double>( raw );      // NOTE: HDEEM returns nano-Joules
    //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ), "Got energy consumption: %f\n", energyConsumption );

    cpuEnergy    = pdb->get( context, PSC_CPU0_ENERGY ) + pdb->get( context, PSC_CPU1_ENERGY );
    instances    = pdb->get( context, PSC_INSTANCES );
    INT64 cycles = pdb->get( context, PSC_EXECUTION_TIME );
    if( cycles > ( INT64 )0 ) {
        time = cycles / NANOSEC_PER_SEC_DOUBLE;
    }
    else {
        time = 0.0;
    }
//    totalInstr = pdb->get( ct, PSC_PAPI_TOT_INS );
}


std::string EnergyConsumption::info() {
    std::stringstream stream;
    stream << '\t' << "Node Energy: " << energyConsumption ;
    stream << '\t' << "CPU Energy: " << cpuEnergy ;
    stream << '\t' << "Instances: " << instances ;
    stream << '\t' << "Exec Time: " << time;
//    stream << '\t' << "TotalInstr: " << totalInstr ;
    return stream.str();
}

std::string EnergyConsumption::toXMLExtra() {
    std::stringstream stream;
    stream << "\t\t<NodeEnergy>" << energyConsumption << "</NodeEnergy>" << std::endl;
    stream << "\t\t<CPUEnergy>" << cpuEnergy << "</CPUEnergy>" << std::endl;
    stream << "\t\t<Instances>" << instances << "</Instances>" << std::endl;
    stream << "\t\t<ExecTime>" << time << "</ExecTime>" << std::endl;
//    stream << "\t\t<TotalInstr>" << totalInstr << "</TotalInstr>" << std::endl;
    return stream.str();
}


Property* EnergyConsumption::clone() {
    return new EnergyConsumption( context, phaseContext );
}
