/**
   @file    RegionNestingStrategy.cc
   @ingroup RegionNestingStrategy
   @brief   Nesting of regions search strategy
   @author  Edmond Kereku
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
#include "RegionNestingStrategy.h"
#include <iostream>

RegionNestingStrategy::RegionNestingStrategy( Property* prop ) : Strategy() {
    property = prop;
}

RegionNestingStrategy::~RegionNestingStrategy() {
}

std::list<Property*> RegionNestingStrategy::create_initial_candidate_properties_set( Region* initial_region ) {
    Property*          prop;
    std::list<Region*> sub_regions;
    //Region *phase_region;
    std::list<Region*>::iterator subr_it;
    std::list<Property*>         initial_candidate_properties_set;
    std::list<ApplProcess>       controlled_processes = dp->get_controlled_processes();

    //vizualization
    //std::cout << "\n The initial region's tree: \n\n";
    //initial_region->print_subregions("__" , application->get_file_name_maping());

    sub_regions = initial_region->get_subregions();

    //all the childrens (subregions) of application's root (main) will be checked for high cache misses
    for( subr_it = sub_regions.begin(); subr_it != sub_regions.end(); subr_it++ ) {
        //don't include the variables
        if( ( *subr_it )->get_type() != VECTOR_REGION && ( *subr_it )->get_type() != DATA_STRUCTURE ) {
            std::list<ApplProcess>::iterator process;
            for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
                //Create a new PropertyContext and connect it with the region
                Context* ct = new Context( *subr_it, process->rank, 0 );
                //Create a property and ad it to the set
                prop = property->clone();
                prop->set_context( ct );
                initial_candidate_properties_set.push_back( prop );
            }
        }
    }
    strategy_steps++;
    return initial_candidate_properties_set;
}

std::list<Property*> RegionNestingStrategy::create_next_candidate_properties_set( std::list<Property*> ev_set ) {
    Property*                      prop;
    std::list<Region*>             sub_regions;
    std::list<Region*>::iterator   subr_it;
    std::list<Property*>           next_candidate_properties_set;
    std::list<Property*>::iterator prop_it;
    std::list<ApplProcess>         controlled_processes = dp->get_controlled_processes();

    std::cerr << std::endl << std::endl << "Strategy Step Nr: " << strategy_steps << std::endl;

    //abort criterion
    if( strategy_steps >= max_strategy_steps ) {
        return next_candidate_properties_set; //empty
    }
    //1st step: analyze the evaluated set

    //If the evaluated set is not empty, push all the evaluated props in the evaluated_queue.
    if( !ev_set.empty() ) {
        for( prop_it = ev_set.begin(); prop_it != ev_set.end(); prop_it++ ) {
            Region* reg = ( *prop_it )->get_region();
            //For all found "non DS" regions, save the property in the call_queue
            //if the region is a CALL region or in the evaluated_queue otherwise
            switch( reg->get_type() ) {
            case CALL_REGION:
            case DATA_STRUCTURE:     //Normally shouldn't be one of them, but anyway...
            case VECTOR_REGION:
                break;
            default:
                evaluated_queue.push_back( *prop_it );
                break;
            }
        }
    }

    //2nd step: Look for subregions of the evaluated regions

    //Get a property from the evaluated_queue if it"s not empty
    //and check it's subregions.
//  while (next_candidate_properties_set.empty() && !(evaluated_queue.empty()))
    while( !( evaluated_queue.empty() ) ) {
        //Get a property from the evaluated_queue
        Property* ev = evaluated_queue.front();
        evaluated_queue.pop_front();

        //Get its context, and the region associated with it
        Context* ct   = ev->get_context();
        Region*  reg  = ct->getRegion();
        int      rank = ct->getRank();

        //Get the subregion list
        sub_regions = reg->get_subregions();

        //look first if there are subregions that are no data structures
        for( subr_it = sub_regions.begin(); subr_it != sub_regions.end(); subr_it++ ) {
            if( ( *subr_it )->get_type() != VECTOR_REGION && ( *subr_it )->get_type() != DATA_STRUCTURE ) {
                std::list<ApplProcess>::iterator process;

                //Create a new PropertyContext and connect it with the region
                Context* new_ct = new Context( *subr_it, rank, 0 );

                //Create a property and ad it to the set
                prop = property->clone();
                prop->set_context( new_ct );
                next_candidate_properties_set.push_back( prop );
            }
        }
    }

    //3d step: look for the called subroutines
    strategy_steps++;

    return next_candidate_properties_set;
}

std::string RegionNestingStrategy::name() {
    return "RegionNestingStrategy";
}

bool RegionNestingStrategy::subroutine_already_checked( Region* reg ) {
    std::list<Region*>::iterator reg_it;

    for( reg_it = checked_subroutines.begin(); reg_it != checked_subroutines.end(); reg_it++ ) {
        if( ( *reg_it ) == reg ) {
            return true;
        }
    }

    checked_subroutines.push_back( reg );
    return false;
}
