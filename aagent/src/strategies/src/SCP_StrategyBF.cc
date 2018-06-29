/**
   @file    SCP_StrategyBF.cc
   @ingroup SCPStrategyBF
   @brief   Generic search strategy
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
#include "SCP_StrategyBF.h"
#ifdef _WITH_CROSS_PLATFORM_PROPS
  #include "L1MissRate.h"
  #include "L2MissRate.h"
  #include "L3MissRate.h"
  #include "TLBMissRate.h"
  #include "papi.h"
#endif
#ifdef __p575
  #include "p6HotSpot.h"
#endif

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

bool SCP_StrategyBF::reqAndConfigureFirstExperiment( Region* r ) {
// TRUE can start; FALSE not ready

    std::list<ApplProcess>           controlled_processes = dp->get_controlled_processes();
    std::list<ApplProcess>::iterator process;
    Prop_List::iterator              prop_it;
    Property*                        prop;

#ifdef _WITH_CROSS_PLATFORM_PROPS
    int retval = PAPI_library_init( PAPI_VER_CURRENT );

    if( retval != PAPI_VER_CURRENT ) {
        psc_dbgmsg( 5, "Could not init PAPI!\n" );
    }
#endif

    phaseRegion = r;

    //creating set of initial properties candidates for all process ranks
    create_initial_candidate_properties_set( phaseRegion );

    if( agent->get_leader() ) {
        psc_dbgmsg( 1, "Strategy Step %d\n", strategy_steps );
    }

    //calling candidate properties to request necessary metrics and transfer them to mrimonitor
    for( prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
        ( *prop_it )->request_metrics();
    }

    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( candProperties, "SET OF CANDIDATE PROPERTIES", false, false );
    }

    strategyIterations = 1;
    configureNextExperiment();

    if( agent->get_leader() ) {
        psc_dbgmsg( 5, "(SCP_StrategyBF::reqAndConfigureFirstExperiment) Strategy Step %d configured\n", strategy_steps );
    }
    return true;
}

void SCP_StrategyBF::configureNextExperiment() {
    dp->transfer_requests_to_processes_no_begin_end_requests();
    psc_dbgmsg( 5, "(SCP_StrategyBF::configureNextExperiment)Strategy Step %d, iteration %d, requests transfered to mrimonitor\n",
                strategy_steps, strategyIterations );
    strategyIterations++;
}

bool SCP_StrategyBF::evaluateAndReqNextExperiment() {
    // TRUE requires next step; FALSE if done
    Prop_List::iterator prop_it;

    if( dp->getResults() == ALL_INFO_GATHERED ) {
        //Information was measured. Now the properties can be evaluated.
        psc_dbgmsg( 5, "(SCP_StrategyBF::evaluateAndReqNextExperiment) Data Provider returned ALL_INFO_GATHERED, step %d, "
                    "iteration %d\n", strategy_steps, strategyIterations );
    }
    else {
        psc_dbgmsg( 5, "(SCP_StrategyBF::evaluateAndReqNextExperiment) Data Provider returned NOT_ALL_INFO_GATHERED, step %d"
                    ", iteration %d\n", strategy_steps, strategyIterations );
        for( prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
            ( *prop_it )->request_metrics();
        }
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
        psc_dbgmsg( 5, "(evaluateAndReqNextExperiment())Strategy Step %d, iteration %d, evaluated, candidate properties"
                    "created, data requested, moving to the next step...\n", strategy_steps, strategyIterations );
        if( psc_get_debug_level() >= 2 ) {
            agent->print_property_set( candProperties, "SET OF CANDIDATE PROPERTIES", false, false );
        }
        strategy_steps++;
        //strategyIterations = 0;
        return true;
    }
}

void SCP_StrategyBF::createTopProps( Region* region, int rank ) {
    int       threads = 1;
    Context*  ct;
    Property* prop;

    if( region->get_rra() == RUNS_AS_THREADED ) {
        threads = appl->getOmpThreads();
    }

    for( int i = 0; i < threads; i++ ) {
#ifdef __p575
        //prop = new p6HotSpot(new Context(region, rank, i),
        //                     new Context(phaseRegion, rank, i));
        //candProperties.push_back(prop);
#endif
#ifdef _WITH_CROSS_PLATFORM_PROPS

        /*in the first run check for for properties availability on the current hardware*/
        if( availProps.find( CPPL1MISS ) == availProps.end() ) {
            prop = new L1MissRate( new Context( region, rank, i ), new Context( phaseRegion, rank, i ), pedanticSearch ? 0.0 : CPPROPS_PROPS_THRESHOLD::L1MissRate );
            if( prop->request_metrics() == ALL_INFO_GATHERED ) {
                availProps[ CPPL1MISS ] = true;
            }
            else {
                availProps[ CPPL1MISS ] = false;
            }
            free( prop );
        }
        if( availProps.find( CPPL2MISS ) == availProps.end() ) {
            prop = new L2MissRate( new Context( region, rank, i ), new Context( phaseRegion, rank, i ), pedanticSearch ? 0.0 : CPPROPS_PROPS_THRESHOLD::L2MissRate );
            if( prop->request_metrics() == ALL_INFO_GATHERED ) {
                availProps[ CPPL2MISS ] = true;
            }
            else {
                availProps[ CPPL2MISS ] = false;
            }
            free( prop );
        }
        if( availProps.find( CPPL3MISS ) == availProps.end() ) {
            prop = new L3MissRate( new Context( region, rank, i ), new Context( phaseRegion, rank, i ), pedanticSearch ? 0.0 : CPPROPS_PROPS_THRESHOLD::L3MissRate );
            if( prop->request_metrics() == ALL_INFO_GATHERED ) {
                availProps[ CPPL3MISS ] = true;
            }
            else {
                availProps[ CPPL3MISS ] = false;
            }
            free( prop );
        }
        if( availProps.find( CPPTLBMISS ) == availProps.end() ) {
            prop = new TLBMissRate( new Context( region, rank, i ), new Context( phaseRegion, rank, i ), pedanticSearch ? 0.0 : CPPROPS_PROPS_THRESHOLD::TLBMissRate );
            if( prop->request_metrics() == ALL_INFO_GATHERED ) {
                availProps[ CPPTLBMISS ] = true;
            }
            else {
                availProps[ CPPTLBMISS ] = false;
            }
            free( prop );
        }

        /*in the further calls check the availability flag and instantiate if true*/
        if( availProps[ CPPL1MISS ] ) {
            prop = new L1MissRate( new Context( region, rank, i ), new Context( phaseRegion, rank, i ), pedanticSearch ? 0.0 : CPPROPS_PROPS_THRESHOLD::L1MissRate );
            candProperties.push_back( prop );
        }
        if( availProps[ CPPL2MISS ] ) {
            prop = new L2MissRate( new Context( region, rank, i ), new Context( phaseRegion, rank, i ), pedanticSearch ? 0.0 : CPPROPS_PROPS_THRESHOLD::L2MissRate );
            candProperties.push_back( prop );
        }
        if( availProps[ CPPL3MISS ] ) {
            prop = new L3MissRate( new Context( region, rank, i ), new Context( phaseRegion, rank, i ), pedanticSearch ? 0.0 : CPPROPS_PROPS_THRESHOLD::L3MissRate );
            candProperties.push_back( prop );
        }
        if( availProps[ CPPTLBMISS ] ) {
            prop = new TLBMissRate( new Context( region, rank, i ), new Context( phaseRegion, rank, i ), pedanticSearch ? 0.0 : CPPROPS_PROPS_THRESHOLD::TLBMissRate );
            candProperties.push_back( prop );
        }
