/**
   @file    p575Strategy.cc
   @ingroup p575Strategy
   @brief   Power6 depth-first search strategy
   @author  Yury Oleynik
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
#include "p575Strategy.h"

#include "p6HotSpot.h"

#include <iostream>
#include "psc_errmsg.h"
#include <analysisagent.h>
#include "return_summary_data.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>

#ifdef __p575
  #include <strings.h>
#endif

bool p575Strategy::reqAndConfigureFirstExperiment( Region* r ) {
/// TRUE can start; FALSE not ready

    std::list<ApplProcess>           controlled_processes = dp->get_controlled_processes();
    std::list<ApplProcess>::iterator process;
    Prop_List::iterator              prop_it;
    Property*                        prop;

    phaseRegion = r;

///creating set of initial properties candidates for all process ranks
    if( agent->get_leader() ) {
        psc_dbgmsg( 1, "Strategy Step %d\n", strategy_steps );
    }
    Context* context, * phaseContext;
    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        context      = new Context( phaseRegion, process->rank, 0 );
        phaseContext = new Context( phaseRegion, process->rank, 0 );

        candProperties.push_back( new p6HotSpot( context, phaseContext ) );
    }

    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( candProperties, "SET OF CANDIDATE PROPERTIES", false, false );
    }

///calling candidate properties to request necessary metrics and transfer them to mrimonitor
    for( prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
        ( *prop_it )->request_metrics();
    }

    strategyIterations = 1;
    //configureNextExperiment();

    if( agent->get_leader() ) {
        psc_dbgmsg( 5, "(p575Strategy::reqAndConfigureFirstExperiment) Strategy Step %d configured\n", strategy_steps );
    }
    return true;
}

void p575Strategy::configureNextExperiment() {
    dp->transfer_requests_to_processes_no_begin_end_requests();
    psc_dbgmsg( 5, "(p575Strategy::configureNextExperiment)Strategy Step %d, iteration %d, requests transfered to mrimonitor\n",
                strategy_steps, strategyIterations );
    strategyIterations++;
}

bool p575Strategy::evaluateAndReqNextExperiment() {
    // TRUE requires next step; FALSE if done
    Prop_List::iterator prop_it;

    if( dp->getResults() == ALL_INFO_GATHERED ) {
        //Information was measured. Now the properties can be evaluated.
        psc_dbgmsg( 5, "(p575Strategy::evaluateAndReqNextExperiment) Data Provider returned ALL_INFO_GATHERED, step %d, iteration %d\n",
                    strategy_steps, strategyIterations );
    }
    else {
        psc_dbgmsg( 5, "(p575Strategy::evaluateAndReqNextExperiment) Data Provider returned NOT_ALL_INFO_GATHERED, step %d, iteration %d\n",
                    strategy_steps, strategyIterations );
        return true;
    }

    foundPropertiesLastStep.clear();
    for( prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
        ( *prop_it )->evaluate();
        if( ( *prop_it )->condition() ) {
            psc_dbgmsg( 5, "Found property %s\n", ( *prop_it )->name().c_str() );
            foundPropertiesLastStep.push_back( *prop_it );
            foundProperties.push_back( *prop_it );
        }
    }

    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( foundPropertiesLastStep, "SET OF FOUND PROPERTIES", true, false );
    }

    candProperties.clear();
    candProperties = create_next_candidate_properties_set( foundPropertiesLastStep );

    for( prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
        ( *prop_it )->request_metrics();
    }

    if( candProperties.size() == 0 ) {
        if( psc_get_debug_level() >= 2 ) {
            agent->print_property_set( foundProperties, "SET OF FINAL PROPERTIES", true, false );
        }
        return false;
    }
    else {
        psc_dbgmsg( 5, "(evaluateAndReqNextExperiment())Strategy Step %d, iteration %d, evaluated, candidate properties created, data requested, moving to the next step...\n",
                    strategy_steps, strategyIterations );
        if( psc_get_debug_level() >= 2 ) {
            agent->print_property_set( candProperties, "SET OF CANDIDATE PROPERTIES", false, false );
        }
        strategy_steps++;
        //strategyIterations = 0;
        return true;
    }
}

std::list<Property*> p575Strategy::create_initial_candidate_properties_set( Region* initial_region ) {
}

std::list<Property*> p575Strategy::create_next_candidate_properties_set( std::list<Property*> ev_set ) {
    Prop_List           returnList, childProp;
    Prop_List::iterator prop_it, prop_it2;

    for( prop_it = ev_set.begin(); prop_it != ev_set.end(); prop_it++ ) {
        childProp = ( *prop_it )->next();
        if( ( *prop_it )->id() == P6HOTSPOT ) {
            psc_dbgmsg( 5, "Property %s pushed back for region nesting analysis\n", ( *prop_it )->name().c_str() );
            propsRefineRegionNesting.push_back( *prop_it );
        }
        for( prop_it2 = childProp.begin(); prop_it2 != childProp.end(); prop_it2++ ) {
            returnList.push_back( ( *prop_it2 ) );
        }
    }

    if( !returnList.empty() ) {
        return returnList; //if there are any child properties in prop. hierarchy return them
    }
    if( propsRefineRegionNesting.empty() ) {
        return returnList; // if properties2refineRegions list is empty return empty returnList
    }
    // now when we have not empty Prop2refineRegions list and properties hierarchy is already investigated
    // continue with region refinement

    std::list<Property*>           candidates;
    std::list<Property*>::iterator p;
    // std::list <ApplProcess> controlled_processes = dp->get_controlled_processes();

    if( agent->get_leader() ) {
        psc_dbgmsg( 5, "Strategy Step %d out of %d, investigating region nesting...\n", strategy_steps, max_strategy_steps );
    }

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //Max strategy step hardcoded!!! change later to the proper value!!!
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    if( strategyIterations >= 200 ) {
        return candidates; //empty
    }
    //refine in nested regions
    //for (p = propsRefineRegionNesting.begin(); p != propsRefineRegionNesting.end(); p++){

    p = propsRefineRegionNesting.begin();
    while( p != propsRefineRegionNesting.end() ) {
        psc_dbgmsg( 5, "Property %s on region %s, reg_type=%d\n", ( *p )->name().c_str(),
                    ( *p )->get_region()->get_name().c_str(), ( *p )->get_region()->get_type() );
        if( ( *p )->get_region()->get_type() == CALL_REGION ) {
            //potential problem here!!! if we have 10^6 ranks, but this current AA controls only 5,
            //we will have checkedSubs.size == 10^6!!!
            if( checkedSubs.size() < ( *p )->get_context()->getRank() + 1 ) {
                psc_dbgmsg( 5, "Resizing to %d\n", ( *p )->get_context()->getRank() + 1 );
                checkedSubs.resize( ( *p )->get_context()->getRank() + 1 );
            }
            Region* reg = appl->get_subroutine( ( *p )->get_region()->get_name().c_str() );

            if( reg != NULL ) {
                psc_dbgmsg( 3, "appl->get_subroutine: %d\n", ( *p )->get_region()->get_ident().start_position );
            }
            psc_dbgmsg( 5, "Property bit is %d\n", ( int )( *p )->id() - ( int )p575PropBegin );
            if( checkedSubs[ ( *p )->get_context()->getRank() ][ reg ].test( ( int )( *p )->id() - ( int )p575PropBegin ) ) {
                psc_dbgmsg( 5, "Checked subs is true:)\n" );
            }
            else {
                psc_dbgmsg( 5, "Checked subs is false:(\n" );
            }
            if( reg != NULL && ( checkedSubs[ ( *p )->get_context()->getRank() ].find( reg ) ==
                                 checkedSubs[ ( *p )->get_context()->getRank() ].end() ||
                                 !checkedSubs[ ( *p )->get_context()->getRank() ][ reg ].test( ( int )( *p )->id() - ( int )p575PropBegin ) ) ) {
                //if (reg != NULL) {
                Context*  ct      = new Context( reg, ( *p )->get_context()->getRank(), ( *p )->get_context()->getThread() );
                Context*  phaseCt = new Context( phaseRegion, ct->getRank(), 0 );
                Property* prop    = ( *p )->clone();
                prop->set_context( ct );
                prop->set_PhaseContext( phaseCt );
                //new StallCyclesProp(ct, phaseCt, PSC_BACK_END_BUBBLE_ALL);
                candidates.push_back( prop );
                checkedSubs[ ( *p )->get_context()->getRank() ][ reg ].set( ( int )( *p )->id() - ( int )p575PropBegin );
                psc_dbgmsg( 5, "Refined to subroutine: %s\n", ( *p )->get_region()->get_name().c_str() );
                //if (psc_get_debug_level() >= 3)
                //  std::cout << "Refined to subroutine: " << (*p)->get_region()->get_name() << std::endl;
            }
        }
        else {
            std::list<Region*>           subRegions = ( *p )->get_region()->get_subregions();
            std::list<Region*>::iterator subr_it;
            //vizualization
            //std::cout << "\n Subregions of " << (*p)->get_region()->str_print(appl->get_file_name_maping())<<"\n\n";
            //(*p)->get_region()->print_subregions("__" , appl->get_file_name_maping());
            //look forst if there are subregions that are no data structures
            psc_dbgmsg( 5, "Refinement of subregion: %s, subregions count %d\n", ( *p )->get_region()->str_print().c_str(),
                        subRegions.size() );
            //if (psc_get_debug_level() >= 3)
            //  std::cout << "Refined to subregions: " << (*p)->get_region()->str_print(appl->get_file_name_maping()) << std::endl;
            for( subr_it = subRegions.begin(); subr_it != subRegions.end(); subr_it++ ) {
                Context*  ct;
                Property* prop;
                //Property *prop1;
                switch( ( *subr_it )->get_type() ) {
                case DATA_STRUCTURE:
                    break;
                //case PARALLEL_REGION:
                //case DO_REGION:
                //case SECTIONS_REGION:
                //case WORKSHARE_REGION:
                /*if (appl->getOmpThreads() > 1) {
                   prop1 = new LoadImbalanceOMPRegionProp(new Context(*subr_it, (*p)->get_context()->getRank(), 0),
                                                         new Context(phaseRegion, ct->getRank(), 0));
                   candidates.push_back(prop1);
                   }

                   for (int i = 0; i < appl->getOmpThreads(); i++) {
                   ct = new Context(*subr_it, (*p)->get_context()->getRank(), i);
                   prop = new StallCyclesProp(ct, new Context(phaseRegion, ct->getRank(), 0), PSC_BACK_END_BUBBLE_ALL);
                   candidates.push_back(prop);
                   }*/
                //break;
                default:
                {
                    psc_dbgmsg( 5, "Refining to subregion: %s\n", ( *subr_it )->str_print().c_str() );
                    int threads = 1;

                    if( ( *subr_it )->get_rra() == RUNS_AS_THREADED ) {
                        threads = appl->getOmpThreads();
                    }

                    for( int i = 0; i < threads; i++ ) {
                        ct = new Context( *subr_it, ( *p )->get_context()->getRank(), i );
                        //prop = new p575DPUHeld(ct, new Context(phaseRegion,ct->getRank(), i));
                        //candProperties.push_back(prop);
                        prop = ( *p )->clone();
                        prop->set_context( ct );
                        prop->set_PhaseContext( new Context( phaseRegion, ct->getRank(), 0 ) );
                        //new StallCyclesProp(ct,new Context(phaseRegion, ct->getRank(), 0), PSC_BACK_END_BUBBLE_ALL);
                        candidates.push_back( prop );
                    }
                    //ct = new Context(*subr_it, (*p)->get_context()->getRank(), 0);
                    //prop = new StallCyclesProp(ct, new Context(phaseRegion, ct->getRank(), ct->getThread()), PSC_BACK_END_BUBBLE_ALL);
                    //candidates.push_back(prop);
                    break;
                }
                }
            }
        } //No call site
        p = propsRefineRegionNesting.erase( p );
    }

    return candidates;
}

std::string p575Strategy::name() {
    return "P6 Single Core Performance Analysis Strategy";
}
