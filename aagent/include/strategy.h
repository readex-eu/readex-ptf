/**
   @file    strategy.h
   @ingroup AnalysisAgent
   @brief   Performance analysis strategy abstraction header
   @author  Edmond Kereku
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2011, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

/**
 * @defgroup Strategies Strategies
 * @ingroup AnalysisAgent
 */

#ifndef STRATEGY_H_
#define STRATEGY_H_

#include "Property.h"
#include "application.h"

/**
 * @class Strategy
 * @ingroup AnalysisAgent
 *
 * @brief The main class representing an analysis strategy
 */
class Strategy {
protected:
    int  max_strategy_steps;
    int  strategy_steps;
    bool require_restart;
    bool pedanticSearch;


    //The strategy algorithm
public:
    Strategy( bool pedantic = false ) :
        strategy_steps( 1 ), max_strategy_steps( 0 ), pedanticSearch( pedantic ) {
    }

    virtual ~Strategy();

    void set_strategy_steps( int steps );

    int get_strategy_steps();

    void set_max_strategy_steps( int steps );

    int get_max_strategy_steps();

    void set_require_restart( bool set );

    bool get_require_restart();

    /**
     * set Persyst Properties in Strategy
     */
    virtual void set_propid_list_persyst( std::list<int> propID_list ) {
    }

    /**
     * @brief a callback called by DataProvider to notify the strategy about a requested metric
     */
    virtual void metric_found_callback( Metric  m,
                                        Context ct );

    /**
     * @brief a callback called by DataProvider to notify the strategy about a requested region definition
     */
    virtual void region_definition_received_callback( Region* reg );

    //virtual
//  virtual std::list< Property* >
//    create_initial_candidate_properties_set( Region *initial_region ) = 0;
//
//  virtual std::list< Property* >
//  create_next_candidate_properties_set( std::list< Property* > ev_set ) = 0;

    virtual std::string name() = 0;

    /**
     * Due to new experiment execution flow it is required that:
     * reqAndConfigureFirstExperiment and evaluateAndReqNextExperiment methods call get_required_info
     * of the candidate properties
     * and configureNextExperiment only calls data_provider->transfer_requests_to_processes.
     */
    virtual bool reqAndConfigureFirstExperiment( Region* initial_region ) = 0; // TRUE can start; FALSE not ready

    virtual bool evaluateAndReqNextExperiment() = 0;                           // TRUE requires next step; FALSE if done

    virtual void configureNextExperiment() = 0;                                // Transfer requests
};

//        std::list< Property* >  get_found_properties();
void clear_found_properties();

void add_to_found_properties( Property* prop );

extern Prop_List foundProperties;

#endif /* STRATEGY_H_ */
