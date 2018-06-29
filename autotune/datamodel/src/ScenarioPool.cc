/**
   @file    ScenarioPool.cc
   @ingroup Autotune
   @brief   Scenario Pool
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

#include "ScenarioPool.h"

#define PSC_AUTOTUNE_SCENARIOS_CHECKS \

const pthread_mutex_t ScenarioPool::lock_init = PTHREAD_MUTEX_INITIALIZER;

ScenarioPool::ScenarioPool() {
    scenarios = new map<int, Scenario*>();
    lock      = lock_init;
}

ScenarioPool::~ScenarioPool() {
    delete scenarios;
    pthread_mutex_destroy( &lock );
}

map<int, Scenario*>* ScenarioPool::getScenarios() {
    return scenarios;
}

bool ScenarioPool::empty( void ) {
    PSC_AUTOTUNE_SCENARIOS_CHECKS;

    pthread_mutex_lock( &lock );
    bool is_empty = scenarios->empty();
    pthread_mutex_unlock( &lock );

    return is_empty;
}

void ScenarioPool::clear() {
    PSC_AUTOTUNE_SCENARIOS_CHECKS;

    pthread_mutex_lock( &lock );
    scenarios->clear();
    pthread_mutex_unlock( &lock );
}


int ScenarioPool::size( void ) {
    PSC_AUTOTUNE_SCENARIOS_CHECKS;

    pthread_mutex_lock( &lock );
    int size = scenarios->size();
    pthread_mutex_unlock( &lock );

    return size;
}

Scenario* ScenarioPool::pop( void ) {
    PSC_AUTOTUNE_SCENARIOS_CHECKS;
    map<int, Scenario*>::iterator scenario_iter;

    pthread_mutex_lock( &lock );
    scenario_iter = scenarios->begin();
    Scenario* scenario = scenario_iter->second;
    //scenario->print();
    scenarios->erase( scenario_iter->first ); // LM -- Erase the pair: must save the data before.
    pthread_mutex_unlock( &lock );

    return scenario;
}

Scenario* ScenarioPool::pop( int id ) {
    PSC_AUTOTUNE_SCENARIOS_CHECKS;
    map<int, Scenario*>::iterator scenario_iter;

    Scenario* scenario = NULL;
    pthread_mutex_lock( &lock );
    for( scenario_iter = scenarios->begin(); scenario_iter != scenarios->end(); scenario_iter++ ) {
        if( scenario_iter->second->getID() == id ) {
            scenario = scenario_iter->second;
            scenarios->erase( scenario_iter->first );
            break;
        }
    }
    pthread_mutex_unlock( &lock );

    return scenario;
}

void ScenarioPool::print( void ) {
    PSC_AUTOTUNE_SCENARIOS_CHECKS;
    map<int, Scenario*>::iterator scenario_iter;

    pthread_mutex_lock( &lock );

    for( scenario_iter = scenarios->begin(); scenario_iter != scenarios->end(); scenario_iter++ ) {
        Scenario* scenario = scenario_iter->second;

        if( scenario != NULL ) {
            scenario->print();
        }
    }

    pthread_mutex_unlock( &lock );
}

void ScenarioPool::push( Scenario* scenario ) {
    PSC_AUTOTUNE_SCENARIOS_CHECKS;

    pthread_mutex_lock( &lock );
    scenarios->insert( make_pair( scenario->getID(), scenario ) );
    pthread_mutex_unlock( &lock );
}


//void ScenarioPool::print() {
//  PSC_AUTOTUNE_SCENARIOS_CHECKS;
//  map<int, Scenario*>::iterator scenario_iter;
//  int i = 0;
//  cout << "Scenario pool:" << endl;
//
//  pthread_mutex_lock( &lock );
//
//  for (scenario_iter = scenarios->begin(); scenario_iter != scenarios->end(); scenario_iter++, i++) {
//    cout << i << ". scenario" << endl;
//    Scenario* scenario = scenario_iter->second;
//    scenario->print();
//  }
//
//  pthread_mutex_unlock( &lock );
//
//  return;
//}

const list<TuningSpecification*>* ScenarioPool::getTuningSpecificationByScenarioID( int id ) {
    return ( *scenarios )[ id ]->getTuningSpecifications();
}

Scenario* ScenarioPool::getScenarioByScenarioID( int id ) {
    return ( *scenarios )[ id ];
}

string ScenarioPool::toString( int    indent,
                               string indentation_character ) {
    PSC_AUTOTUNE_SCENARIOS_CHECKS;
    map<int, Scenario*>::iterator scenario_iter;
    string                        base_indentation;
    for( int i = 0; i < indent; i++ ) {
        base_indentation.append( indentation_character );
    }
    stringstream temp;

    pthread_mutex_lock( &lock );

    temp << base_indentation << "Total Scenarios:" << scenarios->size() << endl << endl;

    for( scenario_iter = scenarios->begin(); scenario_iter != scenarios->end(); scenario_iter++ ) {
        Scenario* scenario = scenario_iter->second;
        if( scenario != NULL ) {
            temp << scenario->toString( indent + 1, indentation_character ) << endl;
        }
        else {
            psc_errmsg( "NULL Scenario in ScenarioPool\n" );
            throw 0;
        }
    }

    pthread_mutex_unlock( &lock );

    return temp.str().c_str();
}
