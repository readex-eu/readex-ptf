/**
   @file    ConfigAnalysis.cc
   @ingroup ConfigAnalysisStrategy
   @brief   Configurable analysis strategy
   @author  Michael Gerndt
   @verbatim
   Revision:       $Revision$
   Revision date:  $Date$
   Committed by:   $Author$

   This file is part of the Periscope performance measurement tool.
   See http://www.lrr.in.tum.de/periscope for details.

   Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
   See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include "global.h"
#include "Tuning.h"
#include "ExecTime.h"
#include "ActiveCyclesObjective.h"
#include "EnergyConsumption.h"
#include "HPCConditional.h"
#include "PipelineExecutionTime.h"
#include "PipelineStageExecutionTime.h"
#include "PipelineStageBufWaitTime.h"
#include "EagerLimitDependent.h"
#include "MPIexcessive.h"
#include "PerformanceCounters.h"
#include "HdeemProp.h"
#include "InterphaseProps.h"
#include <iostream>
#include "psc_errmsg.h"
#include <analysisagent.h>
#include "selective_debug.h"
#include "TuningParameter.h"
#ifdef __p575
#include <strings.h>
#endif


#include "PropertyRequest.h"
#include "DataProvider.h"
#include "ConfigAnalysis.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>

/**
 * @brief Constructor
 * @ingroup ConfigAnalysis
 *
 * @param reqs      List of property requests
 * @param pedantic  Pedantic feature is switched on, i.e. are all properties going to be reported.
 **/
ConfigAnalysis::ConfigAnalysis( std::list< PropertyRequest* >* reqs,
                                bool                           pedantic ) : Strategy( pedantic ) {
    requests = reqs;
}

/**
 * @brief Destructor
 * @ingroup ConfigAnalysis
 *
 * Cleans-up the found property list and the performance database.
 **/
ConfigAnalysis::~ConfigAnalysis() {
//TODO: Check why it fails! Maybe it gets destroyed during startegyRequest destructor call.
//    for ( const auto& request : *requests ) {
//        delete request;
//    }
    delete requests;
    clear_found_properties();
    pdb->clean();
}

/**
 * @brief Creates a candidate property list for the first run.
 *
 * @ingroup ConfigAnalysis
 *
 * Creates the requested candidate properties.
 *
 * @param r Phase region
 * @return Returns true as this is single-step strategy
 */
bool ConfigAnalysis::reqAndConfigureFirstExperiment( Region* r ) {
    psc_dbgmsg( 1, "Configurable Analysis Strategy: ConfigureFirstExperiment\n" );

    phaseRegion = r;

    //TODO: PDB has to be always cleaned for the AutoTune strategies. We need a holistic solution for this.
    // Alternatively the access window of the PDB can be set to point only on the last executed iteration.
    pdb->clean();

    std::list < ApplProcess > controlled_processes = dp->get_controlled_processes();

    /*
     * Since Configurable Strategy already knows the regions from the Property Requests it
     * can already instantiate candidate properties here and doesn't need to subscribe for the measurement results by DP.
     */

    // Loop over Property Requests
    for( const auto& request : *requests ) {
        std::list<int>*         propIds  = request->getPropertyIDs();
        std::list<std::string>* entities = request->getEntities();

        // Loop over property IDs requested in this request
        for( const auto& propertyId : *propIds ) {
            // Loop over regions which have to be analyzed in this Property Request
            for( const auto& entity : *entities ) {
                // Loop over all processes controlled by this AA
                for( const auto& process : controlled_processes ) {
                    if( processInRanks( request->getRanks(), process.rank ) ) {
                        // If this rank is among the ranks to be analyzed by this PropertyRequest then instantiate the corresponding property
                        //createProperty( propertyId, process->rank, Application::instance().getRegionByID( *reg ) );
                        // sends only regionID or rts callpath string
                        createProperty( propertyId, process.rank, entity );
                    }
                }
            }
        }
    }

    // Print created candidate properties
    if( psc_get_debug_level() >= 3 ) {
        agent->print_property_set( candProperties, "SET OF CANDIDATE PROPERTIES", false, false );
    }


    // Make the created candidate properties to submit their measurement requests to the DP.
    for( const auto& property : candProperties ) {
        property->request_metrics();
    }


    return true;
}

/**
 * @brief Configures next experiment.
 *
 * @ingroup ConfigAnalysis
 *
 * Requests measurements for the created candidate properties.
 *
 */
void ConfigAnalysis::configureNextExperiment() {
    dp->transfer_requests_to_processes();
}

/**
 * @brief Evaluates the properties from the experiment.
 *
 * @ingroup ConfigAnalysis
 *
 * Evaluates the properties from the candidate list and moves them to the found
 * list. Found properties are then marked with their purpose for the frontend.
 *
 * @return FALSE since this is a single step strategy.
 */
bool ConfigAnalysis::evaluateAndReqNextExperiment() {
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
    for( const auto& property : candProperties ) {
        property->evaluate();
        if( property->condition() ) {
            foundProperties.push_back( property );
        }
    }

    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( foundProperties, "SET OF FOUND PROPERTIES", true, true );
    }

    candProperties.clear();

    return false;
}

/**
 * @brief Returns the name of the strategy.
 *
 * @ingroup ConfigAnalysis
 *
 */
std::string ConfigAnalysis::name() {
    return "ConfigAnalysis";
}

/**
 * Marks whether the configAnalysis is for rts_based or region_based
 * @return bool
 */
bool ConfigAnalysis::isRtsBased() {
    return withRtsSupport();
}


/**
 * @brief Creates a candidate property.
 *
 * @ingroup ConfigAnalysis
 *
 * Creates a candidate property.
 *
 * @param propId   Id of the property
 * @param rank     process
 * @param entityID region/rts for which the property is evaluated
 */
//TO DO Context object has to be chanaged for rts
void ConfigAnalysis::createProperty( int propId, int rank, std::string entityID ) {
    Context*  pCt  = new Context( phaseRegion, rank, 0 );
    Property* prop = NULL;
    Context*  rCt  = NULL;

    if( isRtsBased() ) {
        Rts* rts = appl->getCalltreeRoot()->getRtsByCallpath(entityID);
   	  //psc_dbgmsg(8, "ConfigAnalysis::createProperty: callpath of found rts <%s>\n",rts->getCallPath().c_str());

        if ( rts == NULL ) {
           psc_errmsg( "Unknown callpath %s in configurable analysis strategy\n", entityID.c_str() );
           abort();
        }
        rCt          = new Context( rts, rank, 0 );
    } else {
        Region* reg = Application::instance().getRegionByID( entityID );
        rCt         = new Context( reg, rank, 0 );
    }

    switch( propId ) {
    case EXECTIME:
        prop = new ExecTimeProp( rCt, pCt, 0.0 );
        break;
    case ENERGY_CONSUMPTION:
        prop = new EnergyConsumption( rCt, pCt );
        break;
    case MPITIME:
        prop = new MPIexcessiveProp( rCt, pCt, PSC_UNDEFINED_METRIC, 0.0 );
        break;
    case HDEEM_ENERGY_CONSUMTION_BLADE:
        prop = new HdeemProp( rCt, pCt );
        break;
    case INTERPHASE_PROPS:
        prop = new InterphaseProps( rCt, pCt );
        break;
    default:
        psc_errmsg( "Unknown property id %d in configurable analysis strategy\n", propId );
        abort();
    }

    prop->set_Purpose( PSC_PROPERTY_PURPOSE_ANALYSIS );
    candProperties.push_back( prop );
}
