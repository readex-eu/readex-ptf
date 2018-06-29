/**
   @file    EnergyGranularity.cc
   @ingroup EnergyGranularityStrategy
   @brief   Energy granularity depth-first search strategy
   @author  Robert Mijakovic
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
#include "EnergyGranularity.h"
#include "application.h"
#include "Metric.h"
#include "strategy.h"
#include "PropertyID.h"
#include "psc_errmsg.h"
#include <iostream>
#include <analysisagent.h>

#include "SuitedForEnergyConfiguration.h"
#include "EnergyInefficient.h"
#include "MemoryBound.h"

EnergyGranularity::EnergyGranularity( bool pedantic, const list<TuningSpecification*>* tuningSpecs ) : Strategy( pedantic ) {
    preconfigureTSs = tuningSpecs;
    list <ApplProcess>           controlled_processes = dp->get_controlled_processes();
    int                          maxRank              = 0;
    list <ApplProcess>::iterator process;
    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        if( process->rank > maxRank ) {
            maxRank = process->rank;
        }
    }
    checkedSubs.resize( maxRank + 1 );
}

/**
 * @brief Preconfigures analysis strategy
 * @ingroup TuningStrategy
 *
 * Preconfigures analysis strategy by sending tuning actions to the MRI monitor.
 *
 */
void EnergyGranularity::preconfigureAnalysis() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ), "EnergyGranularity: call to preconfigureAnalysis\n" );

    list<ApplProcess>                          controlled_processes = dp->get_controlled_processes();
    list<ApplProcess>::iterator                process;
    list<TuningSpecification*>::const_iterator preconfigureTS;
    map<TuningParameter*, int>                 values;
    map<TuningParameter*, int>::const_iterator tuningVariant;
    string                                     tuningParameterName;
    int                                        tuningActionType;
    int                                        tuningParameterValue;

    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        if( !preconfigureTSs ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ), "EnergyGranularity: Preconfigure Tuning specification empty!\n" );
            break;
        }
        for( preconfigureTS = preconfigureTSs->begin(); preconfigureTS != preconfigureTSs->end(); preconfigureTS++ ) {
            if( !processInRanks( ( *preconfigureTS )->getRanks(), process->rank ) ) {
                break;
            }
            values = ( *preconfigureTS )->getVariant()->getValue();
            if( ( *preconfigureTS )->getTypeOfVariantContext() == variant_context_type( REGION_LIST ) ) {
                VariantContext context = ( *preconfigureTS )->getVariantContext();

                for( tuningVariant = values.begin(); tuningVariant != values.end(); tuningVariant++ ) {
                    tuningParameterName  = tuningVariant->first->getName();
                    tuningActionType     = tuningVariant->first->getRuntimeActionType();
                    tuningParameterValue = tuningVariant->second;

                    list<Region*>*          regions = context.context_union.region_list;
                    list<Region*>::iterator region;
                    for( region = regions->begin(); region != regions->end(); region++ ) {
                        char str[ 2000 ];
                        if( tuningActionType == TUNING_ACTION_VARIABLE_INTEGER ) {
                            sprintf( str, "tuningaction (%d, %s, %d) = (TUNING_ACTION_VARIABLE, %s, %d);\n", ( *region )->get_ident().file_id,
                                     dp->regionType2reqSpec( ( *region )->get_type(), PSC_UNDEFINED_METRIC ).c_str(),
                                     ( *region )->get_ident().rfl, tuningParameterName.c_str(), tuningParameterValue );
                        }
                        else if( tuningActionType == TUNING_ACTION_FUNCTION_POINTER ) {
                            sprintf( str, "tuningaction (%d, %s, %d) = (TUNING_ACTION_FUNCTION, %s, %d);\n", ( *region )->get_ident().file_id,
                                     dp->regionType2reqSpec( ( *region )->get_type(), PSC_UNDEFINED_METRIC ).c_str(),
                                     ( *region )->get_ident().rfl, tuningParameterName.c_str(), tuningParameterValue );
                        }
                        else if( tuningActionType == TUNING_ACTION_NONE ) {
                            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ), "Tuning action type is NONE\n" );
                            break;
                        }
                        else {
                            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ), "Tuning action type is uninitialized\n" );
                            break;
                        }

                        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ),
                                    "Creating tuning action (rank:%d, name:%s, type:%d, value:%d)\n",
                                    process->rank, tuningParameterName.c_str(), tuningActionType, tuningParameterValue );
                        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ), "requesting: %s\n", str );
                        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ), "Requested tuning action for process %d.\n",
                                    process->rank );
                        dp->write_line( &( *process ), str );
                        dp->wait_for_ok( *process );
                    }
                }
            }
        }
    }
}


