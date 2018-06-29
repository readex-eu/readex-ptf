/**
   @file    PerfDynamicsAnalysis.hpp
   @ingroup PerfDynamicsAnalysis
   @brief   Performance Dynamics Analysis strategy header
   @author  Yury Oleynik
   @verbatim
    Revision:       $Revision$
    Revision date:  Dec 10, 2013
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2015, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

/**
 * @defgroup PerfDynamicsAnalysis Performance Dynamics Analysis Strategy
 * @ingroup Strategies
 */

#ifndef PERFDYNAMICSANALYSIS_HPP_
#define PERFDYNAMICSANALYSIS_HPP_

#include <string>
#include <map>

#include "global.h"
#include "application.h"
#include "Metric.h"
#include "strategy.h"

//#include "DSP_Engine.hpp"

#include "TDA_QSequence.hpp"


using namespace std;

class PerfDynamicsStrategy : public Strategy {
    Region*   phaseRegion;
    Prop_List candProperties;
    Prop_List foundPropertiesLastStep;
    Strategy* staticStrategy;
    string    staticStrategyName;
    bool      test_mode;

    std::map<std::string, TDA_Stuff*> tda_stuffs;


    //map<string, DSP_Engine*> engines;

    int burst_begin;
    int burst_length;


public:
    PerfDynamicsStrategy( string       staticStrategyName,
                          Application* application,
                          int          duration,
                          bool         pedantic = false );

    ~PerfDynamicsStrategy() {
        delete staticStrategy;
        map<string, TDA_Stuff*>::iterator it;
        for( it = tda_stuffs.begin(); it != tda_stuffs.end(); it++ ) {
            delete( it->second );
        }
    }

    std::list <Property*>create_initial_candidate_properties_set( Region* initial_region );

    std::list <Property*>create_next_candidate_properties_set( std::list< Property* > ev_set );

    std::string name();

    bool reqAndConfigureFirstExperiment( Region* initial_region ); // TRUE can start; FALSE not ready

    bool evaluateAndReqNextExperiment();                           // TRUE requires next step; FALSE if done

    void configureNextExperiment();

    void region_definition_received_callback( Region* reg );
};



#endif /* PERFDYNAMICSANALYSIS_HPP_ */
