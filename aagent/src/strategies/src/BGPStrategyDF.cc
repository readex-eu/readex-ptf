/**
   @file    BGPStrategyDF.cc
   @ingroup BGPStrategyDF
   @brief   Power6 breadth-first search strategy
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
#include "BGPStrategyDF.h"
#include "LC2DMissRate.h"


#include <iostream>
#include "psc_errmsg.h"
#include <analysisagent.h>
//#include "../mrimonitor/return_summary_data.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <cmath>

#include <strings.h>

bool BGPStrategyDF::reqAndConfigureFirstExperiment( Region* r ) {
// TRUE can start; FALSE not ready

    std::list<ApplProcess>           controlled_processes = dp->get_controlled_processes();
    std::list<ApplProcess>::iterator process;
    Prop_List::iterator              prop_it;
    Property*                        prop;

    phaseRegion = r;
    psc_dbgmsg( 5, "Using Phase region %s, file %d rfl %d \n", phaseRegion->get_name().c_str(),
                phaseRegion->get_ident().file_id, phaseRegion->get_ident().rfl );

    //creating set of initial properties candidates for all process ranks
    create_initial_candidate_properties_set( phaseRegion );

    if( agent->get_leader() ) {
        psc_dbgmsg( 1, "Strategy Step %d\n", strategy_steps );
    }

    //calling candidate properties to request necessary metrics and transfer them to mrimonitor
    for( prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
        ( *prop_it )->request_metrics();
    }

    strategyIterations = 1;
    configureNextExperiment();

    if( agent->get_leader() ) {
        psc_dbgmsg( 5, "(BGPStrategyDF::reqAndConfigureFirstExperiment) Strategy Step %d configured\n", strategy_steps );
    }
    return true;
}

void BGPStrategyDF::configureNextExperiment() {
    dp->transfer_requests_to_processes_no_begin_end_requests();
    psc_dbgmsg( 5, "(BGPStrategyDF::configureNextExperiment)Strategy Step %d, iteration %d, requests transfered to mrimonitor\n",
                strategy_steps, strategyIterations );
    strategyIterations++;
}

bool BGPStrategyDF::evaluateAndReqNextExperiment() {
    // TRUE requires next step; FALSE if done
    Prop_List::iterator prop_it;

    //check iterations limit reached
    if( strategyIterations > max_strategy_steps ) {
        return false;
    }

    if( dp->getResults() == ALL_INFO_GATHERED ) {
        //Information was measured. Now the properties can be evaluated.
        psc_dbgmsg( 5, "(BGPStrategyDF::evaluateAndReqNextExperiment) Data Provider returned ALL_INFO_GATHERED, step %d, iteration %d\n",
                    strategy_steps, strategyIterations );
    }
    else {
        psc_dbgmsg( 5, "(BGPStrategyDF::evaluateAndReqNextExperiment) Data Provider returned NOT_ALL_INFO_GATHERED, step %d, iteration %d\n",
                    strategy_steps, strategyIterations );
        return true;
    }

    double max_severity = 0;
    psc_dbgmsg( 2, "size of candidate properties %d\n", candProperties.size() );
    foundPropertiesLastStep.clear();
    for( prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
        ( *prop_it )->evaluate();

        //compute maximum of the severities
        if( ( *prop_it )->severity() > max_severity ) {
            max_severity = ( *prop_it )->severity();
        }

        if( ( *prop_it )->condition() ) {
            psc_dbgmsg( 11, "Found property %s\n", ( *prop_it )->name().c_str() );
            foundPropertiesLastStep.push_back( *prop_it );
            foundProperties.push_back( *prop_it );
        }
    }

    candProperties.clear();

    candProperties = create_next_candidate_properties_set( foundPropertiesLastStep );

    psc_dbgmsg( 2, "size of found list this step %d\n", foundPropertiesLastStep.size() );
    psc_dbgmsg( 2, "size of total found list %d\n", foundProperties.size() );
    psc_dbgmsg( 2, "maximum detected severity was %f\n", max_severity );

    for( prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
        ( *prop_it )->request_metrics();
    }

    if( candProperties.size() == 0 ) {
        //if (psc_get_debug_level() >= 2)
        //  agent->print_property_set(foundProperties,  "SET OF FINAL PROPERTIES", true,false);
        return false;
    }
    else {
        psc_dbgmsg( 5, "(evaluateAndReqNextExperiment())Strategy Step %d, iteration %d, evaluated, candidate properties created, data requested, moving to the next step...\n",
                    strategy_steps, strategyIterations );

        strategy_steps++;

        return true;
    }
}

void BGPStrategyDF::createTopProps( Region* region, int rank ) {
    int       threads = 1;
    Context*  ct;
    Property* prop;

    if( region->get_rra() == RUNS_AS_THREADED ) {
        threads = appl->getOmpThreads();
    }

    for( int i = 0; i < threads; i++ ) {
        prop = new LC2DMissRateProp( new Context( region, rank, i ),
                                     pedanticSearch ? 0.0 : BGP_PROPS_THRESHOLD::LC2DMissRateProp );
        candProperties.push_back( prop );
    }
}

std::list<Property*> BGPStrategyDF::create_initial_candidate_properties_set( Region* initial_region ) {
    std::list<ApplProcess>           controlled_processes = dp->get_controlled_processes();
    std::list<ApplProcess>::iterator process;

    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        std::list<Region*>::iterator checkedRegions_it;

        std::list<Region*> checkedRegions;
        //std::list <Region*> childRegions = (initial_region)->get_subregions();
        //psc_dbgmsg(7, "Phase region has %d subregions\n", childRegions.size());

        //
        std::list<Region*> childRegions;
        childRegions.push_back( initial_region );
        psc_dbgmsg( 11, "Starting with %d regions\n", childRegions.size() );

        std::list<Region*> tempListPointer;
        int                ref_iteration = 0;

        psc_dbgmsg( 11, "Starting the main do-while for rank %d\n", process->rank );
        do {
            //loop over the current set of properties to be refined
            tempListPointer = childRegions;
            childRegions    = checkedRegions;
            checkedRegions  = tempListPointer;
            childRegions.clear();

            psc_dbgmsg( 11, "childRegions has %d subregions\n", childRegions.size() );
            psc_dbgmsg( 11, "checkedRegions has %d subregions\n", checkedRegions.size() );
            psc_dbgmsg( 11, "Lists swap successful\n" );
            for( checkedRegions_it = ( checkedRegions ).begin(); checkedRegions_it != ( checkedRegions ).end();
                 checkedRegions_it++ ) {
                psc_dbgmsg( 11, "Creating initial set of properties for region %s\n", ( *checkedRegions_it )->get_name().c_str() );

                if( ( *checkedRegions_it )->get_type() == DATA_STRUCTURE ) {
                    psc_dbgmsg( 11, "get_type()==DATA_STRUCTURE\n" );
                }
                else {
                    createTopProps( ( *checkedRegions_it ), process->rank );
                }

                if( ( *checkedRegions_it )->get_type() == CALL_REGION ) {
                    //potential problem here!!! if we have 10^6 ranks, but this current AA controls only 5,
                    //we will have checkedSubs.size == 10^6!!!
                    if( checkedSubs.size() < process->rank + 1 ) {
                        psc_dbgmsg( 11, "Resizing to %d\n", process->rank + 1 );
                        checkedSubs.resize( process->rank + 1 );
                    }

                    Region* reg = appl->get_subroutine( ( *checkedRegions_it )->get_name().c_str() );

                    if( reg != NULL ) {
                        psc_dbgmsg( 11, "appl->get_subroutine: %d\n", ( *checkedRegions_it )->get_ident().start_position );
                        if( checkedSubs[ process->rank ][ reg ] ) {
                            psc_dbgmsg( 11, "Checked subs is true:)\n" );
                        }
                        else {
                            psc_dbgmsg( 11, "Checked subs is false:(\n" );
                        }
                    }
                    if( reg != NULL
                        && ( checkedSubs[ process->rank ].find( reg ) == checkedSubs[ process->rank ].end()
                             || !checkedSubs[ process->rank ][ reg ] ) ) {
                        if( reg->get_type() == DATA_STRUCTURE ) {
                            psc_dbgmsg( 1, "get_type()==DATA_STRUCTURE inside CALL_REGION\n" );
                        }
                        else {
                            childRegions.push_back( reg );
                        }

                        checkedSubs[ process->rank ][ reg ] = true;
                        psc_dbgmsg( 11, "Refined to subroutine: %s\n", reg->get_name().c_str() );
                    }
                }
                else {
                    std::list<Region*>           subRegions = ( *checkedRegions_it )->get_subregions();
                    std::list<Region*>::iterator subr_it;
                    psc_dbgmsg( 11, "Refinement of subregion: %s, subregions count %d\n",
                                ( *checkedRegions_it )->str_print().c_str(), subRegions.size() );

                    for( subr_it = subRegions.begin(); subr_it != subRegions.end(); subr_it++ ) {
                        switch( ( *subr_it )->get_type() ) {
                        case DATA_STRUCTURE:
                            break;
                        default:
                        {
                            childRegions.push_back( *subr_it );
                            psc_dbgmsg( 11, "Refined to subregion: %s\n", ( *subr_it )->str_print().c_str() );
                            break;
                        }
                        }
                    }
                }
            }
            ref_iteration++;
            psc_dbgmsg( 11, "Number of regions for rank %d on level %d is %d\n", process->rank, ref_iteration,
                        childRegions.size() );
        }
        while( ( childRegions ).size() != 0 );
    }
    return candProperties;
}

std::list<Property*> BGPStrategyDF::create_next_candidate_properties_set( std::list<Property*> ev_set ) {
    Prop_List           returnList, childProp;
    Prop_List::iterator prop_it, prop_it2;

    for( prop_it = ev_set.begin(); prop_it != ev_set.end(); prop_it++ ) {
        childProp = ( *prop_it )->next();
        for( prop_it2 = childProp.begin(); prop_it2 != childProp.end(); prop_it2++ ) {
            returnList.push_back( ( *prop_it2 ) );
        }
    }

    return returnList;
}

std::string BGPStrategyDF::name() {
    std::stringstream stream;
    stream << " BlueGene P cache inefficiency search strategy (demonstration)";
    return stream.str();
}
