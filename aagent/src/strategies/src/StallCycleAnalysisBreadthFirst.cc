/**
   @file    StallCycleAnalysisBreadthFirst.cc
   @ingroup StallCycleAnalysisBreadthFirst
   @brief   Itanium2 stall cycles breadth-first search strategy
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
#include "StallCycleAnalysisBreadthFirst.h"
#include "application.h"
#include "Metric.h"
#include "strategy.h"
#include "StallCycles.h"
#include "StallCyclesIntegerLoads.h"
#include "LoadImbalanceOMPRegion.h"
#include "DominatingL3MissProp.h"
#include "DominatingL2MissProp.h"
#include "L2MissesProp.h"
#include "L3MissesProp.h"
#include "PropertyID.h"
#include "psc_errmsg.h"
#include <iostream>
#include <analysisagent.h>

bool StallCycleAnalysisBreadthFirst::reqAndConfigureFirstExperiment( Region* r ) {
    ; // TRUE can start; FALSE not ready
    std::list<ApplProcess>           controlled_processes = dp->get_controlled_processes();
    std::list<Region*>               sub_regions;
    std::list<Region*>::iterator     subr_it;
    StallCyclesProp*                 prop;
    std::list<ApplProcess>::iterator process;
    Prop_List::iterator              prop_it;

    //vizualization
    //std::cout << "\n The initial region's tree: \n\n";
    //initial_region->print_subregions("__" , application->get_file_name_maping());

    phaseRegion = r;

    if( agent->get_leader() ) {
        psc_dbgmsg( 1, "Strategy Step %d\n", strategy_steps );
    }

    candProperties = create_initial_candidate_properties_set( r );
    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( candProperties, "SET OF CANDIDATE PROPERTIES", false, false );
    }

    strategy_steps++;

    for( prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
        ( *prop_it )->request_metrics();
    }

    //dp->transfer_requests_to_processes_no_begin_end_requests();
    return true;
}

void StallCycleAnalysisBreadthFirst::configureNextExperiment() {
    Prop_List::iterator prop_it;

    //for(prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++)
    //  (*prop_it)->get_required_info();

    dp->transfer_requests_to_processes_no_begin_end_requests();
}

bool StallCycleAnalysisBreadthFirst::evaluateAndReqNextExperiment() {
    // TRUE requires next step; FALSE if done
    Prop_List::iterator prop_it;

    //Check whether all information was gathered in the experiment
    //Otherwise start another experiment

    if( dp->getResults() == ALL_INFO_GATHERED ) {
        //Information was measured. Now the properties can be evaluated.
    }
    else {
        return true;
    }

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

StallCyclesProp* StallCycleAnalysisBreadthFirst::duplicateAndRefine( StallCyclesProp* p, Metric m ) {
    StallCyclesProp* np = new StallCyclesProp( p->get_context(), p->get_phaseContext(), m,
                                               pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::StallCyclesProp );
    return np;
}
;

std::list<Property*> StallCycleAnalysisBreadthFirst::create_initial_candidate_properties_set( Region* r ) {
    std::list<ApplProcess>           controlled_processes = dp->get_controlled_processes();
    std::list<Region*>               regions;
    std::list<Property*>             candidates;
    StallCyclesProp*                 prop;
    Property*                        prop1;
    std::list<ApplProcess>::iterator process;
    std::list<Region*>::iterator     reg;

    phaseRegion = r;
    propsRefineRegionNesting.clear();
    regions = appl->get_regions();

    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        Context* pCt = new Context( phaseRegion, process->rank, 0 );
        for( reg = regions.begin(); reg != regions.end(); reg++ ) {
            int  threads = 1;
            char routine_name[ 200 ];
            strcpy( routine_name, ( ( *reg )->get_name() ).c_str() );
            if( ( *reg )->get_type() == CALL_REGION && strncmp( routine_name, "MPI", 3 ) != 0 ) {
                continue;
            }
            if( ( *reg )->get_rra() == RUNS_AS_THREADED ) {
                threads = appl->getOmpThreads();
            }
            for( int i = 0; i < appl->getOmpThreads(); i++ ) {
                Context* ct = new Context( ( *reg ), process->rank, i );
                prop = new StallCyclesProp( ct, pCt, PSC_BACK_END_BUBBLE_ALL,
                                            pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::StallCyclesProp );
                candidates.push_back( prop );
                candidates.push_back( duplicateAndRefine( prop, PSC_BE_FLUSH_BUBBLE_BRU ) );
                candidates.push_back( duplicateAndRefine( prop, PSC_BE_FLUSH_BUBBLE_XPN ) );
                candidates.push_back( duplicateAndRefine( prop, PSC_BE_L1D_FPU_BUBBLE_FPU ) );
                candidates.push_back( duplicateAndRefine( prop, PSC_BE_L1D_FPU_BUBBLE_L1D ) );
                candidates.push_back( duplicateAndRefine( prop, PSC_BE_EXE_BUBBLE_GRALL ) );
                candidates.push_back( duplicateAndRefine( prop, PSC_BE_EXE_BUBBLE_GRGR ) );
                candidates.push_back( duplicateAndRefine( prop, PSC_BE_EXE_BUBBLE_FRALL ) );
                candidates.push_back( new StallCyclesIntegerLoadsProp( prop->get_context(), prop->get_phaseContext(),
                                                                       pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::StallCyclesIntegerLoadsProp ) );
            }

            switch( ( *reg )->get_type() ) {
            case PARALLEL_REGION:
            case DO_REGION:
            case SECTIONS_REGION:
            case WORKSHARE_REGION:
                if( appl->getOmpThreads() > 1 ) {
                    prop1 = new LoadImbalanceOMPRegionProp( new Context( *reg, process->rank, 0 ), pCt,
                                                            pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::StallCyclesIntegerLoadsProp );
                    candidates.push_back( prop1 );
                }
                break;
            default:
                break;
            }
        }
    }

    return candidates;
}
;

std::list<Property*> StallCycleAnalysisBreadthFirst::create_next_candidate_properties_set( std::list<Property*> foundProperties ) {
    std::list<Property*>           candidates;
    std::list<Property*>::iterator p;
    std::list<ApplProcess>         controlled_processes = dp->get_controlled_processes();

    //if (psc_get_debug_level()>=3)
    //  std::cout << "Strategy Step Nr: " << strategy_steps << std::endl;
    //psc_dbgmsg(1, "Strategy Step %d\n",strategy_steps);

    if( strategy_steps >= max_strategy_steps ) {
        return candidates; //empty
    }
    // First refine in more precise properties
    for( p = foundProperties.begin(); p != foundProperties.end(); p++ ) {
        if( ( *p )->severity() > 5.0 ) {
            StallCyclesProp*             prop;
            StallCyclesIntegerLoadsProp* np;

            switch( ( *p )->id() ) {
            case STALLCYCLES:
            {
                prop = dynamic_cast<StallCyclesProp*>( *p );

                switch( prop->get_metric() ) {
                case PSC_BE_EXE_BUBBLE_FRALL:
                    candidates.push_back( new DominatingL3MissProp( prop->get_context(), prop->get_phaseContext(),
                                                                    pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::DominatingL3MissProp ) );
                    candidates.push_back( new DominatingL2MissProp( prop->get_context(), prop->get_phaseContext(),
                                                                    pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::DominatingL2MissProp ) );
                    candidates.push_back( new L2MissProp( prop->get_context(), prop->get_phaseContext(),
                                                          pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::L2MissProp ) );
                    candidates.push_back( new L3MissProp( prop->get_context(), prop->get_phaseContext(),
                                                          pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::L3MissProp ) );
                    break;
                case PSC_BE_L1D_FPU_BUBBLE_L1D:
                    candidates.push_back( duplicateAndRefine( prop, PSC_BE_L1D_FPU_BUBBLE_L1D_FULLSTBUF ) );
                    candidates.push_back( duplicateAndRefine( prop, PSC_BE_L1D_FPU_BUBBLE_L1D_DCURECIR ) );
                    candidates.push_back( duplicateAndRefine( prop, PSC_BE_L1D_FPU_BUBBLE_L1D_HPW ) );
                    candidates.push_back( duplicateAndRefine( prop, PSC_BE_L1D_FPU_BUBBLE_L1D_TLB ) );
                    candidates.push_back( duplicateAndRefine( prop, PSC_BE_L1D_FPU_BUBBLE_L1D_L2BPRESS ) );
                    break;
                case PSC_BE_L1D_FPU_BUBBLE_FPU:
                    candidates.push_back( duplicateAndRefine( prop, PSC_FP_FALSE_SIRSTALL ) );
                    candidates.push_back( duplicateAndRefine( prop, PSC_FP_FLUSH_TO_ZERO ) );
                    candidates.push_back( duplicateAndRefine( prop, PSC_FP_TRUE_SIRSTALL ) );
                    break;
                case PSC_BE_RSE_BUBBLE_ALL:
                    break;
                default:
                    break;
                }     //Switch on metric in stall cycles property
                break;
            }
            case STALLCYCLESINTEGERLOADS:
                np = dynamic_cast<StallCyclesIntegerLoadsProp*>( *p );
                candidates.push_back( new DominatingL3MissProp( np->get_context(), np->get_phaseContext(),
                                                                pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::DominatingL3MissProp ) );
                candidates.push_back( new DominatingL2MissProp( np->get_context(), np->get_phaseContext(),
                                                                pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::DominatingL2MissProp ) );
                candidates.push_back( new L2MissProp( np->get_context(), np->get_phaseContext(),
                                                      pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::L2MissProp ) );
                candidates.push_back( new L3MissProp( np->get_context(), np->get_phaseContext(),
                                                      pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::L3MissProp ) );
                break;
            default:
                break;
            } //Switch on property id
        }     // if property is significant
    }         //loop over all properties
    return candidates;
}
;

std::string StallCycleAnalysisBreadthFirst::name() {
    return "StallCycleAnalysisBreadthFirstStrategy";
}
