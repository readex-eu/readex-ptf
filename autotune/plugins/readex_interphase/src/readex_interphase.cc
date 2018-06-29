/**
 @file    readex_interphase.cc
 @ingroup ReadexInterphasePlugin
 @brief   READEX interphase tuning plugin for inter-phase tuning
 @author  Madhura Kumaraswamy
 @verbatim
 Revision:       $Revision$
 Revision date:  $Date$
 Committed by:   $Author$

 This file is part of the Periscope performance measurement tool.
 See http://www.lrr.in.tum.de/periscope for details.

 Copyright (c) 2018, Technische Universitaet Muenchen, Germany
 See the COPYING file in the base directory of the package for details.
 @endverbatim
 */

#include <vector>
#include <list>
#include <map>
#include <limits>
#include <float.h>
#include <iterator>
#include <utility>
#include <regex>
#include <cmath>
#include "search_common.h"
#include "frontend.h"
#include "RandomSearch.h"
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/optional/optional.hpp>
#include <boost/optional.hpp>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <boost/version.hpp>
using boost::property_tree::ptree;
#include "../include/readex_interphase.h"

std::ostringstream result_oss;
int samples;

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
 * @ingroup ReadexInterphasePlugin
 *
 * @param context a pointer to a context for a plugin
 * @param pool_set a pointer to a set of pools for a plugin
 */
void ReadexInterphasePlugin::initialize(DriverContext* context, ScenarioPoolSet* pool_set) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: call to initialize()\n");

    int threadsLB(1);
    int threadsStep(1);
    tuningStep = 0;
    current_step = 0;
    this->context = context;
    this->pool_set = pool_set;

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: got pool and context\n");
    /*
     * Access tuning parameters from the global configTree
     */

    try {
        this->min_freq = atoi(configTree.get < std::string > ("Configuration.tuningParameter.frequency.min_freq").c_str());
        this->max_freq = atoi(configTree.get < std::string > ("Configuration.tuningParameter.frequency.max_freq").c_str());
        this->freq_step = atoi(configTree.get < std::string > ("Configuration.tuningParameter.frequency.freq_step").c_str());
        threadsLB = atoi(configTree.get < std::string > ("Configuration.tuningParameter.openMPThreads.lower_value").c_str());
        threadsStep = atoi(configTree.get < std::string > ("Configuration.tuningParameter.openMPThreads.step").c_str());
    } catch (exception &e) {
        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: %s\n", e.what());
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

    if( ( this->max_freq - this->min_freq)%this->freq_step != 0 )
        psc_abort( "invalid frequency step\n" );

    /* Check the datatype of each entry */
    if( isnan ( threadsLB ) )
        psc_abort( "Lower value of OpenMP Thread has to be a number\n" );

    if( isnan ( threadsStep ) )
        psc_abort( "OpenMP Thread Step has to be a number\n" );

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: got options from the global configTree \n");

    TuningParameter* cpuFrequency = new TuningParameter();
    cpuFrequency->setId(0);
    cpuFrequency->setName("CPU_FREQ");
    cpuFrequency->setPluginType(Readex_Interphase);
    cpuFrequency->setRange(min_freq, max_freq, freq_step);
    cpuFrequency->setRuntimeActionType(TUNING_ACTION_FUNCTION_POINTER);
    tuningParameters.push_back(cpuFrequency);

    //lower_value for OpenMP threads is less than --ompnumthreads
    if( threadsLB > context->getOmpnumthreads() )
        psc_abort( "The lower value can not be greater than the total number of openMP threads\n" );

    TuningParameter* numberOfThreads = new TuningParameter();
    numberOfThreads->setId(1);
    numberOfThreads->setName("NUMTHREADS");
    numberOfThreads->setPluginType(Readex_Interphase);
    numberOfThreads->setRange(threadsLB, context->getOmpnumthreads(), threadsStep);
    numberOfThreads->setRuntimeActionType(TUNING_ACTION_FUNCTION_POINTER);
    tuningParameters.push_back(numberOfThreads);

    int min(10), max(30), step(2);
    try {
        min = atoi(configTree.get < std::string > ("Configuration.tuningParameter.uncore.min_freq").c_str());
        max = atoi(configTree.get < std::string > ("Configuration.tuningParameter.uncore.max_freq").c_str());
        step = atoi(configTree.get < std::string > ("Configuration.tuningParameter.uncore.freq_step").c_str());
    } catch (exception &e) {
        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: %s\n", e.what());
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
    if( (max - min)%step != 0 )
        psc_abort( "invalid frequency step\n" );

    TuningParameter* uncoreFrequnecy = new TuningParameter();
    uncoreFrequnecy->setId(2);
    uncoreFrequnecy->setName("UNCORE_FREQ");
    uncoreFrequnecy->setPluginType(Readex_Interphase);
    uncoreFrequnecy->setRange(min, max, step);
    uncoreFrequnecy->setRuntimeActionType(TUNING_ACTION_FUNCTION_POINTER);
    tuningParameters.push_back(uncoreFrequnecy);

    if (PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins) == AutotunePlugins) {
        cout << "ReadexInterphasePlugin: Tuning parameters are:\n";
        for (int i = 0; i < tuningParameters.size(); i++) {
            cout << tuningParameters[i]->toString() << endl;
        }
    }

    int major, minor;
    string name, description;

    // Read search algorithm

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: Obtaining search algorithm.\n");

    std::string searchAlgorithmName = "random";

    context->loadSearchAlgorithm(searchAlgorithmName, &major, &minor, &name, &description);
    searchAlgorithm = context->getSearchAlgorithmInstance(searchAlgorithmName);

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: Search algorithm: %s\n", searchAlgorithmName.c_str());

    print_loaded_search(major, minor, name, description);
    searchAlgorithm->initialize(context, pool_set);

    try {
        std::string samplesString;
        samplesString = configTree.get < std::string > ("Configuration.periscope.searchAlgorithm.samples");
        samples = atoi(samplesString.c_str());
        ((RandomSearch*) searchAlgorithm)->setSampleCount(samples);
        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: Random search - requested samples %d\n", samples);
    } catch (exception &e) {
        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: %s\n", e.what());
    }


    std::string timeUnit;
    std::string energyUnit;
    std::string currencyUnit;


    //Read objectives

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: Obtaining objectives.\n");
    std::list < std::string > objectiveFunctionList;
    try {
        if (opts.has_configurationfile) {
            BOOST_FOREACH(ptree::value_type & v, configTree.get_child("Configuration.objectives"))
              {
                std::string name = v.second.data();
                psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "Objective Function: %s \n", name.c_str());
                objectiveFunctionList.push_back(name);
              }
            timeUnit = configTree.get < std::string > ("Configuration.periscope.metricUnits.timeUnit");
            energyUnit = configTree.get < std::string > ("Configuration.periscope.metricUnits.eneryUnit");
            currencyUnit = configTree.get < std::string > ("Configuration.periscope.metricUnits.currencyUnit");

        }
    } catch (exception &e) {
        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: %s\n", e.what());
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

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: initialize() finished \n\n");
}


/**
 * @brief Operations to be done at the start of a tuning step.
 *
 * The tuning parameters that were created at initialization are used to create a variant space.
 * The variant space and the regions are then used to create a search space that is passed to the
 * search algorithm.
 *
 * @ingroup ReadexInterphasePlugin
 *
 */
