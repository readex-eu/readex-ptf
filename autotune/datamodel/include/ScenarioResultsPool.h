#ifndef _PSC_OBJECTIVE_FUNCTION_VALUES__
#define _PSC_OBJECTIVE_FUNCTION_VALUES__

#include <pthread.h>
#include <map>
#include <list>
#include "MetaProperty.h"
#include "selective_debug.h"
using namespace std;

class ScenarioResult {
private:
    int                id;
    list<MetaProperty> properties;
public:
    bool empty( void );

    int size( void );

    void push( MetaProperty );

    list<MetaProperty>getProperties( void );
};

class ScenarioResultsPool {
private:
    map<int, list<ScenarioResult> > results_per_search_step;
    map<int, ScenarioResult >       results_per_scenario_id;
    pthread_mutex_t                 lock;
    static const pthread_mutex_t    lock_init;
public:
    ScenarioResultsPool();

    ~ScenarioResultsPool();

    bool empty( void );

    int size( void );

    void
    push( MetaProperty, int );

    void clear();

    list<MetaProperty>getScenarioResultsByID( int scenario_id );

    list<ScenarioResult>getScenarioResultsPerSearchStep( int search_step );

    map<int, list<MetaProperty> >getProperties( void );

    string toString( int    indent,
                     string indentation_character );
};

#endif