bool EnergyGranularity::reqAndConfigureFirstExperiment( Region* r ) {
    psc_dbgmsg( 4, "EnergyGranularity: In reg and configure first\n" );
    ; // TRUE can start; FALSE not ready
    list <ApplProcess>           controlled_processes = dp->get_controlled_processes();
    list <Region*>               sub_regions;
    list <Region*>::iterator     subr_it;
    Property*                    prop;
    list <ApplProcess>::iterator process;
    Prop_List::iterator          prop_it;


    pdb->clean();

    //vizualization
    //cout << "\n The initial region's tree: \n\n";
    //initial_region->print_subregions("__", application->get_file_name_maping());

    phaseRegion = r;
    sub_regions = r->get_subregions();
    propsRefineRegionNesting.clear();

    checkedSubs.clear();

    if( agent->get_leader() ) {
        psc_dbgmsg( 1, "EnergyGranularity: Strategy Step %d\n", strategy_steps );
    }

// Iterates through all processes and for each process create a candidate property for the phase region
    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        Context* ct, * pCt;

        ct   = new Context( phaseRegion, process->rank, 0 );
        pCt  = new Context( phaseRegion, process->rank, 0 );
        prop = new SuitedForEnergyConfiguration( ct, pCt, pedanticSearch ? 0.0 : ENERGY_GRANULARITY_THRESHOLD::RAPLGranularity );
        candProperties.push_back( prop );
    }
    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( candProperties, "SET OF CANDIDATE PROPERTIES", false, false );
    }

    strategy_steps++;
// For each property from the candidate property set requests required info
    for( prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
        ( *prop_it )->request_metrics();
    }

    return true;
}


void EnergyGranularity::configureNextExperiment() {
    Prop_List::iterator prop_it;

    preconfigureAnalysis();

    dp->transfer_requests_to_processes_no_begin_end_requests();
}



bool EnergyGranularity::evaluateAndReqNextExperiment() {
    // TRUE requires next step; FALSE if done
    Prop_List::iterator          prop_it;
    list <ApplProcess>::iterator process;
    list <ApplProcess>           controlled_processes = dp->get_controlled_processes();
    double                       max_severity         = 0;

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
        agent->print_property_set( foundPropertiesLastStep,  "SET OF FOUND PROPERTIES", true, true );
    }


    //Determine new candidate properties

    // Removes the previous candidate property set
    candProperties.clear();

/*  for (process = controlled_processes.begin(); process != controlled_processes.end(); process++) {
    for (int i = 0; i < appl->getOmpThreads(); i++) {
      pdb->erase(phaseRegion->get_ident().file_id, phaseRegion->get_ident().rfl, process->rank, i, PSC_PAPI_TOT_CYC);
    }
   }*/

//  for (process = controlled_processes.begin(); process != controlled_processes.end(); process++) {
//    for (int i = 0; i < appl->getOmpThreads(); i++) {
//      pdb->erase(phaseRegion->get_ident().file_id, phaseRegion->get_ident().rfl, process->rank, i, PSC_EXECUTION_TIME);
//    }
//  }

    // Create candidate properties set from a set of found properties from the last step
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

