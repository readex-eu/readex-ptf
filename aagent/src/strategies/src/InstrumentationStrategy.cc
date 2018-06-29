/**
   @file    InstrumentationStrategy.cc
   @ingroup InstrumentationStrategy
   @brief   Instrumentation Strategy
   @author  Haseeb Zia
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
#include "InstrumentationStrategy.h"
#include "return_summary_data.h"
#include "psc_errmsg.h"
#include "OverheadProp.h"
#include "RequiredRegionProp.h"
#include "selective_debug.h"
#include <sstream>
#include <cstring>

InstrumentationStrategy::InstrumentationStrategy( std::string strategy, bool pedantic ) :
    Strategy( pedantic ) {
    profiling_done = true;
    if( strcmp( strategy.c_str(), "overhead" ) == 0 ) {
        inst_type      = OVERHEAD_BASED;
        profiling_done = false;
    }
    if( strcmp( strategy.c_str(), "all_overhead" ) == 0 ) {
        inst_type      = ALL_OVERHEAD_BASED;
        profiling_done = false;
    }
    if( strcmp( strategy.c_str(), "analysis" ) == 0 ) {
        inst_type      = ANALYSIS_BASED;
        profiling_done = false;
    }
    set_require_restart( true );
    region_cnt = 0;
}

InstrumentationStrategy::~InstrumentationStrategy() {
}

bool InstrumentationStrategy::reqAndConfigureFirstExperiment( Region* initial_region ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ),
                "Instrumentation strategy: submitting requests for profiling run\n" );
    std::vector<std::map<Region*, bool> > checkedSubs;
    std::list<ApplProcess>                controlled_processes = dp->get_controlled_processes();
    std::list<ApplProcess>::iterator      process;

    std::list<Region*>           regions;
    std::list<Region*>::iterator reg;
    regions = appl->get_regions();

    for( reg = regions.begin(); reg != regions.end(); reg++ ) {
        Region* region = *reg;
        for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
            pdb->request( new Context( region, process->rank, 0 ), PSC_EXECUTION_TIME );
            //if (region->get_type() == USER_REGION) {
            //  pdb->request(new Context(region, process->rank, 0), PSC_PAPI_TOT_CYC);
            //}
            pdb->request( new Context( region, process->rank, 0 ), PSC_MRI_OVERHEAD );
            pdb->request( new Context( region, process->rank, 0 ), PSC_MRI_LIBCALLS );
            if( inst_type == ALL_OVERHEAD_BASED || inst_type == ANALYSIS_BASED ) {
                pdb->request( new Context( region, process->rank, 0 ), PSC_PAPI_TOT_CYC );
                pdb->request( new Context( region, process->rank, 0 ), PSC_BACK_END_BUBBLE_ALL );
                pdb->request( new Context( region, process->rank, 0 ), PSC_BE_FLUSH_BUBBLE_ALL );
                pdb->request( new Context( region, process->rank, 0 ), PSC_BE_L1D_FPU_BUBBLE_L1D_FULLSTBUF );
            }
        }
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "requests for profiling run submitted\n" );
    return true;
}

bool InstrumentationStrategy::evaluateAndReqNextExperiment() {                  // TRUE requires next step; FALSE if done
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ),
                "Instrumentation strategy: evaluating profiling run experiment\n" );
    Region*                          region               = appl->get_phase_region();
    std::list<ApplProcess>           controlled_processes = dp->get_controlled_processes();
    std::list<ApplProcess>::iterator process;
    int                              threads = 1, max_rank = 0, max_thread = 0;
    INT64                            tmp;
    if( ( region )->get_rra() == RUNS_AS_THREADED ) {
        threads = appl->getOmpThreads();
    }

    dp->getResults();

    for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
        for( int i = 0; i < threads; i++ ) {
            if( tmp < pdb->get( new Context( region, process->rank, i ), PSC_EXECUTION_TIME ) ) {
                tmp        = pdb->get( new Context( region, process->rank, i ), PSC_EXECUTION_TIME );
                max_rank   = process->rank;
                max_thread = i;
            }
        }
    }

    //Was originally max_rank, be careful might crash in agent that does not control mpi_rank 0
    createOverheadProperties( region, max_rank, max_thread );

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "overhead analysis done...\n" );
    pdb->clean();

    if( ( inst_type == OVERHEAD_BASED || inst_type == ALL_OVERHEAD_BASED ) && foundProperties.size() == 0 ) {
        set_require_restart( false );
    }

    return false;
}
void InstrumentationStrategy::setProfiling_done( bool value ) {
    profiling_done = value;
}

bool InstrumentationStrategy::requireProfilingRun() {
    return !profiling_done;
}

std::string InstrumentationStrategy::name() {
    return "InstrumentationStrategy";
}

void InstrumentationStrategy::requiredRegions() {
    if( inst_type == ANALYSIS_BASED ) {
        std::list<ApplProcess>           controlled_processes = dp->get_controlled_processes();
        std::list<Request*>              requests;
        std::list<ApplProcess>::iterator process;
        std::list<Request*>::iterator    request;
        std::list<std::string>           reqrgns;
        std::list<std::string>::iterator reqrgnsitr;
        Context*                         ct;

        RequiredRegionProp* requiredRgns = new RequiredRegionProp( new Context( new Region(), 0, 0 ),
                                                                   new Context( appl->get_phase_region(), 0, 0 ),
                                                                   pedanticSearch ? 0.0 : OVERHEAD_PROPS_THRESHOLD::RequiredRegionProp );
        for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
            requests = dp->getRankSpecificRequests( process->rank );
            for( request = requests.begin(); request != requests.end(); request++ ) {
                ct = ( *request )->ct;
                std::ostringstream ost;
                ost << "f: " << ( ct->getRegion() )->get_ident().file_name << ", r: " << ( ct->getRegion() )->get_ident().rfl
                    << ";";
                reqrgns.push_back( ost.str() );
                //psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(Autoinstrument),"InstrumentationStrategy::requiredRegions(): required region >>%s<<\n", ost.str().c_str());
            }
        }

        reqrgns.sort();
        reqrgns.unique();
        for( reqrgnsitr = reqrgns.begin(); reqrgnsitr != reqrgns.end(); reqrgnsitr++ ) {
            requiredRgns->add_region( *reqrgnsitr );
        }
        foundProperties.push_back( requiredRgns );
    }
    if( inst_type == OVERHEAD_BASED || inst_type == ALL_OVERHEAD_BASED ) {
        set_require_restart( false );
    }
}

void InstrumentationStrategy::createOverheadProperties( Region* phase_region, int rank, int thread ) {
    double f        = 1;
    double overhead = 0;

    std::list<Region*>           regions;
    std::list<Region*>::iterator reg;
    regions = appl->get_regions();

    for( reg = regions.begin(); reg != regions.end(); reg++ ) {
        Region* region = *reg;
        double  my_relative_overhead;

        INT64 my_calls = pdb->try_get( new Context( region, rank, thread ), PSC_INSTANCES );
        if( my_calls <= 0 ) {
            continue;
        }
        INT64 my_time         = pdb->get( new Context( region, rank, thread ), PSC_EXECUTION_TIME );
        INT64 nested_libcalls = pdb->get( new Context( region, rank, thread ), PSC_MRI_LIBCALLS ) - 2 * my_calls;
        INT64 nested_overhead = pdb->get( new Context( region, rank, thread ), PSC_MRI_OVERHEAD );
        INT64 excl_overhead   = pdb->get( new Context( region, rank, thread ), PSC_MRI_EXCL_OVERHEAD );

        double pure_time;
        if( inst_type == ALL_OVERHEAD_BASED || inst_type == ANALYSIS_BASED ) {
            pure_time = ( ( double )( my_time - ( double )my_calls / 2 * ( double )InstrumentationOverhead -
                                      ( double )nested_libcalls / 2.0 * ( double )InstrumentationOverhead ) );
            //double pure_time=(double) (my_time - nested_overhead); //not correct since the overhead is canceled out from execution time
            my_relative_overhead = ( excl_overhead * 100 ) / pure_time;
        }
        else if( inst_type == OVERHEAD_BASED ) {
            pure_time = ( ( double )( my_time - my_calls * InstrumentationOverhead ) -
                          ( ( ( double )nested_libcalls / 2.0 ) * ( double )InstrumentationOverhead ) );
            my_relative_overhead = ( double )( ( my_calls ) * InstrumentationOverhead * 100 ) / pure_time;
        }
        if( my_relative_overhead > 5.0 ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "\nInst Overhead: (%s,%d) %s: ovhd %: %f\n\t corrected own "
                        "time=%f, \n\t subtracted ovhd: %f, \n\t own ovhd: %f (%d inst) \n\t nested ovhd: %f \n\t excl ovhd: %f\n\n",
                        region->get_ident().file_name.c_str(), region->get_ident().start_position, region->get_name().c_str(),
                        my_relative_overhead, pure_time / NANOSEC_PER_SEC_DOUBLE,
                        ( ( double )my_calls / 2.0 + ( double )nested_libcalls / 2.0 ) * ( double )InstrumentationOverhead
                        / NANOSEC_PER_SEC_DOUBLE, ( my_calls ) * InstrumentationOverhead / NANOSEC_PER_SEC_DOUBLE, my_calls,
                        nested_overhead / NANOSEC_PER_SEC_DOUBLE, excl_overhead / NANOSEC_PER_SEC_DOUBLE );
        }

        //psc_dbgmsg(6, " OVERHEAD \t\t %s %d, \n \t\t accumulated calls %f + my_calls %f / time %f , \n \t\t (%d)/(%d+%d*%d-%f)*100=%f\n",
        //           region->get_name().c_str(),
        //           region->get_key(),
        //           overhead+my_calls,
        //           my_calls,
        //           my_time,
        //           (my_calls)*InstrumentationOverhead,
        //           my_time,
        //           InstrumentationOverhead,
        //           my_calls,
        //           overhead*InstrumentationOverhead,
        //           my_relative_overhead
        //           );

        if( my_relative_overhead < 0 ) {
            my_relative_overhead = 100;
        }
        if( my_relative_overhead > 5.0 && my_time != 0 && region->get_key() != appl->get_phase_region()->get_key() ) {
            OverheadProp* overprop = new OverheadProp( new Context( region, rank, thread ),
                                                       new Context( appl->get_phase_region(), rank, thread ),
                                                       my_relative_overhead,
                                                       pedanticSearch ? 0.0 : OVERHEAD_PROPS_THRESHOLD::OverheadProp );
            foundProperties.push_back( overprop );
        }
    }

    region_cnt++;
    //return overhead + (double) pdb->get(new Context(phase_region, rank, thread), INSTANCES);
}
void InstrumentationStrategy::configureNextExperiment() {
    dp->transfer_requests_to_processes_no_begin_end_requests();
    psc_dbgmsg( 5, "Instrumentation strategy :requests submitted\n" );
}
