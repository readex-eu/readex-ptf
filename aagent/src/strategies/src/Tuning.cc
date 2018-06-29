/**
   @file    Tuning.cc
   @ingroup TuningStrategy
   @brief   Autotune search strategy
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
#include "EnergyConsumption.h"
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

#include "ScorepMPIStrategy.h"
#include "OpenMPAnalysisScoreP.h"
#include "ConfigAnalysis.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>

/**
 * @brief Constructor
 * @ingroup TuningStrategy
 *
 * @param scenarios A list of scenarios that has to evaluated
 * @param request   Strategy request containing the name of per-experiment analysis strategy
 * @param rts_based RTS based tuning (true/false).
 * @param pedantic  Pedantic feature is switched on, i.e. are all properties going to be reported.
 **/
TuningStrategy::TuningStrategy( const std::list<Scenario*>* scenarios,
                                StrategyRequest*            request,
                                bool                        rts_based,
                                bool                        pedantic ) : Strategy( pedantic ) {
    scenarioList     = scenarios;
    analysisStrategy = NULL;
    rtsBased         = rts_based;

    if( agent->get_leader() && psc_get_debug_level() >= 3 ) {
        for( const auto& scenario : *scenarioList ) {
            printf( "TuningStrategy scenario:\n" );
            scenario->print();
            fflush( stdout );
        }
    }

    map_scenarios_to_ranks();

    if( !analysisStrategy ) {
        if( request && request->getGeneralInfo()->strategy_name == "MPI" ) {
            analysisStrategy = new ScorepMPIStrategy( request->getGeneralInfo()->pedantic );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ),
                        "TuningStrategy has loaded the MPI sub-strategy.\n" );
        }
        else if( request && request->getGeneralInfo()->strategy_name == "OMP" ) {
            analysisStrategy = new OpenMPAnalysisScoreP( request->getGeneralInfo()->pedantic );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ),
                        "TuningStrategy has loaded the OMP sub-strategy.\n" );
        }
        else if( request && request->getGeneralInfo()->strategy_name == "ConfigAnalysis" ) {
            analysisStrategy = new ConfigAnalysis( request->getConfiguration().configuration_union.PropertyRequest_list,  request->getGeneralInfo()->pedantic );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ),
                        "TuningStrategy has loaded the configurable analysis sub-strategy.\n" );
        }
        else {
            analysisStrategy = NULL;
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ),
                        "TuningStrategy has no analysis sub-strategy specified.\n" );
        }
    }
}

TuningStrategy::~TuningStrategy() {
    //NOTE: Properties are deleted by clear_found_properties by on_check handler.
    delete analysisStrategy;

//TODO: Check why it fails! Maybe it gets destroyed during startegyRequest destructor call.
//    for ( const auto& scenario : *scenarioList ) {
//        delete scenario;
//    }
//    delete scenarioList;
}


/**
 * @brief Requests tuning actions from the Score-P monitor.
 * @ingroup TuningStrategy
 *
 * Requests dynamic tuning actions from the Score-P monitor.
 *
 */
void TuningStrategy::requestTuningActions() {
    for( const auto scenarioList : TS_related_scenarios_per_rank ) {
        int rank = scenarioList.first;
        for( const auto& scenario : scenarioList.second ) {
            std::list<TuningSpecification*>* tuningSpecificationList = scenario->getTuningSpecifications();
            if( !tuningSpecificationList ) {
                continue;
            }
            for( const auto& tuningSpecification : *tuningSpecificationList ) {
                if( processInRanks( tuningSpecification->getRanks(), rank ) ) {
                    dp->addTuningRequest( rank, tuningSpecification );
                }
            }
        }
    }
}


/**
 * @brief Creates a candidate property list for the first run.
 *
 * @ingroup TuningStrategy
 *
 * Creates a candidate property list for the first run and prepares requests
 * for runtime information. The requested property list is consisted of requests
 * of a particular per-experiment analysis strategy and tuning objectives.
 *
 * @param r Region for which properties are requested
 * @return Returns true as this is single-step strategy
 */
