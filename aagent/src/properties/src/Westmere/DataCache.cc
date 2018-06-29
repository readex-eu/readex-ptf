/**
   @file    DataCache.cc
   @ingroup WestmereProperties
   @brief   Westmere specific property
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

#include "DataCache.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>

PropertyID DataCache::id() {
    return DATA_CACHE;
}

void DataCache::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool DataCache::condition() const {
    return cycles > 0 && severity() > threshold;
}

double DataCache::confidence() const {
    return 1.0;
}

double DataCache::severity() const {
    return evaluationResult * ( cycles / phaseCycles ) * 100;
}

Context* DataCache::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type DataCache::request_metrics() {
    pdb->request( context, PSC_PAPI_LD_INS );
    pdb->request( context, PSC_PAPI_SR_INS );
    pdb->request( context, PSC_NP_INSTRUCTION_RETIRED );
    pdb->request( context, PSC_PAPI_TOT_CYC );
    pdb->request( phaseContext, PSC_PAPI_TOT_CYC );
    return ALL_INFO_GATHERED;
}

std::string DataCache::name() {
    return "Data Cache";
}

void DataCache::evaluate() {
    phaseCycles = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );
    cycles      = pdb->get( context, PSC_PAPI_TOT_CYC );

    Mem_Inst_Retired_Loads  = pdb->get( context, PSC_PAPI_LD_INS );
    Mem_Inst_Retired_Stores = pdb->get( context, PSC_PAPI_SR_INS );
    Instruction_Retired     = pdb->get( context, PSC_NP_INSTRUCTION_RETIRED );

    evaluationResult = ( double )( Mem_Inst_Retired_Loads + Mem_Inst_Retired_Stores ) / ( double )Instruction_Retired;
}

Property* DataCache::clone() {
    DataCache* prop = new DataCache( context, phaseContext );
    return prop;
}

std::string DataCache::info() {
    std::stringstream stream;

    stream << '\t' << " Mem_Inst_Retired_Loads: " << Mem_Inst_Retired_Loads << '\t' << " Mem_Inst_Retired_Stores: "
           << Mem_Inst_Retired_Stores << '\t' << " Instruction_Retired: " << Instruction_Retired;

    return stream.str();
}

std::string DataCache::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<Mem_Inst_Retired_Loads>" << Mem_Inst_Retired_Loads << "</Mem_Inst_Retired_Loads>" << std::endl;
    stream << "\t\t<Mem_Inst_Retired_Stores>" << Mem_Inst_Retired_Stores << "</Mem_Inst_Retired_Stores>" << std::endl;
    stream << "\t\t<Instruction_Retired>" << Instruction_Retired << "</Instruction_Retired>" << std::endl;

    stream << "\t\t<phaseCycles>" << phaseCycles << "</phaseCycles>" << std::endl;
    stream << "\t\t<cycles>" << cycles << "</cycles>" << std::endl;

    return stream.str();
}
