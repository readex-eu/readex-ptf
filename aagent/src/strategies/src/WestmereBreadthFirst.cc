/**
   @file    WestmereBreadthFirst.cc
   @ingroup WestmereBreadthFirst
   @brief   Westmere breadth-first search strategy
   @author  Michael Gerndt
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2013, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */
#include "global.h"
#include "psc_errmsg.h"
#include "analysisagent.h"
#include "WestmereBreadthFirst.h"
#include "AddressAliasing.h"
#include "DataCache.h"
#include "DTLB_Misses.h"
#include "ExecutionStall.h"
#include "strategy.h"
#include "InstructionStarvation.h"
#include "LongLatencyInstructionException.h"
#include "Misprediction.h"
#include "WM_L2MissRate.h"
#include "ConfigurableProperty.h"

//#include "WM_L3MissRate.h"


bool WestmereBreadthFirst::reqAndConfigureFirstExperiment( Region* r ) {
    std::list<ApplProcess>           controlled_processes = dp->get_controlled_processes();
    std::list<Region*>               sub_regions;
    std::list<Region*>::iterator     subr_it;
    std::list<ApplProcess>::iterator process;
    Prop_List::iterator              prop_it;

    phaseRegion = r;

    if( agent->get_leader() ) {
        psc_dbgmsg( 1, "WESTMERE: Strategy Step %d\n", strategy_steps );
    }

    candProperties.clear();
    candProperties = create_initial_candidate_properties_set( r );
    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( candProperties, "SET OF CANDIDATE PROPERTIES", false, false );
    }

    strategy_steps++;

    for( prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
        ( *prop_it )->request_metrics();
    }

    return true;
}

void WestmereBreadthFirst::configureNextExperiment() {
    Prop_List::iterator prop_it;

    dp->transfer_requests_to_processes_no_begin_end_requests();
}

bool WestmereBreadthFirst::evaluateAndReqNextExperiment() {
    // TRUE requires next step; FALSE if done
    Prop_List::iterator prop_it;

    //Evaluate candidate properties

    foundPropertiesLastStep.clear();
    for( prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
        ( *prop_it )->evaluate();
        if( ( *prop_it )->condition() ) {
            foundPropertiesLastStep.push_back( *prop_it );
            foundProperties.push_back( *prop_it );
        }
    }

    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( foundPropertiesLastStep, "SET OF FOUND PROPERTIES", true, true );
    }

    //Determine new candidate properties

    candProperties.clear();
    candProperties = create_next_candidate_properties_set( foundPropertiesLastStep );

    for( prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
        ( *prop_it )->request_metrics();
    }

    if( candProperties.size() == 0 ) {
        return false;
    }
    else {
        psc_dbgmsg( 1, "Strategy Step %d\n", strategy_steps );
        if( psc_get_debug_level() >= 2 ) {
            agent->print_property_set( candProperties, "SET OF CANDIDATE PROPERTIES", false, false );
        }
        strategy_steps++;
        return true;
    }
}

