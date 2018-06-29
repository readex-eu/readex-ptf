/**
   @file    StallCycleAnalysis.cc
   @ingroup StallCycleAnalysis
   @brief   Itanium2 stall cycles analysis search strategy
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
#include "StallCycleAnalysis.h"
#include "application.h"
#include "Metric.h"
#include "strategy.h"
#include "StallCycles.h"
#include "StallCyclesIntegerLoads.h"
#include "DominatingL3MissProp.h"
#include "LoadImbalanceOMPRegion.h"
#include "L2MissesProp.h"
#include "L3MissesProp.h"
#include "ConvIntFloatProp.h"
#include "MissingPrefetchProp.h"
#include "PotentialBundleBankConflictProp.h"
#include "ShortSoftwarePipelinedLoopProp.h"
#include "AdvanceSpeculativeLoadProp.h"
#include "BadCodeProp.h"
#include "PropertyID.h"
#include "psc_errmsg.h"
#include <iostream>
#include <analysisagent.h>

StallCycleAnalysis::StallCycleAnalysis( bool pedantic ) :
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

bool StallCycleAnalysis::reqAndConfigureFirstExperiment( Region* r ) {
    psc_dbgmsg( 4, "Stall cycle: In reg and configure first\n" );
    ; // TRUE can start; FALSE not ready
    std::list<ApplProcess>           controlled_processes = dp->get_controlled_processes();
    std::list<Region*>               sub_regions;
    std::list<Region*>::iterator     subr_it;
    Property*                        prop;
    std::list<ApplProcess>::iterator process;
    Prop_List::iterator              prop_it;

    pdb->clean();

    //vizualization
    //std::cout	<< "\n The initial region's tree: \n\n";
    //initial_region->print_subregions("__" , application->get_file_name_maping());

    phaseRegion = r;
    sub_regions = r->get_subregions();
    propsRefineRegionNesting.clear();

    checkedSubs.clear();

    if( agent->get_leader() ) {
        psc_dbgmsg( 1, "In stall cycle: Strategy Step %d\n", strategy_steps );
    }

    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        prop = new StallCyclesProp( new Context( phaseRegion, process->rank, 0 ), new Context( phaseRegion, process->rank, 0 ),
                                    PSC_BACK_END_BUBBLE_ALL, pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::StallCyclesProp );
        candProperties.push_back( prop );
    }
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

void StallCycleAnalysis::configureNextExperiment() {
    Prop_List::iterator prop_it;

    /*	psc_dbgmsg(4,"requesting required measurements\n");
       for(prop_it = candProperties.begin() ; prop_it != candProperties.end() ; prop_it++)
       (*prop_it)->get_required_info();
     */
    dp->transfer_requests_to_processes_no_begin_end_requests();
}

