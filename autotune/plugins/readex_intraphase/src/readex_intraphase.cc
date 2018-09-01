/**
 @file    readex_intraphase.cc
 @ingroup ReadexIntraphasePlugin
 @brief   Readex intra-phase tuning plugin based on dvfs_taurus
 @author  Madhura Kumaraswamy
 @verbatim
 Revision:       $Revision$
 Revision date:  $Date$
 Committed by:   $Author$

 This file is part of the Periscope performance measurement tool.
 See http://www.lrr.in.tum.de/periscope for details.

 Copyright (c) 2016, Technische Universitaet Muenchen, Germany
 See the COPYING file in the base directory of the package for details.
 Copyright (c) 2016, Technische Universitaet Dresden, Germany
 See the COPYING file in the base directory of the package for details.
 @endverbatim
 */

#include "../include/readex_intraphase.h"

#include <vector>
#include <list>
#include <map>
#include <limits>
#include <float.h>
#include <iterator>
#include <utility>
#include <memory>

#include "search_common.h"
#include "frontend.h"
#include "IndividualSearch.h"
#include "RandomSearch.h"
#include "GDE3Search.h"
#include "ExhaustiveATPSearch.h"
#include "IndividualATPSearch.h"
#include "ATPService.h"
#include "regxx.h"
#include "timing.h"

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/optional/optional.hpp>
#include <boost/optional.hpp>

std::ostringstream result_oss;
int                readexScenario;
atpService*        atp_srvc;

/**
 * @brief Initialize the plugin's data structures.
 *
 * The tuning parameter list is created.
 *
 * Search algorithms are loaded here when required. This can be done as follows:
 *
 *    searchAlgorithm = loadSearchAlgorithm("name");
 *
 * where "name" refers to one of the available search algorithms (currently only exhaustive).
 *
 * @ingroup ReadexIntraphasePlugin
 *
 * @param context a pointer to a context for a plugin
 * @param pool_set a pointer to a set of pools for a plugin
 */
void ReadexIntraphasePlugin::initialize(DriverContext* context, ScenarioPoolSet* pool_set) {

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: call to initialize()\n");
    tuningStep      = 0;
    scenario_no_atp = 0;
    this->context   = context;
    this->pool_set  = pool_set;
    min_freq        = 1200;
    max_freq        = 2400;
    freq_step       = 400;

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: got pool and context\n");
    /*Whether or not ATP, we increase tuningStep*/
    tuningStep++;

//    context->loadSearchAlgorithm( "exhaustive", &major, &minor, &name, &description );
//    searchAlgorithm = context->getSearchAlgorithmInstance( "exhaustive" );

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: initialize() finished \n\n");
}

/**
 * @brief Operations to be done at the start of a tuning step.
 *
 * The tuning parameters that were created at initialization are used to create a variant space.
 * The variant space and the regions are then used to create a search space that is passed to the
 * search algorithm.
 *
 * @ingroup ReadexIntraphasePlugin
 *
 */
