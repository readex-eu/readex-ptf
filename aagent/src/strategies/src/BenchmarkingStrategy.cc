/**
   @file    BenchmarkingStrategy.cc
   @ingroup BenchmarkingStrategy
   @brief   Benchmarking strategy
   @author  Diana Gudu
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
#include "BenchmarkingStrategy.h"
#include "BenchmarkingAllProps.h"

// ---- Non-generic NP ----------------------------
#include "AddressAliasing.h"
#include "DataCache.h"
#include "DTLB_Misses.h"
#include "ExecutionStall.h"
#include "InstructionStarvation.h"
#include "LongLatencyInstructionException.h"
#include "Misprediction.h"
// ------------------------------------------------

// ----- Generic NP ------------------------
#include "BranchMisprediction.h"
#include "L1D.h"
#include "L2Misses.h"
// -----------------------------------------

#include <iostream>
#include "psc_errmsg.h"
#include <analysisagent.h>
#include "return_summary_data.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <cmath>

#ifdef __p575
        #include <strings.h>
#endif

bool BenchmarkingStrategy::reqAndConfigureFirstExperiment( Region* r ) {
// TRUE can start; FALSE not ready

    std::list <ApplProcess>           controlled_processes = dp->get_controlled_processes();
    std::list <ApplProcess>::iterator process;
    Prop_List::iterator               prop_it;
    Property*                         prop;

    phaseRegion = r;
    psc_dbgmsg( 5, "Using Phase region %s, file %d rfl %d \n", phaseRegion->get_name().c_str(),
                phaseRegion->get_ident().file_id, phaseRegion->get_ident().rfl );

    //creating set of initial properties candidates for all process ranks
    create_initial_candidate_properties_set( phaseRegion );


    if( agent->get_leader() ) {
        psc_dbgmsg( 1, "Strategy Step %d\n", strategy_steps );
    }


    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( candProperties, "SET OF CANDIDATE PROPERTIES", false, false );
    }

    //calling candidate properties to request necessary metrics and transfer them to mrimonitor
    for( prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
        ( *prop_it )->request_metrics();
    }


    strategyIterations = 1;
    //configureNextExperiment();

    if( agent->get_leader() ) {
        psc_dbgmsg( 5, "(BenchmarkingStrategy::reqAndConfigureFirstExperiment) Strategy Step %d configured\n", strategy_steps );
    }
    return true;
}

void BenchmarkingStrategy::configureNextExperiment() {
    printf( "BenchmarkingStrategy, no of candidate props: %zu\n", candProperties.size() );
    dp->transfer_requests_to_processes_no_begin_end_requests();
    psc_dbgmsg( 5, "(BenchmarkingStrategy::configureNextExperiment)Strategy Step %d, iteration %d, requests transfered to mrimonitor\n", strategy_steps, strategyIterations );
    strategyIterations++;
}


bool BenchmarkingStrategy::evaluateAndReqNextExperiment() {
    // TRUE requires next step; FALSE if done
    Prop_List::iterator prop_it;

    //check iterations limit reached
    if( strategyIterations > max_strategy_steps ) {
        return false;
    }

    if( dp->getResults() == ALL_INFO_GATHERED ) {
        //Information was measured. Now the properties can be evaluated.
        psc_dbgmsg( 5, "(BenchmarkingStrategy::evaluateAndReqNextExperiment) Data Provider returned ALL_INFO_GATHERED, step %d, iteration %d\n", strategy_steps, strategyIterations );
    }
    else {
        psc_dbgmsg( 5, "(BenchmarkingStrategy::evaluateAndReqNextExperiment) Data Provider returned NOT_ALL_INFO_GATHERED, step %d, iteration %d\n", strategy_steps, strategyIterations );
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
        psc_dbgmsg( 5, "(evaluateAndReqNextExperiment())Strategy Step %d, iteration %d, evaluated, candidate properties created, data requested, moving to the next step...\n", strategy_steps, strategyIterations );
        if( psc_get_debug_level() >= 2 ) {
            agent->print_property_set( candProperties, "SET OF CANDIDATE PROPERTIES", false, false );
        }
        strategy_steps++;
        //strategyIterations = 0;
        return true;
    }
}


void BenchmarkingStrategy::restartSearchStep() {
    Prop_List::iterator prop_it;
    candProperties.clear();
    pdb->clean();
    dp->cleanRequests();

    candProperties = create_next_candidate_properties_set( foundPropertiesLastStep );

    for( prop_it = candProperties.begin(); prop_it != candProperties.end(); prop_it++ ) {
        ( *prop_it )->request_metrics();
    }

    if( candProperties.size() == 0 ) {
        if( psc_get_debug_level() >= 2 ) {
            agent->print_property_set( foundProperties, "SET OF FINAL PROPERTIES", true, false );
        }
    }
    else {
        psc_dbgmsg( 5, "Restarting Strategy Step %d! properties created, data requested, repeating search...\n", strategy_steps, strategyIterations );
        if( psc_get_debug_level() >= 2 ) {
            agent->print_property_set( candProperties, "SET OF CANDIDATE PROPERTIES", false, false );
        }
        //strategy_steps++;
        //strategyIterations=0;
    }
}

void BenchmarkingStrategy::createTopProps( Context* ct, Context* phaseCt ) {
    int       threads = 1;
    Property* prop;

    prop = new BenchmarkingAllProps( ct, phaseCt, pedanticSearch ? 0.0 : BENCHMARKING_PROPS_THRESHOLD::BenchmarkingAllProps );
    candProperties.push_back( prop );

    prop = new BranchMisprediction( ct, phaseCt, pedanticSearch ? 0.0 : BENCHMARKING_PROPS_THRESHOLD::BranchMisprediction );
    candProperties.push_back( prop );

    prop = new L1D( ct, phaseCt, pedanticSearch ? 0.0 : BENCHMARKING_PROPS_THRESHOLD::L1D );
    candProperties.push_back( prop );

    prop = new L2Misses( ct, phaseCt, pedanticSearch ? 0.0 : BENCHMARKING_PROPS_THRESHOLD::L2Misses );
    candProperties.push_back( prop );

    prop = new AddressAliasing( ct, phaseCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::AddressAliasing );
    candProperties.push_back( prop );

    prop = new DataCache( ct, phaseCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::DataCache );
    candProperties.push_back( prop );

    prop = new DTLB_Misses( ct, phaseCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::DTLB_Misses );
    candProperties.push_back( prop );

    prop = new ExecutionStall( ct, phaseCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::ExecutionStall );
    candProperties.push_back( prop );

    prop = new InstructionStarvation( ct, phaseCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::InstructionStarvation );
    candProperties.push_back( prop );


    prop = new LongLatencyInstructionException( ct, phaseCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::LongLatencyInstructionException );
    candProperties.push_back( prop );

    prop = new Misprediction( ct, phaseCt, pedanticSearch ? 0.0 : WESTMERE_PROPS_THRESHOLD::Misprediction );
    candProperties.push_back( prop );
}


std::list <Property*> BenchmarkingStrategy::create_initial_candidate_properties_set( Region* r ) {
    std::list <ApplProcess> controlled_processes = dp->get_controlled_processes();
    std::list <Region*>     regions;
    std::list <Property*>   candidates;

    std::list <ApplProcess>::iterator process;
    std::list<Region*>::iterator      reg;

    phaseRegion = r;
    propsRefineRegionNesting.clear();
    regions = appl->get_regions();
    //regions.push_back(phaseRegion);


    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        Context* pCt = new Context( phaseRegion, process->rank, 0 );
        for( reg = regions.begin(); reg != regions.end(); reg++ ) {
            int  threads = 1;
            char routine_name[ 200 ];
            strcpy( routine_name, ( ( *reg )->get_name() ).c_str() );
            if( ( *reg )->get_type() == CALL_REGION && strncmp( routine_name, "MPI", 3 ) == 0 ) {
                continue;
            }
            if( ( *reg )->get_type() == DATA_STRUCTURE ) {
                continue;
            }


            if( ( *reg )->get_rra() == RUNS_AS_THREADED ) {
                threads = appl->getOmpThreads();
            }
            for( int i = 0; i < appl->getOmpThreads(); i++ ) {
                Context* ct = new Context( ( *reg ), process->rank, i );
                createTopProps( ct, pCt );
            }
        }
    }

    return candidates;
};

/*std::list< Property* > BenchmarkingStrategy::create_initial_candidate_properties_set(Region *initial_region) {
   std::list <ApplProcess> controlled_processes = dp->get_controlled_processes();
   std::list <ApplProcess>::iterator process;

   for (process = controlled_processes.begin(); process != controlled_processes.end(); process++) {

    std::list <Region*>::iterator checkedRegions_it;

    std::list <Region*> checkedRegions;
    //std::list <Region*> childRegions = (initial_region)->get_subregions();
    //psc_dbgmsg(7, "Phase region has %d subregions\n", childRegions.size());

    //
    std::list <Region*> childRegions;
    childRegions.push_back(initial_region);
    psc_dbgmsg(7, "Starting with %d regions\n", childRegions.size());


    std::list <Region*> tempListPointer;
    int ref_iteration = 0;

    psc_dbgmsg(7, "Starting the main do-while for rank %d\n", process->rank);
    do {
      //loop over the current set of properties to be refined
      tempListPointer = childRegions;
      childRegions = checkedRegions;
      checkedRegions = tempListPointer;
      childRegions.clear();

      psc_dbgmsg(7, "childRegions has %d subregions\n", childRegions.size());
      psc_dbgmsg(7, "checkedRegions has %d subregions\n", checkedRegions.size());
      psc_dbgmsg(7, "Lists swap successful\n");
      for (checkedRegions_it = (checkedRegions).begin(); checkedRegions_it != (checkedRegions).end(); checkedRegions_it++) {

        psc_dbgmsg(7, "Creating initial set of properties for region %s\n", (*checkedRegions_it)->get_name().c_str());

        if ((*checkedRegions_it)->get_type() == DATA_STRUCTURE)
          psc_dbgmsg(7, "get_type() == DATA_STRUCTURE\n");
        else
          createTopProps((*checkedRegions_it), process->rank);

        if ((*checkedRegions_it)->get_type() == CALL_REGION) {
          //potential problem here!!! if we have 10^6 ranks, but this current AA controls only 5,
          //we will have checkedSubs.size == 10^6!!!
          if (checkedSubs.size() < process->rank + 1) {
            psc_dbgmsg(7, "Resizing to %d\n", process->rank + 1);
            checkedSubs.resize(process->rank + 1);
          }

          Region* reg = appl->get_subroutine((*checkedRegions_it)->get_name().c_str());

          if (reg != NULL) {
            psc_dbgmsg(7, "appl->get_subroutine: %d\n", (*checkedRegions_it)->get_ident().start_position);
            if(checkedSubs[process->rank][reg])
              psc_dbgmsg(7, "Checked subs is true:)\n");
            else
              psc_dbgmsg(7, "Checked subs is false:(\n");
          }
          if (reg != NULL &&
              (checkedSubs[process->rank].find(reg)==checkedSubs[process->rank].end() ||
              !checkedSubs[process->rank][reg])) {

            if (reg->get_type() == DATA_STRUCTURE)
              psc_dbgmsg(7, "get_type() == DATA_STRUCTURE inside CALL_REGION\n");
            else
              childRegions.push_back(reg);

            checkedSubs[process->rank][reg] = true;
            psc_dbgmsg(7, "Refined to subroutine: %s\n", reg->get_name().c_str());
          }
        } else {
          std::list <Region*> subRegions = (*checkedRegions_it)->get_subregions();
          std::list <Region*>::iterator subr_it;
          psc_dbgmsg(7, "Refinement of subregion: %s, subregions count %d\n",
                     (*checkedRegions_it)->str_print().c_str(), subRegions.size());

          for(subr_it = subRegions.begin(); subr_it != subRegions.end(); subr_it++) {
            switch ((*subr_it)->get_type()) {
            case  DATA_STRUCTURE:
              break;
            default:
              childRegions.push_back(*subr_it);
              psc_dbgmsg(7, "Refined to subregion: %s\n", (*subr_it)->str_print().c_str());
              break;
            }
          }
        }
      }
      ref_iteration++;
      psc_dbgmsg(7, "Number of regions for rank %d on level %d is %d\n",
                 process->rank, ref_iteration, childRegions.size());
    } while ((childRegions).size() != 0);
   }
   return candProperties;
   }*/

std::list< Property* > BenchmarkingStrategy::create_next_candidate_properties_set( std::list< Property* > ev_set ) {
    Prop_List           returnList, childProp;
    Prop_List::iterator prop_it, prop_it2;
    std::list <Region*> regions;


    for( prop_it = ev_set.begin(); prop_it != ev_set.end(); prop_it++ ) {
        childProp = ( *prop_it )->next();
        for( prop_it2 = childProp.begin(); prop_it2 != childProp.end(); prop_it2++ ) {
            returnList.push_back( ( *prop_it2 ) );
        }
    }

    return returnList;
}

std::string BenchmarkingStrategy::name() {
    std::stringstream stream;
    stream << " Benchmarking Strategy";
    return stream.str();
}
