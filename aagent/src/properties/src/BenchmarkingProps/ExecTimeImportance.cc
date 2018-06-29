/**
   @file    ExecTimeImportance.cc
   @ingroup BenchmarkingProperties
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

#include "ExecTimeImportance.h"
#include "global.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"
#include <sstream>

ExecTimeImportanceProp::ExecTimeImportanceProp( Context* ct, Context* phaseCt, double threshold ) :
    Property( ct ), threshold( threshold ) {
    phaseCycles  = 0;
    context      = ct;
    phaseContext = phaseCt;
    energyConsumption = 0.0;
    totalInstr   = 0;
}

ExecTimeImportanceProp::~ExecTimeImportanceProp() {
}

PropertyID ExecTimeImportanceProp::id() {
    return EXECTIMEIMPORTANCE;
}

void ExecTimeImportanceProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread() << std::endl;
}

bool ExecTimeImportanceProp::condition() const {
    if( phaseCycles > 0 ) {
        return ( ( ( double )cycles / ( double )phaseCycles * 100 ) > threshold && energyConsumption >= 0 );
    }
    else {
        return false;
    }
}

double ExecTimeImportanceProp::confidence() const {
    return 1.0;
}

double ExecTimeImportanceProp::severity() const {
    if( phaseCycles > 0 ) {
        return ( double )cycles / ( double )phaseCycles * 100;
    }
    else {
        return 0.0;
    }
}

Context* ExecTimeImportanceProp::get_phaseContext() {
    return phaseContext;
}

void ExecTimeImportanceProp::RequestNestedCalls( Region* reg ) {
    std::list<Region*>           subRegions = reg->get_subregions();
    std::list<Region*>::iterator subr_it;

    //psc_dbgmsg(6, "ExecTimeImportanceProp: Request subregions of: %s\n", reg->str_print().c_str());
    //if (psc_get_debug_level()>=3) std::cout << "Refined to subregions: " << (*p)->get_region()->str_print(appl->get_file_name_maping()) << std::endl;
    for( subr_it = subRegions.begin(); subr_it != subRegions.end(); subr_it++ ) {
        if( ( *subr_it )->get_type() == CALL_REGION ) {
            Context* ct = new Context( ( *subr_it ), context->getRank(), 0 );
            //psc_dbgmsg(6, "   ExecTimeImportanceProp: Request measurement for %s\n", (*subr_it)->str_print().c_str());
            pdb->request( ct, PSC_EXECUTION_TIME );
        }
        else {
            if( ( *subr_it )->get_type() != DATA_STRUCTURE ) {
                RequestNestedCalls( ( *subr_it ) );
            }
        }
    }
}

Gather_Required_Info_Type ExecTimeImportanceProp::request_metrics() {
//    Context* ct = new Context( context->getRegion(), context->getRank(), 0 );
    Context* ct = context->copy();
    pdb->request( ct, PSC_EXECUTION_TIME );
    pdb->request( phaseContext, PSC_EXECUTION_TIME );

//    pdb->request( ct, PSC_NODE_ENERGY );

    //forall call regions in this region request execution time
    /*
     * Evaluation of the nested regions is disabled since we don't have
     * information on regions hierarchy in the Score-P data
     */
    //RequestNestedCalls(context->getRegion());

    return ALL_INFO_GATHERED;
}



std::string ExecTimeImportanceProp::name() {
    return "ExecTimeImportance";
}


void ExecTimeImportanceProp::EvaluateNestedCalls( Region* reg ) {
    std::list<Region*>           subRegions = reg->get_subregions();
    std::list<Region*>::iterator subr_it;

    //psc_dbgmsg(6, "ExecTimeImportance: Evaluate to subregion: %s\n", context->getRegion()->str_print().c_str());
    //if (psc_get_debug_level()>=3) std::cout << "Refined to subregions: " << (*p)->get_region()->str_print(appl->get_file_name_maping()) << std::endl;
    for( subr_it = subRegions.begin(); subr_it != subRegions.end(); subr_it++ ) {
        if( ( *subr_it )->get_type() == CALL_REGION ) {
            Context* ct     = new Context( ( *subr_it ), context->getRank(), 0 );
            INT64    cycles = pdb->get( ct, PSC_EXECUTION_TIME );
            if( cycles > 0 ) {
                //psc_dbgmsg(6, "   ExecTimeImportanceProp: measurement for %s  %ld\n", (*subr_it)->str_print().c_str(), cycles);
                nestedCycles += cycles;
            }
            delete ct;
        }
        else {
            if( ( *subr_it )->get_type() != DATA_STRUCTURE ) {
                EvaluateNestedCalls( ( *subr_it ) );
            }
        }
    }
}


void ExecTimeImportanceProp::evaluate() {
    //    Context* ct = new Context( context->getRegion(), context->getRank(), 0 );
        Context* ct = context->copy();
        phaseCycles  = pdb->get( phaseContext, PSC_EXECUTION_TIME );
        nestedCycles = 0;

        /*
         * Evaluation of the nested regions is disabled since we don't have
         * information on regions hierarchy in the Score-P data
         */
        //EvaluateNestedCalls(context->getRegion());

        cycles = pdb->get( ct, PSC_EXECUTION_TIME );
        if( cycles > ( INT64 )0 ) {
            cycles = cycles - nestedCycles;
        }
        else {
            cycles = 0;
        }

        if( withRtsSupport() ) {
    //         Context* ct = new Context( context->getRts(), context->getRank(), 0 );
    //         INT64    raw = pdb->get( ct, PSC_NODE_ENERGY );
             energyConsumption = pdb->get( ct, PSC_NODE_ENERGY );
             cpuEnergy=pdb->get(ct, PSC_CPU0_ENERGY) + pdb->get(ct, PSC_CPU1_ENERGY);
        }
//             totalInstr = pdb->get( ct, PSC_PAPI_TOT_INS );
        //psc_dbgmsg(6, "ExecTimeImportance: Cycles=%ld, NestedCycles=%ld\n", cycles, nestedCycles);
        delete ct;
}


std::string ExecTimeImportanceProp::toXMLExtra() {
    std::stringstream stream;
    stream << "\t\t<cycles>" << cycles << "</cycles>" << std::endl;
    stream << "\t\t<nestedCycles>" << nestedCycles << "</nestedCycles>" << std::endl;
    stream << "\t\t<NodeEnergy>" << energyConsumption << "</NodeEnergy>" << std::endl;
    stream << "\t\t<CPUEnergy>" << cpuEnergy << "</CPUEnergy>" << std::endl;
//    stream << "\t\t<TotalInstr>" << totalInstr << "</TotalInstr>" << std::endl;
    return stream.str();
}

Property* ExecTimeImportanceProp::clone() {
    ExecTimeImportanceProp* prop = new ExecTimeImportanceProp( context, phaseContext, threshold );
    return prop;
}

std::string ExecTimeImportanceProp::info() {
    std::stringstream stream;

    stream << " Exclusive cycles: " << cycles;
    stream << ", nested cycles: " << nestedCycles;
    stream << ", nodeEnergy: " << energyConsumption;
    stream << ", CPUEnergy: " << cpuEnergy ;
//    stream << ", totalInstr: " << totalInstr;

    return stream.str();
}


//BOOST_CLASS_EXPORT_IMPLEMENT(ExecTimeImportanceProp);