void ReadexIntraphasePlugin::startTuningStep(void) {
    int threadsLB(1), id(0);
    int threadsStep(1);

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: call to startTuningStep()\n");

    /*Tuning Step for ATP*/

    /*Check ATP execution mode*/
    atp_execution_mode = getenv("ATP_EXECUTION_MODE");
    if ( tuningStep == ATP_PARAMETER_TUNING )
    {
        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: Performs Tuning Step: %d\n", tuningStep);
        struct stat   buffer;

        if( atp_execution_mode != NULL && strcmp (atp_execution_mode, "DTA") == 0 && stat ("ATP_description_file.json", &buffer) == 0)
        {
            std::unordered_map <std::string, vector< TuningParameter* > > domain_tp_mapping;
            /*Application Tuning Parameters will be requested from ATP Server*/
            RegistryService* regsrv = fe->get_registry();
            double g_timedout = fe->get_global_timeout();
            atp_srvc = new atpService(g_timedout);
            if (atp_srvc->startATPServer(regsrv) !=-1)
            {
                psc_dbgmsg (PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ),
                            "Readex Tuning Plugin is ready to send request to ATP Server\n");
                //get application tuning parameters from ATP Server
                domain_tp_mapping = atp_srvc->getATPSpecs();
            }

            if ( domain_tp_mapping.size()>0 )
            {
                /*Load search Algorithm for ATP*/
                int major_atp, minor_atp;
                string name_atp, description_atp;

                //Read the search algorithm from readex_configuration file Now only exhaustiveATP
                psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(ApplTuningParameter), "ReadexIntraphasePlugin: Obtaining search algorithm for ATP.\n");

                std::string searchAlgorithmName;
                try {
                    searchAlgorithmName = configTree.get < std::string > ("Configuration.periscope.atp.searchAlgorithm.name");
                } catch (exception &e) {
                    psc_dbgmsg(1, "ReadexIntraphasePlugin: no search algorithm for ATP given in configuration file. Using default.\n");
                    searchAlgorithmName = "exhaustive_atp";
                }
                psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: Search algorithm for ATP: %s\n", searchAlgorithmName.c_str());

                context->loadSearchAlgorithm(searchAlgorithmName, &major_atp, &minor_atp, &name_atp, &description_atp);
                searchAlgorithm = context->getSearchAlgorithmInstance(searchAlgorithmName);

                if (searchAlgorithmName == "individual_atp" ) {
                    ((IndividualATPSearch*)searchAlgorithm)->setATPService (atp_srvc);
                    ((IndividualATPSearch*) searchAlgorithm)->setScenariosToKeep(1);
                }
                else if ( searchAlgorithmName == "exhaustive_atp" )
                    ((ExhaustiveATPSearch*)searchAlgorithm)->setATPService (atp_srvc);
                else{
                    perror(" ATP only supports individual_atp and exhaustive_atp searchAlgorithm\n");
                    throw PTF_PLUGIN_ERROR(NULL_REFERENCE);
                }

                if (searchAlgorithm) {
                    print_loaded_search(major_atp, minor_atp, name_atp, description_atp);

                    searchAlgorithm->initialize(context, pool_set);
                } else {
                    perror("NULL pointer in ATPsearchAlgorithm\n");
                    throw PTF_PLUGIN_ERROR(NULL_REFERENCE);
                }

                // If there is no constraint, treat ATPs as usual tuning parameters. Because get ATPSpecs already gives all the ATPs across all the domains. If there is constraint,
                for (std::unordered_map <std::string, vector< TuningParameter* > >::iterator it = domain_tp_mapping.begin(); it != domain_tp_mapping.end(); ++it){

                    /*For each domain add all the tuning parameters of that domain to the search space*/
                    VariantSpace* variantSpace = new VariantSpace();
                    SearchSpace* searchSpace = new SearchSpace();
                    for ( vector< TuningParameter* >::iterator atp_it = it->second.begin(); atp_it != it->second.end(); ++atp_it )
                    {
                        (*atp_it)->setId(id++);
                        tuningParameters.push_back (*atp_it);
                        variantSpace->addTuningParameter(*atp_it);
                    }
                    searchSpace->setVariantSpace(variantSpace);
                    std::list<Rts*> rtsList = appl->getCalltreeRoot()->getValidRtsOfRegion(appl->get_phase_region(), NULL);
                    if (rtsList.empty()){
                        psc_abort("ReadexIntraphasePlugin: No rts based Phase\n");
                    }
                    std::list<Rts*>::iterator rts_it;
                    for (rts_it = rtsList.begin(); rts_it != rtsList.end(); rts_it++) {
                        searchSpace->addRts(*rts_it);
                    }
                    //add domain
                    searchSpace->addDomain(it->first);
                    searchAlgorithm->addSearchSpace(searchSpace);
                }
                psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: Tuning parameters across all domains are:\n");
                for (int i = 0; i < tuningParameters.size(); i++) {
                    cout << tuningParameters[i]->toString() << endl;
                }

                std::string timeUnit;
                std::string energyUnit;
                std::string currencyUnit;

                //Read objectives
                psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: Obtaining objectives.\n");
                std::list < std::string > objectiveFunctionList;


                try {
                    if (opts.has_configurationfile) {
                        // extract the significant regions from configuration file provided by readex-dyn-detect tool
                        BOOST_FOREACH(ptree::value_type & v, configTree.get_child("Configuration.objectives"))
                        {
                            std::string name = v.second.data();
                            psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "Objective Function: %s \n", name.c_str());
                            objectiveFunctionList.push_back(name);
                        }
                        timeUnit = configTree.get < std::string > ("Configuration.periscope.metricUnits.timeUnit");
                        energyUnit = configTree.get < std::string > ("Configuration.periscope.metricUnits.energyUnit");
                        currencyUnit = configTree.get < std::string > ("Configuration.periscope.metricUnits.currencyUnit");
                    }

                } catch (exception &e) {
                    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: %s\n", e.what());
                }

                if (objectiveFunctionList.size() > 0) {
                    BOOST_FOREACH(std::string & s, objectiveFunctionList)
                    {
                        if (s == "Energy") {
                            std::string unit = energyUnit;
                            searchAlgorithm->addObjectiveFunction(new EnergyObjective(energyUnit));
                            objectives.push_back(new EnergyObjective(energyUnit));
                          } else if (s == "NormalizedEnergy") {
                              std::string unit = energyUnit.append("/instr");
                              searchAlgorithm->addObjectiveFunction(new NormalizedEnergyObjective(unit));
                              objectives.push_back(new NormalizedEnergyObjective(unit));
                          } else if (s == "CPUEnergy") {
                              std::string unit = energyUnit;
                              searchAlgorithm->addObjectiveFunction(new CPUEnergyObjective(unit));
                              objectives.push_back(new CPUEnergyObjective(unit));
                          } else if (s == "NormalizedCPUEnergy") {
                              std::string unit = energyUnit.append("/instr");
                              searchAlgorithm->addObjectiveFunction(new NormalizedCPUEnergyObjective(unit));
                              objectives.push_back(new NormalizedCPUEnergyObjective(unit));
                          } else if (s == "EDP") {
                              std::string unit = energyUnit.append("*").append(timeUnit);
                              searchAlgorithm->addObjectiveFunction(new EDPObjective(unit));
                              objectives.push_back(new EDPObjective(unit));
                          } else if (s == "NormalizedEDP") {
                              std::string unit = energyUnit.append("*").append(timeUnit).append("/instr");
                              searchAlgorithm->addObjectiveFunction(new NormalizedEDPObjective(unit));
                              objectives.push_back(new NormalizedEDPObjective(unit));
                          } else if (s == "ED2P") {
                              std::string unit = energyUnit.append("*").append(timeUnit).append("2");
                              searchAlgorithm->addObjectiveFunction(new ED2PObjective(unit));
                              objectives.push_back(new ED2PObjective(unit));
                          } else if (s == "NormalizedED2P") {
                              std::string unit = energyUnit.append("*").append(timeUnit).append("2").append("/instr");
                              searchAlgorithm->addObjectiveFunction(new NormalizedED2PObjective(unit));
                              objectives.push_back(new NormalizedED2PObjective(unit));
                          } else if (s == "Time") {
                              std::string unit = timeUnit;
                              searchAlgorithm->addObjectiveFunction(new TimeObjective(unit));
                              objectives.push_back(new TimeObjective(unit));
                          } else if (s == "NormalizedTime") {
                              std::string unit = timeUnit.append("/instr");
                              searchAlgorithm->addObjectiveFunction(new NormalizedTimeObjective(unit));
                              objectives.push_back(new NormalizedTimeObjective(unit));
                          } else if (s == "TCO") {
                              std::string unit = currencyUnit;
                              searchAlgorithm->addObjectiveFunction(new TCOObjective(unit));
                              objectives.push_back(new TCOObjective(unit));
                          } else if (s == "NormalizedTCO") {
                              std::string unit = currencyUnit.append("/instr");
                              searchAlgorithm->addObjectiveFunction(new NormalizedTCOObjective(unit));
                              objectives.push_back(new NormalizedTCOObjective(unit));
                          }
                          else
                              psc_errmsg("Unknown objective %s\n", s.c_str());
                      }
                  } else {
                      std::string unit = energyUnit;
                      searchAlgorithm->addObjectiveFunction(new EnergyObjective(unit));
                      objectives.push_back(new EnergyObjective(unit));
                  }
                }

        } else {
            tuningStep = HARDWARE_PARAMETER_TUNING;
        }
        //tuningStep++; //tuningStep for Hardware parameter Tuning Anyway for NOW
    }

    /*Tuning Step for HARDWARE TUNING PARAMETER*/
    if (tuningStep == HARDWARE_PARAMETER_TUNING) {

        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: Performs Tuning Step: %d\n", tuningStep);
        /*
         * Access tuning parameters from the global configTree
         */

        int default_cpu_freq(2500);
        try {
            this->min_freq = atoi(configTree.get < std::string > ("Configuration.tuningParameter.frequency.min_freq").c_str());
            this->max_freq = atoi(configTree.get < std::string > ("Configuration.tuningParameter.frequency.max_freq").c_str());
            this->freq_step = atoi(configTree.get < std::string > ("Configuration.tuningParameter.frequency.freq_step").c_str());
            default_cpu_freq = atoi(configTree.get < std::string > ("Configuration.tuningParameter.frequency.default").c_str());
        } catch (exception &e) {
            psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: %s\n", e.what());
        }

        try {
            threadsLB = atoi(configTree.get < std::string > ("Configuration.tuningParameter.openMPThreads.lower_value").c_str());
            threadsStep = atoi(configTree.get < std::string > ("Configuration.tuningParameter.openMPThreads.step").c_str());
        } catch (exception &e) {
            psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: %s\n", e.what());
        }


        /* Check the datatype of each entry */
        if( isnan ( this->min_freq ) )
            psc_abort( "min_freq attribute has to be a number\n" );
        if( isnan ( this->max_freq ) )
            psc_abort( "max_freq attribute has to be a number\n" );
        if( isnan ( this->freq_step ) )
            psc_abort( "freq_step attribute has to be a number\n" );

        /*check if minimum frequency is smaller than maximum frequency*/
        if( this->min_freq > this->max_freq )
            psc_abort( "Minimum frequency can not be greater than maximum frequency\n" );

        if( ( this->max_freq - this->min_freq)%this->freq_step != 0 ){
          int steps=(this->max_freq - this->min_freq)/this->freq_step;
          this->max_freq=this->min_freq+steps*this->freq_step;
        }

        /* Check the datatype of each entry */
        if( isnan ( threadsLB ) )
            psc_abort( "Lower value of OpenMP Thread has to be a number\n" );

        if( isnan ( threadsStep ) )
            psc_abort( "OpenMP Thread Step has to be a number\n" );

        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: got options from the global configTree \n");

        /* TODO add the best configuration of the tuning parameter. For now Clear Tuning Parameter*/

        if ( tuningParameters.size() >0 )
            tuningParameters.clear();

        /*If ATP is enabled, create tuning  parameter with the optimum ATPs */

        if( atp_execution_mode != NULL && strcmp (atp_execution_mode, "DTA") == 0 )
        {
            for ( auto it = optimum_atp_map.begin(); it != optimum_atp_map.end(); ++it )
            {
                TuningParameter* atp_i = new TuningParameter();
                atp_i->setId(id++);
                atp_i->setName(it->first);
                atp_i->setPluginType(Readex_Intraphase);
                std::cout << "First: " << it->first << " Second "<< it->second << endl;
                atp_i->setRange(it->second, it->second, it->second);
                atp_i->setRuntimeActionType(TUNING_ACTION_FUNCTION_POINTER);
                tuningParameters.push_back(atp_i);
            }
        }

        TuningParameter* numberOfThreads = new TuningParameter();
        numberOfThreads->setId(id++);
        numberOfThreads->setName("NUMTHREADS");
        numberOfThreads->setPluginType(Readex_Intraphase);
        //lower_value for OpenMP threads is less than --ompnumthreads
        if( threadsLB > context->getOmpnumthreads() )
            psc_abort( "The lower value can not be greater than the total number of openMP threads\n" );

        numberOfThreads->setRange(threadsLB, context->getOmpnumthreads(), threadsStep);
        numberOfThreads->setDefaultValue(context->getOmpnumthreads());
        numberOfThreads->setRuntimeActionType(TUNING_ACTION_FUNCTION_POINTER);
        tuningParameters.push_back(numberOfThreads);



        TuningParameter* cpuFrequency = new TuningParameter();
        cpuFrequency->setId(id++);
        cpuFrequency->setName("CPU_FREQ");
        cpuFrequency->setPluginType(Readex_Intraphase);
        cpuFrequency->setRange(min_freq, max_freq, freq_step);
        cpuFrequency->setDefaultValue(default_cpu_freq);
        cpuFrequency->setRuntimeActionType(TUNING_ACTION_FUNCTION_POINTER);
        tuningParameters.push_back(cpuFrequency);

        int min(1000), max(3000), step(1000), default_uncore_freq(3000);
        try {
            min = atoi(configTree.get < std::string > ("Configuration.tuningParameter.uncore.min_freq").c_str());
            max = atoi(configTree.get < std::string > ("Configuration.tuningParameter.uncore.max_freq").c_str());
            step = atoi(configTree.get < std::string > ("Configuration.tuningParameter.uncore.freq_step").c_str());
            default_uncore_freq = atoi(configTree.get < std::string > ("Configuration.tuningParameter.uncore.default").c_str());

        } catch (exception &e) {
            psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: %s\n", e.what());
        }

        /* Check the datatype of each entry */
        if( isnan ( min ) )
            psc_abort( "min_freq attribute has to be a number\n" );
        if( isnan ( max ) )
            psc_abort( "max_freq attribute has to be a number\n" );
        if( isnan ( step ) )
            psc_abort( "freq_step attribute has to be a number\n" );

        /*check if minimum frequency is smaller than maximum frequency*/
        if( min > max )
            psc_abort( "Minimum frequency can not be greater than maximum frequency\n" );
        if( (max - min)%step != 0 ){
          int steps=(max-min)/step;
          max=min+steps*step;
        }


        TuningParameter* uncoreFrequency = new TuningParameter();
        uncoreFrequency->setId(id++);
        uncoreFrequency->setName("UNCORE_FREQ");
        uncoreFrequency->setPluginType(UNKOWN_PLUGIN);
        uncoreFrequency->setRange(min, max, step);
        uncoreFrequency->setDefaultValue(default_uncore_freq);
        uncoreFrequency->setRuntimeActionType(TUNING_ACTION_FUNCTION_POINTER);
        tuningParameters.push_back(uncoreFrequency);

        if (PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins) == AutotunePlugins) {
            cout << "ReadexIntraphasePlugin: Tuning parameters are:\n";
            for (int i = 0; i < tuningParameters.size(); i++) {
                cout << tuningParameters[i]->toString() << endl;
            }
        }

        int major, minor;
        string name, description;

        // Read search algorithm

        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: Obtaining search algorithm.\n");

        std::string searchAlgorithmName= "individual";
        try {
            searchAlgorithmName = configTree.get < std::string > ("Configuration.periscope.searchAlgorithm.name");
        } catch (exception &e) {
            psc_dbgmsg(1, "ReadexIntraphasePlugin: no search algorithm given in configuration file. Using default.\n");
        }
        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: Search algorithm: %s\n", searchAlgorithmName.c_str());

        context->loadSearchAlgorithm(searchAlgorithmName, &major, &minor, &name, &description);
        searchAlgorithm = context->getSearchAlgorithmInstance(searchAlgorithmName);

        if (searchAlgorithm) {
            print_loaded_search(major, minor, name, description);
            searchAlgorithm->initialize(context, pool_set);
        } else {
            perror("NULL pointer in searchAlgorithm\n");
            throw PTF_PLUGIN_ERROR(NULL_REFERENCE);
        }

        if (searchAlgorithmName == "individual") {
            try {
                std::string keepString;
                keepString = configTree.get < std::string > ("Configuration.periscope.searchAlgorithm.keep");
                int keep = atoi(keepString.c_str());
                ((IndividualSearch*) searchAlgorithm)->setScenariosToKeep(keep);
                psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: Individual search - scenarios to keep %d\n", keep);
            } catch (exception &e) {
            }
        }

        if (searchAlgorithmName == "random") {
            try {
                std::string samplesString;
                samplesString = configTree.get < std::string > ("Configuration.periscope.searchAlgorithm.samples");
                int samples = atoi(samplesString.c_str());
                ((RandomSearch*) searchAlgorithm)->setSampleCount(samples);
                psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: Random search - requested samples %d\n", samples);
            } catch (exception &e) {
            }
        }

        if (searchAlgorithmName == "gde3") {
            try {
                std::string confString;
                confString = configTree.get < std::string > ("Configuration.periscope.searchAlgorithm.populationSize");
                int value = atoi(confString.c_str());
                ((GDE3Search*) searchAlgorithm)->setPopulationSize(value);
                psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: Random search - requested population size %d\n", value);
            } catch (exception &e) {
            }
            try {
                std::string confString;
                confString = configTree.get < std::string > ("Configuration.periscope.searchAlgorithm.maxGenerations");
                int value = atoi(confString.c_str());
                ((GDE3Search*) searchAlgorithm)->setMaxGenerations(value);
                psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: Random search - requested maximal number of generations %d\n", value);
            } catch (exception &e) {
            }
            try {
                std::string confString;
                confString = configTree.get < std::string > ("Configuration.periscope.searchAlgorithm.timer");
                int value = atoi(confString.c_str());
                ((GDE3Search*) searchAlgorithm)->setTimer(value);
                psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: Random search - maximal runtime of %d seconds\n", value);
            } catch (exception &e) {
            }

        }

        VariantSpace* variantSpace = new VariantSpace();
        SearchSpace* searchSpace = new SearchSpace();
        for (auto& tuningParameter : tuningParameters) {
            variantSpace->addTuningParameter(tuningParameter); // TODO Currently no check if there is constraint, then treat ATP as usual tuning parameters. Otherwise, add only hardware tuning parameters
        }
        searchSpace->setVariantSpace(variantSpace);
        std::list<Rts*> rtsList = appl->getCalltreeRoot()->getValidRtsOfRegion(appl->get_phase_region(), NULL);
        if (rtsList.empty()){
            psc_abort("ReadexIntraphasePlugin: No rts based Phase\n");
        }
        std::list<Rts*>::iterator rts_it;
        for (rts_it = rtsList.begin(); rts_it != rtsList.end(); rts_it++) {
            searchSpace->addRts(*rts_it);
        }
        searchAlgorithm->addSearchSpace(searchSpace);

        std::string timeUnit;
        std::string energyUnit;
        std::string currencyUnit;

        //Read objectives
        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: Obtaining objectives.\n");
        std::list < std::string > objectiveFunctionList;
        try {
            if (opts.has_configurationfile) {
                // extract the significant regions from configuration file provided by readex-dyn-detect tool
                BOOST_FOREACH(ptree::value_type & v, configTree.get_child("Configuration.objectives"))
                {
                    std::string name = v.second.data();
                    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "Objective Function: %s \n", name.c_str());
                    objectiveFunctionList.push_back(name);
                }
                timeUnit = configTree.get < std::string > ("Configuration.periscope.metricUnits.timeUnit");
                energyUnit = configTree.get < std::string > ("Configuration.periscope.metricUnits.energyUnit");
                currencyUnit = configTree.get < std::string > ("Configuration.periscope.metricUnits.currencyUnit");
            }

        } catch (exception &e) {
            psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: %s\n", e.what());
        }

        if (objectiveFunctionList.size() > 0) {
            BOOST_FOREACH(std::string & s, objectiveFunctionList)
            {
                if (s == "Energy") {
                    std::string unit = energyUnit;
                    searchAlgorithm->addObjectiveFunction(new EnergyObjective(energyUnit));
                    objectives.push_back(new EnergyObjective(energyUnit));
                } else if (s == "NormalizedEnergy") {
                    std::string unit = energyUnit.append("/instr");
                    searchAlgorithm->addObjectiveFunction(new NormalizedEnergyObjective(unit));
                    objectives.push_back(new NormalizedEnergyObjective(unit));
                } else if (s == "CPUEnergy") {
                    std::string unit = energyUnit;
                    searchAlgorithm->addObjectiveFunction(new CPUEnergyObjective(unit));
                    objectives.push_back(new CPUEnergyObjective(unit));
                } else if (s == "NormalizedCPUEnergy") {
                    std::string unit = energyUnit.append("/instr");
                    searchAlgorithm->addObjectiveFunction(new NormalizedCPUEnergyObjective(unit));
                    objectives.push_back(new NormalizedCPUEnergyObjective(unit));
                } else if (s == "EDP") {
                    std::string unit = energyUnit.append("*").append(timeUnit);
                    searchAlgorithm->addObjectiveFunction(new EDPObjective(unit));
                    objectives.push_back(new EDPObjective(unit));
                } else if (s == "NormalizedEDP") {
                    std::string unit = energyUnit.append("*").append(timeUnit).append("/instr");
                    searchAlgorithm->addObjectiveFunction(new NormalizedEDPObjective(unit));
                    objectives.push_back(new NormalizedEDPObjective(unit));
                } else if (s == "ED2P") {
                    std::string unit = energyUnit.append("*").append(timeUnit).append("2");
                    searchAlgorithm->addObjectiveFunction(new ED2PObjective(unit));
                    objectives.push_back(new ED2PObjective(unit));
                } else if (s == "NormalizedED2P") {
                    std::string unit = energyUnit.append("*").append(timeUnit).append("2").append("/instr");
                    searchAlgorithm->addObjectiveFunction(new NormalizedED2PObjective(unit));
                    objectives.push_back(new NormalizedED2PObjective(unit));
                } else if (s == "Time") {
                    std::string unit = timeUnit;
                    searchAlgorithm->addObjectiveFunction(new TimeObjective(unit));
                    objectives.push_back(new TimeObjective(unit));
                } else if (s == "NormalizedTime") {
                    std::string unit = timeUnit.append("/instr");
                    searchAlgorithm->addObjectiveFunction(new NormalizedTimeObjective(unit));
                    objectives.push_back(new NormalizedTimeObjective(unit));
                } else if (s == "TCO") {
                    std::string unit = currencyUnit;
                    searchAlgorithm->addObjectiveFunction(new TCOObjective(unit));
                    objectives.push_back(new TCOObjective(unit));
                } else if (s == "NormalizedTCO") {
                    std::string unit = currencyUnit.append("/instr");
                    searchAlgorithm->addObjectiveFunction(new NormalizedTCOObjective(unit));
                    objectives.push_back(new NormalizedTCOObjective(unit));
                }
                else
                    psc_errmsg("Unknown objective %s\n", s.c_str());
            }
        } else {
            std::string unit = energyUnit;
            searchAlgorithm->addObjectiveFunction(new EnergyObjective(unit));
            objectives.push_back(new EnergyObjective(unit));
        }
    }
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: call to startTuningStep() finished\n");
}