void ReadexInterphasePlugin::startTuningStep(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: call to startTuningStep()\n");

    tuningStep++;
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: Current tuning step = %d\n",tuningStep);

    if (tuningStep == 1) {
        VariantSpace* variantSpace = new VariantSpace();
        SearchSpace* searchSpace = new SearchSpace();

        for (auto& tuningParameter : tuningParameters) {
            variantSpace->addTuningParameter(tuningParameter);
        }
        searchSpace->setVariantSpace(variantSpace);
        std::list<Rts*> rtsList = appl->getCalltreeRoot()->getValidRtsOfRegion(appl->get_phase_region(), NULL);
        if (rtsList.empty()){
            psc_abort("ReadexInterphasePlugin: No rts based Phase\n");
        }
        std::list<Rts*>::iterator rts_it;
        for (rts_it = rtsList.begin(); rts_it != rtsList.end(); rts_it++) {
            searchSpace->addRts(*rts_it);
        }
        searchAlgorithm->addSearchSpace(searchSpace);
    }

//    if (tuningStep == 2) {
//        for (auto& tuningParameter : tuningParameters) {
//            tuningParameter->setRange(getDefaultTuningValue(tuningParameter->getName()), getDefaultTuningValue(tuningParameter->getName()), 1);
//        }
//    }

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: call to startTuningStep() finished\n");
}


/**
 * @brief Defines a pre-analysis strategy.
 *
 * Found properties of the pre-analysis strategy are stored in the arp.
 *
 * @ingroup ReadexInterphasePlugin
 *
 * @return true if pre-analysis is required false otherwise
 *
 */
