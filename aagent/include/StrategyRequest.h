/**
   @file    StrategyRequest.h
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

#ifndef STRATEGYREQUEST_H_
#define STRATEGYREQUEST_H_

#include "Scenario.h"
#include <boost/serialization/list.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>




//RM: Maybe it would be nice to move the typedefs into the class
typedef struct StrategyRequestGeneralInfo_t {
    std::string strategy_name;
    bool        pedantic;
    int         delay_phases;
    int         delay_seconds;
    int         analysis_duration;

    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& strategy_name;
        ar& pedantic;
        ar& delay_phases;
        ar& delay_seconds;
        ar& analysis_duration;
    }

    bool operator==( const struct StrategyRequestGeneralInfo_t& in ) const {
        return strategy_name == in.strategy_name
               && pedantic == in.pedantic
               && delay_phases == in.delay_phases
               && delay_seconds == in.delay_seconds
               && analysis_duration == in.analysis_duration;
    }

    bool operator!=( const struct StrategyRequestGeneralInfo_t& in ) const {
        return !( *this == in );
    }
} StrategyRequestGeneralInfo;

typedef union StrategyRequestConfigurationUnion_t {
    std::list<Scenario*>*            TuneScenario_list;
    std::list<int>*                  PersystPropertyID_list;
    std::list<PropertyRequest*>*     PropertyRequest_list;
    std::list<TuningSpecification*>* TuningSpecification_list;
} StrategyRequestConfigurationUnion;

enum strategy_configuration_type {
    ANALYSIS = 0,
    TUNE,
    PERSYST,
    CONFIG,
    PRECONFIGURATION
};

typedef struct StrategyRequestConfiguration_t {
    int                               type;
    StrategyRequestConfigurationUnion configuration_union;

    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& type;
        if( type == strategy_configuration_type( TUNE ) ) {
            ar& configuration_union.TuneScenario_list;
        }
        else if( type == strategy_configuration_type( PERSYST ) ) {
            ar& configuration_union.PersystPropertyID_list;
        }
        else if( type == strategy_configuration_type( CONFIG ) ) {
            ar& configuration_union.PropertyRequest_list;
        }
        else if( type == strategy_configuration_type( PRECONFIGURATION ) ) {
            ar& configuration_union.TuningSpecification_list;
        }
    }

    bool operator==( const struct StrategyRequestConfiguration_t& in ) const {
        if( type != in.type ) {
            return false;
        }

        if( type == strategy_configuration_type( TUNE ) ) {
            if( configuration_union.TuneScenario_list != in.configuration_union.TuneScenario_list ) {
                return false;
            }
        }

        if( type == strategy_configuration_type( PERSYST ) ) {
            if( configuration_union.PersystPropertyID_list != in.configuration_union.PersystPropertyID_list ) {
                return false;
            }
        }

        if( type == strategy_configuration_type( CONFIG ) ) {
            if( configuration_union.PropertyRequest_list != in.configuration_union.PropertyRequest_list ) {
                return false;
            }
        }

        if( type == strategy_configuration_type( PRECONFIGURATION ) ) {
            if( configuration_union.TuningSpecification_list != in.configuration_union.TuningSpecification_list ) {
                return false;
            }
        }

        return true;
    }

    bool operator!=( const struct StrategyRequestConfiguration_t& in ) const {
        return !( *this == in );
    }
} StrategyRequestConfiguration;

class StrategyRequest {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& configuration;
        ar& general_info;
        ar& sub_strategy_request;
    }

private:
    StrategyRequestConfiguration configuration;
    StrategyRequestGeneralInfo*  general_info;
    StrategyRequest*             sub_strategy_request;
    StrategyRequest();

public:
    StrategyRequest( StrategyRequestGeneralInfo* );

    StrategyRequest( list<Scenario*>*,
                     StrategyRequestGeneralInfo* );

    StrategyRequest( list<int>*,
                     StrategyRequestGeneralInfo* );

    StrategyRequest( list<PropertyRequest*>*,
                     StrategyRequestGeneralInfo* );

    StrategyRequest( list<TuningSpecification*>*,
                     StrategyRequestGeneralInfo* );

    virtual ~StrategyRequest();

    StrategyRequestGeneralInfo* getGeneralInfo();

    int getTypeOfConfiguration();

    const StrategyRequestConfiguration getConfiguration();

    void setSubStrategyRequest( StrategyRequest* strategy );

    StrategyRequest* getSubStrategyRequest( void );

    void printStrategyRequest();
};

#endif /* STRATEGYREQUEST_H_ */
