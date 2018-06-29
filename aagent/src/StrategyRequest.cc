/**
   @file    StrategyRequest.cc
   @ingroup Communication
   @brief   Strategy request description
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

#include "StrategyRequest.h"
#include "selective_debug.h"

using std::list;




StrategyRequest::StrategyRequest() {
}

StrategyRequest::StrategyRequest( StrategyRequestGeneralInfo* strategyRequest ) {
    configuration.type   = strategy_configuration_type( ANALYSIS );
    general_info         = strategyRequest;
    sub_strategy_request = NULL;
}

StrategyRequest::StrategyRequest( list <Scenario*>* tune_scenario_list, StrategyRequestGeneralInfo* strategyRequest ) {
    configuration.type                                  = strategy_configuration_type( TUNE );
    configuration.configuration_union.TuneScenario_list = tune_scenario_list;
    general_info                                        = strategyRequest;
    sub_strategy_request                                = NULL;
}

StrategyRequest::StrategyRequest( list<int>* Persyst_property_ids, StrategyRequestGeneralInfo* strategyRequest ) {
    configuration.type                                       = strategy_configuration_type( PERSYST );
    configuration.configuration_union.PersystPropertyID_list = Persyst_property_ids;
    general_info                                             = strategyRequest;
    sub_strategy_request                                     = NULL;
}

StrategyRequest::StrategyRequest( list <PropertyRequest*>* property_request_list, StrategyRequestGeneralInfo* strategyGeneralInfo ) {
    configuration.type                                     = strategy_configuration_type( CONFIG );
    configuration.configuration_union.PropertyRequest_list = property_request_list;
    general_info                                           = strategyGeneralInfo;
    sub_strategy_request                                   = NULL;
}

StrategyRequest::StrategyRequest( list<TuningSpecification*>* tuning_specification_list, StrategyRequestGeneralInfo* strategyGeneralInfo ) {
    configuration.type                                         = strategy_configuration_type( PRECONFIGURATION );
    configuration.configuration_union.TuningSpecification_list = tuning_specification_list;
    general_info                                               = strategyGeneralInfo;
    sub_strategy_request                                       = NULL;
}

StrategyRequest::~StrategyRequest() {
    if( configuration.type == strategy_configuration_type( TUNE ) ) {
        delete configuration.configuration_union.TuneScenario_list;
    }
    else if( configuration.type == strategy_configuration_type( PERSYST ) ) {
        delete configuration.configuration_union.PersystPropertyID_list;
    }
    else if( configuration.type == strategy_configuration_type( CONFIG ) ) {
        for( auto& property_request : *configuration.configuration_union.PropertyRequest_list ) {
            delete property_request;
        }
        delete configuration.configuration_union.PropertyRequest_list;
    }
    else if( configuration.type == strategy_configuration_type( PRECONFIGURATION ) ) {
        for( const auto& tuning_spec : *configuration.configuration_union.TuningSpecification_list ) {
            delete tuning_spec;
        }
        delete configuration.configuration_union.TuningSpecification_list;
    }
    delete general_info;
    //delete sub_strategy_request;
}

StrategyRequestGeneralInfo* StrategyRequest::getGeneralInfo() {
    return general_info;
}

int StrategyRequest::getTypeOfConfiguration() {
    return configuration.type;
}

const StrategyRequestConfiguration StrategyRequest::getConfiguration() {
    return configuration;
}

void StrategyRequest::setSubStrategyRequest( StrategyRequest* strategy ) {
    sub_strategy_request = strategy;
}

StrategyRequest* StrategyRequest::getSubStrategyRequest( void ) {
    return sub_strategy_request;
}

void StrategyRequest::printStrategyRequest( void ) {
    if( !general_info ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Nothing to print for strategy request. General info is NULL!" );
        return;
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Strategy general info: " );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), " - strategy name: %s\n", general_info->strategy_name.c_str() );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), " - delay phases: %d\n", general_info->delay_phases );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), " - delay seconds: %d\n", general_info->delay_seconds );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), " - analysis duration: %d\n", general_info->analysis_duration );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), " - pedantic: %s\n", general_info->pedantic ? "true" : "false" );

    list< Scenario* >*            scenarios;
    list< int >*                  PropertyIDs;
    list< PropertyRequest* >*     requests;
    list< TuningSpecification* >* TuningSpecs;
    switch( configuration.type ) {
    case ANALYSIS:
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "No strategy specific configuration!\n" );
        break;
    case TUNE:
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Tuning strategy specific configuration!\n" );
        scenarios = configuration.configuration_union.TuneScenario_list;
        for( const auto& scenario : *scenarios ) {
            scenario->print();
        }
        break;
    case PERSYST:
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Persyst strategy specific configuration!\n" );
        if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ) ) ) {
            cout << "PropertyID: " << endl;
            PropertyIDs = configuration.configuration_union.PersystPropertyID_list;
            for( const auto& PropertyID : *PropertyIDs ) {
                cout << PropertyID;
            }
            cout << endl;
        }
        break;
    case CONFIG:
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Configurable analysis strategy specific configuration!\n" );
        if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ) ) ) {
            cout << "PropertyID: " << endl;
            requests = configuration.configuration_union.PropertyRequest_list;
            for( const auto& request : *requests ) {
                request->print();
            }
            cout << endl;
        }
        break;
    case PRECONFIGURATION:
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Preconfiguration for analysis strategy specific configuration!\n" );
        if( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ) ) ) {
            cout << "Tuning specification: " << endl;
            TuningSpecs = configuration.configuration_union.TuningSpecification_list;
            for( const auto& TuningSpec : *TuningSpecs ) {
                cout << TuningSpec->toString( 2, "-" );
            }
        }
        break;
    default:
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Non defined strategy specific configuration!\n" );
        break;
    }

    if( sub_strategy_request && active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ) ) ) {
        cout << "Sub-strategy request:" << endl;
        sub_strategy_request->printStrategyRequest();
    }
}
