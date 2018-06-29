/**
   @file    SpeedupAnalysis.cc
   @ingroup PCAPPlugin
   @brief   Plugin interface
   @author  Shajulin Benedict
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2012-2015, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include "PCAPPlugin.h"
#include <sstream>
#include <math.h>
#include <map>
//#include "omp.h"
#include "frontend_statemachine.h"
#include "MetaProperty.h"
#include "frontend.h"
#include "application.h"


//#include "psc_agent.h"
//#include "PropertyID.h"
//#include "Scenario.h"

//extern vector<MetaProperty> energyOpenMP_scalability_prop;
list<int> mapped_thread_value;
list<int> scenario_prop_size;
list<int> twopowern_list;


/*
 * Operations to be done at the start of a tuning step.
 *
 * The tuning parameters that were created at initialization are used to create a variant space.
 * The variant space and the regions are then used to create a search space that is passed to the
 * search algorithm.
 *
 */
void PCAPPlugin::startTuningStep1SpeedupAnalysis( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPPlugin: call to startTuningStep1SpeedupAnalysis()\n" );

    scenario_prop_size.clear();
    twopowern_list.push_back( 1 );
    twopowern_list.push_back( 2 );
    twopowern_list.push_back( 4 );
}


/*
 * The Created Scenario Pool (csp) is populated here.
 *
 * The scenarios need to be created and added to the first pool.  To create the scenarios, a
 * search algorithm can be used or it can be done directly by the plugin.
 *
 * After this step, the Periscope will verify that scenarios were added to the csp.
 *
 */
void PCAPPlugin::createScenarios1SpeedupAnalysis( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPPlugin: call to createScenarios1SpeedupAnalysis()\n" );



    //Calculate 2^n and add it here
    double power_of_n      = 0;
    bool   power_of_n_flag = true;


    //Initialization for the scalability scenarios - DO we need to create scenarios for 2^n times?
    for( int i = 1; i <= number_of_cores; i++ ) {
        //Calculate the power of n flag here
        if( power_of_n == i ) {
            power_of_n_flag = true;
        }
        else {
            power_of_n_flag = false;
        }

        if( power_of_n_flag || i == 1 ) {
            power_of_n = pow( 2, i );
            //std::cout << "  power_of_n " << power_of_n<<std::endl;

            //The scenario push_back code goes here
            //Initialization
            Scenario*                    scalability_scenario;
            list<TuningSpecification* >* scalability_tuning_spec_list;
            TuningSpecification*         scalability_tuning_spec;
            Variant*                     scalability_variant;

            //Other Initialization
            list<unsigned int>*     processes_list;
            list<Region*>*          region_list;
            list<PropertyRequest*>* execTime_property_requests_list;

            //Always assign processes value as rank 0
            processes_list = new list<unsigned int>;
            processes_list->push_back( 0 ); //sss process number should be zero for threaded applications?

            //Always assign region as phase_region
            region_list = new list<Region*>;
            region_list->push_back( app.get_phase_region() );

            //Always assign execution Time as properties
            execTime_property_requests_list = new list<PropertyRequest*>;
            PropertyRequest* execTime_property_requests;

            list<int>* propertyIDs;
            propertyIDs = new list<int>;
            propertyIDs->push_back( 50 ); //PHASE_REGION_EXEC_TIME
            propertyIDs->clear();         //Currently, property IDs are zero

            execTime_property_requests = new PropertyRequest( propertyIDs );

            execTime_property_requests_list->push_back( execTime_property_requests );
            execTime_property_requests_list->clear(); //sss test purpose

            //Insert mapped variants' value (ie the number of thread value here)
            map<TuningParameter*, int> mapped_var_value;
            mapped_var_value[ OpenMP_tp ] = i;
            mapped_thread_value.push_back( i );


            //Create new objects
            scalability_variant          = new Variant( mapped_var_value );
            scalability_tuning_spec      = new TuningSpecification( scalability_variant, processes_list, region_list );
            scalability_tuning_spec_list = new list<TuningSpecification*>();

            //Insert tuning specifications in the TSP list
            scalability_tuning_spec_list->push_back( scalability_tuning_spec );

            //Insert TSP list, phase region, and rank into the scenarios
            //scalability_scenario = new Scenario(energyOpenMP_Appl.get_phase_region(), scalability_tuning_spec_list, execTime_property_requests_list);
            scalability_scenario = new Scenario( app.get_phase_region(), scalability_tuning_spec_list, NULL ); //Property IDs are zero
            pool_set->csp->push( scalability_scenario );


            power_of_n_flag = false;


            //Do we need to delete all lists and databases here (except scenario)?
//      delete[] scalability_tuning_spec_list;
//      delete[] scalability_tuning_spec;
//      delete[] scalability_variant;
//      delete[] processes_list;
//      delete[] region_list;
//      delete[] execTime_property_requests_list;
//      delete[] execTime_property_requests;
//      delete[] propertyIDs;
        }
        else {
            power_of_n_flag = false;
        }
    }
}