/**
 * @brief Defines a pre-analysis strategy.
 *
 * Found properties of the pre-analysis strategy are stored in the arp.
 *
 * @ingroup ReadexIntraphasePlugin
 *
 * @return true if pre-analysis is required false otherwise
 *
 */
bool ReadexIntraphasePlugin::analysisRequired(StrategyRequest** strategy) {
    return false;
}

/**
 * @brief The Created Scenario Pool (csp) is populated here.
 *
 * The scenarios need to be created and added to the first pool. To create the scenarios, a
 * search algorithm can be used or it can be done directly by the plugin.
 *
 * After this step, the Periscope will verify that scenarios were added to the csp.
 *
 * @ingroup ReadexIntraphasePlugin
 *
 */
void ReadexIntraphasePlugin::createScenarios(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: call to createScenarios()\n");

    if (tuningParameters.size() > 0 && (tuningStep <= 2)){ // create scenarios only for tuning step 1 and 2
        searchAlgorithm->createScenarios();
    }
    //if (tuningStep == 1) {
        //searchAlgorithm->createScenarios();
    //} else {
        //nothing. Scneario is generated in finish tuning step
    //}
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: call to createScenarios() finished\n");
}

/**
 * @brief Preparatory steps for the scenarios are done here.
 *
 * If there are any preparatory steps required by some or all scenarios in the csp (for example:
 * the project may need to be re-compiled), then they are to be performed here. After each
 * scenario is prepared, they are migrated from the csp to the Prepared Scenario Pool (psp).
 *
 * In some cases, no preparation may be necessary and the plugin can simply move all scenarios
 * from the csp to the psp.
 *
 * After this step, the Periscope will verify that scenarios were added to the psp.
 *
 * @ingroup ReadexIntraphasePlugin
 *
 */
