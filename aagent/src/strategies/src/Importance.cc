/**
   @file    Importance.cc
   @ingroup ImportanceStrategy
   @brief   Importance search strategy
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
#include "Importance.h"
#include "ExecTimeImportance.h"
#include "application.h"
#include "Metric.h"
#include "strategy.h"
#include "PropertyID.h"
#include "psc_errmsg.h"
#include "EnergyConsumption.h"
#include <iostream>
#include <string.h>
#include <analysisagent.h>


using namespace std;

bool Importance::reqAndConfigureFirstExperiment( Region* r ) {
    if( agent->get_leader() ) {
        psc_dbgmsg( 1, "Strategy Step %d\n", strategy_steps );
    }

    /* Store phase region. Even though it is supposed to be NULL, since it is not known beforehand */
    phaseRegion = r;

    /* Add global request for EXECUTION_TIME */
    dp->addMeasurementRequest( NULL, PSC_EXECUTION_TIME );
    /* Subscribe for metric EXUCTION_TIME */
    dp->globalMetricSubscribe( this, PSC_EXECUTION_TIME );

    if( withRtsSupport() ) {
        std::string newMetric;
        //Read the config tree to send proper measurement request
        try {
            newMetric = configTree.get < std::string > ("Configuration.periscope.metrics.node_energy");
            dp->addMeasurementRequest( NULL, PSC_NODE_ENERGY );
        }
        catch (exception &e) {
        }
        try {
            newMetric = configTree.get < std::string > ("Configuration.periscope.metrics.cpu0_energy");
            dp->addMeasurementRequest( NULL, PSC_CPU0_ENERGY );
        }
        catch (exception &e) {
        }
        try {
            newMetric = configTree.get < std::string > ("Configuration.periscope.metrics.cpu1_energy");
            dp->addMeasurementRequest( NULL, PSC_CPU1_ENERGY );
        }
        catch (exception &e) {
        }
    }

    //add Energy consumption request
//    dp->addMeasurementRequest( NULL, PSC_NODE_ENERGY );

    //add PAPI_TOT_INS for normalized energy
//    dp->addMeasurementRequest( NULL, PSC_PAPI_TOT_INS);
//    dp->globalMetricSubscribe( this, PSC_NODE_ENERGY );

    strategy_steps++;

    return true;
}

std::list<Property*> Importance::create_initial_candidate_properties_set( Region* phaseRegion ) {
    std::list<ApplProcess>           controlled_processes = dp->get_controlled_processes();
    std::list<Region*>               regions;
    std::list<Property*>             candidates;
    Property*                        prop;
    std::list<ApplProcess>::iterator process;
    std::list<Region*>::iterator     reg;

    propsRefineRegionNesting.clear();
    regions = appl->get_regions();

        for( reg = regions.begin(); reg != regions.end(); reg++ ) {
            if( ( *reg )->get_type() == SUB_REGION || ( *reg )->get_type() == USER_REGION ) {
                for( process = controlled_processes.begin(); process != controlled_processes.end(); process++ ) {
                    Context* ct = new Context( ( *reg ), process->rank, 0 );
                    //			Context *ct = new Context((*reg),0, 0);
                    Context* pCt = new Context( phaseRegion, process->rank, 0 );
                    //			Context *pCt = new Context(phaseRegion, 0, 0);
                    prop = new ExecTimeImportanceProp( ct, pCt );
                    candidates.push_back( prop );
                }
            }
        }
    return candidates;
}