#endif
    }
}

std::list<Property*> SCP_StrategyBF::create_initial_candidate_properties_set( Region* initial_region ) {
    std::list<ApplProcess>           controlled_processes = dp->get_controlled_processes();
    std::list<ApplProcess>::iterator process;

    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        std::list<Region*>::iterator checkedRegions_it;

        std::list<Region*> checkedRegions;
        std::list<Region*> childRegions = ( initial_region )->get_subregions();
        psc_dbgmsg( 7, "Phase region has %d subregions\n", childRegions.size() );
        std::list<Region*> tempListPointer;
        int                ref_iteration = 0;

        psc_dbgmsg( 7, "Starting the main do-while for rank %d\n", process->rank );
        do {
            //loop over the current set of properties to be refined
            tempListPointer = childRegions;
            childRegions    = checkedRegions;
            checkedRegions  = tempListPointer;
            childRegions.clear();

            psc_dbgmsg( 7, "childRegions has %d subregions\n", childRegions.size() );
            psc_dbgmsg( 7, "checkedRegions has %d subregions\n", checkedRegions.size() );
            psc_dbgmsg( 7, "Lists swap successful\n" );
            for( checkedRegions_it = ( checkedRegions ).begin(); checkedRegions_it != ( checkedRegions ).end();
                 checkedRegions_it++ ) {
                psc_dbgmsg( 7, "Creating initial set of properties for region %s\n", ( *checkedRegions_it )->get_name().c_str() );

                if( ( *checkedRegions_it )->get_type() == DATA_STRUCTURE ) {
                    psc_dbgmsg( 7, "get_type()==DATA_STRUCTURE\n" );
                }
                else {
                    createTopProps( ( *checkedRegions_it ), process->rank );
                }

                if( ( *checkedRegions_it )->get_type() == CALL_REGION ) {
                    //potential problem here!!! if we have 10^6 ranks, but this current AA controls only 5,
                    //we will have checkedSubs.size == 10^6!!!
                    if( checkedSubs.size() < process->rank + 1 ) {
                        psc_dbgmsg( 7, "Resizing to %d\n", process->rank + 1 );
                        checkedSubs.resize( process->rank + 1 );
                    }

                    Region* reg = appl->get_subroutine( ( *checkedRegions_it )->get_name().c_str() );

                    if( reg != NULL ) {
                        psc_dbgmsg( 7, "appl->get_subroutine: %d\n", ( *checkedRegions_it )->get_ident().start_position );
                        if( checkedSubs[ process->rank ][ reg ] ) {
                            psc_dbgmsg( 7, "Checked subs is true:)\n" );
                        }
                        else {
                            psc_dbgmsg( 7, "Checked subs is false:(\n" );
                        }
                    }
                    if( reg != NULL
                        && ( checkedSubs[ process->rank ].find( reg ) == checkedSubs[ process->rank ].end()
                             || !checkedSubs[ process->rank ][ reg ] ) ) {
                        if( reg->get_type() == DATA_STRUCTURE ) {
                            psc_dbgmsg( 7, "get_type()==DATA_STRUCTURE inside CALL_REGION\n" );
                        }
                        else {
                            childRegions.push_back( reg );
                        }

                        checkedSubs[ process->rank ][ reg ] = true;
                        psc_dbgmsg( 7, "Refined to subroutine: %s\n", reg->get_name().c_str() );
                    }
                }
                else {
                    std::list<Region*>           subRegions = ( *checkedRegions_it )->get_subregions();
                    std::list<Region*>::iterator subr_it;
                    psc_dbgmsg( 7, "Refinement of subregion: %s, subregions count %d\n", ( *checkedRegions_it )->str_print().c_str(),
                                subRegions.size() );

                    for( subr_it = subRegions.begin(); subr_it != subRegions.end(); subr_it++ ) {
                        switch( ( *subr_it )->get_type() ) {
                        case DATA_STRUCTURE:
                            break;
                        default:
                        {
                            childRegions.push_back( *subr_it );
                            psc_dbgmsg( 7, "Refined to subregion: %s\n", ( *subr_it )->str_print().c_str() );
                            break;
                        }
                        }
                    }
                }
            }
            ref_iteration++;
            psc_dbgmsg( 7, "Number of regions for rank %d on level %d is %d\n", process->rank, ref_iteration,
                        childRegions.size() );
        }
        while( ( childRegions ).size() != 0 );
    }
    return candProperties;
}

std::list<Property*> SCP_StrategyBF::create_next_candidate_properties_set( std::list<Property*> ev_set ) {
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

std::string SCP_StrategyBF::name() {
    return "SCP_StrategyBF";
}
