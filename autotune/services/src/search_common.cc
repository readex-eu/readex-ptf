/**
   @file    search_common.cc
   @ingroup SearchAlgorithms
   @brief   Common functionality used by all Search Algorithms
   @author  Isaias Compres
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include "frontend.h"
#include "search_common.h"
#include "ISearchAlgorithm.h"

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/optional/optional.hpp>
#include <boost/optional.hpp>



//****Energy****

std::string EnergyObjective::getName() {
    return "Energy";
}

double EnergyObjective::objective(int scenario_id, ScenarioResultsPool* srp) {
    std::list<MetaProperty> properties = srp->getScenarioResultsByID(scenario_id);
    return objective(properties);
}


double EnergyObjective::objective(std::list<MetaProperty>& properties) {
    double energy = 0;

    fflush (stdout);
    for( auto property : properties ) {
        if (std::stoi(property.getId()) == ENERGY_CONSUMPTION ||
            std::stoi(property.getId()) == INTERPHASE_PROPS) {
            energy += property.getSeverity();
        }
        if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
            addInfoType addInfo = property.getExtraInfo();
            addInfoType::iterator iter = addInfo.find("NodeEnergy");
            if (iter != property.getExtraInfo().end()) {
                energy += std::stod(iter->second);
            } else {
                psc_errmsg("NodeEnergy not found\n");
            }
        }
    }

    //psc_dbgmsg(6, "Energy: %f;\n", energy);

    return energy;
}


//****NormalizedEnergy****

std::string NormalizedEnergyObjective::getName() {
    return "NormalizedEnergy";
}

double NormalizedEnergyObjective::objective(int scenario_id, ScenarioResultsPool* srp) {
    std::list<MetaProperty> properties = srp->getScenarioResultsByID(scenario_id);
    return objective(properties);
}


double NormalizedEnergyObjective::objective(std::list<MetaProperty>& properties) {
    double energy =0, totInstr = 0.0;

    fflush (stdout);
    for( auto property : properties ) {
        if (std::stoi(property.getId()) == ENERGY_CONSUMPTION ||
            std::stoi(property.getId()) == INTERPHASE_PROPS   ||
            std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
            addInfoType::iterator iter;
            addInfoType addInfo = property.getExtraInfo();
            iter = addInfo.find("TotalInstr");
            if (iter != property.getExtraInfo().end()) {
                 totInstr = std::stod(iter->second);
            }
            else {
                psc_errmsg("TotalInstr not found\n");
            }
            if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                addInfoType::iterator iter = addInfo.find("NodeEnergy");
                if (iter != property.getExtraInfo().end()) {
                    energy += std::stod(iter->second)/totInstr;
                } else {
                    psc_errmsg("NodeEnergy not found\n");
                }
            }
            else {
                energy += property.getSeverity()/totInstr;
            }
        }
    }

//    psc_dbgmsg(6, "NormalizedEnergy: %f;\n", energy);
    return energy;
}





//****CPUEnergy****

std::string CPUEnergyObjective::getName() {
    return "CPUEnergy";
}

double CPUEnergyObjective::objective(int scenario_id, ScenarioResultsPool* srp) {
    std::list<MetaProperty> properties = srp->getScenarioResultsByID(scenario_id);
    return objective(properties);
}


double CPUEnergyObjective::objective(std::list<MetaProperty>& properties) {
    double energy = 0.0;

    for( auto property : properties ) {
        if (std::stoi(property.getId()) == ENERGY_CONSUMPTION ||
            std::stoi(property.getId()) == INTERPHASE_PROPS   ||
            std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
            addInfoType addInfo = property.getExtraInfo();
            //for (const auto& info : addInfo ) {
            //  printf("Iterator->first %s  ->second %s\n",info.first.c_str(), info.second.c_str());fflush(stdout);
            //}
            addInfoType::iterator it = addInfo.find("CPUEnergy");
            if (it != property.getExtraInfo().end()) {
                std::string cpuString = it->second;
                INT64 cpuEnergy = std::stod(cpuString);
                energy += cpuEnergy;
            } else {
                psc_errmsg("CPUEnergy not found\n");
            }
        }
    }

    //psc_dbgmsg(6, "CPU Energy: %f;\n", energy);

    return energy;
}


//****NormalizedCPUEnergy****

std::string NormalizedCPUEnergyObjective::getName() {
    return "NormalizedCPUEnergy";
}

double NormalizedCPUEnergyObjective::objective(int scenario_id, ScenarioResultsPool* srp) {
    std::list<MetaProperty> properties = srp->getScenarioResultsByID(scenario_id);
    return objective(properties);
}


double NormalizedCPUEnergyObjective::objective(std::list<MetaProperty>& properties) {
    double energy =0, totInstr = 0.0;

    fflush (stdout);
    for( auto property : properties ) {
        if (std::stoi(property.getId()) == ENERGY_CONSUMPTION ||
            std::stoi(property.getId()) == INTERPHASE_PROPS   ||
            std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
            addInfoType::iterator it;
            addInfoType addInfo = property.getExtraInfo();
            it = addInfo.find("TotalInstr");
            if (it != property.getExtraInfo().end()) {
                totInstr = std::stod(it->second);
            }
            else {
                psc_errmsg("TotalInstr not found\n");
            }
            it = addInfo.find("CPUEnergy");
            if (it != property.getExtraInfo().end()) {
                energy += std::stod(it->second)/totInstr;
            } else {
                psc_errmsg("CPUEnergy not found\n");
            }

        }
//      if (std::stoi(property.getId()) == INTERPHASE_PROPS) {
//            totInstr += property.getSeverity();
//        }
    }
//    psc_dbgmsg(6, "NormalizedCPUEnergy: %f;\n", energy);
    return energy;
}



//****EDP****


std::string EDPObjective::getName() {
    return "EDP";
}

double EDPObjective::objective(int scenario_id, ScenarioResultsPool* srp) {
    std::list<MetaProperty> properties = srp->getScenarioResultsByID(scenario_id);
    return objective(properties);
}


double EDPObjective::objective(std::list<MetaProperty>& properties) {
    double energy = 0;
    double time = -1.0;

    for( auto property : properties ) {
        if (std::stoi(property.getId()) == ENERGY_CONSUMPTION ||
            std::stoi(property.getId()) == INTERPHASE_PROPS   ||
            std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
            addInfoType::iterator it;
            addInfoType addInfo = property.getExtraInfo();

            if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                addInfoType::iterator iter = addInfo.find("NodeEnergy");
                if (iter != property.getExtraInfo().end()) {
                    energy += std::stod(iter->second);
                } else {
                    psc_errmsg("NodeEnergy not found\n");
                }
            }
            else {
                energy += property.getSeverity();
            }

            if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                it = addInfo.find("cycles");
            } else {
                it = addInfo.find("ExecTime");
            }

            if (it != property.getExtraInfo().end()) {
                double time1(0.0);
                if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                    time1 = std::stod(it->second)/NANOSEC_PER_SEC_DOUBLE;
                } else {
                    time1 = std::stod(it->second);
                }
                if (time < time1) {
                    time = time1;
                }
            } else {
                psc_errmsg("ExecTime not found\n");
            }
        }
    }

    //psc_dbgmsg(6, "EDP: %f;\n", energy * time);

    return energy * time;
}


//****NormalizedEDP****


std::string NormalizedEDPObjective::getName() {
    return "NormalizedEDP";
}

double NormalizedEDPObjective::objective(int scenario_id, ScenarioResultsPool* srp) {
    std::list<MetaProperty> properties = srp->getScenarioResultsByID(scenario_id);
    return objective(properties);
}


double NormalizedEDPObjective::objective(std::list<MetaProperty>& properties) {
    double energy = 0;
    double time = -1.0;
    double totInstr = 0.0;

    for( auto property : properties ) {
        if (std::stoi(property.getId()) == ENERGY_CONSUMPTION ||
            std::stoi(property.getId()) == INTERPHASE_PROPS   ||
            std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
            addInfoType::iterator it;
            addInfoType addInfo = property.getExtraInfo();
            it = addInfo.find("TotalInstr");
            if (it != property.getExtraInfo().end()) {
                totInstr = std::stod(it->second);
            }
            else {
                psc_errmsg("TotalInstr not found\n");
            }

            if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                it = addInfo.find("cycles");
            } else {
                it = addInfo.find("ExecTime");
            }

            if (it != property.getExtraInfo().end()) {
                double time1(0.0);
                if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                    time1 = std::stod(it->second)/totInstr/NANOSEC_PER_SEC_DOUBLE;
                } else {
                    time1 = std::stod(it->second)/totInstr;
                }
                if (time < time1) {
                    time = time1;
                }
            } else {
                psc_errmsg("ExecTime not found\n");
            }

            if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                addInfoType::iterator iter = addInfo.find("NodeEnergy");
                if (iter != property.getExtraInfo().end()) {
                    energy += std::stod(iter->second)/totInstr;
                } else {
                    psc_errmsg("NodeEnergy not found\n");
                }
            }
            else {
                energy += property.getSeverity()/totInstr;
            }
        }
//        if (std::stoi(property.getId()) == INTERPHASE_PROPS) {
//            totInstr += property.getSeverity();
//        }
    }

    //psc_dbgmsg(6, "Normalized EDP: %1.13f;\n", energy * time);

    return energy * time;
}


//****ED2P****


std::string ED2PObjective::getName() {
    return "ED2P";
}

double ED2PObjective::objective(int scenario_id, ScenarioResultsPool* srp) {
    std::list<MetaProperty> properties = srp->getScenarioResultsByID(scenario_id);
    return objective(properties);
}


double ED2PObjective::objective(std::list<MetaProperty>& properties) {
    double energy = 0;
    double time = -1.0;

    for ( auto property : properties ) {
        if (std::stoi(property.getId()) == ENERGY_CONSUMPTION ||
            std::stoi(property.getId()) == INTERPHASE_PROPS   ||
            std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
            addInfoType::iterator it;
            addInfoType addInfo = property.getExtraInfo();

            if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                addInfoType::iterator iter = addInfo.find("NodeEnergy");
                if (iter != property.getExtraInfo().end()) {
                    energy += std::stod(iter->second);
                } else {
                    psc_errmsg("NodeEnergy not found\n");
                }
            }
            else {
                energy += property.getSeverity();
            }

            if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                it = addInfo.find("cycles");
            } else {
                it = addInfo.find("ExecTime");
            }

            if (it != property.getExtraInfo().end()) {
                double time1(0.0);
                if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                    time1 = std::stod(it->second)/NANOSEC_PER_SEC_DOUBLE;
                } else {
                    time1 = std::stod(it->second);
                }
                if (time < time1) {
                    time = time1;
                }
            } else {
                psc_errmsg("ExecTime not found\n");
            }
        }
    }

    //psc_dbgmsg(6, "ED2P: %f;\n", energy * time * time);

    return energy * time * time;
}


//****NormalizedED2P****


std::string NormalizedED2PObjective::getName() {
    return "NormalizedED2P";
}

double NormalizedED2PObjective::objective(int scenario_id, ScenarioResultsPool* srp) {
    std::list<MetaProperty> properties = srp->getScenarioResultsByID(scenario_id);
    return objective(properties);
}


double NormalizedED2PObjective::objective(std::list<MetaProperty>& properties) {
    double energy = 0;
    double time = -1.0;
    double totInstr = 0.0;

    for ( auto property : properties ) {
        if (std::stoi(property.getId()) == ENERGY_CONSUMPTION ||
            std::stoi(property.getId()) == INTERPHASE_PROPS   ||
            std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
            addInfoType::iterator it;
            addInfoType addInfo = property.getExtraInfo();
            it = addInfo.find("TotalInstr");
            if (it != property.getExtraInfo().end()) {
                totInstr = std::stod(it->second);
            }  else {
                psc_errmsg("TotalInstr not found\n");
            }

            if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                it = addInfo.find("cycles");
            } else {
                it = addInfo.find("ExecTime");
            }

            if (it != property.getExtraInfo().end()) {
                double time1(0.0);
                if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                    time1 = std::stod(it->second)/totInstr/NANOSEC_PER_SEC_DOUBLE;
                } else {
                    time1 = std::stod(it->second)/totInstr;
                }
                if (time < time1) {
                    time = time1;
                }
            } else {
                psc_errmsg("ExecTime not found\n");
            }

            if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                addInfoType::iterator iter = addInfo.find("NodeEnergy");
                if (iter != property.getExtraInfo().end()) {
                    energy += std::stod(iter->second)/totInstr;
                } else {
                    psc_errmsg("NodeEnergy not found\n");
                }
            } else {
                energy += property.getSeverity()/totInstr;
            }
        }
//        if (std::stoi(property.getId()) == INTERPHASE_PROPS) {
//            totInstr += property.getSeverity();
//        }
    }

    //psc_dbgmsg(6, "Normalized ED2P: %1.13f;\n", energy * time * time);

    return energy * time * time;
}


//****Time****

std::string TimeObjective::getName() {
    return "Time";
}

double TimeObjective::objective(int scenario_id, ScenarioResultsPool* srp) {
    std::list<MetaProperty> properties = srp->getScenarioResultsByID(scenario_id);
    return objective(properties);
}


double TimeObjective::objective(std::list<MetaProperty>& properties) {
    double time = 0;

    for ( auto property : properties ) {
        if (std::stoi(property.getId()) == ENERGY_CONSUMPTION ||
            std::stoi(property.getId()) == INTERPHASE_PROPS   ||
            std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
            addInfoType::iterator it;
            addInfoType addInfo = property.getExtraInfo();
            if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                it = addInfo.find("cycles");
            }
            else {
                it = addInfo.find("ExecTime");
            }
            if (it != property.getExtraInfo().end()) {
                std::string timeString = it->second;
                if (time < std::stod(timeString)) {
                    if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                        time = std::stod(timeString) / NANOSEC_PER_SEC_DOUBLE;
                    }
                    else
                        time = std::stod(timeString);
                }
            } else {
                psc_errmsg("ExecTime not found\n");
            }
        }
    }

    //psc_dbgmsg(6, "Time: %f;\n", time);

    return time;
}


//****NormalizedTime****

std::string NormalizedTimeObjective::getName() {
    return "NormalizedTime";
}

double NormalizedTimeObjective::objective(int scenario_id, ScenarioResultsPool* srp) {
    std::list<MetaProperty> properties = srp->getScenarioResultsByID(scenario_id);
    return objective(properties);
}


double NormalizedTimeObjective::objective(std::list<MetaProperty>& properties) {
    double time = 0.0;
    double totInstr = 0.0;

    for ( auto property : properties ) {
        if (std::stoi(property.getId()) == ENERGY_CONSUMPTION ||
            std::stoi(property.getId()) == INTERPHASE_PROPS   ||
            std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
            addInfoType::iterator it;
            addInfoType addInfo = property.getExtraInfo();
            it = addInfo.find("TotalInstr");
            if (it != property.getExtraInfo().end()) {
                totInstr = std::stod(it->second);
            }
            else {
                psc_errmsg("TotalInstr not found\n");
            }
            if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                it = addInfo.find("cycles");
            }
            else {
                it = addInfo.find("ExecTime");
            }
            if (it != property.getExtraInfo().end() && totInstr != 0.0) {
                double time1(0.0);
                if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                    time1 = std::stod(it->second)/totInstr/ NANOSEC_PER_SEC_DOUBLE;
                }
                else {
                    time1 = std::stod(it->second)/totInstr;
                }
                if (time < time1) {
                    time = time1;
                }
            } else {
                psc_errmsg("ExecTime not found\n");
            }
        }
//        if (std::stoi(property.getId()) == INTERPHASE_PROPS) {
//            totInstr += property.getSeverity();
//        }

    }

//    psc_dbgmsg(6, "Normalized Time: %1.13f;\n", time);

    return time;
}


//****TCO****
TCOObjective::TCOObjective(std::string unitN):ObjectiveFunction(unitN) {
    unit = unitN;
    try {
        if (opts.has_configurationfile) {
            // extract the significant regions from configuration file provided by readex-dyn-detect tool
            costJoule    = std::stof(configTree.get < std::string > ("Configuration.CostPerJoule"));
            costCoreHour = std::stof(configTree.get < std::string > ("Configuration.CostPerCoreHour"));
        }

    } catch (exception &e) {
        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "TCO Objective: %s\n", e.what());
        costJoule=1.0;
        costCoreHour=1.0;
    }

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "TCO Objective: %f € per Joule, %f € per core hour\n", costJoule, costCoreHour);
}

std::string TCOObjective::getName() {
    return "TCO";
}

double TCOObjective::objective(int scenario_id, ScenarioResultsPool* srp) {
    std::list<MetaProperty> properties = srp->getScenarioResultsByID(scenario_id);
    return objective(properties);
}


double TCOObjective::objective(std::list<MetaProperty>& properties) {
    double time = 0;
    double energy = 0;

    for ( auto property : properties ) {
       if (std::stoi(property.getId()) == ENERGY_CONSUMPTION ||
           std::stoi(property.getId()) == INTERPHASE_PROPS ||
           std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
            addInfoType::iterator it;
            addInfoType addInfo = property.getExtraInfo();

            if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                addInfoType::iterator iter = addInfo.find("NodeEnergy");
                if (iter != property.getExtraInfo().end()) {
                    energy += std::stod(iter->second);
                } else {
                    psc_errmsg("NodeEnergy not found\n");
                }
            }
            else {
                energy += property.getSeverity();
            }

            if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                it = addInfo.find("cycles");
            } else {
                it = addInfo.find("ExecTime");
            }

            if (it != property.getExtraInfo().end()) {
                double time1(0.0);
                if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                    time1 = std::stod(it->second)/NANOSEC_PER_SEC_DOUBLE;
                } else {
                    time1 = std::stod(it->second);
                }
                if (time < time1) {
                    time = time1;
                }
            } else {
                psc_errmsg("ExecTime not found\n");
            }
        }
    }

    //psc_dbgmsg(6, "Time: %f;\n", time);
    return time*fe->get_ompnumthreads()*fe->get_mpinumprocs()*costCoreHour/3600+energy*costJoule;
    //return time;
}


//****normalized TCO****

NormalizedTCOObjective::NormalizedTCOObjective(std::string unitN):ObjectiveFunction(unitN) {
    unit = unitN;
    try {
        if (opts.has_configurationfile) {
            // extract the significant regions from configuration file provided by readex-dyn-detect tool
            costJoule    = std::stof(configTree.get < std::string > ("Configuration.CostPerJoule"));
            costCoreHour = std::stof(configTree.get < std::string > ("Configuration.CostPerCoreHour"));
        }

    } catch (exception &e) {
        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "TCO Objective: %s\n", e.what());
        costJoule=1.0;
        costCoreHour=1.0;
    }

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "TCO Objective: %f € per Joule, %f € per core hour\n", costJoule, costCoreHour);
}


std::string NormalizedTCOObjective::getName() {
    return "NormalizedTCO";
}

double NormalizedTCOObjective::objective(int scenario_id, ScenarioResultsPool* srp) {
    std::list<MetaProperty> properties = srp->getScenarioResultsByID(scenario_id);
    return objective(properties);
}


double NormalizedTCOObjective::objective(std::list<MetaProperty>& properties) {
    double time = 0;
    double energy = 0;
    double totInstr = 0.0;

    for ( auto property : properties ) {
        if (std::stoi(property.getId()) == ENERGY_CONSUMPTION ||
            std::stoi(property.getId()) == INTERPHASE_PROPS ||
            std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
            addInfoType::iterator it;
            addInfoType addInfo = property.getExtraInfo();
            it = addInfo.find("TotalInstr");
            if (it != property.getExtraInfo().end()) {
                totInstr = std::stod(it->second);
            }  else {
                psc_errmsg("TotalInstr not found\n");
            }

            if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                it = addInfo.find("cycles");
            } else {
                it = addInfo.find("ExecTime");
            }

            if (it != property.getExtraInfo().end()) {
                double time1(0.0);
                if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                    time1 = std::stod(it->second)/totInstr/NANOSEC_PER_SEC_DOUBLE;
                } else {
                    time1 = std::stod(it->second)/totInstr;
                }
                if (time < time1) {
                    time = time1;
                }
            } else {
                psc_errmsg("ExecTime not found\n");
            }

            if (std::stoi(property.getId()) == EXECTIMEIMPORTANCE) {
                addInfoType::iterator iter = addInfo.find("NodeEnergy");
                if (iter != property.getExtraInfo().end()) {
                    energy += std::stod(iter->second)/totInstr;
                } else {
                    psc_errmsg("NodeEnergy not found\n");
                }
            } else {
                energy += property.getSeverity()/totInstr;
            }
        }
    }

    return (time*fe->get_ompnumthreads()*fe->get_mpinumprocs()*costCoreHour/3600+energy*costJoule);
}


//****ptf_min****

std::string PTF_minObjective::getName() {
    return "ptf_min";
}

double PTF_minObjective::objective(int scenario_id, ScenarioResultsPool* srp) {
    std::list<MetaProperty> properties = srp->getScenarioResultsByID(scenario_id);
    return objective(properties);
}


double PTF_minObjective::objective(std::list<MetaProperty>& properties) {
    double minimum = 0.0;

    try {
        minimum = properties.front().getSeverity();

        for( auto property : properties ) {
            double current = property.getSeverity();
            if( minimum > current ) {
                minimum = current;
            }
        }
    }
    catch( const std::out_of_range& oor ) {
        psc_errmsg("ptf_min: no properties\n" );
    }
    //psc_dbgmsg(6, "ptf_min: %f;\n", minimum);

    return minimum;
}



//****ptf_max****

std::string PTF_maxObjective::getName() {
    return "ptf_max";
}

double PTF_maxObjective::objective(int scenario_id, ScenarioResultsPool* srp) {
    std::list<MetaProperty> properties = srp->getScenarioResultsByID(scenario_id);
    return objective(properties);
}


double PTF_maxObjective::objective(std::list<MetaProperty>& properties) {
    double maximum = 0.0;

   try {
       maximum = properties.front().getSeverity();

       for( auto property : properties ) {
           double current = property.getSeverity();
           if( maximum < current ) {
               maximum = current;
           }
       }
   }
   catch( const std::out_of_range& oor ) {
       psc_errmsg("ptf_max: no properties\n" );
   }
    //psc_dbgmsg(6, "ptf_max: %f;\n", maximum);

    return maximum;
}




//****inverse_speedup****

//! [Objective_Function_InverseSpeedup]
std::string Inverse_speedupObjective::getName() {
    return "inverse_speedup";
}

double Inverse_speedupObjective::objective(int scenario_id, ScenarioResultsPool* srp) {
   list<MetaProperty> properties      = srp->getScenarioResultsByID( scenario_id );
   base_properties = srp->getScenarioResultsByID( 0 );

   return objective(properties);
}


double Inverse_speedupObjective::objective(std::list<MetaProperty>& properties) {
   // base execution time for single thread on every process
   vector<double> base_ExecTime;
   double         invSpeedup = 0.0;

   // since we pushed a single property, the value will be execTime
   for( auto property : base_properties ) {
       base_ExecTime.push_back( property.getSeverity() );
   }

   int scenario_id = std::stoi(properties.begin()->getExtraInfo().at("ScenarioID"));
   if( scenario_id == 0 ) {
       invSpeedup = 1.0;
   }
   else {
       // execution time for current scenario
       //NOTE: Not sure why only last ExecTime is used and why there is a loop at all. -RM
       double ExecTime = 1.0;
       for( auto property : properties ) {
           ExecTime = property.getSeverity();
       }
       invSpeedup = ExecTime / base_ExecTime[ properties.front().getProcess() ];
   }
   //psc_dbgmsg(6, "InverseSpeedup: %f;\n", invSpeedup );

   return invSpeedup; // returning inverse speedup
}


//! [Objective_Function_InverseSpeedup]


ISearchAlgorithm::ISearchAlgorithm() : search_steps( 0 ) {
    //setObjectiveFunction( ptf_min );
}

ISearchAlgorithm::~ISearchAlgorithm() {
}
