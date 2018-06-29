/**
   @file    SerializationInParallelProgram.cc
   @ingroup OpenMPProperties
   @brief   OpenMP Serialization in Parallel Program Overhead property
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
 */

#include "SerializationInParallelProgram.h"
#include "global.h"
#include "PropertyID.h"
#include <iostream>
#include <vector>
#include "psc_errmsg.h"
#include "application.h"
using namespace std;

PropertyID SerializationInParallelProgramProp::id() {
    return SERIALIZATIONINPARALLELPROGRAM;
}

bool SerializationInParallelProgramProp::condition() const {
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        std::cout << waitCycles[ i ] << "\t";
    }

    return true;
}

double SerializationInParallelProgramProp::confidence() const {
    return 1.0;
}

double SerializationInParallelProgramProp::severity() const {
    return 0.0;
}

Context* SerializationInParallelProgramProp::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type SerializationInParallelProgramProp::request_metrics() {
    pdb->request( phaseContext, PSC_EXECUTION_TIME );
    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        pdb->request( ct, PSC_IMPLICIT_BARRIER_TIME );
    }
    return ALL_INFO_GATHERED;
}

std::string SerializationInParallelProgramProp::name() {
    return "Serialization";
}

void SerializationInParallelProgramProp::evaluate() {
    /*
       std::list <ApplProcess> controlled_processes_s=dp->get_controlled_processes();
       std::list <Region*> regions_s;
       std::list<Region*>::iterator reg_s;
       std::list <ApplProcess>::iterator process_s;
       regions_s=appl->get_regions();

       waitCycles.resize(appl->getOmpThreads(),0);
       singleRegionCycles = 0.0;
       for (reg_s=regions_s.begin(); reg_s!=regions_s.end(); reg_s++) {
       int threads=1;
       //std::cout<<"sssssssssssss" <<(*reg_s)->get_type() << std::endl;

       if ((*reg_s)->get_rra()==RUNS_AS_THREADED){
       for (process_s=controlled_processes_s.begin(); process_s!=controlled_processes_s.end(); process_s++) {
       //Context *pCt=new Context(phaseRegion,process_s->rank,0);
       switch ((*reg_s)->get_type()) {
       case SINGLE_REGION:
       std::cout<<" sssssssssssss " <<(*reg_s)->get_type() << " Region is " <<(*reg_s)->get_name()<< std::endl;

       for (int i=0; i<appl->getOmpThreads(); i++){
       Context *ct= new Context(context->getRegion(), context->getRank(), i);
       pdb->request(ct, PSC_SINGLE_REGION_CYCLE);
       }
       //Gather_Required_Info_Type info_gather = ALL_INFO_GATHERED;
       //dp->transfer_requests_to_processes();


       for (int i=0;i<appl->getOmpThreads();i++){
       Context *ct= new Context(context->getRegion(), context->getRank(), i);
       waitCycles[i]=	pdb->get(ct, PSC_SINGLE_REGION_CYCLE);
       cout << " WaitCycles   " << waitCycles[i] ;
       delete ct;
       }
       singleRegionCycles=waitCycles[0];
       for (int i=1; i<appl->getOmpThreads(); i++){
       if (waitCycles[i] > singleRegionCycles)
       singleRegionCycles = waitCycles[i];
       }
       cout << endl;
       cout << " SingleRegionCycles = " << singleRegionCycles << endl;
       break;

       default:
       break;
       }
       }
       }

       }
     */
    waitCycles.resize( appl->getOmpThreads(), 0 );

    for( int i = 0; i < appl->getOmpThreads(); i++ ) {
        Context* ct = new Context( context->getRegion(), context->getRank(), i );
        waitCycles[ i ] = pdb->get( ct, PSC_IMPLICIT_BARRIER_TIME );
        //std::cout << (double) waitCycles[i] << "  "; //sss implicit barrier time is only for main thread??? all others are zero
        delete ct;
    }
    phaseCycles = pdb->get( phaseContext, PSC_EXECUTION_TIME );
}

Property* SerializationInParallelProgramProp::clone() {
    SerializationInParallelProgramProp* prop = new SerializationInParallelProgramProp( context, phaseContext );
    return prop;
}

INT64 SerializationInParallelProgramProp::exec_time() {
    return phaseCycles;
}