void ReadexIntraphasePlugin::prepareScenarios(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: call to prepareScenarios()\n");

    while (!pool_set->csp->empty()) {
        pool_set->psp->push(pool_set->csp->pop());
    }
}

/**
 * @brief Populate the Experiment Scenario Pool (esp) for the next experiment.
 *
 * This is the final step before the experiments are executed. Scenarios are moved from the
 * psp to the esp, depending on the number of processes and whether they can be executed
 * in parallel.
 *
 * After this step, the Periscope will verify that scenarios were added to the esp.
 *
 * @ingroup ReadexIntraphasePlugin
 *
 * @param numprocs         Number of processes required for the experiment
 * @param analysisRequired Defines is per-experiment analysis required
 * @param strategy         Strategy request for per-experiment analysis
 *
 */
void ReadexIntraphasePlugin::defineExperiment(int numprocs, bool& analysisRequired, StrategyRequest** strategy) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: call to defineExperiment()\n");

    Scenario* scenario = pool_set->psp->pop();
    // currently it should work as we get only one rts for the phase region
    std::list<Rts*> rtsList = appl->getCalltreeRoot()->getValidRtsOfRegion(appl->get_phase_region(), NULL);
    if (rtsList.empty())
        psc_abort("ReadexIntraphasePlugin: No rts based Phase\n");

    std::list<Rts*>::iterator rts_it;
    for (rts_it = rtsList.begin(); rts_it != rtsList.end(); rts_it++) {
        scenario->setSingleTunedRtsWithPropertyALLRanks(*rts_it, ENERGY_CONSUMPTION);
    }

    pool_set->esp->push(scenario);
    if( psc_get_debug_level() >= 2 ) {
      scenario->print();
    }

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: Processing significant regions\n");
    code_significant_regions = appl->get_sig_regions_list();
    std::list<Region*>::iterator code_sig_regions_it;

    if (code_significant_regions.size() > 0) {
        StrategyRequestGeneralInfo* analysisStrategyRequest = new StrategyRequestGeneralInfo;
        analysisStrategyRequest->strategy_name     = "ConfigAnalysis";
        analysisStrategyRequest->pedantic          = 1;
        analysisStrategyRequest->delay_phases      = 0;
        analysisStrategyRequest->delay_seconds     = 0;
        analysisStrategyRequest->analysis_duration = 1;

        list<PropertyRequest*>* reqList = new list<PropertyRequest*>;

        for (code_sig_regions_it = code_significant_regions.begin(); code_sig_regions_it != code_significant_regions.end(); code_sig_regions_it++) {
            //get list of rts
            std::list<Rts*> rtsList = appl->getCalltreeRoot()->getValidRtsOfRegion(*code_sig_regions_it, NULL);
            std::list<Rts*>::iterator rts_it;
            for (rts_it = rtsList.begin(); rts_it != rtsList.end(); rts_it++) {
                PropertyRequest* req = new PropertyRequest();
                req->addPropertyID(ENERGY_CONSUMPTION);
                req->addRts(*rts_it);
                req->addAllProcesses();
                reqList->push_back(req);
            }
        }
        //appl->get_get_energy_metrics().node_energy;
        //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "Node Energy: %f;\n",  appl->get_energy_metrics().node_energy );

        StrategyRequest* sub_strategy = new StrategyRequest(reqList, analysisStrategyRequest);
        (*strategy) = sub_strategy;
        (*strategy)->printStrategyRequest();
        analysisRequired = true;
    }
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: call to defineExperiment() finished\n");
}

