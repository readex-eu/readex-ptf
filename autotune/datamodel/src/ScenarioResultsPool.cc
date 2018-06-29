#include "ScenarioResultsPool.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <sstream>
using namespace std;

#include "stdio.h"

#define PSC_AUTOTUNE_PROPERTIES_POOL_CHECKS

const pthread_mutex_t ScenarioResultsPool::lock_init = PTHREAD_MUTEX_INITIALIZER;

ScenarioResultsPool::ScenarioResultsPool() {
    lock = lock_init;
}

ScenarioResultsPool::~ScenarioResultsPool() {
    pthread_mutex_destroy( &lock );
}

bool ScenarioResultsPool::empty( void ) {
    PSC_AUTOTUNE_PROPERTIES_POOL_CHECKS;

    pthread_mutex_lock( &lock );
    bool is_empty = results_per_scenario_id.empty();
    pthread_mutex_unlock( &lock );

    return is_empty;
}

int ScenarioResultsPool::size( void ) {
    PSC_AUTOTUNE_PROPERTIES_POOL_CHECKS;

    pthread_mutex_lock( &lock );
    int size = results_per_scenario_id.size();
    pthread_mutex_unlock( &lock );

    return size;
}

void ScenarioResultsPool::push( MetaProperty property,
                                int          search_step ) {
    PSC_AUTOTUNE_PROPERTIES_POOL_CHECKS;

    pthread_mutex_lock( &lock );

    try {
        int scenario_id = atoi( property.getExtraInfo().at( "ScenarioID" ).c_str() );
        results_per_scenario_id[ scenario_id ].push( property );
        results_per_search_step[ search_step ].push_back( results_per_scenario_id[ scenario_id ] );
    }
    catch( ... ) {
        psc_errmsg( "toXMLExtra function is missing in your property or it does not provide ScenarioID information.\n" );
        throw 0;
    }

    pthread_mutex_unlock( &lock );
}


list<MetaProperty> ScenarioResultsPool::getScenarioResultsByID( int scenario_id ) {
    try {
        return results_per_scenario_id.at( scenario_id ).getProperties();
    }
    catch( const std::out_of_range& oor ) {
        typedef std::map<int, ScenarioResult>::iterator Iterator;

        psc_errmsg( "No information in scenario results pool for scenario id %d!\n", scenario_id );
        psc_errmsg( "    Known scenario ids:\n" );
        for( Iterator i = results_per_scenario_id.begin(); i != results_per_scenario_id.end(); ++i ) {
            psc_errmsg( "        %d\n", i->first );
        }

        throw 0;
    }
}

list<ScenarioResult> ScenarioResultsPool::getScenarioResultsPerSearchStep( int search_step ) {
    try {
        results_per_search_step.at( search_step );
    }
    catch( const std::out_of_range& oor ) {
        psc_errmsg( "No information in scenario results pool for search step %d\n", search_step );
        throw 0;
    }
    return results_per_search_step.at( search_step );
}

bool ScenarioResult::empty( void ) {
    return properties.empty();
}

int ScenarioResult::size( void ) {
    return properties.size();
}

void ScenarioResult::push( MetaProperty property ) {
    properties.push_back( property );
}

void ScenarioResultsPool::clear() {
    results_per_search_step.clear();
    results_per_scenario_id.clear();
}

list<MetaProperty> ScenarioResult::getProperties() {
    return properties;
}

string ScenarioResultsPool::toString( int    indent,
                                      string indentation_character ) {
    string                              base_indentation;
    map<int, ScenarioResult >::iterator results_iter;
    list<MetaProperty>::iterator        properties_iter;
    for( int i = 0; i < indent; i++ ) {
        base_indentation.append( indentation_character );
    }
    stringstream temp;

    temp << "Total ScenarioResults: " << results_per_scenario_id.size() << endl;
    for( results_iter = results_per_scenario_id.begin(); results_iter != results_per_scenario_id.end(); results_iter++ ) {
        list<MetaProperty> properties = results_iter->second.getProperties();
        temp << base_indentation << "Scenario Results (Properties) for Scenario with ID: " << results_iter->first <<
        "; Total: " << properties.size() << ";" << endl;
        for( properties_iter = properties.begin(); properties_iter != properties.end(); properties_iter++ ) {
            temp << base_indentation << indentation_character << ( *properties_iter ).toString() << endl;
        }
    }

    return temp.str().c_str();
}