bool TuningStrategy::reqAndConfigureFirstExperiment( Region* r ) {
// TRUE can start; FALSE not ready

    phaseRegion = r;

    // Requests objective properties for a tuned region - For this properties purpose is set to PURPOSE_TUNING
    for( const auto PRs_it : PR_related_scenarios_per_rank ) {
        int rank = PRs_it.first;
        for( const auto& scenario : PRs_it.second ) {
            int                                scenarioId      = scenario->getID();
            const std::list<PropertyRequest*>* propertyReqList = scenario->getPropertyRequests();
            for( const auto& propertyRequest : *propertyReqList ) {
                if( processInRanks( propertyRequest->getRanks(), rank ) ) {
                    if( rtsBased )
                        createCandidatePropertyList( scenario->getRts()->getCallPath(), scenarioId, rank, propertyRequest );
                    else
                        createCandidatePropertyList( scenario->getRegion()->getRegionID(), scenarioId, rank, propertyRequest );
                }
            }
        }
    }

    // Request tuning actions
    requestTuningActions();

    if( psc_get_debug_level() >= 3 ) {
        agent->print_property_set( candProperties, "SET OF CANDIDATE PROPERTIES", false, false );
    }
    if( agent->get_leader() ) {
        psc_dbgmsg( 3, "Tuning Strategy Step %d\n", strategy_steps );
    }

    for( const auto& property : candProperties ) {
        property->request_metrics();
    }

    // Runs other analysis strategies's configure first experiment, i.e.,
    // per-experiment analysis, and prepares requests for runtime information
    if( analysisStrategy ) {
        analysisStrategy->reqAndConfigureFirstExperiment( r );
    }

    return true;
}

/**
 * @brief Configures next experiment.
 *
 * @ingroup TuningStrategy
 *
 * Requests tuning actions from tuning strategies and sends them to the MRI monitor.
 *
 */
void TuningStrategy::configureNextExperiment() {
    psc_dbgmsg( 3, "Tuning Strategy configureNextExperiment\n" );

    dp->transfer_requests_to_processes();

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ), "configureNextExperiment done\n" );
}

/**
 * @brief Evaluates the properties from the previous and requests new experiment.
 *
 * @ingroup TuningStrategy
 *
 * Evaluates the properties from the candidate list and moves them to the found
 * list. Found properties are then marked with their purpose for the frontend.
 *
 * @return Decision on continuation of tuning analysis strategy.
 */
bool TuningStrategy::evaluateAndReqNextExperiment() {
    bool cont = false;

    //if (psc_get_debug_level() >= 2)
    //agent->print_property_set( candProperties, "SET OF CANDIDATE PROPERTIES", false, true );

    //Evaluate candidate properties
    psc_dbgmsg( 3, "%s: analyzing results... %d candidate properties.\n", name().c_str(), candProperties.size() );

    for( const auto& property : candProperties ) {
        property->evaluate();
        if( property->condition() ) {
            // All candidate properties created by this strategy have purpose tuning. If not then print an error message.
            if( property->get_Purpose() != PSC_PROPERTY_PURPOSE_TUNING ) {
                psc_errmsg( "A candidate property %s of the Tuning strategy has purpose which is not tuning!" );
            }

            foundProperties.push_back( property );
        }
        else {
            //If the property was not found it should be deleted. Found ones removed by the other entity.
            delete property;
        }
    }

    if( analysisStrategy ) {
        cont = analysisStrategy->evaluateAndReqNextExperiment();
    }

    /*
     * foundProperties was changed to be a global list which is pushed by all strategies
     * Here we go over the list of all found properties (the ones created by this tuning strategy,
     * its analysis strategy as well as potentially *WARNING* other strategies running on this
     * agent) and assign a ScenarioID as well as its purpose to be analysis)
     */
    for( const auto& property : foundProperties ) {
        if( property->get_Purpose() != PSC_PROPERTY_PURPOSE_TUNING ) {
            property->set_Purpose( PSC_PROPERTY_PURPOSE_ANALYSIS );
            std::list<Scenario*> scenario_list = PR_related_scenarios_per_rank[ property->get_rank() ];
            for( const auto& scenario : scenario_list ) {
                property->add_ScenarioId( scenario->getID() );
            }
        }
    }

    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( foundProperties, "SET OF FOUND PROPERTIES", true, true );
    }

    return cont;
}

/**
 * @brief Returns the name of the tuning strategy.
 *
 * @ingroup TuningStrategy
 *
 */
std::string TuningStrategy::name() {
    return "TuningStrategy";
}

//TODO remove this
/**
 * @brief Creates a candidate property list.
 *
 * Creates a candidate property list from a tuned region and property requests.
 * Marks properties with scenario id and purpose.
 */
void TuningStrategy::createCandidatePropertyList( Region*          region,
                                                  int              scenarioId,
                                                  int              processRank,
                                                  PropertyRequest* propertyRequest ) {
//    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ),
//                "Creating an objective for scenario %d (rank:%d)\n",
//                scenarioId, processRank );

    for( const auto& propertyId : *propertyRequest->getPropertyIDs() ) {
        Context* phaseContext  = new Context( phaseRegion, processRank, 0 );
        Context* regionContext = new Context( region, processRank, 0 );

        Property* prop;

        switch( propertyId ) {
        case EXECTIME:
            prop = new ExecTimeProp( regionContext /*Create outside and followConfigAnalysis*/, phaseContext, 0.0 );
            break;

        case ENERGY_CONSUMPTION:
            prop = new EnergyConsumption( regionContext, phaseContext );
            break;

        case HDEEM_ENERGY_CONSUMTION_BLADE:
            prop = new HdeemProp( regionContext, phaseContext );
            break;

        default:
            prop = NULL;
            psc_errmsg( "Unknown objective in the tuning strategy: %i\n", propertyId );
            abort();
        }

        prop->add_ScenarioId( scenarioId );
        prop->set_Purpose( PSC_PROPERTY_PURPOSE_TUNING );
        candProperties.push_back( prop );
    }
}