/*
 * Preparatory steps for the scenarios are done here.
 *
 * If there are any preparatory steps required by some or all scenarios in the csp (for example:
 * the project may need to be re-compiled), then they are to be performed here.  After each
 * scenario is prepared, they are migrated from the csp to the Prepared Scenario Pool (psp).
 *
 * In some cases, no preparation may be necessary and the plugin can simply move all scenarios
 * from the csp to the psp.
 *
 * After this step, the Periscope will verify that scenarios were added to the psp.
 *
 */
void PCAPPlugin::prepareScenarios1SpeedupAnalysis( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPPlugin: call to prepareScenarios1SpeedupAnalysis()\n" );

    //no preparation is necessary, just move the elements on the pools
    while( !pool_set->csp->empty() ) {
        std::cout << " Create Scenarios are not empty " << std::endl;
        pool_set->psp->push( pool_set->csp->pop() );
    }
}

/*
 * Populate the Experiment Scenario Pool (esp) for the next experiment.
 *
 * This is the final step before the experiments are executed. Scenarios are moved from the
 * psp to the esp, depending on the number of processes and whether they can be executed
 * in parallel.
 *
 * After this step, the Periscope will verify that scenarios were added to the esp.
 *
 */
void PCAPPlugin::defineExperiment1SpeedupAnalysis( int               numprocs,
                                                   bool&             analysisRequired,
                                                   StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPPlugin: call to defineExperiment1SpeedupAnalysis()\n" );

    int       i;
    Scenario* scenario;

    //for(i=0; !psp->empty() && i<numprocs ; i++){
    //for(i=0; !psp->empty(); i++){
    //select scenari//o of this rank
    scenario = pool_set->psp->pop();
    const list<TuningSpecification*>* ts = scenario->getTuningSpecifications();
    if( ts->size() != 1 ) {
        perror( "energyOpenMP Plugin can't currently handle multiple TuningSpecifications\n" );
        throw 0;
    }

    //int thread_val_to_set_in_frontend = collect_mapped_thread_value();

    //Go through the list and get the thread variant value and then set them with the frontend
    //fe->set_ompnumthreads(thread_val_to_set_in_frontend); //sss to test purpose
    //appl->setOmpThreads(thread_val_to_set_in_frontend); //sss



    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                "setting single rank 0 in scenario with id: %d\n", scenario->getID() );
    //define rank in the tuning specification
    ts->front()->setSingleRank( 0 ); //Rank is always 0 for OpenMP



    //energyOpenMP_Appl.get_phase_region(),EXECTIME sss
    scenario->setSingleTunedRegionWithPropertyRank( app.get_phase_region(), EXECTIME, 0 );
    pool_set->esp->push( scenario );
    //}

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "[#### AUTOTUNE ####]: Added 1 scenario in this experiment.\n", i );

    energyOpenMP_scalability_prop.clear();
    // TODO check whether we really want to allow plugins to access the frontend data directly -IC
    energyOpenMP_scalability_prop = fe->metaproperties_;
    scenario_prop_size.push_back( energyOpenMP_scalability_prop.size() );



    StrategyRequestGeneralInfo* strategyRequestGeneralInfo = new StrategyRequestGeneralInfo;

    strategyRequestGeneralInfo->strategy_name     = "OMP";
    strategyRequestGeneralInfo->pedantic          = 1;
    strategyRequestGeneralInfo->delay_phases      = 0;
    strategyRequestGeneralInfo->delay_seconds     = 0;
    strategyRequestGeneralInfo->analysis_duration = 1;

    *strategy = new StrategyRequest( strategyRequestGeneralInfo );
}

