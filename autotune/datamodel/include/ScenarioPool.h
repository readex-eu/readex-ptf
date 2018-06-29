/**
   @file    ScenarioPool.cc
   @ingroup Autotune
   @brief   Scenario Pool header
   @author  Isaias Compres
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#ifndef _SCENARIO_POOL_
#define _SCENARIO_POOL_

#include "Scenario.h"
#include "psc_errmsg.h"
#include "Variant.h"
#include <pthread.h>
#include <queue>
using namespace std;

class ScenarioPool {
private:
    map<int, Scenario*>*         scenarios;
    pthread_mutex_t              lock;
    static const pthread_mutex_t lock_init;
public:
    ScenarioPool();

    ~ScenarioPool();

    bool empty( void );

    int size( void );

    Scenario* pop( void );

    Scenario* pop( int id );

    void push( Scenario* );

    const list<TuningSpecification*>* getTuningSpecificationByScenarioID( int id );

    Scenario* getScenarioByScenarioID( int id );

    map<int, Scenario*>* getScenarios();

    void print( void );

    void clear();

    string toString( int    indent,
                     string indentation_character );
};

#endif