bool ReadexInterphasePlugin::analysisRequired(StrategyRequest** strategy) {
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
 * @ingroup ReadexInterphasePlugin
 *
 */
void ReadexInterphasePlugin::createScenarios(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: call to createScenarios()\n");
    // Clear all the scenario pools
    pool_set->csp->clear();
    pool_set->psp->clear();
    pool_set->esp->clear();
    pool_set->srp->clear();
    pool_set->fsp->clear();
    if(tuningStep >= 2) {
        std::map<TuningParameter*, int> defaultConfig;
        for (auto tuningParameter : tuningParameters) {
            defaultConfig.insert(std::make_pair(tuningParameter, getDefaultTuningValue(tuningParameter->getName())));
        }

        for(auto& phase : interph::dtaPhases) {
            //Find the tuning parameter setting (configuration) of the best phase
            auto phase_pos = interph::dtaPhases.find(interph::configForClusters.find(phase.second->clusterID)->second.first);
            std::map<TuningParameter*, int> configForPhase;
            std::map<std::string,std::map<TuningParameter*, int> > configForRts;

            if(tuningStep >= 3) {
                if( phase_pos != interph::dtaPhases.end()){
                    //Get config for phase
                    configForPhase = phase_pos->second->scenarioConfig;

                }
                else { //If we get an empty config, it means that the phase was labeled as NOISE point. Assign default TP values
                    configForPhase = defaultConfig;
                }
            }
            else {
                configForPhase = defaultConfig;
            }

            Variant* variant = new Variant(configForPhase);
            std::list<string>* phaseRtsList = new list<string>;
            phaseRtsList->push_back(appl->getCalltreeRoot()->getCallPath());
            std::list<TuningSpecification*> *tsList = new list<TuningSpecification*>;
            TuningSpecification* ts = new TuningSpecification(variant, phaseRtsList);
            tsList->push_back(ts);

            if(tuningStep >= 3) {
                //Create tuning specifications for rts's of the current phase
                for (auto sig_region : code_significant_regions) {
                    std::list<Rts*> rtsList = appl->getCalltreeRoot()->getValidRtsOfRegion(sig_region, NULL);
                    for (auto rts : rtsList) {
                        if( phase_pos != interph::dtaPhases.end()){
                            auto rts_config_map = interph::rts_best_objs[phase.second->clusterID - 1];
                            auto rts_pos = std::find_if(rts_config_map.begin(), rts_config_map.end(),
                                    [rts](auto& rts_inf) {return (rts_inf.first == rts->getCallPath()); });
                            //If rts was found in rts_config_map,set the config for the rts. If not, assign static best configuration
                            if( rts_pos != rts_config_map.end() ) {
                                configForRts.insert(std::make_pair(rts->getCallPath(),interph::dtaPhases.find(rts_pos->second.second)->second->scenarioConfig));
                            }
                            else {
                                configForRts.insert(std::make_pair(rts->getCallPath(),configForPhase));
                            }
                        }
                        else { //If we get an empty config map, the phase was labeled as NOISE point. Assign default TP values to the rts's
                            configForRts.insert(std::make_pair(rts->getCallPath(),defaultConfig));
                        }
                    }
                }

                for(auto& c : configForRts) {
                    std::list<string>* rtsList = new list<string>;
                    rtsList->push_back(c.first);
                    Variant* rts_variant = new Variant(c.second);
                    TuningSpecification* ts = new TuningSpecification(rts_variant, rtsList);
                    tsList->push_back(ts);
                }
            }

            //Create new scenario for the new tuning specification
            Scenario* sc;

            sc = new Scenario(tsList);
            sc->setRtsBased();
            pool_set->csp->push(sc);
        }
    }
    else
        searchAlgorithm->createScenarios();
}


/**
 * @brief Return the default values for the tuning parameters for tuning step = 2
 */
int ReadexInterphasePlugin::getDefaultTuningValue(std::string tp_name) {
    if(tp_name == "CPU_FREQ") {
        return 2500000;
    }
    else if(tp_name == "NUMTHREADS" ) {
        return context->getOmpnumthreads();
    }
    else if(tp_name == "UNCORE_FREQ") {
        return 30;
    }
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
 * @ingroup ReadexInterphasePlugin
 *
 */
void ReadexInterphasePlugin::prepareScenarios(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: call to prepareScenarios()\n");

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
 * @ingroup ReadexInterphasePlugin
 *
 * @param numprocs         Number of processes required for the experiment
 * @param analysisRequired Defines is per-experiment analysis required
 * @param strategy         Strategy request for per-experiment analysis
 *
 */
void ReadexInterphasePlugin::defineExperiment(int numprocs, bool& analysisRequired, StrategyRequest** strategy) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: call to defineExperiment()\n");

    Scenario* scenario = pool_set->psp->pop();
    std::list<Rts*> rtsList = appl->getCalltreeRoot()->getValidRtsOfRegion(appl->get_phase_region(), NULL);
    if (rtsList.empty())
        psc_abort("ReadexInterphasePlugin: No rts based Phase\n");

    std::list<Rts*>::iterator rts_it;
    for (rts_it = rtsList.begin(); rts_it != rtsList.end(); rts_it++) {
        scenario->setSingleTunedRtsWithPropertyALLRanks(*rts_it, INTERPHASE_PROPS);
    }

    pool_set->esp->push(scenario);

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: Processing significant regions\n");
    code_significant_regions = appl->get_sig_regions_list();
    std::list<Region*>::iterator code_sig_regions_it;
    printf("code_significant_regions.size(): %d\n",code_significant_regions.size());fflush(stdout);

    if (code_significant_regions.size() > 0) {
        StrategyRequestGeneralInfo* analysisStrategyRequest = new StrategyRequestGeneralInfo;
        analysisStrategyRequest->strategy_name = "ConfigAnalysis";
        analysisStrategyRequest->pedantic = 1;
        analysisStrategyRequest->delay_phases = 0;
        analysisStrategyRequest->delay_seconds = 0;
        analysisStrategyRequest->analysis_duration = 1;

        list<PropertyRequest*>* reqList = new list<PropertyRequest*>;

        for (code_sig_regions_it = code_significant_regions.begin(); code_sig_regions_it != code_significant_regions.end(); code_sig_regions_it++) {
            //get list of rts
            std::list<Rts*> rtsList = appl->getCalltreeRoot()->getValidRtsOfRegion(*code_sig_regions_it, NULL);
            std::list<Rts*>::iterator rts_it;
            for (rts_it = rtsList.begin(); rts_it != rtsList.end(); rts_it++) {
                PropertyRequest* req = new PropertyRequest();
                req->addPropertyID(INTERPHASE_PROPS);
                req->addRts(*rts_it);
                req->addAllProcesses();
                reqList->push_back(req);
            }
        }

        StrategyRequest* sub_strategy = new StrategyRequest(reqList, analysisStrategyRequest);
        (*strategy) = sub_strategy;
        (*strategy)->printStrategyRequest();
        analysisRequired = true;
    }

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: call to defineExperiment() finished\n");
}


/**
 * @brief Returns does the application has to be restarted for the next experiment.
 *
 * Return true if a restart of the application is required for the next experiment,
 * false otherwise.
 *
 * @ingroup ReadexInterphasePlugin
 *
 * @return true if an application has to be restarted false otherwise
 *
 */
bool ReadexInterphasePlugin::restartRequired(std::string& env, int& numprocs, std::string& command, bool& is_instrumented) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: call to restartRequired()\n");
    if(current_step < tuningStep) {
        current_step = tuningStep;
        return true;  //restart only before starting the tuning before every tuning step
    }
    return false; // no restart required for individual experiments
}


/**
 * @brief Returns the status of the current search iteration.
 *
 * Returns true if the current search iteration is finished (by either the plugin itself or as
 * a result of the search algorithm), false otherwise.
 *
 * @ingroup ReadexInterphasePlugin
 *
 * @return true if the plugin has finished search false otherwise
 *
 */
bool ReadexInterphasePlugin::searchFinished(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: call to searchFinished()\n");
    return searchAlgorithm->searchFinished();
}


/**
 * @brief Final operation of a tuning step.
 *
 * If any post-processing is necessary before entering the next tuning iteration, it is to be
 * done here.
 *
 * @ingroup ReadexInterphasePlugin
 *
 */
void ReadexInterphasePlugin::finishTuningStep(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: call to processResults()\n");
}


map<TuningParameter*, int> ReadexInterphasePlugin::getTuningParameters(int scenario_id) {
    Scenario* sc = (*pool_set->fsp->getScenarios())[scenario_id];
    list<TuningSpecification*>* tuningSpec = sc->getTuningSpecifications();

    for (auto ts : (*tuningSpec)) {
        return ts->getVariant()->getValue();
    }
}


/**
 * @brief Returns true if the plugin finished the tuning process, false otherwise.
 * @ingroup ReadexInterphasePlugin
 * @return true if the plugin has finished tuning false otherwise
 */
bool ReadexInterphasePlugin::tuningFinished(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: call to tuningFinished()\n");
    using namespace interph;

    switch(tuningStep) {
    case 1:
    {
        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: Inserting phase data in Tuning Step 1\n");
        for (int scenario_id = 0; scenario_id < pool_set->fsp->size(); scenario_id++) {
            Scenario* scenario = pool_set->fsp->getScenarioByScenarioID(scenario_id);
            for (int i = 0; i < objectives.size(); i++) {
                double objValue = objectives[i]->objective(scenario_id, pool_set->srp);
                scenario->addResult(objectives[i]->getName(), objValue);
            }
            insertPhaseData(scenario_id, pool_set);
        }

        calcComputeIntensity();
        normalizeIdentifiers();
        clust::cluster();
        normalizeObjectives(objectives);
        computeBestConfigForCluster(objectives);
        //rtstree::printTree(appl->getCalltreeRoot());
        return false;
    }
    case 2:
    {//Insert the default values
        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: Inserting default phase data in Tuning Step 2\n");
        for (auto& scenario_map : (*pool_set->fsp->getScenarios())) {
            for (int i = 0; i < objectives.size(); i++) {
                double objValue = objectives[i]->objective(scenario_map.first, pool_set->srp);
                scenario_map.second->addResult(objectives[i]->getName(), objValue);
            }
            insertPhaseData(scenario_map.first, pool_set);
        }
        return false;
    }
    case 3:
    {
        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: Inserting verification data in Tuning Step 3\n");

        rtstree::modifyTree(appl->getCalltreeRoot(),interph::clust::num_clusters);
        insertTuningResultForNode(objectives);
        exportData(tuningParameters,objectives);
        rtstree::printTree(appl->getCalltreeRoot());
        rtstree::displaySavings();

        for (auto& scenario_map : (*pool_set->fsp->getScenarios())) {
            for (int i = 0; i < objectives.size(); i++) {
                double objValue = objectives[i]->objective(scenario_map.first, pool_set->srp);
                scenario_map.second->addResult(objectives[i]->getName(), objValue);
            }
            insertPhaseData(scenario_map.first, pool_set);
        }

        //Compute true savings
        insertResultForNode(objectives);
        //rtstree::printTree(appl->getCalltreeRoot());
        return true;
    }
    }
}

void interph::getMeasurements(list<MetaProperty> &properties, std::vector<double> &ident, std::vector<std::string> &tags, unsigned int &phase_iter ) {
    for (auto& p : properties) {
        if(p.isRtsBased()) {
            addInfoType addInfo = p.getExtraInfo();
            auto retVal = [&addInfo](addInfoType::iterator &it) mutable {
                if (it != addInfo.end()) {
                    std::string str = it->second;
                    return std::stod(str.c_str());
                } else return 0.0;
            };

            addInfoType::iterator it = addInfo.find(tags.back());
            phase_iter = (unsigned int)retVal(it);
            for(int id_p = 0; id_p < ident.size()-1; id_p++) {
                addInfoType::iterator tag_inf = addInfo.find(tags[id_p]);
                if(tags[id_p] == "Instances") {
                    if(retVal(tag_inf) > ident.at(id_p)) {
                        ident.at(id_p) = retVal(tag_inf);
                        //Add exec time if instances > 0
                        it = addInfo.find("ExecTime");
                        ident.at(++id_p) = retVal(it);
                    }
                }
                else ident.at(id_p) += retVal(tag_inf);
            }
        }
    }
}


void interph::insertRtsData(int &scenario_id, ScenarioPoolSet* pool_set, std::map<unsigned int, phaseInfo*>::iterator phase_pos,
        std::list<Region*> &code_significant_regions, std::vector<ObjectiveFunction*> &objectives, std::vector<std::string> &tags,
        unsigned int &phase_iter, int tuningStep) {
    //Insert info for each valid rts of the significant regions
    if (code_significant_regions.size() > 0) {
        for (auto sig_region : code_significant_regions) {
            //get list of rts's for this region
            std::list<Rts*> rtsList = appl->getCalltreeRoot()->getValidRtsOfRegion(sig_region, NULL);
            //get properties
            for( auto rts : rtsList) {
                std::string call_path = rts->getCallPath();
                if (tuningStep == 3) {
                    call_path = std::regex_replace(call_path, std::regex("/Cluster=\\d+"), "");
                }
                auto rts_pos = std::find_if(phase_pos->second->rtsInfo.begin(), phase_pos->second->rtsInfo.end(),
                        [&call_path](auto& rts_inf) {return (rts_inf->callpath == call_path);});
                if( rts_pos != phase_pos->second->rtsInfo.end()) {
                    list<MetaProperty> *properties = pool_set->arp->getPropertiesForScenarioIDandEntity(scenario_id, call_path);
                    if(!properties->empty()) {
                        for(auto obj : objectives) {
                            double objValue = obj->objective(*properties);
                            switch(tuningStep) {
                            case 2:
                                (*rts_pos)->defaultObjValues.insert(std::make_pair(obj->getName(),objValue));
                                break;
                            case 3:
                                (*rts_pos)->objValues[obj->getName()] = objValue;
                                break;
                            }
                        }
                        double totInstr(0.0), L3Misses(0.0), BranchInstr(0.0), computeIntensity(0.0),execTime(0.0),instances(0.0);
                        std::vector<double> rts_ident{totInstr, L3Misses, BranchInstr,instances,execTime};

                        interph::getMeasurements(*properties, rts_ident, tags, phase_iter );

                        for(int rts_id_p = 0; rts_id_p < rts_ident.size(); rts_id_p++) {
                            switch(tuningStep) {
                            case 2:
                                (*rts_pos)->defaultPhaseIdentifiers.insert(std::make_pair(tags[rts_id_p],rts_ident[rts_id_p]));
                                break;
                            case 3:
                                (*rts_pos)->phaseIdentifiers[tags[rts_id_p]] = rts_ident[rts_id_p];
                                break;
                            }
                        }
                    }
                    delete properties;
                }
            }
        }
    }
}


void ReadexInterphasePlugin::insertPhaseData( int scenario_id, ScenarioPoolSet* pool_set ) {
    using namespace interph;

    list<MetaProperty> properties = pool_set->srp->getScenarioResultsByID(scenario_id);

    unsigned int phase_iter;
    double totInstr(0.0), L3Misses(0.0), BranchInstr(0.0), computeIntensity(0.0),execTime(0.0),instances(0.0);
    std::vector<double> phase_ident{totInstr, L3Misses, BranchInstr,instances,execTime};
    std::vector<std::string> tags{"TotalInstr","L3Misses","BranchInstr","Instances","ExecTime","PhaseIter"};

    for(int phase_id_p = 0; phase_id_p < phase_ident.size(); phase_id_p++) {
    }

    //Insert a new record when we encounter the next phase
    getMeasurements(properties, phase_ident, tags, phase_iter );

    switch(tuningStep) {
    case 1:
    {
        phaseInfo* phaseInf = new phaseInfo;
        for(int phase_id_p = 0; phase_id_p < phase_ident.size(); phase_id_p++) {
            phaseInf->phaseIdentifiers.insert(std::make_pair(tags[phase_id_p],phase_ident[phase_id_p]));
        }

        phaseInf->scenarioID = scenario_id;
        for(auto obj : objectives) {
            double objValue = obj->objective(scenario_id, pool_set->srp);
            phaseInf->objValues.insert(std::make_pair(obj->getName(),objValue));
        }

        phaseInf->scenarioConfig = getTuningParameters(scenario_id);

        //Insert info for each valid rts of the significant regions
        if (code_significant_regions.size() > 0) {
            for (auto sig_region : code_significant_regions) {
                //get list of rts's for this region
                std::list<Rts*> rtsList = appl->getCalltreeRoot()->getValidRtsOfRegion(sig_region, NULL);
                //get properties
                for( auto rts : rtsList) {
                    std::unique_ptr<rts_info> rts_info_l(new rts_info);
                    list<MetaProperty> *properties = pool_set->arp->getPropertiesForScenarioIDandEntity(scenario_id, rts->getCallPath());
                    if(!properties->empty()) {
                        rts_info_l->callpath = rts->getCallPath();
                        for(auto obj : objectives) {
                            double objValue = obj->objective(*properties);
                            rts_info_l->objValues.insert(std::make_pair(obj->getName(),objValue));
                        }
                        double totInstr(0.0), L3Misses(0.0), BranchInstr(0.0), computeIntensity(0.0),execTime(0.0),instances(0.0);
                        std::vector<double> rts_ident{totInstr, L3Misses, BranchInstr,instances,execTime};

                        getMeasurements(*properties, rts_ident, tags, phase_iter );

                        for(int rts_id_p = 0; rts_id_p < rts_ident.size(); rts_id_p++) {
                            rts_info_l->phaseIdentifiers.insert(std::make_pair(tags[rts_id_p],rts_ident[rts_id_p]));
                        }

                        phaseInf->rtsInfo.insert(std::move(rts_info_l));
                        delete properties;
                    }
                }
            }
        }

        dtaPhases.insert(std::make_pair(phase_iter-1,phaseInf)); //curr_phase-1 because dp->getCurrentIterationNumber is incremented even the application is restarted
        break;
    }
    case 2:
    {
        std::map<unsigned int, phaseInfo*>::iterator phase_pos = dtaPhases.find(phase_iter-1-static_cast<unsigned int>(samples)); //curr_phase-samples because dp->getCurrentIterationNumber gets incremented even the application is restarted
        if(phase_pos != dtaPhases.end()) {
            for(int phase_id_p = 0; phase_id_p < phase_ident.size(); phase_id_p++) {
                phase_pos->second->defaultPhaseIdentifiers[tags[phase_id_p]] += phase_ident[phase_id_p];
            }
            for(auto obj : objectives) {
                double objValue = obj->objective(scenario_id, pool_set->srp);
                phase_pos->second->defaultObjValues.insert(std::make_pair(obj->getName(),objValue));
            }

            insertRtsData(scenario_id, pool_set, phase_pos, code_significant_regions, objectives, tags, phase_iter, tuningStep);
        }
        else  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: Could not find the phase iteration! Inserting default objective values failed. \n");
        break;
    }
    case 3:
    {
        std::map<unsigned int, phaseInfo*>::iterator phase_pos = dtaPhases.find(phase_iter-1-2*static_cast<unsigned int>(samples)); //curr_phase-samples because dp->getCurrentIterationNumber gets incremented even the application is restarted
        if(phase_pos != dtaPhases.end()) {
            for(int phase_id_p = 0; phase_id_p < phase_ident.size(); phase_id_p++) {
                phase_pos->second->phaseIdentifiers[tags[phase_id_p]] = phase_ident[phase_id_p];
            }
            for(auto obj : objectives) {
                double objValue = obj->objective(scenario_id, pool_set->srp);
                phase_pos->second->objValues[obj->getName()] = objValue;
            }

            insertRtsData(scenario_id, pool_set, phase_pos, code_significant_regions, objectives, tags, phase_iter, tuningStep);
        }
        else psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: Could not find the phase iteration! Inserting objective values failed for the verification step.\n");
    }
    }
}


void interph::normalizeObjectives(std::vector<ObjectiveFunction*> &objectives) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: Normalizing objectives\n");
    for(auto &i : interph::dtaPhases) {
        double tot_ins = i.second->phaseIdentifiers.find("TotalInstr")->second;
        for(auto &obj : i.second->objValues) {
            //Normalize the objective value only if the objective doesn't contain Normalized as it was already normalized before.
            if( obj.first.find("Normalized") == std::string::npos) {
                i.second->normalizedObj.insert(std::make_pair(obj.first,obj.second/tot_ins));
            }
            else i.second->normalizedObj.insert(std::make_pair(obj.first,obj.second));
        }
        for(auto &rts_inf : i.second->rtsInfo) {
            double tot_ins = rts_inf->phaseIdentifiers.find("TotalInstr")->second;
            for(auto &obj_i : rts_inf->objValues) {
                if( obj_i.first.find("Normalized") == std::string::npos) {
                    rts_inf->normalizedObj.insert(std::make_pair(obj_i.first,obj_i.second/tot_ins));
                }
                else rts_inf->normalizedObj.insert(std::make_pair(obj_i.first,obj_i.second));
            }
        }
    }
}