/*
 * Collect mapped scenario value

   //int PCAPPlugin::collect_mapped_thread_value(){
   //  std::cout << " Extracting the list of Tuning Specifications "<< std::endl;

   const list<TuningSpecification*> *ts_to_map;
   ts_to_map = new list<TuningSpecification*>;
   ts_to_map = mapping_scenario->getTuningSpecifications();


   typedef map<TuningParameter*, int>::iterator tp_energyOpenMP_iter;
   typedef list<TuningSpecification*>::iterator ts_energyOpenMP_iter;
   printf("--------------------------------------------------------------------------------\n");
   printf("Scenario ID: %d\n", mapping_scenario_id);fflush(stdout);
   printf(" Region information:\n");fflush(stdout);
   printf(" TuningSpecifications: %d\n", ts_to_map->size());fflush(stdout);
   int ts_energyOpenMP_count = 0;

    for(ts_energyOpenMP_iter tsi = ts_to_map->begin(); tsi != ts_to_map->end() ; tsi++, ts_energyOpenMP_count++){
      printf(" - TuningSpecification: %d\n", ts_energyOpenMP_count);fflush(stdout);
      map<TuningParameter*, int> thread_values = (*tsi)->getVariant()->getValue();
      printf(" -- Variant (%d TuningParameters):\n", thread_values.size());fflush(stdout);
      int par_count = 0;
      for(tp_energyOpenMP_iter i = thread_values.begin(); i != thread_values.end(); i++, par_count++) {
        printf(" --- TuningParameter %d:\n", par_count);fflush(stdout);
        printf(" ---- Name: %s\n", i->first->getName().c_str());fflush(stdout);
        printf(" ---- Value: %d\n", i->second);fflush(stdout);
      }
    }


   int mapping_scenario_id = mapped_thread_value.front();
   mapped_thread_value.pop_front();

   return mapping_scenario_id;


   }
 */



/*
 * Final operation of a tuning step.
 *
 * If any post-processing is necessary before entering the next tuning iteration, it is to be
 * done here.
 */
void PCAPPlugin::finishTuningStep1SpeedupAnalysis( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPPlugin: call to finishTuningStep1SpeedupAnalysis()\n" );

    energyOpenMP_scalability_prop.clear();
    // TODO check whether we really want to allow plugins to access the frontend data directly -IC
    energyOpenMP_scalability_prop = fe->metaproperties_;
    std::vector<MetaProperty> local_meta_prop;

    scenario_prop_size.push_back( energyOpenMP_scalability_prop.size() );


    //get the scenario id
    scenario_prop_size.pop_front();
    int scenario_candidate_size = scenario_prop_size.front();


    int thread_for_config =  twopowern_list.front();

    int count = 0;
    //Mark to the number of properties with the correct configuration
    //for (std::list<MetaProperty >::iterator s_itt = energyOpenMP_scalability_prop.begin(); s_itt != energyOpenMP_scalability_prop.end(); s_itt++ ){
    for( int i = 0; i < energyOpenMP_scalability_prop.size(); i++ ) {
        if( count < scenario_candidate_size ) {
            std::string threads_str = boost::lexical_cast<std::string>( thread_for_config );
            std::string procs_str   = boost::lexical_cast<std::string>( ( energyOpenMP_scalability_prop[ i ] ).getMaxProcs() );

            std::string config_change = procs_str  + "x" + threads_str;

            //std::cout << " thread_for_config " << thread_for_config << " Configuration string " << config_change <<" scenario_candidate_size " << scenario_candidate_size  << " Procs " << procs_str << " thread " <<  threads_str << std::endl;

            ( energyOpenMP_scalability_prop[ i ] ).setConfiguration( config_change );
            string prop_string = ( energyOpenMP_scalability_prop[ i ] ).toXML();
            std::cout << prop_string << std::endl;

            local_meta_prop.push_back( energyOpenMP_scalability_prop[ i ] );

            count++;
        }
        else {
            scenario_candidate_size = scenario_prop_size.front();
            scenario_prop_size.pop_front();

            twopowern_list.pop_front();
            thread_for_config =  twopowern_list.front();


            count = 0;

            std::string threads_str = boost::lexical_cast<std::string>( thread_for_config );
            std::string procs_str   = boost::lexical_cast<std::string>( ( energyOpenMP_scalability_prop[ i ] ).getMaxProcs() );

            std::string config_change = procs_str  + "x" + threads_str;

            std::cout << " Configuration string " << config_change << " scenario_candidate_size " << scenario_candidate_size  << " Procs " << procs_str << " thread " <<  threads_str << std::endl;

            ( energyOpenMP_scalability_prop[ i ] ).setConfiguration( config_change );
            string prop_string = ( energyOpenMP_scalability_prop[ i ] ).toXML();
            std::cout << prop_string << std::endl;

            local_meta_prop.push_back( energyOpenMP_scalability_prop[ i ] );

            count++;
        }
    }

    std::cout << " list of local_meta_prop " << local_meta_prop.size() << std::endl;
    // TODO check whether we really want to allow plugins to access the frontend data directly -IC
    fe->metaproperties_ = local_meta_prop;


    //Ensure that the psc file is new :::TODO::
    std::cout << " Invoking do_pre_analysis from plugin " << std::endl;
    fe->do_pre_analysis();
    fe->export_scalability_properties();

    //return searchAlgorithm_e->searchFinished();

    tuningStep++;
}