/**
 * @brief Creates a candidate property list.
 *
 * Creates a candidate property list from a tuned region and property requests.
 * Marks properties with scenario id and purpose.
 */
void TuningStrategy::createCandidatePropertyList( std::string      entityID,
                                                  int              scenarioId,
                                                  int              processRank,
                                                  PropertyRequest* propertyRequest ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ),
                "Creating an objective for scenario %d (rank:%d)\n",
                scenarioId, processRank );

    for( const auto& propertyId : *propertyRequest->getPropertyIDs() ) {
        Context* phaseContext = new Context( phaseRegion, processRank, 0 );
        Context* rContext     = NULL;

        if( rtsBased )
        {
            Rts* rts = appl->getCalltreeRoot()->getRtsByCallpath( entityID );
        //psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAgentStrategy ), "Tuning Strategy::createCandidatePropertyList: callpath of found rts <%s>\n",rts->getCallPath().c_str());

            if ( rts == NULL ) {
                delete phaseContext;
                psc_errmsg( "Unknown callpath %s in tuning strategy\n", entityID.c_str() );
                abort();
            }
            rContext = new Context( rts, processRank, 0 );
        }
        else
        {
            Region* reg = appl->getRegionByID( entityID );
            rContext    = new Context( reg, processRank, 0 );
        }

        Property* prop;

        switch( propertyId ) {
            case EXECTIME:
                prop = new ExecTimeProp( rContext, phaseContext, 0.0 );
                break;

            case ENERGY_CONSUMPTION:
                prop = new EnergyConsumption( rContext, phaseContext );
                break;

            case HDEEM_ENERGY_CONSUMTION_BLADE:
                prop = new HdeemProp( rContext, phaseContext );
                break;

            case INTERPHASE_PROPS:
                prop = new InterphaseProps( rContext, phaseContext );
                break;

            default:
                prop = NULL;
                delete phaseContext;
                delete rContext;
                psc_errmsg( "Unknown objective in the tuning strategy: %i\n", propertyId );
                abort();
        }

        prop->add_ScenarioId( scenarioId );
        prop->set_Purpose( PSC_PROPERTY_PURPOSE_TUNING );
        candProperties.push_back( prop );
    }
}

void TuningStrategy::map_scenarios_to_ranks() {
    std::list<ApplProcess> appl_processes = dp->get_controlled_processes();
    for( const auto& process : appl_processes ) {
        int                  rank = process.rank;
        std::list<Scenario*> empty_list;
        TS_related_scenarios_per_rank[ rank ] = empty_list;
        PR_related_scenarios_per_rank[ rank ] = empty_list;

        if( psc_get_debug_level() >= 8 ) {
            std::cout << "Application process rank " << rank << " has following scenarios:" << endl;
        }

        for( const auto& scenario : *scenarioList ) {
            if( psc_get_debug_level() >= 8 ) {
                std::cout << "  Scenario " << scenario->getID() << " as ";
            }
            if( scenario->getRankAffectedBySceanrioTS( rank ) ) {
                if( psc_get_debug_level() >= 8 ) {
                    std::cout << "  a tuning specification ";
                }
                TS_related_scenarios_per_rank[ rank ].push_back( scenario );
            }
            if( scenario->getRankAffectedBySceanrioPR( rank ) ) {
                if( psc_get_debug_level() >= 8 ) {
                    std::cout << "  a property request";
                }
                PR_related_scenarios_per_rank[ rank ].push_back( scenario );
            }
            if( psc_get_debug_level() >= 8 ) {
                std::cout << endl;
            }
        }
    }

    /*
     * The below block of code is not needed
     */
    //If the configurable strategy is requested, properties are created but the properties are not marked
    //with a scenario. Here, we use as default the first scenario. May be we should use all of them.
//  for (process = controlled_processes.begin(); process != controlled_processes.end(); process++) {
//    if (process->getScenariosPerTuningSpecification()->size()==0){
//        Scenario *sc= *scenarios->begin();
//        process->addScenarioPerTuningSpecification(sc);
//    }
//    if (process->getScenariosPerPropertyRequest()->size()==0){
//        Scenario *sc= *scenarios->begin();
//        process->addScenarioPerPropertyRequest(sc);
//    }
//  }
}
