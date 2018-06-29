#ifndef _PSC_ANALYSIS_RESULTS_POOL
#define _PSC_ANALYSIS_RESULTS_POOL

#include <pthread.h>
#include <map>
#include <list>
#include "MetaProperty.h"
#include "selective_debug.h"
using namespace std;

class AnalysisResultsPool {
private:
    map<int, list<MetaProperty> > results_per_tuning_step;
    map<int, list<MetaProperty> > results_per_experiment;
    pthread_mutex_t               lock;
    static const pthread_mutex_t  lock_init;
public:
    AnalysisResultsPool();

    ~AnalysisResultsPool();

    bool empty( void );

    int size( void );

    void pushPreAnalysisProperty( MetaProperty property,
                                  int          tuning_step );

    void pushExperimentProperty( MetaProperty property,
                                 int          experiment_number );

    list<MetaProperty>getPreAnalysisProperties( int tuning_step );

    list<MetaProperty>getExperimentProperties( int experiment_number );

    map<int, list<MetaProperty> >getAllPreAnalysisProperties( void );

    map<int, list<MetaProperty> >getAllExperimentProperties( void );

    list<MetaProperty>* getPropertiesForScenarioIDandEntity(int id, std::string entity);

    string toString( int    indent,
                     string indentation_character );
};

#endif
