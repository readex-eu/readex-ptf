/**
   @file    StallCycles.cc
   @ingroup Itanium2Properites
   @brief   Itanium2 specific property
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

#include "StallCycles.h"
#include "global.h"
#include "PropertyID.h"
#include "psc_errmsg.h"
#include "InstrumentationStrategy.h"
#include "selective_debug.h"

#include <iostream>
#include <sstream>

PropertyID StallCyclesProp::id() {
    return STALLCYCLES;
}

/**
 * Returns a sub id used in the output file for the current property depending on the Metric used.
 * @return	Metric used for the current property
 */
std::string StallCyclesProp::subId() {
    std::stringstream sstr;
    sstr << get_metric();
    return sstr.str();
}

void StallCyclesProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool StallCyclesProp::condition() const {
    return ( ( double )stallCycles / ( double )phaseCycles * 100 ) > threshold;
}

double StallCyclesProp::confidence() const {
    return 1.0;
}

double StallCyclesProp::severity() const {
    return ( double )stallCycles / ( double )phaseCycles * 100;
}

Metric StallCyclesProp::get_metric() {
    return m;
}

Context* StallCyclesProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type StallCyclesProp::request_metrics() {
    pdb->request( context, m );
    pdb->request( phaseContext, PSC_PAPI_TOT_CYC );
    if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( SCAOVHD ) ) ) {
        pdb->erase( phaseContext, PSC_MRI_LIBCALLS );
        pdb->request( phaseContext, PSC_MRI_LIBCALLS );
        pdb->erase( phaseContext, PSC_MRI_OVERHEAD );
        pdb->request( phaseContext, PSC_MRI_OVERHEAD );
        pdb->erase( context, PSC_MRI_OVERHEAD );
        pdb->request( context, PSC_MRI_OVERHEAD );
    }
    return ALL_INFO_GATHERED;
}

std::string StallCyclesProp::name() {
    switch( m ) {
    case PSC_BACK_END_BUBBLE_ALL:
        return "IA64 Pipeline Stall Cycles";
    case PSC_BE_FLUSH_BUBBLE_ALL:
        return "Stalls due to pipeline flush";
    case PSC_BE_FLUSH_BUBBLE_BRU:
        return "Stalls due to branch misprediction flush";
    case PSC_BE_FLUSH_BUBBLE_XPN:
        return "Stalls due to exception flush";
    case PSC_BE_L1D_FPU_BUBBLE_ALL:
        return "Stalls due to floating point exceptions or L1D TLB misses";
    case PSC_BE_L1D_FPU_BUBBLE_L1D:
        return "Stalls due to L1D TLB misses ...";
    case PSC_BE_EXE_BUBBLE_ALL:
        return "Stalls due to waiting for data delivery to register";
    case PSC_BE_EXE_BUBBLE_GRALL:
        return "Stalls due to waiting for integer register";
    case PSC_BE_EXE_BUBBLE_GRGR:
        return "Stalls due to waiting for integer results";
    case PSC_BE_EXE_BUBBLE_FRALL:
        return "Stalls due to waiting for FP register";
    case PSC_BE_RSE_BUBBLE_ALL:
        return "Stalls due to register stack engine";
    case PSC_BACK_END_BUBBLE_FE:
        return "Stalls due to instruction fetch";
    case PSC_BE_L1D_FPU_BUBBLE_FPU:
        return "Stalls due to Flush to zero or SIR stalls";
    case PSC_BE_L1D_FPU_BUBBLE_L1D_FULLSTBUF:
        return "Stalls due to full store buffer";
    case PSC_BE_L1D_FPU_BUBBLE_L1D_DCURECIR:
        return "Stalls due to data cache unit recirculating";
    case PSC_BE_L1D_FPU_BUBBLE_L1D_HPW:
        return "Stalls due to hardware page walker";
    case PSC_BE_L1D_FPU_BUBBLE_L1D_TLB:
        return "Stalls due to L2DTLB to L1DTLB transfer";
    case PSC_BE_L1D_FPU_BUBBLE_L1D_L2BPRESS:
        return "Stalls due to L2 Back Pressure";
    case PSC_FP_FLUSH_TO_ZERO:
        return "Stalls due to flush to zero";
    case PSC_FP_FALSE_SIRSTALL:
        return "Stalls due to safe instruction recognition without a trap";
    case PSC_FP_TRUE_SIRSTALL:
        return "Stalls due to safe instruction recognition leading to a trap";
    default:
        return "Unknown metric in stall cycle property";
    }
}

void StallCyclesProp::evaluate() {
    stallCycles = pdb->get( context, m );
    phaseCycles = pdb->get( phaseContext, PSC_PAPI_TOT_CYC );
    if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( SCAOVHD ) ) ) {
        exclOvhdCycles      = pdb->get( context, PSC_MRI_EXCL_OVERHEAD );
        phaseInclOvhdCycles = pdb->get( phaseContext, PSC_MRI_OVERHEAD );
        if( context->getRank() == 0 && context->getThread() == 0 && exclOvhdCycles > 0 ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( SCAOVHD ),
                        "SCA Overhead (%s,%d): %f percent of phase cycles (%f seconds). Incl Ovhd Phase: %f seconds. Call overhead %f (%d)\n",
                        context->getFileName().c_str(), context->getStartPosition(),
                        ( double )exclOvhdCycles * 100 / ( double )phaseCycles, ( double )phaseCycles / NANOSEC_PER_SEC_DOUBLE,
                        ( double )phaseInclOvhdCycles / NANOSEC_PER_SEC_DOUBLE,
                        ( double )pdb->get( phaseContext, PSC_MRI_LIBCALLS ) / 2.0 * ( double )InstrumentationOverhead / NANOSEC_PER_SEC_DOUBLE,
                        pdb->get( phaseContext, PSC_MRI_LIBCALLS ) );
        }
    }
}

Property* StallCyclesProp::clone() {
    StallCyclesProp* prop = new StallCyclesProp( context, phaseContext, m );
    return prop;
}
