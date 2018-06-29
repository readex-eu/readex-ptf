/**
   @file    WestmereDepthFirst.cc
   @ingroup WestmereDepthFirst
   @brief   Westmere stall cycles depth-first search strategy
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
#include "WestmereDepthFirst.h"
#include "application.h"
#include "Metric.h"
#include "strategy.h"
#include "PropertyID.h"
#include "psc_errmsg.h"
#include <iostream>
#include <analysisagent.h>

#include "AddressAliasing.h"
#include "DataCache.h"
#include "DTLB_Misses.h"
#include "ExecutionStall.h"
#include "InstructionStarvation.h"
#include "LongLatencyInstructionException.h"
#include "Misprediction.h"
#include "WM_L1MissRate.h"
#include "WM_L2MissRate.h"
#include "WM_TLBMissRate.h"

WestmereDepthFirst::WestmereDepthFirst( bool pedantic ) :
    Strategy( pedantic ) {
    std::list<ApplProcess>           controlled_processes = dp->get_controlled_processes();
    int                              maxRank              = 0;
    std::list<ApplProcess>::iterator process;
    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        if( process->rank > maxRank ) {
            maxRank = process->rank;
        }
    }
    checkedSubs.resize( maxRank + 1 );
}

bool WestmereDepthFirst::reqAndConfigureFirstExperiment( Region* r ) {
    psc_dbgmsg( 4, "WestmereDepthFirst: In reg and configure first\n" );
    ; // TRUE can start; FALSE not ready
    std::list<ApplProcess>           controlled_processes = dp->get_controlled_processes();
    std::list<Region*>               sub_regions;
    std::list<Region*>::iterator     subr_it;
    Property*                        prop;
    std::list<ApplProcess>::iterator process;
    Prop_List::iterator              prop_it;

    pdb->clean();

    //vizualization
    //std::cout << "\n The initial region's tree: \n\n";
    //initial_region->print_subregions("__" , application->get_file_name_maping());

    phaseRegion = r;
    sub_regions = r->get_subregions();
    propsRefineRegionNesting.clear();

    checkedSubs.clear();

    if( agent->get_leader() ) {
        psc_dbgmsg( 1, "WestmereDepthFirst: Strategy Step %d\n", strategy_steps );
    }

    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        Context* ct, * pCt;

        ct   = new Context( phaseRegion, process->rank, 0 );
        pCt  = new Context( phaseRegion, process->rank, 0 );
        prop = new AddressAliasing( ct, pCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::AddressAliasing );
        candProperties.push_back( prop );

        ct   = new Context( phaseRegion, process->rank, 0 );
        pCt  = new Context( phaseRegion, process->rank, 0 );
        prop = new DataCache( ct, pCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::DataCache );
        candProperties.push_back( prop );

        ct   = new Context( phaseRegion, process->rank, 0 );
        pCt  = new Context( phaseRegion, process->rank, 0 );
        prop = new DTLB_Misses( ct, pCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::DTLB_Misses );
        candProperties.push_back( prop );

        ct   = new Context( phaseRegion, process->rank, 0 );
        pCt  = new Context( phaseRegion, process->rank, 0 );
        prop = new ExecutionStall( ct, pCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::ExecutionStall );
        candProperties.push_back( prop );

        ct   = new Context( phaseRegion, process->rank, 0 );
        pCt  = new Context( phaseRegion, process->rank, 0 );
        prop = new InstructionStarvation( ct, pCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::InstructionStarvation );
        candProperties.push_back( prop );

        ct   = new Context( phaseRegion, process->rank, 0 );
        pCt  = new Context( phaseRegion, process->rank, 0 );
        prop = new LongLatencyInstructionException( ct, pCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::LongLatencyInstructionException );
        candProperties.push_back( prop );

        ct   = new Context( phaseRegion, process->rank, 0 );
        pCt  = new Context( phaseRegion, process->rank, 0 );
        prop = new Misprediction( ct, pCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::Misprediction );
        candProperties.push_back( prop );

        ct   = new Context( phaseRegion, process->rank, 0 );
        pCt  = new Context( phaseRegion, process->rank, 0 );
        prop = new WM_TLBMissRate( ct, pCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::WM_TLBMissRate );
        candProperties.push_back( prop );

        ct   = new Context( phaseRegion, process->rank, 0 );
        pCt  = new Context( phaseRegion, process->rank, 0 );
        prop = new WM_L1MissRate( ct, pCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::WM_L1MissRate );
        candProperties.push_back( prop );

        ct   = new Context( phaseRegion, process->rank, 0 );
        pCt  = new Context( phaseRegion, process->rank, 0 );
        prop = new WM_L2MissRate( ct, pCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::WM_L2MissRate );
        candProperties.push_back( prop );
    }
    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( candProperties, "SET OF CANDIDATE PROPERTIES", false, false );
    }

    strategy_steps++;

    for( prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
        ( *prop_it )->request_metrics();
    }

    return true;
}

void WestmereDepthFirst::configureNextExperiment() {
    Prop_List::iterator prop_it;

    dp->transfer_requests_to_processes_no_begin_end_requests();
}

bool WestmereDepthFirst::evaluateAndReqNextExperiment() {
    // TRUE requires next step; FALSE if done
    Prop_List::iterator              prop_it;
    std::list<ApplProcess>::iterator process;
    std::list<ApplProcess>           controlled_processes = dp->get_controlled_processes();
    double                           max_severity         = 0;

    //Check whether all information was gathered in the experiment
    //Otherwise start another experiment

    if( dp->getResults() == ALL_INFO_GATHERED ) {
        //Information was measured. Now the properties can be evaluated.
    }
    else {
        //Information is missing
        return true;
    }

    //Evaluate candidate properties
    psc_dbgmsg( 2, "%s: analysing results...\n", name().c_str(), candProperties.size() );
    foundPropertiesLastStep.clear();
    for( prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
        ( *prop_it )->evaluate();

        if( ( *prop_it )->condition() ) {
            //compute maximum of the severities
            if( ( *prop_it )->severity() > max_severity ) {
                max_severity = ( *prop_it )->severity();
            }
            foundPropertiesLastStep.push_back( *prop_it );
            foundProperties.push_back( *prop_it );
        }
    }

    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( foundPropertiesLastStep, "SET OF FOUND PROPERTIES", true, true );
    }

    //Determine new candidate properties

    candProperties.clear();

    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        for( int i = 0; i < appl->getOmpThreads(); i++ ) {
            pdb->erase( phaseRegion->get_ident().file_id, phaseRegion->get_ident().rfl, process->rank, i, PSC_PAPI_TOT_CYC );
        }
    }

    candProperties = create_next_candidate_properties_set( foundPropertiesLastStep );

    psc_dbgmsg( 2, "%s: size of candidate properties %d\n", name().c_str(), candProperties.size() );
    psc_dbgmsg( 2, "%s: size of found list this step %d\n", name().c_str(), foundPropertiesLastStep.size() );
    psc_dbgmsg( 2, "%s: size of total found list %d\n", name().c_str(), foundProperties.size() );
    psc_dbgmsg( 2, "%s: maximum detected severity was %f\n", name().c_str(), max_severity );

    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( candProperties, "SET OF CANDIDATE PROPERTIES", false, false );
    }

    psc_dbgmsg( 4, "Requesting required measurements\n" );
    for( prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
        ( *prop_it )->request_metrics();
    }

    if( candProperties.size() == 0 ) {
        return false;
    }
    else {
        return true;
    }
}

std::list<Property*> WestmereDepthFirst::create_next_candidate_properties_set( std::list<Property*> foundProperties ) {
    std::list<Property*>           candProperties;
    std::list<Property*>::iterator p, p1;
    std::list<ApplProcess>         controlled_processes = dp->get_controlled_processes();

    if( agent->get_leader() ) {
        psc_dbgmsg( 1, "Strategy Step %d\n", strategy_steps );
    }
    if( strategy_steps >= max_strategy_steps ) {
        return candProperties; //empty
    }
    strategy_steps++;

    // Save found properties for further region refinement
    if( !foundProperties.empty() ) {
        for( p = foundProperties.begin(); p != foundProperties.end(); p++ ) {
            propsRefineRegionNesting.push_back( ( *p ) );
        }
    }

    //refine in nested regions
    p = propsRefineRegionNesting.begin();
    while( p != propsRefineRegionNesting.end() ) {
        if( ( *p )->get_region()->get_type() == CALL_REGION ) {
            if( checkedSubs.size() < ( *p )->get_context()->getRank() + 1 ) {
                checkedSubs.resize( ( *p )->get_context()->getRank() + 1 );
            }
            Region* reg = appl->get_subroutine( ( *p )->get_region()->get_name().c_str() );
            if( reg != NULL && ( checkedSubs[ ( *p )->get_context()->getRank() ].find( reg ) ==
                                 checkedSubs[ ( *p )->get_context()->getRank() ].end() || !checkedSubs[ ( *p )->get_context()->getRank() ][ reg ] ) ) {
                Context*  ct;
                Context*  pCt;
                Property* prop;

                // Duplicate property for called subroutine
                // Insert in set of candidates
                ct   = new Context( reg, ( *p )->get_context()->getRank(), ( *p )->get_context()->getThread() );
                prop = ( *p )->clone();
                prop->set_context( ct );
                candProperties.push_back( prop );

                // Search for all other properties for that subroutine
                p1 = p;
                p1++;

                while( p1 != propsRefineRegionNesting.end() ) {
                    if( appl->get_subroutine( ( *p1 )->get_region()->get_name().c_str() ) == reg ) {
                        ct   = new Context( reg, ( *p1 )->get_context()->getRank(), ( *p1 )->get_context()->getThread() );
                        prop = ( *p1 )->clone();
                        prop->set_context( ct );
                        candProperties.push_back( prop );
                        p1 = propsRefineRegionNesting.erase( p1 );
                    }
                    else {
                        p1++;
                    }
                }
                // Mark subroutine as searched.
                checkedSubs[ ( *p )->get_context()->getRank() ][ reg ] = true;
                psc_dbgmsg( 3, "Refined to subroutine: %s\n", ( *p )->get_region()->get_name().c_str() );
                if( psc_get_debug_level() >= 5 ) {
                    std::cout << "Refined to subroutine: " << ( *p )->get_region()->get_name() << std::endl;
                }
            } // end handle not yet processed subroutine
        }     // end handle call to subroutine
        else { // Handle nested regions
            std::list<Region*>           subRegions = ( *p )->get_region()->get_subregions();
            std::list<Region*>::iterator subr_it;

            //std::cout << "\n Subregions of " << (*p)->get_region()->str_print(appl->get_file_name_maping())<<"\n\n";
            //(*p)->get_region()->print_subregions("__" , appl->get_file_name_maping());

            // if there are subregions that are no data structures
            psc_dbgmsg( 3, "Refined to subregions of: %s\n", ( *p )->get_region()->str_print().c_str() );
            for( subr_it = subRegions.begin(); subr_it != subRegions.end(); subr_it++ ) {
                Context*  ct;
                Context*  pCt;
                Property* prop;

                switch( ( *subr_it )->get_type() ) {
                case DATA_STRUCTURE:
                    break;
//        case PARALLEL_REGION:
//        case DO_REGION:
//        case SECTIONS_REGION:
//        case WORKSHARE_REGION:
//          if( appl->getOmpThreads() > 1) {
//
//            for (int i = 0; i < appl->getOmpThreads(); i++) {
//              ct = new Context(*subr_it, (*p)->get_context()->getRank(), i);
//              prop = new StallCyclesProp(ct, new Context(phaseRegion, ct->getRank(), 0), BACK_END_BUBBLE_ALL);
//              candidates.push_back(prop);
//            }
//            break;

                case CALL_REGION:
                    char routine_name[ 200 ];
                    strcpy( routine_name, ( ( *subr_it )->get_name() ).c_str() );
                    if( strncmp( routine_name, "MPI_", 4 ) == 0 || strncmp( routine_name, "mpi_", 4 ) == 0 ) {
                        break;
                    }
                //Otherwise continue to default
                default:
                {
                    // Duplicate property for the region
                    // Insert in set of candidates
                    ct   = new Context( ( *subr_it ), ( *p )->get_context()->getRank(), ( *p )->get_context()->getThread() );
                    prop = ( *p )->clone();
                    prop->set_context( ct );
                    candProperties.push_back( prop );
                }
                }
            } // loop over subregions
        }     //No call site; handle nested regions
        p = propsRefineRegionNesting.erase( p );
    }

    return candProperties;
}
;

std::string WestmereDepthFirst::name() {
    return "WestmereDepthFirstStrategy";
}