list <Property*> EnergyGranularity::create_next_candidate_properties_set( list< Property* > foundProperties ) {
    list <Property*>           candProperties;
    list <Property*>::iterator p, p1;
    list <ApplProcess>         controlled_processes = dp->get_controlled_processes();
    Property*                  prop;
    Property*                  prop1;

    if( agent->get_leader() ) {
        psc_dbgmsg( 1, "Strategy Step %d\n", strategy_steps );
    }
    if( strategy_steps >= max_strategy_steps ) {
        return candProperties; //empty
    }
    // Ask for the properties of regions whose execution time is longer than the threshold
    for( p = foundPropertiesLastStep.begin(); p != foundPropertiesLastStep.end(); p++ ) {
        Context* ct, * pCt;
        if( ( *p )->id() == ENERGY_SUITED ) {
            ct    = new Context( ( *p )->get_context()->getRegion(), ( *p )->get_context()->getRank(), ( *p )->get_context()->getThread() );
            pCt   = new Context( ( *p )->get_context()->getRegion(), ( *p )->get_context()->getRank(), ( *p )->get_context()->getThread() );
            prop1 = new MemoryBound( ct, pCt, pedanticSearch ? 0.0 : ENERGY_GRANULARITY_THRESHOLD::MemoryBound );
            candProperties.push_back( prop1 );
#ifdef _WITH_ENOPT
            ct   = new Context( ( *p )->get_context()->getRegion(), ( *p )->get_context()->getRank(), ( *p )->get_context()->getThread() );
            pCt  = new Context( ( *p )->get_context()->getRegion(), ( *p )->get_context()->getRank(), ( *p )->get_context()->getThread() );
            prop = new EnergyInefficient( ct, pCt, pedanticSearch ? 0.0 : ENERGY_GRANULARITY_THRESHOLD::EnergyInefficiency );
            candProperties.push_back( prop );
#else
            psc_dbgmsg( 1, "EnergyGranularity: Energy Efficiency property is not enabled during compile-time\n" );
#endif      /* _WITH_ENOPT */
        }
        else {
            continue;
        }
    }


    //RM: Debug only, should be removed afterwards
    //if (psc_get_debug_level() >= 2)
    //  agent->print_property_set(candProperties,  "SET OF CANDIDATE PROPERTIES AGGREGATED FROM THE LAST STEP", true, true);

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
        if( ( *p )->id() == ENERGY_SUITED ) {
            if( ( *p )->get_region()->get_type() == CALL_REGION ) { // Handle call to subroutine
                if( checkedSubs.size() < ( *p )->get_context()->getRank() + 1 ) {
                    checkedSubs.resize( ( *p )->get_context()->getRank() +  1 );
                }
                Region* reg = appl->get_subroutine( ( *p )->get_region()->get_name().c_str() );
                if( reg != NULL &&
                    ( checkedSubs[ ( *p )->get_context()->getRank() ].find( reg ) == checkedSubs[ ( *p )->get_context()->getRank() ].end()
                      || !checkedSubs[ ( *p )->get_context()->getRank() ][ reg ] ) ) {
                    Context*  ct;
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
                        cout << "Refined to subroutine: " << ( *p )->get_region()->get_name() << endl;
                    }
                } // end handle not yet processed subroutine
            }
            else { // Handle nested regions
                list <Region*>           subRegions = ( *p )->get_region()->get_subregions();
                list <Region*>::iterator subr_it;


                //cout << "\n Subregions of " << (*p)->get_region()->str_print(appl->get_file_name_maping()) << "\n\n";
                //(*p)->get_region()->print_subregions("__", appl->get_file_name_maping());

                // if there are subregions that are no data structures
                psc_dbgmsg( 3, "Refined to subregions of: %s\n", ( *p )->get_region()->str_print().c_str() );
                for( subr_it = subRegions.begin(); subr_it != subRegions.end(); subr_it++ ) {
                    Context*  ct;
                    Property* prop;

                    switch( ( *subr_it )->get_type() ) {
                    case  DATA_STRUCTURE:
                        break;
//          case PARALLEL_REGION:
//          case DO_REGION:
//          case SECTIONS_REGION:
//          case WORKSHARE_REGION:
//            if (appl->getOmpThreads() > 1) {
//
//              for (int i = 0; i < appl->getOmpThreads(); i++) {
//                ct = new Context(*subr_it, (*p)->get_context()->getRank(), i);
//                prop = new StallCyclesProp(ct, new Context(phaseRegion, ct->getRank(), 0), PSC_BACK_END_BUBBLE_ALL);
//                candidates.push_back(prop);
//              }
//            break;

                    case CALL_REGION:
                        char routine_name[ 200 ];
                        strcpy( routine_name, ( ( *subr_it )->get_name() ).c_str() );
                        if( strncmp( routine_name, "MPI_", 4 ) == 0 || strncmp( routine_name, "mpi_", 4 ) == 0 ) {
                            break;
                        }
                    // Otherwise continue to default
                    default:
                        // Duplicate property for the region
                        // Insert in set of candidates
                        ct   = new Context( ( *subr_it ), ( *p )->get_context()->getRank(), ( *p )->get_context()->getThread() );
                        prop = ( *p )->clone();
                        prop->set_context( ct );
                        candProperties.push_back( prop );
                    }
                } // loop over subregions
            }     // No call site; handle nested regions
            //p = propsRefineRegionNesting.erase(p);
        }
        p = propsRefineRegionNesting.erase( p );
    }


    return candProperties;
};


string EnergyGranularity::name() {
    return "EnergyGranularityStrategy";
}
