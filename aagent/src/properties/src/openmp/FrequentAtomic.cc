/**
   @file    FrequentAtomic.cc
   @ingroup OpenMPProperties
   @brief   OpenMP Frequent Atomic Property
   @author  Shajulin Benedict
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim

   @todo This is not stable, system crashes upon certain situations
 */
#include "FrequentAtomic.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"

PropertyID FrequentAtomicProp::id() {
    return FREQUENT_ATOMIC_OMP;
}

void FrequentAtomicProp::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl; //sss<< "                  " <<
    //  context->getRegion()->str_print(appl->get_file_name_maping()) <<
    //  std::endl;
}

bool FrequentAtomicProp::condition() const {
    /*
       std::cout << "Atomic Region: " << phaseCycles << "\n";
       for (int i=0; i<appl->getOmpThreads(); i++){
       std::cout << waitCycles[i] << "\t" << std::endl;
       }
       std::cout << std::endl;
     */
    atomicRegionCycles = waitCycles[ 0 ];
    //how to evaluate this situation?
    for( int i = 1; i < appl->getOmpThreads(); i++ ) {
        if( waitCycles[ i ] > atomicRegionCycles && waitCycles[ i ] != 0 ) {
            atomicRegionCycles = waitCycles[ i ];
        }
    }
    return atomicRegionCycles / ( double )phaseCycles * 100 > threshold;
}

double FrequentAtomicProp::confidence() const {
    return 1.0;
}

double FrequentAtomicProp::severity() const {
    //notice that inaccurate measurement of time may cause certain portion of code having a longer than that phase cycle
    //in that case, around down to 100 will suffice

    if( ( atomicRegionCycles / ( double )phaseCycles * 100 ) > 100.0 ) {
        return 100.0;
    }
    else {
        return atomicRegionCycles / ( double )phaseCycles * 100;
    }
}

Context* FrequentAtomicProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type FrequentAtomicProp::request_metrics() {
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        pdb->request( ct, PSC_OMP_ATOMIC_CYCLE );
    }
    pdb->request( phaseContext, PSC_EXECUTION_TIME );
    return ALL_INFO_GATHERED;
}

std::string FrequentAtomicProp::name() {
    return "Frequent Atomic Overhead";
}

void FrequentAtomicProp::evaluate() {
    waitCycles.resize( appl->getOmpThreads(), 0 );

    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        //does this include the synchronization time at the barrier after critical region? If so, the
        //variation of this waitCycle should reflect the timing difference of each thread

        waitCycles[ i ] = pdb->get( ct, PSC_OMP_ATOMIC_CYCLE );
        delete ct;
    }
    phaseCycles = pdb->get( phaseContext, PSC_EXECUTION_TIME );
}

INT64 FrequentAtomicProp::exec_time() {
    return 0;
}
Property* FrequentAtomicProp::clone() {
    FrequentAtomicProp* prop = new FrequentAtomicProp( context, phaseContext );
    return prop;
}