/**
 * @brief Returns does the application has to be restarted for the next experiment.
 *
 * Return true if a restart of the application is required for the next experiment,
 * false otherwise.
 *
 * @ingroup ReadexIntraphasePlugin
 *
 * @return true if an application has to be restarted false otherwise
 *
 */
bool ReadexIntraphasePlugin::restartRequired(std::string& env, int& numprocs, std::string& command, bool& is_instrumented) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: call to restartRequired()\n");
    return false; // no restart required
}

/**
 * @brief Returns the status of the current search iteration.
 *
 * Returns true if the current search iteration is finished (by either the plugin itself or as
 * a result of the search algorithm), false otherwise.
 *
 * @ingroup ReadexIntraphasePlugin
 *
 * @return true if the plugin has finished search false otherwise
 *
 */
bool ReadexIntraphasePlugin::searchFinished(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: call to searchFinished()\n");
    if (tuningParameters.size() > 0) {
        return searchAlgorithm->searchFinished();
    }
    else
        return true;
}

/**
 * @brief Final operation of a tuning step.
 *
 * If any post-processing is necessary before entering the next tuning iteration, it is to be
 * done here.
 *
 * @ingroup ReadexIntraphasePlugin
 *
 */
void ReadexIntraphasePlugin::finishTuningStep(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: call to finishTuningStep()\n");
}

/**
 * @brief Returns true if the plugin finished the tuning process, false otherwise.
 *
 * @ingroup ReadexIntraphasePlugin
 *
 * @return true if the plugin has finished tuning false otherwise
 *
 */