//TODO: Remove this function because we don't need normalized obj values to compute savings
void ReadexInterphasePlugin::normalizeDefaultObjectives() {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: Normalizing default objectives\n");
    for(auto &i : interph::dtaPhases) {
        double tot_ins = i.second->defaultPhaseIdentifiers.find("TotalInstr")->second;
        i.second->normalizedDefaultObj.insert(std::make_pair("NormalizedNodeEnergy",i.second->defaultObjValues.find("NodeEnergy")->second/tot_ins));
        i.second->normalizedDefaultObj.insert(std::make_pair("NormalizedCPUEnergy",i.second->defaultObjValues.find("CPUEnergy")->second/tot_ins));
        double norm_time = i.second->defaultObjValues.find("ExecTime")->second/tot_ins;
        i.second->normalizedDefaultObj.insert(std::make_pair("NormalizedExecTime",norm_time));
    }
}


void interph::calcComputeIntensity() {
    for(auto &i : interph::dtaPhases) {
        double computeIns = i.second->phaseIdentifiers.find("TotalInstr")->second/i.second->phaseIdentifiers.find("L3Misses")->second;
        i.second->phaseIdentifiers.insert(std::make_pair("ComputeIntensity",computeIns));
    }
}


void interph::normalizeIdentifiers() {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: Normalizing identifiers\n");
    using namespace interph;
    double max_comp(0.0),max_br(0.0);
    double min_comp(std::numeric_limits<double>::max()), min_br(std::numeric_limits<double>::max());

    for(auto &p1: dtaPhases) {
        if(p1.second->phaseIdentifiers.find("ComputeIntensity")->second > max_comp) {
            max_comp = p1.second->phaseIdentifiers.find("ComputeIntensity")->second;
        }
        if(p1.second->phaseIdentifiers.find("ComputeIntensity")->second < min_comp) {
            min_comp = p1.second->phaseIdentifiers.find("ComputeIntensity")->second;
        }

        if(p1.second->phaseIdentifiers.find("BranchInstr")->second/p1.second->phaseIdentifiers.find("TotalInstr")->second > max_br) {
            max_br = p1.second->phaseIdentifiers.find("BranchInstr")->second/p1.second->phaseIdentifiers.find("TotalInstr")->second;
        }
        if(p1.second->phaseIdentifiers.find("BranchInstr")->second/p1.second->phaseIdentifiers.find("TotalInstr")->second < min_br) {
            min_br = p1.second->phaseIdentifiers.find("BranchInstr")->second/p1.second->phaseIdentifiers.find("TotalInstr")->second;
        }
    }

    for(auto &i: dtaPhases) {
        i.second->normalizedFeatures = std::make_pair(((i.second->phaseIdentifiers.find("ComputeIntensity")->second-min_comp)/(max_comp-min_comp)),
                ((i.second->phaseIdentifiers.find("BranchInstr")->second/i.second->phaseIdentifiers.find("TotalInstr")->second)
                        - min_br)/(max_br-min_br));
        i.second->phaseIdentifiers.emplace(std::make_pair("BranchInstructions",
                (i.second->phaseIdentifiers.find("BranchInstr")->second/i.second->phaseIdentifiers.find("TotalInstr")->second)));
    }
}


