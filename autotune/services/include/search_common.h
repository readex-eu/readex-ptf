/**
   @file    search_common.h
   @ingroup SearchAlgorithms
   @brief   Common functionality used by all Search Algorithms header
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

#ifndef SEARCH_COMMON_H_
#define SEARCH_COMMON_H_

#include <list>
//#include "ISearchAlgorithm.h"
#include "ScenarioResultsPool.h"
#include "ScenarioPoolSet.h"
#include "ATPService.h"

//extern atpService* atp_srvc;

class ObjectiveFunction{
public:
    std::string unit;
    virtual double objective(int scenario_id, ScenarioResultsPool* properties)=0;
    virtual double objective(std::list<MetaProperty>& props)=0;
    virtual std::string getName()=0;
    std::string getUnit(){return unit;}
    ObjectiveFunction(std::string unitName){unit=unitName;}
    ~ObjectiveFunction(){};
};

class EnergyObjective:public ObjectiveFunction{
public:
    double objective(int scenario_id, ScenarioResultsPool* properties);
    double objective(std::list<MetaProperty>& props);
    std::string getName();
    EnergyObjective(std::string unitN):ObjectiveFunction(unitN){};
};

class NormalizedEnergyObjective:public ObjectiveFunction{
public:
    double objective(int scenario_id, ScenarioResultsPool* properties);
    double objective(std::list<MetaProperty>& props);
    std::string getName();
    NormalizedEnergyObjective(std::string unitN):ObjectiveFunction(unitN){};
};

class EDPObjective:public ObjectiveFunction{
public:
    double objective(int scenario_id, ScenarioResultsPool* properties);
    double objective(std::list<MetaProperty>& props);
    std::string getName();
    EDPObjective(std::string unitN):ObjectiveFunction(unitN){};
};

class NormalizedEDPObjective:public ObjectiveFunction{
public:
    double objective(int scenario_id, ScenarioResultsPool* properties);
    double objective(std::list<MetaProperty>& props);
    std::string getName();
    NormalizedEDPObjective(std::string unitN):ObjectiveFunction(unitN){};
};

class CPUEnergyObjective:public ObjectiveFunction{
public:
    double objective(int scenario_id, ScenarioResultsPool* properties);
    double objective(std::list<MetaProperty>& props);
    std::string getName();
    CPUEnergyObjective(std::string unitN):ObjectiveFunction(unitN){};
};

class NormalizedCPUEnergyObjective:public ObjectiveFunction{
public:
    double objective(int scenario_id, ScenarioResultsPool* properties);
    double objective(std::list<MetaProperty>& props);
    std::string getName();
    NormalizedCPUEnergyObjective(std::string unitN):ObjectiveFunction(unitN){};
};

class TimeObjective:public ObjectiveFunction{
public:
    double objective(int scenario_id, ScenarioResultsPool* properties);
    double objective(std::list<MetaProperty>& props);
    std::string getName();
    TimeObjective(std::string unitN):ObjectiveFunction(unitN){};
};

class NormalizedTimeObjective:public ObjectiveFunction{
public:
    double objective(int scenario_id, ScenarioResultsPool* properties);
    double objective(std::list<MetaProperty>& props);
    std::string getName();
    NormalizedTimeObjective(std::string unitN):ObjectiveFunction(unitN){};
};

class TCOObjective:public ObjectiveFunction{
    double costJoule, costCoreHour;
public:
    double objective(int scenario_id, ScenarioResultsPool* properties);
    double objective(std::list<MetaProperty>& props);
    std::string getName();
    TCOObjective(std::string unitN);
};

class NormalizedTCOObjective:public ObjectiveFunction{
    double costJoule, costCoreHour;
public:
    double objective(int scenario_id, ScenarioResultsPool* properties);
    double objective(std::list<MetaProperty>& props);
    std::string getName();
    NormalizedTCOObjective(std::string unitN);
};

class ED2PObjective:public ObjectiveFunction{
public:
    double objective(int scenario_id, ScenarioResultsPool* properties);
    double objective(std::list<MetaProperty>& props);
    std::string getName();
    ED2PObjective(std::string unitN):ObjectiveFunction(unitN){};
};

class NormalizedED2PObjective:public ObjectiveFunction{
public:
    double objective(int scenario_id, ScenarioResultsPool* properties);
    double objective(std::list<MetaProperty>& props);
    std::string getName();
    NormalizedED2PObjective(std::string unitN):ObjectiveFunction(unitN){};
};

class PTF_minObjective:public ObjectiveFunction{
public:
    double objective(int scenario_id, ScenarioResultsPool* properties);
    double objective(std::list<MetaProperty>& props);
    std::string getName();
    PTF_minObjective(std::string unitN):ObjectiveFunction(unitN){};
};

class PTF_maxObjective:public ObjectiveFunction{
public:
    double objective(int scenario_id, ScenarioResultsPool* properties);
    double objective(std::list<MetaProperty>& props);
    std::string getName();
    PTF_maxObjective(std::string unitN):ObjectiveFunction(unitN){};
};

class Inverse_speedupObjective:public ObjectiveFunction{
    std::list<MetaProperty> base_properties;
public:
    double objective(int scenario_id, ScenarioResultsPool* properties);
    double objective(std::list<MetaProperty>& props);
    std::string getName();
    Inverse_speedupObjective(std::string unitN):ObjectiveFunction(unitN){};
};
#endif