bool ReadexIntraphasePlugin::tuningFinished(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: call to tuningFinished()\n");

    std::list<TuningSpecification*> *tsList = new list<TuningSpecification*>;

    if (tuningStep == ATP_PARAMETER_TUNING) {
        if( atp_execution_mode != NULL && strcmp (atp_execution_mode, "DTA") == 0 ) {
            int optimumSc = searchAlgorithm->getOptimum();
            std::ostringstream atp_result_oss;
            atp_result_oss << "Optimum Scenario for ATP: " << optimumSc << endl << endl;
            for ( int i = 0; i < tuningParameters.size(); i++ )
            {
                if (getTuningValue(optimumSc,i) != -1 )
                {
                    atp_result_oss << "\t " << tuningParameters[i]->getName() << ": ";
                    atp_result_oss << getTuningValue(optimumSc, i) << endl;
                    optimum_atp_map.insert(std::make_pair<std::string, int>(tuningParameters[i]->getName(), getTuningValue(optimumSc, i)));
                }
            }

            atp_result_oss << "\nATP Results:\n";
            atp_result_oss << "Scenario\t|  ";

            for (int i = 0; i < tuningParameters.size(); i++) {
                atp_result_oss << tuningParameters[i]->getName() << "\t|  ";
            }

            for (int i = 0; i < objectives.size(); i++) {
                atp_result_oss << objectives[i]->getName() << "\t  ";
            }
            atp_result_oss << endl;

            for (int scenario_id = 0; scenario_id < pool_set->fsp->size(); scenario_id++) {
                atp_result_oss << scenario_id << "\t\t|  ";
                for (int i = 0; i < tuningParameters.size(); i++) {
                    if (getTuningValue(scenario_id, i) != -1) {
                        atp_result_oss << getTuningValue(scenario_id, i) << "\t\t|  ";
                    } else {
                        atp_result_oss << "\t\t\t|  ";
                    }
                }
                for (int i = 0; i < objectives.size(); i++) {
                    atp_result_oss << objectives[i]->objective(scenario_id, pool_set->srp) << "\t  ";
                }
                atp_result_oss << endl;
            }
            atp_result_oss << endl << endl;
            cout << atp_result_oss.str();

            scenario_no_atp =  pool_set->fsp->size();
            /*clear all types of scenario pools, objectives, search algorithm, result_oss */
            pool_set->csp->clear();
            pool_set->psp->clear();
            pool_set->esp->clear();
            pool_set->srp->clear();
            pool_set->fsp->clear();

            objectives.clear();
            searchAlgorithm->clear();
        }
        tuningStep++;
        return false;
    }
    if (tuningStep == HARDWARE_PARAMETER_TUNING) { //<=

        //for (int scenario_id = 0; scenario_id < pool_set->fsp->size(); scenario_id++) {
        for (int scenario_id = scenario_no_atp; scenario_id < (scenario_no_atp+pool_set->fsp->size()); scenario_id++) {
            Scenario* scenario = pool_set->fsp->getScenarioByScenarioID(scenario_id);
            for (int i = 0; i < objectives.size(); i++) {
                double objValue = objectives[i]->objective(scenario_id, pool_set->srp);
                scenario->addResult(objectives[i]->getName(), objValue);
            }
        }

        std::map<TuningParameter*, int> configurationForVariant, worstConfigurationForVariant;

        std::map<int, double> energyForScenario = searchAlgorithm->getSearchPath();
        int optimumSc = searchAlgorithm->getOptimum();
        int worstSc   = searchAlgorithm->getWorst();
        result_oss << endl << "Worst Scenario: " << worstSc << endl;
        result_oss << "Optimum Scenario: " << optimumSc << endl << endl;

        list<Scenario*> bestScenarios;
        Scenario* bestScenario = (*pool_set->fsp->getScenarios())[optimumSc];
        bestScenarios.push_back(bestScenario);
        double bestPhaseTime;

        for (int i = 0; i < tuningParameters.size(); i++) {
            if (getTuningValue(optimumSc, i) != -1) {
                result_oss << "\t " << tuningParameters[i]->getName() << ": ";
                result_oss << getTuningValue(optimumSc, i) << endl;
                configurationForVariant.insert(std::pair<TuningParameter*, int>(tuningParameters[i], getTuningValue(optimumSc, i)));
            }
            if (getTuningValue(worstSc, i) != -1) {
                worstConfigurationForVariant.insert(std::pair<TuningParameter*, int>(tuningParameters[i], getTuningValue(worstSc, i)));
            }
        }
        result_oss << endl;

        Variant* variant = new Variant(configurationForVariant);
        std::list<Rts*> rtsList = appl->getCalltreeRoot()->getValidRtsOfRegion(appl->get_phase_region(), NULL);
        std::list<string> *callpathList = new std::list<string>;

//        std::list<Rts*>::iterator rts_it;
//      for (rts_it = rtsListbegin(); rts_it != rtsList.end(); rts_it++) {
            callpathList->push_back((*(rtsList.begin()))->getCallPath());
//      }

        TuningSpecification* ts = new TuningSpecification(variant, callpathList);
        tsList->push_back(ts);

        //Best and worst normalized energy
        double staticBestNormPhaseObj = std::numeric_limits<double>::max();
        double staticWorstNormPhaseObj = std::numeric_limits<double>::min();
        int optimumNormSc(-1), worstNormSc(-1);

//        for (int scenario_id = 0; scenario_id < pool_set->fsp->size(); scenario_id++) {
//            for (int i = 0; i < objectives.size(); i++) {
//                //Find best scenario for Normalized Energy
//                if(objectives[i]->getName() == "NormalizedEnergy") {
//                    if(objectives[i]->objective(scenario_id, pool_set->srp) < staticBestNormPhaseObj) {
//                        staticBestNormPhaseObj = objectives[i]->objective(scenario_id, pool_set->srp);
//                        optimumNormSc = scenario_id;
//                    }
//                    if(objectives[i]->objective(scenario_id, pool_set->srp) > staticWorstNormPhaseObj) {
//                        staticWorstNormPhaseObj = objectives[i]->objective(scenario_id, pool_set->srp);
//                        worstNormSc = scenario_id;
//                    }
//                }
//            }
//        }

        result_oss << "\nAll Results:\n";
        result_oss << "Scenario\t|  ";

        for (int i = 0; i < tuningParameters.size(); i++) {
            result_oss << tuningParameters[i]->getName() << "\t|  ";
        }

        for (int i = 0; i < objectives.size(); i++) {
          result_oss << "\t " << objectives[i]->getName() << "(" << objectives[i]->getUnit() << ")" << "| \t";
        }
        result_oss << endl;


        std::map<std::string, double> phaseObj, worstPhaseObj;
        std::map<std::string, double> bestPhaseNormObj,worstPhaseNormObj;

        double staticBestPhaseObj = 0.0, staticWorstPhaseObj = 0.0;

        //for (int scenario_id = 0; scenario_id < pool_set->fsp->size(); scenario_id++) {
        for (int scenario_id = scenario_no_atp; scenario_id < (scenario_no_atp+pool_set->fsp->size()); scenario_id++) {
            result_oss << scenario_id << "\t\t|  ";
            for (int i = 0; i < tuningParameters.size(); i++) {
                if (getTuningValue(scenario_id, i) != -1) {
                    result_oss << getTuningValue(scenario_id, i) << "\t\t|  ";
                } else {
                    result_oss << "\t\t\t|  ";
                }
            }
            for (int i = 0; i < objectives.size(); i++) {
                result_oss << objectives[i]->objective(scenario_id, pool_set->srp) << "\t|  ";
                // get best static phase time
                if( bestScenario->getID() == scenario_id && (objectives[i]->getName() == "Time") )
                {
                    bestPhaseTime = objectives[i]->objective(scenario_id, pool_set->srp);
                }
            }

            //Add objectives for best phase scenario
            if( scenario_id == optimumSc ) {
                for (int obj = 0; obj < objectives.size(); obj++) {
                    staticBestPhaseObj = objectives[0]->objective(scenario_id, pool_set->srp);
                    phaseObj.insert(std::pair<std::string, double>(objectives[obj]->getName(), objectives[obj]->objective(scenario_id, pool_set->srp)));
                }
            }
            if( scenario_id == worstSc ) {
                for (int obj = 0; obj < objectives.size(); obj++) {
                    staticWorstPhaseObj = objectives[0]->objective(scenario_id, pool_set->srp);
                    worstPhaseObj.insert(std::pair<std::string, double>(objectives[obj]->getName(), objectives[obj]->objective(scenario_id, pool_set->srp)));
                }
            }
            if( scenario_id == optimumNormSc ) {
                for (int obj = 0; obj < objectives.size(); obj++) {
                    bestPhaseNormObj.insert(std::pair<std::string, double>(objectives[obj]->getName(), objectives[obj]->objective(scenario_id, pool_set->srp)));
                }
            }
            if( scenario_id == worstNormSc ) {
                for (int obj = 0; obj < objectives.size(); obj++) {
                    worstPhaseNormObj.insert(std::pair<std::string, double>(objectives[obj]->getName(), objectives[obj]->objective(scenario_id, pool_set->srp)));
                }
            }
            result_oss << endl;
        }

        // for (int i = 0; i < objectives.size(); i++) {
        //     result_oss << objectives[i]->getName() << " in " << objectives[i]->getUnit() << endl;
        // }
        // result_oss << endl;
        //

        //Add best and worst configurations for the normalized energy for the phase
//        std::map<TuningParameter*, int> configurationForVariantNorm, worstConfigurationForVariantNorm;
//        for (int i = 0; i < tuningParameters.size(); i++) {
//            if (getTuningValue(optimumNormSc, i) != -1)
//                configurationForVariantNorm.insert(std::pair<TuningParameter*, int>(tuningParameters[i], getTuningValue(optimumNormSc, i)));
//            if (getTuningValue(worstNormSc, i) != -1)
//                worstConfigurationForVariantNorm.insert(std::pair<TuningParameter*, int>(tuningParameters[i], getTuningValue(worstNormSc, i)));
//        }
//
//        std::vector<int> scenarios_to_insert{optimumSc,worstSc,optimumSc,optimumNormSc,worstNormSc};

        std::vector<int> scenarios_to_insert{optimumSc,worstSc,optimumSc};
        for(int sc_list_it = 0; sc_list_it < scenarios_to_insert.size(); sc_list_it++) {
            double phaseCpuEnergy, phaseTime, phaseNodeEnergy, phaseInstr;
            INT64 phaseInstances = 0;
            std::map<std::string, double>  phaseExtraInfo;
            list<MetaProperty> properties;
            properties = pool_set->srp->getScenarioResultsByID(scenarios_to_insert[sc_list_it]);
            //Add extra info for phase
            insertExtraInfo( &properties, phaseCpuEnergy, phaseTime, phaseNodeEnergy, phaseInstances, phaseExtraInfo, phaseInstr );
            switch(sc_list_it) {
            case 0:
                appl->getCalltreeRoot()->insertPluginResult("readex_intraphase", staticBestPhaseObj, configurationForVariant, phaseObj, phaseExtraInfo, STATIC_BEST );
                break;
            case 1:
                appl->getCalltreeRoot()->insertPluginResult("readex_intraphase", staticWorstPhaseObj, worstConfigurationForVariant, worstPhaseObj, phaseExtraInfo, STATIC_WORST );
                break;
            case 2:
                appl->getCalltreeRoot()->insertPluginResult("readex_intraphase", staticBestPhaseObj, configurationForVariant, phaseObj, phaseExtraInfo, RTS_BEST );
                break;
//            case 3:
//                appl->getCalltreeRoot()->insertPluginResult("readex_intraphase", staticBestNormPhaseObj, configurationForVariantNorm, bestPhaseNormObj, phaseExtraInfo, STATIC_NORMALIZED_BEST );
//                break;
//            case 4:
//                appl->getCalltreeRoot()->insertPluginResult("readex_intraphase", staticWorstNormPhaseObj, worstConfigurationForVariantNorm, worstPhaseNormObj, phaseExtraInfo, STATIC_NORMALIZED_WORST );
//                break;
            }
        }


        result_oss << endl << "Best configurations for individual rts's:" << endl << endl;
        code_significant_regions = appl->get_sig_regions_list();
        std::list<Region*>::iterator code_sig_regions_it;

        if (code_significant_regions.size() > 0) {
            ptree rtsTree;
            ptree rts_s;
            for (code_sig_regions_it = code_significant_regions.begin(); code_sig_regions_it != code_significant_regions.end(); code_sig_regions_it++) {
                //get list of rts
                std::list<Rts*> rtsList = appl->getCalltreeRoot()->getValidRtsOfRegion(*code_sig_regions_it, NULL);
                std::list<Rts*>::iterator rts_it;
                Rts* rts, rts_in;
                ptree rts_i;
                for (rts_it = rtsList.begin(); rts_it != rtsList.end(); rts_it++) {
                    rts = *rts_it;
                    rts_i.put( "callpath", rts->getCallPath() );
                    int optScenario = -1, worstScenario = -1;
                    double optimum = 0.0;
                    double worstObjValue = 0.0, worstNormObj = 0.0;
                    double staticObjValue = 0.0;
                    int optNormScenario(-1), worstNormScenario(-1);
                    double optNormObj(0.0),staticNormObj(0.0);

                    //for (int scenario_id = 0; scenario_id < pool_set->fsp->size(); scenario_id++) {
                    for (int scenario_id = scenario_no_atp; scenario_id < (scenario_no_atp+pool_set->fsp->size()); scenario_id++) {
                        //get properties
                        Scenario* scenario = pool_set->fsp->getScenarioByScenarioID(scenario_id);
                        list<MetaProperty> *properties = pool_set->arp->getPropertiesForScenarioIDandEntity(scenario_id, rts->getCallPath());
                        double objValue = objectives[0]->objective(*properties);
                        double normObjValue(0.0);
                        for (int i = 0; i < objectives.size(); i++) {
                            if(objectives[i]->getName() == "NormalizedEnergy")
                                normObjValue = objectives[i]->objective(*properties);
                        }
                        if ((optScenario == -1 || optimum > objValue) && objValue != 0.0 ) {
                            optScenario = scenario_id;
                            optimum = objValue;
                        }
                        if( scenario_id == worstSc ) {
                            worstObjValue = objValue;
                        }
//                        if(worstScenario == -1 || objValue > worstObjValue) {
//                            worstScenario = scenario_id;
//                            worstObjValue = objValue;
//                        }
                        if(scenario_id == optimumSc ) {
                            staticObjValue = objValue;
                        }

                        //Optimum normalized objective
//                        if (optNormScenario == -1 || optNormObj > normObjValue) {
//                            optNormScenario = scenario_id;
//                            optNormObj = normObjValue;
//                        }
//                        if(worstScenario == -1 || normObjValue > worstNormObj) {
//                            worstNormScenario = scenario_id;
//                            worstNormObj = normObjValue;
//                        }
//                        if( scenario_id == worstNormSc ) {
//                            worstNormObj = normObjValue;
//                        }
//                        if(scenario_id == optimumNormSc ) {
//                            staticNormObj = normObjValue;
//                        }
//
                        delete properties;
                    }

                    // Determine the static best and the dynamic best configurations only if optimum scenario != -1
                    if( optimumSc != -1 && optScenario != -1 && worstObjValue != 0.0 ) {
                        result_oss << "RTS: " << rts->getCallPath() << endl;
                        for( int num_config = 0; num_config <= 2; num_config++ ) {
                        std::map<TuningParameter*, int> configuration;
                        std::map<std::string, double>   obj;
                        std::map<std::string, double>   normObj;
                        std::map<std::string, double>   extraInfo;
                        double cpuEnergy(0.0), time(0.0), nodeEnergy(0.0), totalInstructions(0.0);
                        INT64 instances = 0;
                        //capture information about rts (still region)
                        std::map<TuningParameter*, int> configurationForVariant;
                        list<MetaProperty> *properties;

                        switch(num_config) {
                        case 0: //worst static configuration
                            properties = pool_set->arp->getPropertiesForScenarioIDandEntity(worstSc, rts->getCallPath());
                            for (int i = 0; i < tuningParameters.size(); i++) {
                                if (getTuningValue(worstSc, i) != -1) {
                                    configuration.insert(std::pair<TuningParameter*, int>(tuningParameters[i], getTuningValue(worstSc, i)));
                                } else {

                                }
                            }
                            break;
                        case 1: //best static configuration
                            properties = pool_set->arp->getPropertiesForScenarioIDandEntity(optimumSc, rts->getCallPath());
                            for (int i = 0; i < tuningParameters.size(); i++) {
                                if (getTuningValue(optimumSc, i) != -1) {
                                    configuration.insert(std::pair<TuningParameter*, int>(tuningParameters[i], getTuningValue(optimumSc, i)));
                                } else {

                                }
                            }
                            break;
                        case 2: //best configuration for the rts
                            properties = pool_set->arp->getPropertiesForScenarioIDandEntity(optScenario, rts->getCallPath());
                            result_oss << "\t " << "Scenario: " << optScenario << endl;
                            for (int i = 0; i < tuningParameters.size(); i++) {
                                if (getTuningValue(optScenario, i) != -1) {
                                    result_oss << "\t " << tuningParameters[i]->getName() << ": \t" << getTuningValue(optScenario, i) << endl;
                                    configuration.insert(std::pair<TuningParameter*, int>(tuningParameters[i], getTuningValue(optScenario, i)));
                                    configurationForVariant.insert(std::pair<TuningParameter*, int>(tuningParameters[i], getTuningValue(optScenario, i)));
                                } else {

                                }
                            }
                            break;
                        case 3: //best normalized static configuration
                            properties = pool_set->arp->getPropertiesForScenarioIDandEntity(optimumNormSc, rts->getCallPath());
                            for (int i = 0; i < tuningParameters.size(); i++) {
                                if (getTuningValue(optimumNormSc, i) != -1) {
                                    configuration.insert(std::pair<TuningParameter*, int>(tuningParameters[i], getTuningValue(optimumNormSc, i)));
                                } else {

                                }
                            }
                            break;
                        case 4: //worst normalized static configuration
                            properties = pool_set->arp->getPropertiesForScenarioIDandEntity(worstNormSc, rts->getCallPath());
                            for (int i = 0; i < tuningParameters.size(); i++) {
                                if (getTuningValue(worstNormSc, i) != -1) {
                                    configuration.insert(std::pair<TuningParameter*, int>(tuningParameters[i], getTuningValue(worstNormSc, i)));
                                } else {

                                }
                            }
                            break;
                        case 5: //best normalized configuration for the rts
                            properties = pool_set->arp->getPropertiesForScenarioIDandEntity(optNormScenario, rts->getCallPath());
                            for (int i = 0; i < tuningParameters.size(); i++) {
                                if (getTuningValue(optNormScenario, i) != -1) {
                                    configuration.insert(std::pair<TuningParameter*, int>(tuningParameters[i], getTuningValue(optimumNormSc, i)));
                                } else {

                                }
                            }
                            break;
                        }
                        for (int i = 0; i < objectives.size(); i++) {
                            if( num_config == 2) {
                                result_oss << "\t " << objectives[i]->getName() << "(" << objectives[i]->getUnit() << ")" << ": \t" << objectives[i]->objective(*properties) << endl;
                            }
                            obj.insert(std::pair<std::string, double>(objectives[i]->getName(), objectives[i]->objective(*properties)));
                        }

                        if( num_config == 2 ) {
                            Variant* variant = new Variant(configurationForVariant);
                            std::list<string> *rtsList = new list<string>;
                            rtsList->push_back(rts->getCallPath());
                            TuningSpecification* ts = new TuningSpecification(variant, rtsList);
                            tsList->push_back(ts);
                            result_oss << endl;
                            result_oss << "\n------------------------" << endl << endl;
                        }

                        //Compute extra information
                        insertExtraInfo( properties, cpuEnergy, time, nodeEnergy, instances, extraInfo, totalInstructions );
                        rts_i.put( "execTime", time);
                        if( bestPhaseTime != 0)
                            rts_i.put( "weight", (time/bestPhaseTime)*100 );
                        else
                            rts_i.put( "weight", 0.0);

                        switch(num_config) {
                        case 0:
                            rts->insertPluginResult("readex_intraphase", worstObjValue, configuration, obj, extraInfo, STATIC_WORST );
                            break;
                        case 1:
                            rts->insertPluginResult("readex_intraphase", staticObjValue, configuration, obj, extraInfo, STATIC_BEST);
                            break;
                        case 2:
                            rts->insertPluginResult("readex_intraphase", optimum, configuration, obj, extraInfo, RTS_BEST);
                            break;
                        case 3:
                            rts->insertPluginResult("readex_intraphase", staticNormObj, configuration, obj, extraInfo, STATIC_NORMALIZED_BEST);
                            break;
                        case 4:
                            rts->insertPluginResult("readex_intraphase", worstNormObj, configuration, obj, extraInfo, STATIC_NORMALIZED_WORST);
                            break;
                        case 5:
                            rts->insertPluginResult("readex_intraphase", optNormObj, configuration, obj, extraInfo, RTS_NORMALIZED_BEST);
                            break;
                        }

                        delete properties;
                    }
                }
                    rts_s.push_back( make_pair( "entry", rts_i ) );
                }
            }

            psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: Creating rts.xml");
            rtsTree.add_child( "RTS" , rts_s );
            #if BOOST_VERSION >=105600
                write_xml( "rts.xml", rtsTree, locale(), xml_writer_settings<ptree::key_type>( ' ', 4 ) );
            #else
                write_xml( "rts.xml", rtsTree, locale(), xml_writer_settings<char>( ' ', 4 ) );
            #endif

        }
//      cout << "Calltree in frontend after tuning" << endl;
//      cout << "=================================" << endl;
//      rtstree::printTree( appl->getCalltreeRoot() );

        //create new readex scenario
        Scenario* sc;

        sc = new Scenario(tsList);
        sc->setRtsBased();
        pool_set->csp->push(sc);

        sc = new Scenario(tsList);
        sc->setRtsBased();
        pool_set->csp->push(sc);

        sc = new Scenario(tsList);
        sc->setRtsBased();
        pool_set->csp->push(sc);

        //sc->print();
        readexScenario = sc->getID();

        tuningStep++;
        return false;
    } else {
        result_oss << endl << "READEX result" << endl;
        result_oss << "===============" << endl;
        result_oss << "Scenario: " << readexScenario << endl;

        double readex_energy = objectives[0]->objective(readexScenario, pool_set->srp);

        for (int i = 0; i < objectives.size(); i++) {
            result_oss << "\t " << objectives[i]->getName() << ": \t" << objectives[i]->objective(readexScenario, pool_set->srp) << endl;
        }

        result_oss << endl;
        for (int i = 0; i < objectives.size(); i++) {
            result_oss << "\t " << objectives[i]->getName() << ": \t" << objectives[i]->objective(readexScenario - 1, pool_set->srp) << endl;
        }

        result_oss << endl;
        for (int i = 0; i < objectives.size(); i++) {
            result_oss << "\t " << objectives[i]->getName() << ": \t" << objectives[i]->objective(readexScenario - 2, pool_set->srp) << endl;
        }

        cout << result_oss.str();
        cout.flush();

//        rtstree::insertDefaultEnergy(appl->getCalltreeRoot());
//        rtstree::printTree( appl->getCalltreeRoot() );
        rtstree::displaySavings();

        return true;
    }

}