void interph::clust::cluster() {
    clust::calcDistanceMatrix(dtaPhases);
    num_clusters = clust::clusterPhases(dtaPhases)-1;
    clust::printClusters(dtaPhases);
    clust::exportClusters(dtaPhases,num_clusters);
}


void interph::clust::calcDistanceMatrix(const std::map<unsigned int,phaseInfo*> pt_x ) {
    const std::map<unsigned int, phaseInfo*> pt_y = pt_x;
    for( auto &i1 : pt_x) {
        std::vector<double> distances_from_curr_point;
        for( auto &i2 : pt_y) {
            //Squared Euclidean is faster than Euclidean
            double distance = std::sqrt((std::pow((i2.second->normalizedFeatures.second-i1.second->normalizedFeatures.second),2) + std::pow((i2.second->normalizedFeatures.first-i1.second->normalizedFeatures.first),2)));
            dist_matrix.insert(std::make_pair(std::make_pair(i1.first,i2.first),distance));
            if(i1.first != i2.first)
                distances_from_curr_point.push_back(distance);
        }
        if(!distances_from_curr_point.empty()) {
            double kNDist = findKNearestNeighbors(distances_from_curr_point);
            k_neigh_dist.push_back(std::make_pair(i1.first,kNDist));
        }
    }
    //Exporting 3-NN distances to a file
    std::ofstream result_file("distances.csv");
    if( !result_file ) {
        psc_errmsg( "ReadexInterphasePlugin: Unable to create the results file.\n" );
    }
    for(auto &dist_pt : k_neigh_dist) {
        result_file << dist_pt.first << "," << std::setprecision(10)  << dist_pt.second;
        result_file << std::endl;
    }
    result_file.flush();
}

//double interph::clust::findKNearestNeighbors(std::vector<double> &distances_from_point ) {
//    std::sort(distances_from_point.begin(),distances_from_point.end());
//    return distances_from_point.at(2);
//}


double interph::clust::findKNearestNeighbors(std::vector<double> &distances_from_point ) {
    std::sort(distances_from_point.begin(),distances_from_point.end());
    //Return the average of 3-NN distance
    return (distances_from_point.at(0) + distances_from_point.at(1) + distances_from_point.at(2))/3;
}


double interph::clust::calculateEps(std::vector<std::pair<unsigned int,double> > &k_nn_dist) {
    //First find the min and max values in order to draw a line between the two points
    std::sort(k_nn_dist.begin(),k_nn_dist.end(),[](std::pair<unsigned int,double>& k1, std::pair<unsigned int,double>& k2){
        return k1.second < k2.second;
    });

    /*Compute the values of A, B and C for Ax+By+C=0 (equation of the line b/w min val and max val)
     * a = y2 - y1
     * b = x1 - x2
     * c = -a*x1 - b*y1
     * Distance of a point (x,y) to the line = |(a*x + b*y + c)| / sqrt(a^2 + b^2 )
     */

    //For x1 and x2, I changed everything to item position of k_nn_dist rather than take the absolute values of the phase numbers
    double a_l = k_nn_dist.back().second - k_nn_dist.front().second;
    double b_l = 0.0 - (double)(k_nn_dist.size()-1);
    double c_l = ((-a_l * 0.0) -(b_l * k_nn_dist.front().second));

    std::vector<std::pair<unsigned int, double> > dist_l;

    for(unsigned int pt = 0; pt < k_nn_dist.size(); ++pt ) {
        dist_l.push_back(std::make_pair(pt, std::abs((a_l * pt + b_l * k_nn_dist[pt].second + c_l) / std::sqrt((a_l * a_l) + (b_l * b_l)))));
    }

    //Now find the max distance from point to line
    unsigned int pos = std::max_element(dist_l.begin(),dist_l.end(),[](auto& p1, auto& p2){
        return p1.second < p2.second; })->first;
    return k_nn_dist.at(pos).second;
}


std::vector<unsigned int> interph::clust::checkDistance(std::map<unsigned int, phaseInfo*>::iterator phase_i, std::map<std::pair<unsigned int,
                                                        unsigned int>,double> dist, double eps) {
    for(auto &d : dist) {
        if(d.first.first == phase_i->first && d.second <= eps && d.first.second != phase_i->first) {
            phase_i->second->neighbors.push_back(d.first.second);
        }
    }
    return phase_i->second->neighbors;
}

void interph::clust::expandCluster(std::vector<unsigned int>& neighbors, std::map<unsigned int, phaseInfo*> &phases, const double eps,
                                   const int& cluster_num, std::map<unsigned int, phaseInfo*>::iterator input_i) {
    input_i->second->clusterID = cluster_num;
    for(int n = 0; n < neighbors.size(); n++) {
        std::map<unsigned int, phaseInfo*>::iterator next_point = phases.find(neighbors[n]);
        if(!next_point->second->visited) {
            next_point->second->visited = true;
            std::vector<unsigned int> neigh = interph::clust::checkDistance(next_point,dist_matrix, eps);
            if(neigh.size() >= MIN_POINTS) {
                neighbors.insert(neighbors.end(),neigh.begin(),neigh.end());
            }
        }
        //A point that was labeled NOISE before may now be part of a new cluster
        if(next_point->second->clusterID == UNCLASSIFIED || next_point->second->clusterID == NOISE)
            next_point->second->clusterID = cluster_num;
    }
}

