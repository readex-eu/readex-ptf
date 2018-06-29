/**
   @file    PipelineStrategy.cc
   @ingroup PipelineStrategy
   @brief   Pipeline search strategy
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
#include "psc_errmsg.h"
#include "analysisagent.h"
#include "PipelineStrategy.h"
#include "PipelineStageExecutionTime.h"
#include "PipelineExecutionTime.h"
#include "strategy.h"

bool PipelineStrategy::reqAndConfigureFirstExperiment( Region* r ) {
    ; // TRUE can start; FALSE not ready
    std::list <ApplProcess>           controlled_processes = dp->get_controlled_processes();
    std::list <Region*>               sub_regions;
    std::list <Region*>::iterator     subr_it;
    std::list <ApplProcess>::iterator process;
    Prop_List::iterator               prop_it;

    //vizualization
    //std::cout << "\n The initial region's tree: \n\n";
    //initial_region->print_subregions("__", application->get_file_name_maping());

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

void PipelineStrategy::configureNextExperiment() {
    Prop_List::iterator prop_it;


    //for(prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++)
    //  (*prop_it)->get_required_info();

    dp->transfer_requests_to_processes_no_begin_end_requests();
}


bool PipelineStrategy::evaluateAndReqNextExperiment() {
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


std::list <Property*> PipelineStrategy::create_initial_candidate_properties_set( Region* r ) {
    std::list <ApplProcess>           controlled_processes = dp->get_controlled_processes();
    std::list <Region*>               regions;
    std::list <Property*>             candidates;
    Property*                         prop;
    Property*                         prop1;
    std::list <ApplProcess>::iterator process;
    std::list<Region*>::iterator      reg;

    phaseRegion = r;
    propsRefineRegionNesting.clear();
    regions = appl->get_regions();



    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
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
                Context* ct, * pCt;

                ct  = new Context( ( *reg ), process->rank, i );
                pCt = new Context( phaseRegion, process->rank, 0 );

                switch( ( *reg )->get_type() ) {
                case VIE_PIPE_INBUFFER_REGION:
                case VIE_PIPE_OUTBUFFER_REGION:
                case VIE_PIPE_STAGE_REGION:
                    prop = new PipelineStageExecutionTime( ct, pCt );
                    break;
                case VIE_PIPELINE_REGION:
                    prop = new PipelineExecutionTime( ct, pCt );
                    break;
                default:
                    break;
                }
                candidates.push_back( prop );
            }
        }
    }

    return candidates;
};


std::list <Property*> PipelineStrategy::create_next_candidate_properties_set( std::list< Property* > foundProperties ) {
    std::list <Property*>           candidates;
    std::list <Property*>::iterator p;
    std::list <ApplProcess>         controlled_processes = dp->get_controlled_processes();

    //if (psc_get_debug_level() >= 3)
    //std::cout << "Strategy Step Nr: " << strategy_steps << std::endl;
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
};


std::string PipelineStrategy::name() {
    return "PipelineStrategy";
}