void ReadexIntraphasePlugin::insertExtraInfo( list<MetaProperty> *properties, double& cpuEnergy, double& time, double& nodeEnergy, INT64& instances,
                                          std::map<std::string, double>& extraInfo, double& totInstr ) {
    list<MetaProperty>::iterator p;
    for (p = properties->begin(); p != properties->end(); p++) {
        if (atoi((*p).getId().c_str()) == ENERGY_CONSUMPTION) {
            addInfoType::iterator it;
            addInfoType addInfo = (*p).getExtraInfo();
            //for (it=addInfo.begin(); it!=addInfo.end(); it++){
            //  printf("Iterator->first %s  ->second %s\n",it->first.c_str(), it->second.c_str());fflush(stdout);
            //}
            it = addInfo.find("CPUEnergy");
            if (it != addInfo.end()) {
                std::string str = it->second;
                INT64 val = atoi(str.c_str());
                cpuEnergy += val;
            } else {
                psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotuneSearch), "CPUEnergy not found\n");
            }

            it = addInfo.find("NodeEnergy");
            if (it != addInfo.end()) {
                std::string str = it->second;
                INT64 val = atoi(str.c_str());
                nodeEnergy += val;
            } else {
                psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotuneSearch), "NodeEnergy not found\n");
            }

            it = addInfo.find("Instances");
            if (it != addInfo.end()) {
                std::string str = it->second;
                INT64 val = atoi(str.c_str());
                if (val > instances) {
                    instances = val;
                    it = addInfo.find("ExecTime");
                    if (it != addInfo.end()) {
                        std::string str = it->second;
                        time = atof(str.c_str());
                    } else {
                        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotuneSearch), "ExecTime not found\n");
                    }
                }
            } else {
                psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotuneSearch), "Instances not found\n");
            }