void Importance::metric_found_callback( Metric m, Context ct_in ) {
//    psc_dbgmsg( 7, "Notification: m=%s, file=%d rfl=%d thread=%d region_type=%d\n", EventList[ m ].EventName, ct_in.getFileId(), ct_in.getRfl(), ct_in.getThread(), ct_in.getRegionType() );

    psc_dbgmsg( 7, "Notification: m=%s, file=%d rfl=%d thread=%d region_type=%d, isRtsBased= %d rts id = %d \n", EventList[ m ].EventName, ct_in.getFileId(), ct_in.getRfl(), ct_in.getThread(), ct_in.getRegionType(), ct_in.isRtsBased(), ct_in.getRtsID() );


    /* Ask application for phase region. If not phase region registered analysis can not be done. */
    Region* phase = appl->get_phase_region();
    if( phase == NULL ) {
        psc_dbgmsg( 3, "Notification is received, however no phase region is registered in the application. Analysis can not be done.\n" );
        return;
    }
    /* Only thread 0 is of interest */
    if( ct_in.getThread() != 0 ) {
        return;
    }

    /* Create the context and the phase context */
    Context*  pCt       = new Context( phase, ct_in.getRank(), 0 );
//    Context*  ct        = new Context( ct_in.getRegion(), ct_in.getRank(), 0 );

    Property* candidate = NULL;
    /*Instantiate a candidate property*/

//    if( m == PSC_EXECUTION_TIME || m == PSC_NODE_ENERGY ) {
//        Context* ct        = new Context( ct_in.getRegion(), ct_in.getRank(), 0 );

        Context* ct;
        if( ct_in.isRtsBased() ) {
            ct = new Context( ct_in.getRts(), ct_in.getRank(), 0 );
        }
        else {
            ct = new Context( ct_in.getRegion(), ct_in.getRank(), 0 );
        }

        candidate = new ExecTimeImportanceProp( ct, pCt );
        /* If no property was created, then contexts should be deleted */
        if( candidate == NULL ) {
            delete ct;
            delete pCt;
            psc_dbgmsg( 7, "Property was NOT created!\n" );
        }
        else {
            psc_dbgmsg( 7, "Property %s created!\n", candidate->name().c_str() );
            candProperties.push_back( candidate );
        }
//    }

}

void Importance::configureNextExperiment() {
    psc_dbgmsg( 5, "Strategy %s: calling dp->transfer_requests_to_processes();\n", this->name().c_str() );
    dp->transfer_requests_to_processes();
}

bool compareProps( Property* first, Property* second ) {
    if( first->get_rank() < second->get_rank() ) {
        return true;
    }
    if( first->get_rank() > second->get_rank() ) {
        return false;
    }
    if( first->severity() > second->severity() ) {
        return true;
    }
    return false;
}

bool Importance::evaluateAndReqNextExperiment() {
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
            foundProperties.push_back( *prop_it );
        }
    }

    if( psc_get_debug_level() >= 8 ) {
        agent->print_property_set( foundProperties, "SET OF FOUND PROPERTIES", true, true );
    }

    foundProperties.sort( compareProps );

    if( psc_get_debug_level() >= 8 ) {
        agent->print_property_set( foundProperties, "SORTED SET OF FOUND PROPERTIES", true, true );
    }


    /*
     * Filtering of properties is disabled here. Probably it should take place in the plugin.
     */
//    Prop_List properties = foundProperties;
//    foundProperties.clear();
//
//    int    rank   = -1;
//    double aggSev = 0.0;
//    for( prop_it = properties.begin(); prop_it != properties.end(); prop_it++ ) {
//        if( rank == -1 )
//            rank = ( *prop_it )->get_rank();
//        if( rank == ( *prop_it )->get_rank() && aggSev < 70 ) {
//            aggSev += ( *prop_it )->severity();
//            foundProperties.push_back( ( *prop_it ) );
//        }
//        else if( rank != ( *prop_it )->get_rank() ) {
//            rank   = ( *prop_it )->get_rank();
//            aggSev = ( *prop_it )->severity();
//            foundProperties.push_back( ( *prop_it ) );
//        }
//    }
//
//    if( psc_get_debug_level() >= 2 )
//        agent->print_property_set( foundProperties, "SET OF FOUND PROPERTIES", true, true );

    return false;
}


std::string Importance::name() {
    return "ImportanceStrategy";
}