int interph::clust::clusterPhases(std::map<unsigned int, phaseInfo*> &input_data) {
    assert(input_data.size()!=0);
    int cluster_num(1);
    double eps = calculateEps(k_neigh_dist);

    for(std::map<unsigned int, phaseInfo*>::iterator input_i = input_data.begin(); input_i != input_data.end(); input_i++) {
        if(input_i->second->visited == true) continue;
        else {
            input_i->second->visited = true;
            std::vector<unsigned int> neigh = interph::clust::checkDistance(input_i,dist_matrix, eps);
            if(neigh.size() >= MIN_POINTS) {
                expandCluster(input_i->second->neighbors, dtaPhases, eps, cluster_num, input_i);
                cluster_num += 1;
            }
            else input_i->second->clusterID = NOISE;
        }
    }
    return cluster_num;
}


void interph::exportData(std::vector<TuningParameter*> tuningParameters, std::vector<ObjectiveFunction*> objectives) {
    result_oss << "\nAll Results:\n";
    result_oss << "Scenario   |    ";
    result_oss << "Phase    |    ";
    result_oss << "ComputeIntensity    |    ";
    result_oss << "BranchInstr    |    ";
    result_oss << "Instructions    |    ";
    result_oss << "L3_TCM    |    ";
    result_oss << "Cluster    |    ";

    for (int i = 0; i < tuningParameters.size(); i++) {
        result_oss << tuningParameters[i]->getName() << "   |   ";
    }

    for (int i = 0; i < objectives.size(); i++) {
        result_oss << objectives[i]->getName() << "   |   ";
    }

    result_oss << endl;

    for (auto dta_phase : dtaPhases ) {
        result_oss << "  " << dta_phase.second->scenarioID << "\t\t";
        result_oss << dta_phase.first << "\t\t";
        result_oss << std::setprecision(4) << std::setw(14) << dta_phase.second->phaseIdentifiers.find("ComputeIntensity")->second << "\t   ";
        result_oss << std::setprecision(4) << std::setw(14) << dta_phase.second->phaseIdentifiers.find("BranchInstr")->second << "\t";
        result_oss << std::setprecision(4) << std::setw(14) << dta_phase.second->phaseIdentifiers.find("TotalInstr")->second << "\t";
        result_oss << std::setprecision(4) << std::setw(14) << dta_phase.second->phaseIdentifiers.find("L3Misses")->second << "\t\t";
        result_oss << dta_phase.second->clusterID << "\t\t";

        for (auto tp : tuningParameters) {
            auto tp_value = std::find_if(dta_phase.second->scenarioConfig.begin(), dta_phase.second->scenarioConfig.end(),
                    [tp](auto& tp_i) {return (tp_i.first->getName() == tp->getName()); });
            result_oss << tp_value->second << "\t\t";
        }
        for (int i = 0; i < objectives.size(); i++) {
            result_oss << std::setprecision(4) << std::setw(14) << dta_phase.second->objValues.find(objectives[i]->getName())->second << "\t\t";
        }
        result_oss << endl;
    }
    cout << result_oss.str();
}


void interph::clust::exportClusters(const std::map<unsigned int, phaseInfo*> results,unsigned int num_clusters) {
    std::ofstream result_file("cluster_results.csv");
    if( !result_file ) {
        psc_errmsg( "ReadexInterphasePlugin: Unable to create the results file.\n" );
    }
    int clust_num = NOISE;
    result_file << "Cluster,Phase,BranchInstr,ComputeIntensity,NormalizedBranchInstr,NormalizedComputeIntensity,Energy,Scenario";

    for(auto sc_config : results.begin()->second->scenarioConfig ) {
        result_file << "," << sc_config.first->getName();
    }
    result_file << std::endl;
    generateFile(results, result_file);
}

void interph::clust::generateFile(const std::map<unsigned int, phaseInfo*> results, std::ofstream &result_file) {
    for( auto i : results ) {
        result_file << i.second->clusterID;
        result_file << "," << i.first;
        result_file << "," << i.second->phaseIdentifiers.find("BranchInstr")->second;
        result_file << "," << i.second->phaseIdentifiers.find("ComputeIntensity")->second;
        result_file << "," << i.second->normalizedFeatures.first;
        result_file << "," << i.second->normalizedFeatures.second;
        result_file << "," << i.second->objValues.begin()->second;
        result_file << "," << i.second->scenarioID;
        for(auto sc_config : i.second->scenarioConfig ) {
            result_file << "," << sc_config.second;
        }
        result_file << std::endl;
    }
    result_file.flush();
}


void interph::computeBestConfigForCluster(std::vector<ObjectiveFunction*> objectives) {
    unsigned int curr_cluster(1);
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: computeBestConfigForCluster entered \n");
    for(; curr_cluster <= interph::clust::num_clusters; curr_cluster++) {
        double best_obj = std::numeric_limits<double>::max();
        double worst_obj = std::numeric_limits<double>::min();
        unsigned int                             phase_num(0);
        std::set<unsigned int>          phases_in_cluster;
        std::unordered_map<std::string,std::pair<double,unsigned int> > rts_best_obj;

        for(auto &dta_phase : dtaPhases) {
            if(dta_phase.second->clusterID == curr_cluster) {
                //Best objective for the phase rts's of the cluster
                if(dta_phase.second->normalizedObj.find(objectives[0]->getName())->second < best_obj) {
                    best_obj = dta_phase.second->normalizedObj.find(objectives[0]->getName())->second;
                    phase_num = dta_phase.first;
                }
                //Insert the phases belonging to the current cluster
                phases_in_cluster.insert(dta_phase.first);

                //Best objective for the rts's of the cluster
                for(auto &rts_inf : dta_phase.second->rtsInfo) {
                    double curr_obj = rts_inf->normalizedObj.find(objectives[0]->getName())->second;
                    auto rts_pos = rts_best_obj.find(rts_inf->callpath);
                    if( rts_inf->objValues.find(objectives[0]->getName())->second != 0.0 ) {  //Compute best objective only for rts's that have obj value > 0.0
                        if( rts_pos == rts_best_obj.end()) {
                            rts_best_obj.insert(std::make_pair(rts_inf->callpath,std::make_pair(curr_obj,dta_phase.first)));
                        }
                        else {
                            if(curr_obj < rts_pos->second.first ) {
                                //total_obj += std::get<1>(rts_pos->second);
                                rts_best_obj[rts_inf->callpath] = std::make_pair(curr_obj,dta_phase.first);
                            }
                        }
                    }
                }
            }
        }

        interph::configForClusters.insert(std::make_pair(curr_cluster,std::make_pair(phase_num,phases_in_cluster)));
        interph::rts_best_objs.push_back(rts_best_obj);
    }
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: computeBestConfigForCluster finished\n");
}



