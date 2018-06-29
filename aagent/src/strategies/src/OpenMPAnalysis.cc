/**
   @file    OpenMPAnalysis.cc
   @ingroup OpenMPStrategy
   @brief   OpenMP search strategy
   @author  Shajulin Benedict
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
#include "OpenMPAnalysis.h"
#include "application.h"
#include "Metric.h"
#include "strategy.h"
#include "PropertyID.h"
#include "psc_errmsg.h"
#include <iostream>
#include <string.h>
#include <analysisagent.h>

#include "ImbalanceOMPBarrier.h"
#include "CriticalRegionOverhead.h"
#include "SerializationCriticalRegion.h"
#include "SequentialInSingle.h"
#include "SequentialInMaster.h"
#include "StartupShutdownOverhead.h"
#include "SequentialInOrderedLoop.h"
#include "ImbalanceInOrderedLoop.h"
#include "ImbalanceInParSections.h"
#include "LoadImbalanceOMPRegion.h"
#include "HotRegionExecTime.h"
#include "OverheadDueToSmallTask.h"
#include "OverheadDueToTaskCreation.h"
#include "TooFineGranularTasks.h"
#include "EmptyTaskinTaskRegion.h"
#include "ImbalancedTaskRegion.h"
#include "HighInclOverheadProp.h"
#include "HighExclOverheadProp.h"

using namespace std;

bool OpenMPAnalysis::reqAndConfigureFirstExperiment( Region* r ) {
    std::list<ApplProcess>           controlled_processes = dp->get_controlled_processes();
    std::list<Region*>               sub_regions;
    std::list<Region*>::iterator     subr_it;
    std::list<ApplProcess>::iterator process;
    Prop_List::iterator              prop_it;
    Property*                        prop;
    phaseRegion = r;
    process     = controlled_processes.begin();

    if( agent->get_leader() ) {
        psc_dbgmsg( 1, "Strategy Step %d\n", strategy_steps );
    }
    candProperties = create_initial_candidate_properties_set( r );

//  prop = new HighInclOverheadProp(new Context(phaseRegion, process->rank, 0), new Context(phaseRegion, process->rank, 0), pedanticSearch ? 0.0 : OVERHEAD_PROPS_THRESHOLD::HighInclOverheadProp);
//  candProperties.push_back(prop);
//  prop = new HighExclOverheadProp(new Context(phaseRegion, process->rank, 0), new Context(phaseRegion, process->rank, 0), pedanticSearch ? 0.0 : OVERHEAD_PROPS_THRESHOLD::HighExclOverheadProp);
//  candProperties.push_back(prop);

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

std::list<Property*> OpenMPAnalysis::create_initial_candidate_properties_set( Region* r ) {
    std::list<ApplProcess>           controlled_processes = dp->get_controlled_processes();
    std::list<Region*>               regions;
    std::list<Property*>             candidates;
    Property*                        prop1, * prop2, * prop3, * prop4, * prop5, * prop6, * prop7;
    std::list<ApplProcess>::iterator process;
    std::list<Region*>::iterator     reg;
    bool                             user_region_present = false;

    phaseRegion = r;
    propsRefineRegionNesting.clear();
    regions = appl->get_regions();
    for( reg = regions.begin(); reg != regions.end(); reg++ ) {
        int threads = 1;
        for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
            Context* pCt = new Context( phaseRegion, process->rank, 0 );

            //Either user region or main region returns the phase context
            if( ( *reg )->get_type() == MAIN_REGION || ( *reg )->get_type() == USER_REGION || ( *reg )->get_type() == PARALLEL_REGION ) {
                prop1 = new HotRegionExecTimeProp( new Context( *reg, process->rank, 0 ), pCt, USER_REGION_TYPE,
                                                   pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::HotRegionExecTimeProp );
                candidates.push_back( prop1 );
            }

            if( ( *reg )->get_rra() == RUNS_AS_THREADED ) {
                threads = appl->getOmpThreads();
                switch( ( *reg )->get_type() ) {
                case CALL_REGION:
                    prop1 = new HotRegionExecTimeProp( new Context( *reg, process->rank, 0 ), pCt, CALL_REGION_TYPE,
                                                       pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::HotRegionExecTimeProp );
                    candidates.push_back( prop1 );
                    break;
                case SUB_REGION:
                    prop1 = new HotRegionExecTimeProp( new Context( *reg, process->rank, 0 ), pCt, SUB_REGION_TYPE,
                                                       pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::HotRegionExecTimeProp );
                    candidates.push_back( prop1 );
                    break;
                case DO_REGION:     //resembles omp parallel for
                    if( appl->getOmpThreads() > 1 ) {
                        prop1 = new StartupShutdownOverheadProp( new Context( *reg, process->rank, 0 ), pCt,
                                                                 pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::StartupShutdownOverheadProp );
                        candidates.push_back( prop1 );
                        prop2 = new LoadImbalanceOMPRegionProp( new Context( *reg, process->rank, 0 ), pCt, PARALLEL_LOOP ),
                        pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::LoadImbalanceOMPRegionProp;
                        candidates.push_back( prop2 );
                    }
                    prop3 = new HotRegionExecTimeProp( new Context( *reg, process->rank, 0 ), pCt, DO_REGION_TYPE ),
                    pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::HotRegionExecTimeProp;
                    candidates.push_back( prop3 );
                    break;
                case TASK_REGION:
                    // if (appl->getOmpThreads() > 1) {
                    //Task regions should be measured from single threads.
                    prop1 = new OverheadDueToTaskCreationProp( new Context( *reg, process->rank, 0 ), pCt,
                                                               pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::OverheadDueToTaskCreationProp );
                    candidates.push_back( prop1 );
                    prop2 = new OverheadDueToSmallTaskProp( new Context( *reg, process->rank, 0 ), pCt,
                                                            pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::OverheadDueToSmallTaskProp );
                    candidates.push_back( prop2 );
                    prop3 = new EmptyTasksinTaskRegionProp( new Context( *reg, process->rank, 0 ), pCt,
                                                            pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::EmptyTasksinTaskRegionProp );
                    candidates.push_back( prop3 );
                    prop4 = new TooFineGranularTasksProp( new Context( *reg, process->rank, 0 ), pCt,
                                                          pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::TooFineGranularTasksProp );
                    candidates.push_back( prop4 );
                    prop5 = new ImbalancedTaskRegionProp( new Context( *reg, process->rank, 0 ), pCt, Imbalanced_Task_Region,
                                                          pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::ImbalancedTaskRegionProp );
                    candidates.push_back( prop5 );
                    prop6 = new ImbalancedTaskRegionProp( new Context( *reg, process->rank, 0 ), pCt,
                                                          Imbalace_Due_To_Uneven_Distribution_of_Tasks,
                                                          pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::ImbalancedTaskRegionProp );
                    candidates.push_back( prop6 );
                    prop7 = new ImbalancedTaskRegionProp( new Context( *reg, process->rank, 0 ), pCt,
                                                          Numberof_Tasks_Smaller_than_Numberof_Threads,
                                                          pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::ImbalancedTaskRegionProp );
                    candidates.push_back( prop7 );
                    // }
                    break;
                case PARALLEL_REGION:
                    if( appl->getOmpThreads() > 1 ) {
                        prop1 = new StartupShutdownOverheadProp( new Context( *reg, process->rank, 0 ), pCt,
                                                                 pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::StartupShutdownOverheadProp );
                        candidates.push_back( prop1 );
                        prop2 = new LoadImbalanceOMPRegionProp( new Context( *reg, process->rank, 0 ), pCt, PARALLEL_OVERALL_REGION,
                                                                pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::LoadImbalanceOMPRegionProp );
                        candidates.push_back( prop2 );
                    }
                    prop3 = new HotRegionExecTimeProp( new Context( *reg, process->rank, 0 ), pCt, PARALLEL_REGION_TYPE,
                                                       pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::HotRegionExecTimeProp );
                    candidates.push_back( prop3 );
                    break;
                case WORKSHARE_DO:
                    if( appl->getOmpThreads() > 1 ) {
                        prop1 = new LoadImbalanceOMPRegionProp( new Context( *reg, process->rank, 0 ), pCt, PARALLEL_LOOP,
                                                                pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::LoadImbalanceOMPRegionProp );
                        candidates.push_back( prop1 );
                    }
                    prop3 = new HotRegionExecTimeProp( new Context( *reg, process->rank, 0 ), pCt, OMP_FOR_REGION_TYPE,
                                                       pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::HotRegionExecTimeProp );
                    candidates.push_back( prop3 );
                    break;
                case WORKSHARE:
                    if( appl->getOmpThreads() > 1 ) {
                        prop1 = new LoadImbalanceOMPRegionProp( new Context( *reg, process->rank, 0 ), pCt, PARALLEL_WORKSHARE_REGION,
                                                                pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::LoadImbalanceOMPRegionProp );
                        candidates.push_back( prop1 );
                    }
                    break;
                case CRITICAL_REGION:
                    if( appl->getOmpThreads() > 1 ) {
                        prop1 = new CriticalRegionOverheadProp( new Context( *reg, process->rank, 0 ), pCt,
                                                                pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::CriticalRegionOverheadProp );
                        candidates.push_back( prop1 );
                        prop2 = new SerializationCriticalRegionProp( new Context( *reg, process->rank, 0 ), pCt,
                                                                     pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::SerializationCriticalRegionProp );
                        candidates.push_back( prop2 );
                    }
                    break;
                case SINGLE_REGION:
                    if( appl->getOmpThreads() > 1 ) {
                        prop1 = new SequentialInSingleProp( new Context( *reg, process->rank, 0 ), pCt,
                                                            pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::SequentialInSingleProp );
                        candidates.push_back( prop1 );
                    }
                    break;
                case MASTER_REGION_BODY:
                    if( appl->getOmpThreads() > 1 ) {
                        prop1 = new SequentialInMasterProp( new Context( *reg, process->rank, 0 ), pCt,
                                                            pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::SequentialInMasterProp );
                        candidates.push_back( prop1 );
                    }
                    break;
                case BARRIER_REGION:
                    if( appl->getOmpThreads() > 1 ) {
                        prop1 = new ImbalanceOMPBarrierProp( new Context( *reg, process->rank, 0 ), pCt,
                                                             pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::ImbalanceOMPBarrierProp );
                        candidates.push_back( prop1 );
                    }
                    break;
                case ORDERED_REGION:
                    if( appl->getOmpThreads() > 1 ) {
                        prop1 = new SequentialInOrderedLoopProp( new Context( *reg, process->rank, 0 ), pCt,
                                                                 pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::SequentialInOrderedLoopProp );
                        candidates.push_back( prop1 );
                        prop2 = new ImbalanceInOrderedLoopProp( new Context( *reg, process->rank, 0 ), pCt,
                                                                pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::ImbalanceInOrderedLoopProp );
                        candidates.push_back( prop2 );
                    }
                    break;

                case SECTIONS_REGION:
                    if( appl->getOmpThreads() > 1 ) {
                        prop1 = new ImbalanceInParSectionsProp( new Context( *reg, process->rank, 0 ), pCt,
                                                                pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::ImbalanceInParSectionsProp );
                        candidates.push_back( prop1 );
                    }
                    break;
                case WORKSHARE_SECTIONS:
                    if( appl->getOmpThreads() > 1 ) {
                        prop1 = new ImbalanceInParSectionsProp( new Context( *reg, process->rank, 0 ), pCt,
                                                                pedanticSearch ? 0.0 : OMP_PROPS_THRESHOLD::ImbalanceInParSectionsProp );
                        candidates.push_back( prop1 );
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }

    return candidates;
}
;

void OpenMPAnalysis::configureNextExperiment() {
    Prop_List::iterator prop_it;

//  for(prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++)
//    (*prop_it)->get_required_info();

    dp->transfer_requests_to_processes_no_begin_end_requests();
}

bool OpenMPAnalysis::evaluateAndReqNextExperiment() {
    Prop_List::iterator              prop_it;
    std::list<ApplProcess>::iterator process;
    std::list<ApplProcess>           controlled_processes = dp->get_controlled_processes();

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
    for( prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
        ( *prop_it )->evaluate();
        if( ( *prop_it )->condition() ) {
            foundPropertiesLastStep.push_back( *prop_it );
            foundProperties.push_back( *prop_it );
        }
    }

    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( foundPropertiesLastStep, "SET OF FOUND PROPERTIES", true, false );
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
        if( psc_get_debug_level() >= 2 ) {
            agent->print_property_set( candProperties, "SET OF CANDIDATE PROPERTIES", false, false );
        }
        strategy_steps++;
        return true;
    }
}

std::list<Property*> OpenMPAnalysis::create_next_candidate_properties_set( std::list<Property*> foundProperties ) {
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
    return candidates;
}
;

std::string OpenMPAnalysis::name() {
    return "OpenMPStrategy";
}
;
