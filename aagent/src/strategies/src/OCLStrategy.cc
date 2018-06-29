/**
   @file    OCLStrategy.cc
   @ingroup OCLStrategy
   @brief   OpenCL search strategy based on global request
   @author  Robert Mijakovic
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2015, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */
#include "global.h"
#include "OCLStrategy.h"
#include "KernelExecutionTime.h"
#include <iostream>
#include "psc_errmsg.h"
#include <analysisagent.h>
#include "return_summary_data.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include "config.h"


OCLStrategy::~OCLStrategy() {
    clear_found_properties();
    pdb->clean();
}


bool OCLStrategy::reqAndConfigureFirstExperiment( Region* r ) {
// TRUE can start; FALSE not ready
    psc_dbgmsg( 1, "OpenCL Global Strategy is initiated\n" );
    phaseRegion = r;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAll ), "on reqAndConfigureFirstExperiment:  phaseRegion address: %x\n", phaseRegion );

    if( agent->get_leader() ) {
        psc_dbgmsg( 1, "OpenCL Global Strategy Step %d\n", strategy_steps );
    }

    return true;
}


bool OCLStrategy::evaluateAndReqNextExperiment() {
    // TRUE requires next step; FALSE if done
    Prop_List::iterator prop_it;
    double              max_severity = 0;

    //Evaluate candidate properties
    psc_dbgmsg( 2, "%s: analyzing results...\n", name().c_str(), candProperties.size() );

    createCandidateProperties();
    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( candProperties, "SET OF CANDIDATE PROPERTIES", false, false );
    }

    foundPropertiesLastStep.clear();

    for( prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
        ( *prop_it )->evaluate();

        //compute maximum of the severities
        if( ( *prop_it )->severity() > max_severity ) {
            max_severity = ( *prop_it )->severity();
        }

        if( ( *prop_it )->condition() ) {
            foundPropertiesLastStep.push_back( *prop_it );
            foundProperties.push_back( *prop_it );
        }
    }
    psc_dbgmsg( 2, "%s: size of candidate properties %d\n", name().c_str(), candProperties.size() );
    psc_dbgmsg( 2, "%s: size of found list this step %d\n", name().c_str(), foundPropertiesLastStep.size() );
    psc_dbgmsg( 2, "%s: size of total found list %d\n", name().c_str(), foundProperties.size() );
    psc_dbgmsg( 2, "%s: maximum detected severity was %f\n", name().c_str(), max_severity );

    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( foundPropertiesLastStep, "SET OF FOUND PROPERTIES", true, true );
    }

    return false;
}


void OCLStrategy::configureNextExperiment() {
    std::list<Request*>::iterator    req;
    std::list<ApplProcess>::iterator process;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAll ), "on configureNextExperiment:  before the loop: phaseRegion address: %p\n", phaseRegion );
    std::list<ApplProcess> controlled_processes = dp->get_controlled_processes();
    psc_dbgmsg( 5, "Requesting OpenCL Global measurements...\n" );

    stringstream mriMetricGlobalRequest;
    mriMetricGlobalRequest << "request[0] global opencl;" << endl;

    stringstream mriMetricLocalRequest;
    mriMetricLocalRequest << "request[0] local ("
                          << phaseRegion->get_ident().file_id << ", "
                          << dp->regionType2reqSpec( phaseRegion->get_type(), PSC_PAPI_TOT_CYC )
                          << ", " << phaseRegion->get_ident().rfl << ") = "
                          << PSC_EXECUTION_TIME << ";" << endl;

    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "requesting: %s\n", mriMetricGlobalRequest.str().c_str() );
        dp->write_line( &( *process ), mriMetricGlobalRequest.str().c_str() );
        dp->wait_for_ok( *process );

        for( int i = 0; i < appl->getOmpThreads(); i++ ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAll ), "on configureNextExperiment:  in the loop: phaseRegion address: %p\n", phaseRegion );
//RM: Should be sufficient to call it only once per process
            pdb->erase( phaseRegion->get_ident().file_id, phaseRegion->get_ident().rfl, process->rank, i, PSC_PAPI_TOT_CYC );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AgentApplComm ), "requesting: %s\n", mriMetricLocalRequest.str().c_str() );
            dp->write_line( &( *process ), mriMetricLocalRequest.str().c_str() );
            dp->wait_for_ok( *process );
        }
    }
}


void OCLStrategy::createCandidateProperties() {
    //1. Receive measurements
    std::list<ApplProcess>           controlled_processes = dp->get_controlled_processes();
    std::list<ApplProcess>::iterator process;
    SummaryDataType                  summary;
    int                              number      = 4, length;
    Gather_Required_Info_Type        gather_info = ALL_INFO_GATHERED;

    psc_dbgmsg( 5, "Creating OpenCL candidate properties\n" );

    std::list<KernelPerfEntry*>           OpenCLperfList = dp->getOpenCLPerfList();
    std::list<KernelPerfEntry*>::iterator entry;

    dp->printOpenCLPerfList( 0, "\t" );

    for( entry = OpenCLperfList.begin(); entry != OpenCLperfList.end(); entry++ ) {
//    cout << (*entry)->toString();
//    printf("Kernel = %s\n", (*entry)->getName().c_str());
        switch( ( *entry )->getMetric() ) {
        case PSC_OPENCL_KERNEL_EXECUTION_TIME:
            Context* ct, *startCtx;

            KernelExecutionTime* prop;

            ct       = new Context( ( *entry )->getFileID(), ( *entry )->getRfl(), ( *entry )->getRank(), ( *entry )->getThread() );
            startCtx = new Context( phaseRegion, ( *entry )->getRank(), ( *entry )->getThread() );
            prop     = new KernelExecutionTime( ct, startCtx, ( *entry )->getName() );
            candProperties.push_back( prop );
            break;
        default:
            break;
        }
    }

    while( !OpenCLperfList.empty() ) {
        delete OpenCLperfList.front(), OpenCLperfList.pop_front();
    }
    OpenCLperfList.clear();
}


std::list<Property*> OCLStrategy::create_initial_candidate_properties_set( Region* initial_region ) {
    Prop_List n;
    return n; //empty
}

std::list<Property*> OCLStrategy::create_next_candidate_properties_set( std::list<Property*> ev_set ) {
    Prop_List n;
    return n; //empty
}

std::string OCLStrategy::name() {
    return "OCLStrategy";
}