//void interph::generateXML(){
//    ptree c_i;
//    ptree br_i;
//    ptree phase_features;
//    ptree clusters;
//    ptree cluster_i;
//
//    for(auto &child : appl->getCalltreeRoot()->getChildren()) {
//
//        for(auto &param : child->getParameter()) {
//            cluster_i.put( "ID", param->param_value );
//        }
//
//        std::map<std::string, std::map< ConfigType, TuningResult* > > tuning_plugin_result;
//
//        auto tuning_result = child->tuning_plugin_result.begin()->second.find(STATIC_BEST);
//
//        auto pos = tuning_result->second->phase_identifier_range.find("ComputeIntensity");
//
//        c_i.put( "min", pos->second.first );
//        c_i.put( "max", pos->second.second);
//
//        auto pos1 = tuning_result->second->phase_identifier_range.find("BranchInstructions");
//
//        br_i.put( "min", pos1->second.first );
//        br_i.put( "max", pos1->second.second );
//
//        phase_features.put_child( "ComputeIntensity", c_i );
//        phase_features.put_child( "BranchInstr", br_i );
//        cluster_i.put_child( "PhaseFeatures", phase_features );
//        std::string phases("");
//        for(auto &ph : tuning_result->second->phases_in_cluster) {
//            phases += std::to_string(ph);
//        }
//        cluster_i.put( "Phases", phases );
//        cluster_tree.add_child( "Cluster", cluster_i );
//    }
//    interph::createXMLFile();
//}


//void interph::createXMLFile() {
//    ptree xml_tree;
//    xml_tree.add_child( "Cluster-Info.Clusters" , cluster_tree );
//#if BOOST_VERSION >=105600
//    write_xml( "phase_info.xml", xml_tree, locale(), xml_writer_settings<ptree::key_type>( ' ', 4 ) );
//#else
//    write_xml( "phase_info.xml", xml_tree, locale(), xml_writer_settings<char>( ' ', 4 ) );
//#endif
//}


void interph::insertTuningResultForNode(std::vector<ObjectiveFunction*> objectives) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: insertTuningResultForNode entered\n");

    result_oss << "\nResults per Cluster:\n";

    for(auto& cluster_i : interph::configForClusters) {
        double ph_best_obj(0.0), ph_default_obj(0.0);
        int    ph_best_scenario;
        std::map<std::string, double>   phase_objectives, phase_extra_info;
        std::map<TuningParameter*, int> ph_best_config;
        std::unordered_map<std::string, std::pair<double,double> > phase_identifier_range; //In the form <"ComputeIntensity",min,max>
        std::string                     modified_callpath = appl->getCalltreeRoot()->getCallPath() + "/Cluster=" + std::to_string(cluster_i.first);

        phase_extra_info.insert(dtaPhases.find(cluster_i.second.first)->second->phaseIdentifiers.begin(),dtaPhases.find(cluster_i.second.first)->second->phaseIdentifiers.end());
        ph_default_obj =  dtaPhases.find(cluster_i.second.first)->second->defaultObjValues.find(objectives[0]->getName())->second;

        for(auto& phase : cluster_i.second.second) {
            auto phase_info = dtaPhases.find(phase);

            //Insert the ranges for the phase identifiers of the cluster
            std::vector<std::string> tags{"ComputeIntensity","BranchInstructions"};

            if(phase_identifier_range.empty()) {
                for(auto tags_i : tags) {
                    phase_identifier_range.insert(std::make_pair(tags_i,std::make_pair(phase_info->second->phaseIdentifiers.find(tags_i)->second,
                            phase_info->second->phaseIdentifiers.find(tags_i)->second)));
                }
            }
            else {
                for(auto tags_i : tags) {
                    if(phase_identifier_range.find(tags_i)->second.first > phase_info->second->phaseIdentifiers.find(tags_i)->second) {
                        phase_identifier_range.find(tags_i)->second.first = phase_info->second->phaseIdentifiers.find(tags_i)->second;
                    }
                    else {
                        if(phase_identifier_range.find(tags_i)->second.second < phase_info->second->phaseIdentifiers.find(tags_i)->second) {
                            phase_identifier_range.find(tags_i)->second.second = phase_info->second->phaseIdentifiers.find(tags_i)->second;
                        }
                    }
                }
            }
        }

        //Insert STATIC_BEST tuning results for the current cluster
        appl->getCalltreeRoot()->getRtsByCallpath(modified_callpath)->insertPluginResult("readex_interphase",
                dtaPhases.find(cluster_i.second.first)->second->objValues.find(objectives[0]->getName())->second,
                dtaPhases.find(cluster_i.second.first)->second->scenarioConfig, dtaPhases.find(cluster_i.second.first)->second->objValues,
                phase_extra_info, STATIC_BEST, phase_identifier_range, cluster_i.second.second );

        //Insert DEFAULT objective values for the current cluster
        appl->getCalltreeRoot()->getRtsByCallpath(modified_callpath)->insertDefaultObjValue(objectives[0]->getName(),ph_default_obj);

        //Export phase results to ostream
        result_oss << "\nCluster: " << cluster_i.first << "\n";
        result_oss << "\nPhase: " << modified_callpath << "\n";
        result_oss << "Optimum Scenario:  " << dtaPhases.find(cluster_i.second.first)->second->scenarioID << "\n";
        result_oss << "Tuning Parameters: \n";
        for (auto tp : dtaPhases.find(cluster_i.second.first)->second->scenarioConfig) {
            result_oss << "\t  " << tp.first->getName() << " = " << tp.second << "\n";
        }
        result_oss << "Objective: \n";
        result_oss << "\t  " << objectives[0]->getName() << " = " << dtaPhases.find(cluster_i.second.first)->second->objValues.find(objectives[0]->getName())->second << "\n\n\n";

        result_oss << "Results for Rts's: \n\n";

        for(auto& best_i : rts_best_objs[cluster_i.first-1]) {
            //Get best phase for each rts
            auto best_phase = dtaPhases.find(best_i.second.second);

            auto rts_p = std::find_if(best_phase->second->rtsInfo.begin(), best_phase->second->rtsInfo.end(),
                    [&best_i](auto& rts_inf) {return (rts_inf->callpath == best_i.first); });

            //First modify the rts's callpath to include /Cluster=num before inserting the tuning results
            std::string rts_modified_callpath = best_i.first;
            std::size_t pos   = best_i.first.find(appl->getCalltreeRoot()->getCallPath());
            std::string add_str = "/Cluster=" + std::to_string(cluster_i.first);
            rts_modified_callpath = rts_modified_callpath.insert(pos + appl->getCalltreeRoot()->getCallPath().length(),add_str);

            //Insert RTS_BEST configuration for current rts
            appl->getCalltreeRoot()->getRtsByCallpath(rts_modified_callpath)->insertPluginResult("readex_interphase",
                    (*rts_p)->objValues.find(objectives[0]->getName())->second, best_phase->second->scenarioConfig, (*rts_p)->objValues,
                    (*rts_p)->phaseIdentifiers, RTS_BEST );

            //Export rts results to ostream
            result_oss << "Rts: " << rts_modified_callpath << "\n";
            result_oss << "Tuning Parameters: \n";
            for (auto tp : best_phase->second->scenarioConfig) {
                result_oss << "\t  " << tp.first->getName() << " = " << tp.second << "\n";
            }
            result_oss << "Objective Value: \n";
            result_oss << "\t  " << objectives[0]->getName() << " = " << (*rts_p)->objValues.find(objectives[0]->getName())->second << "\n\n\n";

            //Insert DEFAULT objective values for current rts
            appl->getCalltreeRoot()->getRtsByCallpath(rts_modified_callpath)->insertDefaultObjValue(objectives[0]->getName(),
                    (*rts_p)->defaultObjValues.find(objectives[0]->getName())->second);

            //            //Get the static best phase for each rts
            auto static_best_phase = dtaPhases.find(cluster_i.second.first);

            auto static_rts_p = std::find_if(static_best_phase->second->rtsInfo.begin(), static_best_phase->second->rtsInfo.end(),
                    [&best_i](auto& rts_inf) {return (rts_inf->callpath == best_i.first); });


            //Check whether we find the current rts for the static best phase
            if( static_rts_p != static_best_phase->second->rtsInfo.end() ) {
                //Insert STATIC_BEST configuration for current rts
                appl->getCalltreeRoot()->getRtsByCallpath(rts_modified_callpath)->insertPluginResult("readex_interphase",
                        (*static_rts_p)->objValues.find(objectives[0]->getName())->second, static_best_phase->second->scenarioConfig,
                        (*static_rts_p)->objValues, (*static_rts_p)->phaseIdentifiers, STATIC_BEST );
            }
            else {
                //Insert 0.0 as the objective value for current rts
                appl->getCalltreeRoot()->getRtsByCallpath(rts_modified_callpath)->insertPluginResult("readex_interphase",
                        0.0, static_best_phase->second->scenarioConfig, (*rts_p)->objValues, (*rts_p)->phaseIdentifiers, STATIC_BEST );
            }
        }
        result_oss << endl;
    }
    //Generate an XML file for the runtime cluster prediction library
    //    interph::generateXML();

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: insertTuningResultForNode finished \n");
}


