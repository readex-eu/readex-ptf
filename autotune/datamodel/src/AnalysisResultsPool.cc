#include "AnalysisResultsPool.h"
#include "Context.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include "stdio.h"
using namespace std;

#define PSC_AUTOTUNE_PROPERTIES_POOL_CHECKS

const pthread_mutex_t AnalysisResultsPool::lock_init = PTHREAD_MUTEX_INITIALIZER;

AnalysisResultsPool::AnalysisResultsPool() {
    lock = lock_init;
}

AnalysisResultsPool::~AnalysisResultsPool() {
    pthread_mutex_destroy( &lock );
}

bool AnalysisResultsPool::empty( void ) {
    PSC_AUTOTUNE_PROPERTIES_POOL_CHECKS;

    pthread_mutex_lock( &lock );
    bool is_empty = results_per_tuning_step.empty();
    pthread_mutex_unlock( &lock );

    return is_empty;
}

int AnalysisResultsPool::size( void ) {
    PSC_AUTOTUNE_PROPERTIES_POOL_CHECKS;

    pthread_mutex_lock( &lock );
    int size = results_per_tuning_step.size();
    pthread_mutex_unlock( &lock );

    return size;
}

void AnalysisResultsPool::pushPreAnalysisProperty( MetaProperty property,
                                                   int          tuning_step ) {
    PSC_AUTOTUNE_PROPERTIES_POOL_CHECKS;

    //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Inserting property in tuning step: %d\n", tuning_step );
    //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Property: %s\n", property.toString().c_str() );

    pthread_mutex_lock( &lock );
    results_per_tuning_step[ tuning_step ].push_back( property );
    pthread_mutex_unlock( &lock );
}


void AnalysisResultsPool::pushExperimentProperty( MetaProperty property,
                                                  int          experiment_number ) {
    PSC_AUTOTUNE_PROPERTIES_POOL_CHECKS;

    //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Inserting property in experiment number: %d\n", experiment_number );
    //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Property: %s\n", property.toString().c_str() );

    pthread_mutex_lock( &lock );
    results_per_experiment[ experiment_number ].push_back( property );
    pthread_mutex_unlock( &lock );
}

list<MetaProperty> AnalysisResultsPool::getPreAnalysisProperties( int tuning_step ) {
    return results_per_tuning_step[ tuning_step ];
}

list<MetaProperty> AnalysisResultsPool::getExperimentProperties( int experiment_number ) {
    return results_per_experiment[ experiment_number ];
}

list<MetaProperty>* AnalysisResultsPool::getPropertiesForScenarioIDandEntity(int id, std::string entity){
	list<MetaProperty> *props=new list<MetaProperty>;
   list<MetaProperty> properties;
   list<MetaProperty>::iterator p;

	for( int exp = 0; exp < results_per_experiment.size(); exp++ ) {
		//build up list of properties
		properties = this->getExperimentProperties( exp );
		for( p = properties.begin(); p != properties.end(); p++ ) {
         int scenarioID = atoi( p->getExtraInfo().at( "ScenarioID" ).c_str() );
         std::string entityID;
         if (p->isRtsBased()){
         	entityID=p->getCallpath();
         }else{
         	entityID=p->getRegionId();
         }
			if (scenarioID==id && entity==entityID){
				props->push_back(*p);
			}
		}
	}
	return props;
}

map<int, list<MetaProperty> > AnalysisResultsPool::getAllPreAnalysisProperties( void ) {
    return results_per_tuning_step;
}

map<int, list<MetaProperty> > AnalysisResultsPool::getAllExperimentProperties( void ) {
    return results_per_experiment;
}

string AnalysisResultsPool::toString( int    indent,
                                      string indentation_character ) {
    string                                  base_indentation;
    map<int, list<MetaProperty> >::iterator results_iter;
    list<MetaProperty>::iterator            properties_iter;
    for( int i = 0; i < indent; i++ ) {
        base_indentation.append( indentation_character );
    }
    stringstream temp;

    temp << "Total Results: " << results_per_tuning_step.size() << endl;
    for( results_iter = results_per_tuning_step.begin(); results_iter != results_per_tuning_step.end(); results_iter++ ) {
        temp << base_indentation << "Analysis Results (Properties) for Scenario with ID: " << results_iter->first <<
        "; Total: " << results_per_tuning_step.size() << ";" << endl;
        for( properties_iter = results_iter->second.begin(); properties_iter != results_iter->second.end(); properties_iter++ ) {
            temp << base_indentation << indentation_character << ( *properties_iter ).toString() << endl;
        }
    }

    return temp.str().c_str();
}