//            it = addInfo.find("TotalInstr");
//            if (it != addInfo.end()) {
//                std::string str = it->second;
//                double val = atof(str.c_str());
//                totInstr += val;
//            } else {
//                psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotuneSearch), "TotalInstr not found\n");
//            }
        }
    }
    extraInfo.insert(std::pair<std::string, double>("NodeEnergy", nodeEnergy));
    extraInfo.insert(std::pair<std::string, double>("CPUEnergy", cpuEnergy));
    extraInfo.insert(std::pair<std::string, double>("Instances", (double) instances));
    extraInfo.insert(std::pair<std::string, double>("ExecTime", time));
//    extraInfo.insert(std::pair<std::string, double>("TotalInstr", totInstr));
}


int ReadexIntraphasePlugin::getTuningValue(int scenario_id, int tp) {
    Scenario* sc = (*pool_set->fsp->getScenarios())[scenario_id];
    list<TuningSpecification*>* tuningSpec = sc->getTuningSpecifications();
    list<TuningSpecification*>::iterator ts;

    for (ts = tuningSpec->begin(); ts != tuningSpec->end(); ts++) {
        map<TuningParameter*, int> tpValues = (*ts)->getVariant()->getValue();
        map<TuningParameter*, int>::iterator tp_iter;
        for (tp_iter = tpValues.begin(); tp_iter != tpValues.end(); tp_iter++) {
            if (tp_iter->first->getName() == tuningParameters[tp]->getName()) {
                return tp_iter->second;
            }
        }
    }
    return -1;
}

/**
 * @brief Prints to the screen (and to a file, where necessary) the tuning advice.
 *
 * @ingroup ReadexIntraphasePlugin
 */
Advice* ReadexIntraphasePlugin::getAdvice(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: call to getAdvice()\n");
    appl->getCalltreeRoot()->insertDefaultTPValues(tuningParameters);

    //Modify the upper limit and step of the TP range if it is less than the default value. This is to ensure that Nico's clustering doesn't break.
    for( auto &tp : tuningParameters) {
        if( tp->getRangeTo() < tp->getDefaultValue() ) {
            if(tp->getName() == "NUMTHREADS") {
                tp->setRangeTo(tp->getDefaultValue(),1);
            }
            else {
                tp->setRangeTo(tp->getDefaultValue(),100);
            }
        }
    }

    generate_tuning_model();
//    if (RegionBestConfig.empty()) {
//        return new Advice(getName(), bestScenarios, energyForScenario, "Energy", pool_set->fsp->getScenarios());
//    }
//
//    return new Advice(getName(), bestScenarios, energyForScenario, "Energy", pool_set->fsp->getScenarios(), RegionBestConfig);
    return NULL;
}

/**
 * @brief Finalize the plugin normally.
 *
 * Remove any allocated memory, objects, file descriptors, etc.
 *
 * @ingroup ReadexIntraphasePlugin
 *
 */
void ReadexIntraphasePlugin::finalize() {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: call to finalize()\n");

    delete searchAlgorithm;
    /*for (int i = 0; i < tuningParameters.size(); i++) {
        delete tuningParameters[i];
    }*/
}

/**
 * @brief Terminate the plugin due to error.
 *
 * Safely remove any allocated memory, objects, file descriptors, etc. This method should
 * be able to be executed safely at any point.
 *
 * @ingroup ReadexIntraphasePlugin
 *
 */
void ReadexIntraphasePlugin::terminate() {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: call to terminate()\n");
}

/*
 * The following are C functions that belong to the Plugin Management Interface, and not to the
 * IPlugin class. Currently the interface includes information and instance creation functions.
 *
 * These are defined in PluginManagement.h .
 */

/**
 * @brief Returns an instance of this particular plugin implementation.
 *
 * Typically, a simple return with new is enough. For example:
 *
 * return a pointer to a new ReadexIntraphasePlugin();
 *
 * @ingroup ReadexIntraphasePlugin
 *
 * @return A pointer to a new ReadexIntraphasePlugin
 */
IPlugin* getPluginInstance(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: call to getPluginInstance()\n");
    return new ReadexIntraphasePlugin();
}

/**
 * @brief Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 *
 * @ingroup ReadexIntraphasePlugin
 *
 * @return The major plugin interface version used by the plugin
 */
int getVersionMajor(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: call to getInterfaceVersionMajor()\n");

    return READEX_INTRAPHASE_VERSION_MAJOR;
}

/**
 * @brief Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 *
 * @ingroup ReadexIntraphasePlugin
 *
 * @return The minor plugin interface version used by the plugin
 *
 */
int getVersionMinor(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: call to getInterfaceVersionMinor()\n");

    return READEX_INTRAPHASE_VERSION_MINOR;
}

/**
 * @brief Returns a string with the name of the plugin.
 *
 * @ingroup ReadexIntraphasePlugin
 *
 * @return Returns a string with the name of the plugin
 *
 */
string getName(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: call to getName()\n");
    return "ReadexIntraphasePlugin";
}

/**
 * @brief Returns a string with a short description of the plugin.
 *
 * @ingroup ReadexIntraphasePlugin
 *
 * @return A string with a short description of the plugin
 *
 */
string getShortSummary(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexIntraphasePlugin: call to getShortSummary()\n");
    return "Readex intra-phase tuning plugin for Taurus";
}