void interph::insertResultForNode(std::vector<ObjectiveFunction*> objectives) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: insertResultForNode entered\n");

    for(auto& cluster_i : interph::configForClusters) {
        double ph_best_obj(0.0), ph_default_obj(0.0);
        int    ph_best_scenario;
        std::map<std::string, double>   phase_objectives, phase_extra_info;
        std::map<TuningParameter*, int> ph_best_config;
        std::string                     modified_callpath = appl->getCalltreeRoot()->getCallPath() + "/Cluster=" + std::to_string(cluster_i.first);
        phase_extra_info.insert(dtaPhases.find(cluster_i.second.first)->second->phaseIdentifiers.begin(),dtaPhases.find(cluster_i.second.first)->second->phaseIdentifiers.end());

        //Insert TUNING_RESULT_STATIC tuning results for the current cluster
        appl->getCalltreeRoot()->getRtsByCallpath(modified_callpath)->insertPluginResult("readex_interphase",
                dtaPhases.find(cluster_i.second.first)->second->objValues.find(objectives[0]->getName())->second,
                dtaPhases.find(cluster_i.second.first)->second->scenarioConfig, dtaPhases.find(cluster_i.second.first)->second->objValues,
                phase_extra_info, TUNING_RESULT_STATIC );

        for(auto& best_i : rts_best_objs[cluster_i.first-1]) {
            //Get best phase for each rts
            auto best_phase = dtaPhases.find(best_i.second.second);

            auto rts_p = std::find_if(best_phase->second->rtsInfo.begin(), best_phase->second->rtsInfo.end(),
                    [&best_i](auto& rts_inf) {return (rts_inf->callpath == best_i.first); });

            //First modify the rts's callpath to include /Cluster=num before inserting the tuning results
            std::string rts_modified_callpath = best_i.first;
            std::size_t pos   = best_i.first.find(appl->getCalltreeRoot()->getCallPath());
            std::string add_str = "/Cluster=" + std::to_string(cluster_i.first);
            rts_modified_callpath = rts_modified_callpath.insert(pos + appl->getCalltreeRoot()->getCallPath().length(),add_str);

            //Insert TUNING_RESULT_RTS configuration for current rts
            appl->getCalltreeRoot()->getRtsByCallpath(rts_modified_callpath)->insertPluginResult("readex_interphase",
                    (*rts_p)->objValues.find(objectives[0]->getName())->second, best_phase->second->scenarioConfig, (*rts_p)->objValues,
                    (*rts_p)->phaseIdentifiers, TUNING_RESULT_RTS );

            //Get the static best phase for each rts
            auto static_best_phase = dtaPhases.find(cluster_i.second.first);

            auto static_rts_p = std::find_if(static_best_phase->second->rtsInfo.begin(), static_best_phase->second->rtsInfo.end(),
                    [&best_i](auto& rts_inf) {return (rts_inf->callpath == best_i.first); });

            //Check whether we find the current rts for the static best phase
            if( static_rts_p != static_best_phase->second->rtsInfo.end() ) {
                //Insert TUNING_RESULT_STATIC configuration for current rts
                appl->getCalltreeRoot()->getRtsByCallpath(rts_modified_callpath)->insertPluginResult("readex_interphase",
                        (*static_rts_p)->objValues.find(objectives[0]->getName())->second, static_best_phase->second->scenarioConfig,
                        (*static_rts_p)->objValues, (*static_rts_p)->phaseIdentifiers, TUNING_RESULT_STATIC );
            }
            else {
                //Insert 0.0 as the objective value for current rts
                appl->getCalltreeRoot()->getRtsByCallpath(rts_modified_callpath)->insertPluginResult("readex_interphase",
                        0.0, static_best_phase->second->scenarioConfig, (*rts_p)->objValues, (*rts_p)->phaseIdentifiers, TUNING_RESULT_STATIC );
            }
        }
    }
    rtstree::displayTrueSavings();
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: insertResultForNode finished \n");
}


void interph::clust::printClusters(std::map<unsigned int, phaseInfo*> &phases) {
    for(auto it : phases) {
        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "Phase iter:%d ",it.first);
        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "Visited:%d ",it.second->visited);
        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ClusterID:%d \n\n",it.second->clusterID);
    }
}


/**
 * @brief Prints to the screen (and to a file, where necessary) the tuning advice.
 *
 * @ingroup ReadexInterphasePlugin
 */
Advice* ReadexInterphasePlugin::getAdvice(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: call to getAdvice()\n");
    generate_tuning_model();

    return NULL;
}

/**
 * @brief Finalize the plugin normally.
 *
 * Remove any allocated memory, objects, file descriptors, etc.
 *
 * @ingroup ReadexInterphasePlugin
 *
 */
void ReadexInterphasePlugin::finalize() {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: call to finalize()\n");
    delete searchAlgorithm;
}

/**
 * @brief Terminate the plugin due to error.
 *
 * Safely remove any allocated memory, objects, file descriptors, etc. This method should
 * be able to be executed safely at any point.
 *
 * @ingroup ReadexInterphasePlugin
 *
 */
void ReadexInterphasePlugin::terminate() {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: call to terminate()\n");
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
 * return a pointer to a new ReadexInterphasePlugin();
 *
 * @ingroup ReadexInterphasePlugin
 *
 * @return A pointer to a new ReadexInterphasePlugin
 */
IPlugin* getPluginInstance(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: call to getPluginInstance()\n");
    return new ReadexInterphasePlugin();
}

/**
 * @brief Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 *
 * @ingroup ReadexInterphasePlugin
 *
 * @return The major plugin interface version used by the plugin
 */
int getVersionMajor(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: call to getInterfaceVersionMajor()\n");

    return 1;
}

/**
 * @brief Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 *
 * @ingroup ReadexInterphasePlugin
 *
 * @return The minor plugin interface version used by the plugin
 *
 */
int getVersionMinor(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: call to getInterfaceVersionMinor()\n");

    return 0;
}

/**
 * @brief Returns a string with the name of the plugin.
 *
 * @ingroup ReadexInterphasePlugin
 *
 * @return Returns a string with the name of the plugin
 *
 */
string getName(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: call to getName()\n");
    return "ReadexInterphasePlugin";
}

/**
 * @brief Returns a string with a short description of the plugin.
 *
 * @ingroup ReadexInterphasePlugin
 *
 * @return A string with a short description of the plugin
 *
 */
string getShortSummary(void) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "ReadexInterphasePlugin: call to getShortSummary()\n");
    return "Readex inter-phase tuning plugin for Taurus";
}