std::list<Property*> WestmereBreadthFirst::create_initial_candidate_properties_set( Region* r ) {
    std::list<ApplProcess>           controlled_processes = dp->get_controlled_processes();
    std::list<Region*>               regions;
    std::list<Property*>             candidates;
    Property*                        prop;
    Property*                        prop1;
    std::list<ApplProcess>::iterator process;
    std::list<Region*>::iterator     reg;

    phaseRegion = r;
    propsRefineRegionNesting.clear();
    regions = appl->get_regions();

    for( process = controlled_processes.begin();
         process != controlled_processes.end(); process++ ) {
        //psc_dbgmsg(1, "WESTMERE: process\n");
        for( reg = regions.begin(); reg != regions.end(); reg++ ) {
            int  threads = 1;
            char routine_name[ 200 ];
            strcpy( routine_name, ( ( *reg )->get_name() ).c_str() );
            //psc_dbgmsg(1, "WESTMERE: region %s \n",((*reg)->get_name()).c_str());
            if( ( *reg )->get_type() == CALL_REGION
                && strncmp( routine_name, "MPI", 3 ) != 0 ) {
                continue;
            }

            if( ( *reg )->get_rra() == RUNS_AS_THREADED ) {
                threads = appl->getOmpThreads();
            }
            //for (int i = 0; i < appl->getOmpThreads(); i++) {
            int      i = 0;
            Context* ct, * pCt;

//        ct = new Context((*reg), process->rank, i);
//        pCt = new Context(phaseRegion, process->rank, 0);
//        prop = new AddressAliasing(ct, pCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::AddressAliasing);
//        candidates.push_back(prop);
//        //((AddressAliasing*)prop)->print();
//

//        ct = new Context((*reg), process->rank, i);
//        pCt = new Context(phaseRegion, process->rank, 0);
//        prop = new WM_L2MissRate(ct, pCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::WM_L2MissRate);
//        candidates.push_back(prop);
//
//        ct = new Context((*reg), process->rank, i);
//        pCt = new Context(phaseRegion, process->rank, 0);
//        prop = new WM_L2MissRate(ct, pCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::WM_L2MissRate);
//        candidates.push_back(prop);

            ct = new Context( ( *reg ), process->rank, i );
            const PropertyConfiguration prop_conf( "Time_QE", QualityExpression( "local::RegionExecution:|time / 1000000" ) );
            prop = new ConfigurableProperty( ct, prop_conf, 0.0 );
            candidates.push_back( prop );

            ct = new Context( ( *reg ), process->rank, i );
            const PropertyConfiguration prop_conf1( "L2_Cache_Access_QE", QualityExpression( "papi::PAPI_L2_DCA:|size" ) );
            prop = new ConfigurableProperty( ct, prop_conf1, 0.0 );
            candidates.push_back( prop );

            ct = new Context( ( *reg ), process->rank, i );
            const PropertyConfiguration prop_conf2( "L2_Hit_Rate_QE", QualityExpression( "papi::PAPI_L2_DCH:|size / papi::PAPI_L2_DCA:|size" ) );
            prop = new ConfigurableProperty( ct, prop_conf2, 0.0 );
            candidates.push_back( prop );

//        ct = new Context((*reg), process->rank, i);
//        pCt = new Context(phaseRegion, process->rank, 0);
//        prop = new WM_L3MissRate(ct, pCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::WM_L3MissRate);
//        candidates.push_back(prop);
//
            //std::cout  << "P" << ct->getRank() << "; \t" << ct->getThread()<< "; \t" ;

//        ct = new Context((*reg), process->rank, i);
//        pCt = new Context(phaseRegion, process->rank, 0);
//        prop = new DTLB_Misses(ct, pCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::DTLB_Misses);
//        candidates.push_back(prop);

//        ct = new Context((*reg), process->rank, i);
//        pCt = new Context(phaseRegion, process->rank, 0);
//        prop = new ExecutionStall(ct, pCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::ExecutionStall);
//        candidates.push_back(prop);
//
//        ct = new Context((*reg), process->rank, i);
//        pCt = new Context(phaseRegion, process->rank, 0);
//        prop = new InstructionStarvation(ct, pCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::InstructionStarvation);
//        candidates.push_back(prop);
//
//        ct = new Context((*reg), process->rank, i);
//        pCt = new Context(phaseRegion, process->rank, 0);
//        prop = new LongLatencyInstructionException(ct, pCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::LongLatencyInstructionException);
//        candidates.push_back(prop);
//
//        ct = new Context((*reg), process->rank, i);
//        pCt = new Context(phaseRegion, process->rank, 0);
//        prop = new Misprediction(ct, pCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::Misprediction);
//        candidates.push_back(prop);
//      }

            switch( ( *reg )->get_type() ) {
            case PARALLEL_REGION:
            case DO_REGION:
            case SECTIONS_REGION:
            case WORKSHARE_REGION:
                break;
            default:
                break;
            }
        }
    }

    psc_dbgmsg( 1, "WESTMERE: returning Candidates \n" );
    return candidates;
}

std::list<Property*> WestmereBreadthFirst::create_next_candidate_properties_set( std::list<Property*> foundProperties ) {
    std::list<Property*>           candidates;
    std::list<Property*>::iterator p;
    std::list<ApplProcess>         controlled_processes = dp->get_controlled_processes();

    //if (psc_get_debug_level() >= 3)
    //  std::cout << "Strategy Step Nr: " << strategy_steps << std::endl;
    //psc_dbgmsg(1, "Strategy Step %d\n", strategy_steps);

    if( strategy_steps >= max_strategy_steps ) {
        return candidates; //empty
    }
    // First refine in more precise properties
    for( p = foundProperties.begin(); p != foundProperties.end(); p++ ) {
        if( ( *p )->severity() > 5.0 ) {
        }
    } //loop over all properties
    return candidates;
}
;

std::string WestmereBreadthFirst::name() {
    return "WestmereBreadthFirstStrategy";
}
