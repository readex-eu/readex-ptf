/**
   @file    EnergyGranularityBF.cc
   @ingroup EnergyGranularityBFStrategy
   @brief   Energy granularity breadth-first search strategy
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
#include "EnergyGranularityBF.h"
#include "application.h"
#include "Metric.h"
#include "strategy.h"
#include "PropertyID.h"
#include "psc_errmsg.h"
#include "SuitedForEnergyConfiguration.h"
#include "EnergyInefficient.h"
#include "MemoryBound.h"
#include "analysisagent.h"
#include <cstring>
#include <iostream>
#include <boost/foreach.hpp>




EnergyGranularityBF::EnergyGranularityBF( TuningSpecification* tuningSpec, bool pedantic ) :
    Strategy( pedantic ), phaseRegion( NULL ), spec( tuningSpec ) {
    if( !tuningSpec ) {
        psc_abort( "The tuning specification can not be NULL!\n" );
    }
}


void EnergyGranularityBF::preconfigureAnalysis() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ), "EnergyGranularity: call to preconfigureAnalysis\n" );

    BOOST_FOREACH( const ApplProcess &process, dp->get_controlled_processes() ) {
        if( !processInRanks( spec->getRanks(), process.rank ) ) {
            break;
        }
        if( spec->getTypeOfVariantContext() != REGION_LIST ) {
            break;
        }

        std::map<TuningParameter*, int> variants = spec->getVariant()->getValue();
        VariantContext                  context  = spec->getVariantContext();

        std::map<TuningParameter*, int>::const_iterator tuningVariant;
        for( tuningVariant = variants.begin(); tuningVariant != variants.end(); ++tuningVariant ) {
            if( tuningVariant->first->getRuntimeActionType() != TUNING_ACTION_FUNCTION_POINTER ) {
                psc_abort( "Un-implemented runtime action type in EnergyGranularityBF!\n" );
            }

            dp->addTuningRequest( process.rank, spec );
        }
    }
}


bool EnergyGranularityBF::reqAndConfigureFirstExperiment( Region* r ) {
    list<ApplProcess>           controlled_processes = dp->get_controlled_processes();
    list<Region*>               sub_regions;
    list<Region*>::iterator     subr_it;
    list<ApplProcess>::iterator process;
    Prop_List::iterator         prop_it;
    Property*                   prop;
    phaseRegion = r;
    process     = controlled_processes.begin();

    if( agent->get_leader() ) {
        psc_dbgmsg( 1, "Strategy Step %d\n", strategy_steps );
    }
    candProperties = create_initial_candidate_properties_set( r );

    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( candProperties, "SET OF CANDIDATE PROPERTIES", false, false );
    }
    strategy_steps++;

    return true;
}


std::list<Property*> EnergyGranularityBF::create_initial_candidate_properties_set( Region* phaseRegion ) {
    list<ApplProcess>           controlled_processes = dp->get_controlled_processes();
    list<Region*>               regions;
    list<Property*>             candidates;
    Property*                   prop;
    list<ApplProcess>::iterator process;
    list<Region*>::iterator     reg;

    propsRefineRegionNesting.clear();
    regions = appl->get_regions();
    for( reg = regions.begin(); reg != regions.end(); reg++ ) {
        for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
            Context* ct  = new Context( ( *reg ), process->rank, 0 );
            Context* pCt = new Context( phaseRegion, process->rank, 0 );
            prop = new SuitedForEnergyConfiguration( ct, pCt, pedanticSearch ? 0.0 : ENERGY_GRANULARITY_THRESHOLD::RAPLGranularity );
            candidates.push_back( prop );
        }
    }

    return candidates;
}


void EnergyGranularityBF::configureNextExperiment() {
    if( !dp->areOldRequestsPending() ) {
        Prop_List::iterator prop_it;

        for( prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
            ( *prop_it )->request_metrics();
        }

        preconfigureAnalysis();
    }

    dp->transfer_requests_to_processes();
}

bool EnergyGranularityBF::evaluateAndReqNextExperiment() {
    list<ApplProcess>::iterator process;
    list<ApplProcess>           controlled_processes = dp->get_controlled_processes();

    //Evaluate candidate properties
    for( Prop_List::iterator prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
        ( *prop_it )->evaluate();
        if( ( *prop_it )->condition() ) {
            foundPropertiesLastStep.push_back( *prop_it );
            foundProperties.push_back( *prop_it );
        }
    }

    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( foundPropertiesLastStep, "SET OF FOUND PROPERTIES", true, true );
    }

    if( strategy_steps > 2 ) {
        return false;
    }


    //Determine new candidate properties
    candProperties.clear();
    candProperties = create_next_candidate_properties_set( foundPropertiesLastStep );

    for( Prop_List::iterator prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
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


std::list<Property*> EnergyGranularityBF::create_next_candidate_properties_set( list<Property*> foundProperties ) {
    std::list<Property*>           candidates;
    std::list<Property*>::iterator prop_it;
    std::list<ApplProcess>         controlled_processes = dp->get_controlled_processes();

    if( agent->get_leader() ) {
        psc_dbgmsg( 1, "Strategy Step %d\n", strategy_steps );
    }
    if( strategy_steps >= max_strategy_steps ) {
        return candidates;         //empty
    }
    for( prop_it = foundProperties.begin(); prop_it != foundProperties.end(); prop_it++ ) {
        Property* p = ( *prop_it );
        Property* p1;
        Context*  ct  = new Context( p->get_context()->getRegion(), p->get_context()->getRank(), p->get_context()->getThread() );
        Context*  pCt = new Context( p->get_context()->getRegion(), p->get_context()->getRank(), p->get_context()->getThread() );

        p1 = new MemoryBound( ct, pCt, pedanticSearch ? 0.0 : ENERGY_GRANULARITY_THRESHOLD::MemoryBound );
        candidates.push_back( p1 );
        ct  = new Context( p->get_context()->getRegion(), p->get_context()->getRank(), p->get_context()->getThread() );
        pCt = new Context( p->get_context()->getRegion(), p->get_context()->getRank(), p->get_context()->getThread() );
        p1  = new EnergyInefficient( ct, pCt, pedanticSearch ? 0.0 : ENERGY_GRANULARITY_THRESHOLD::EnergyInefficiency );
        candidates.push_back( p1 );
    }
    return candidates;
}


std::string EnergyGranularityBF::name() {
    return "EnergyGranularityBFStrategy";
}