bool StallCycleAnalysis::evaluateAndReqNextExperiment() {
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

        //compute maximum of the severities
        if( ( *prop_it )->severity() > max_severity ) {
            max_severity = ( *prop_it )->severity();
        }

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

StallCyclesProp* StallCycleAnalysis::duplicateAndRefine( StallCyclesProp* p, Metric m ) {
    StallCyclesProp* np = new StallCyclesProp( p->get_context(), p->get_phaseContext(), m,
                                               pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::StallCyclesProp );
    return np;
}
;

std::list<Property*> StallCycleAnalysis::create_next_candidate_properties_set( std::list<Property*> foundProperties ) {
    std::list<Property*>           candidates;
    std::list<Property*>::iterator p;
    std::list<ApplProcess>         controlled_processes = dp->get_controlled_processes();

    if( agent->get_leader() ) {
        psc_dbgmsg( 1, "Strategy Step %d\n", strategy_steps );
    }
    if( strategy_steps >= max_strategy_steps ) {
        return candidates; //empty
    }
    strategy_steps++;

    //If general stall cycle property, save it for further region refinement
    if( !foundProperties.empty() ) {
        for( p = foundProperties.begin(); p != foundProperties.end(); p++ ) {
            if( ( *p )->id() == STALLCYCLES ) {
                StallCyclesProp* prop = dynamic_cast<StallCyclesProp*>( *p );

                if( prop->get_metric() == PSC_BACK_END_BUBBLE_ALL ) {
                    propsRefineRegionNesting.push_back( prop );
                    if( psc_get_debug_level() >= 3 ) {
                        std::cout << "Pushing back " << ( prop )->get_region()->str_print() << "\n\n";
                    }
                }
            }
        }
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
                case PSC_BACK_END_BUBBLE_ALL:
                    candidates.push_back( duplicateAndRefine( prop, PSC_BE_FLUSH_BUBBLE_ALL ) );
                    candidates.push_back( duplicateAndRefine( prop, PSC_BE_L1D_FPU_BUBBLE_ALL ) );
                    candidates.push_back( duplicateAndRefine( prop, PSC_BE_EXE_BUBBLE_ALL ) );
                    candidates.push_back( duplicateAndRefine( prop, PSC_BE_RSE_BUBBLE_ALL ) );
//          if (prop->get_region()->get_type() == LOOP_REGION) {
//            candidates.push_back(new ConvIntFloatProp(prop->get_context(),prop->get_phaseContext(), pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::ConvIntFloatProp ));
//            candidates.push_back(new MissingPrefetchProp(prop->get_context(),prop->get_phaseContext(), pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::MissingPrefetchProp));
//            candidates.push_back(new PotentialBundleBankConflictProp(prop->get_context(),prop->get_phaseContext(), pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::PotentialBundleBankConflictProp));
//            candidates.push_back(new ShortSoftwarePipelinedLoopProp(prop->get_context(),prop->get_phaseContext()), pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::ShortSoftwarePipelinedLoopProp);
//            candidates.push_back(new AdvanceSpeculativeLoadProp(prop->get_context(),prop->get_phaseContext()), pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::AdvanceSpeculativeLoadProp);
//            candidates.push_back(new BadCodeProp(prop->get_context(),prop->get_phaseContext()), pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::BadCodeProp);
//          }
                    break;
                case PSC_BE_FLUSH_BUBBLE_ALL:
                    candidates.push_back( duplicateAndRefine( prop, PSC_BE_FLUSH_BUBBLE_BRU ) );
                    candidates.push_back( duplicateAndRefine( prop, PSC_BE_FLUSH_BUBBLE_XPN ) );
                    break;
                case PSC_BE_L1D_FPU_BUBBLE_ALL:
                    candidates.push_back( duplicateAndRefine( prop, PSC_BE_L1D_FPU_BUBBLE_FPU ) );
                    candidates.push_back( duplicateAndRefine( prop, PSC_BE_L1D_FPU_BUBBLE_L1D ) );
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
                case PSC_BE_EXE_BUBBLE_ALL:
                    candidates.push_back( duplicateAndRefine( prop, PSC_BE_EXE_BUBBLE_GRALL ) );
                    candidates.push_back( duplicateAndRefine( prop, PSC_BE_EXE_BUBBLE_GRGR ) );
                    candidates.push_back( duplicateAndRefine( prop, PSC_BE_EXE_BUBBLE_FRALL ) );
                    np = new StallCyclesIntegerLoadsProp( prop->get_context(), prop->get_phaseContext(),
                                                          pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::StallCyclesIntegerLoadsProp );
                    candidates.push_back( np );
                    candidates.push_back( new DominatingL3MissProp( prop->get_context(), prop->get_phaseContext(),
                                                                    pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::DominatingL3MissProp ) );
                    candidates.push_back( new L2MissProp( prop->get_context(), prop->get_phaseContext(),
                                                          pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::L2MissProp ) );
                    candidates.push_back( new L3MissProp( prop->get_context(), prop->get_phaseContext(),
                                                          pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::L3MissProp ) );
                    break;
                case PSC_BE_RSE_BUBBLE_ALL:
                    break;
                default:
                    break;
                }     //Switch on metric in stall cycles property
            }
            case STALLCYCLESINTEGERLOADS:
                break;
            default:
                break;
            } //Switch on property id
        }     // if property is significant
    }         //loop over all properties

    if( !candidates.empty() ) {
        return candidates;
    }

    //refine in nested regions
    //for (p = propsRefineRegionNesting.begin(); p != propsRefineRegionNesting.end(); p++) {
    p = propsRefineRegionNesting.begin();
    while( p != propsRefineRegionNesting.end() ) {
        if( ( *p )->get_region()->get_type() == CALL_REGION ) {
            if( checkedSubs.size() < ( *p )->get_context()->getRank() + 1 ) {
                checkedSubs.resize( ( *p )->get_context()->getRank() + 1 );
            }
            Region* reg = appl->get_subroutine( ( *p )->get_region()->get_name().c_str() );
            if( reg != NULL && ( checkedSubs[ ( *p )->get_context()->getRank() ].find( reg ) ==
                                 checkedSubs[ ( *p )->get_context()->getRank() ].end() || !checkedSubs[ ( *p )->get_context()->getRank() ][ reg ] ) ) {
                Context*  ct      = new Context( reg, ( *p )->get_context()->getRank(), ( *p )->get_context()->getThread() );
                Context*  phaseCt = new Context( phaseRegion, ct->getRank(), 0 );
                Property* prop    = new StallCyclesProp( ct, phaseCt, PSC_BACK_END_BUBBLE_ALL,
                                                         pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::StallCyclesProp );
                candidates.push_back( prop );

                checkedSubs[ ( *p )->get_context()->getRank() ][ reg ] = true;
                psc_dbgmsg( 3, "Refined to subroutine: %s\n", ( *p )->get_region()->get_name().c_str() );
                if( psc_get_debug_level() >= 5 ) {
                    std::cout << "Refined to subroutine: " << ( *p )->get_region()->get_name() << std::endl;
                }
            }
        }
        else {
            std::list<Region*>           subRegions = ( *p )->get_region()->get_subregions();
            std::list<Region*>::iterator subr_it;
            //vizualization
            //std::cout << "\n Subregions of " << (*p)->get_region()->str_print(appl->get_file_name_maping())<<"\n\n";
            //(*p)->get_region()->print_subregions("__" , appl->get_file_name_maping());

            //look fIrst if there are subregions that are no data structures
            psc_dbgmsg( 3, "Refined to subregion: %s\n", ( *p )->get_region()->str_print().c_str() );
            //if (psc_get_debug_level()>=3) std::cout << "Refined to subregions: " << (*p)->get_region()->str_print(appl->get_file_name_maping()) << std::endl;
            for( subr_it = subRegions.begin(); subr_it != subRegions.end(); subr_it++ ) {
                //Context *ct;
                Context* ct = new Context( phaseRegion, ( *p )->get_context()->getRank(), ( *p )->get_context()->getThread() );
                ;
                Property* prop;
                Property* prop1;

                switch( ( *subr_it )->get_type() ) {
                case DATA_STRUCTURE:
                    break;
//        case PARALLEL_REGION:
//        case DO_REGION:
//        case SECTIONS_REGION:
//        case WORKSHARE_REGION:
//          if (appl->getOmpThreads() > 1) {
//
//            for (int i = 0; i < appl->getOmpThreads(); i++) {
//              ct = new Context(*subr_it, (*p)->get_context()->getRank(), i);
//              prop = new StallCyclesProp(ct, new Context(phaseRegion, ct->getRank(), 0), BACK_END_BUBBLE_ALL, pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::StallCyclesProp);
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
                    // regions nested in parallel regions.
                    int threads = 1;

                    if( ( *subr_it )->get_rra() == RUNS_AS_THREADED ) {
                        threads = appl->getOmpThreads();
                    }

                    for( int i = 0; i < threads; i++ ) {
                        ct   = new Context( *subr_it, ( *p )->get_context()->getRank(), i );
                        prop = new StallCyclesProp( ct, new Context( phaseRegion, ct->getRank(), 0 ), PSC_BACK_END_BUBBLE_ALL,
                                                    pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::StallCyclesProp );
                        candidates.push_back( prop );
                    }
//        ct = new Context(*subr_it, (*p)->get_context()->getRank(), 0);
//        prop = new StallCyclesProp(ct, new Context(phaseRegion, ct->getRank(), ct->getThread()), PSC_BACK_END_BUBBLE_ALL, pedanticSearch ? 0.0 : IA64_PROPS_THRESHOLD::StallCyclesProp);
//        candidates.push_back(prop);
                    break;
                }
            }
        }
    }     //No call site
    p = propsRefineRegionNesting.erase( p );
}

return candidates;
}
;

std::string StallCycleAnalysis::name() {
    return "StallCycleAnalysisStrategy";
}
