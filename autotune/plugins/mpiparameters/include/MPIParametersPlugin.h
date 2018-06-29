/**
   @file    MPIParametersPlugin.h
   @ingroup MPIParametersPlugin
   @brief   Demo Plugin
   @author  Toni Pimenta, Eduardo César, Anna Sikora
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

/**
 @defgroup MPIParametersPlugin MPI Parameters Plugin
 @ingroup AutotunePlugins
*/

#ifndef MPIPARAMETERS_PLUGIN_H_
#define MPIPARAMETERS_PLUGIN_H_
#include "AutotunePlugin.h"
#include "ISearchAlgorithm.h"
#include <vector>
#include <map>
#include <string>
#include <iostream>

using namespace std;

enum mpiType {
	unknown, SEARCHALG, INDKEEP, GDE3POPSZ, SAMPCOUNT, AUTO_EAGER_LIMIT,
	USE_BULK_XFER, BULK_MIN_MSG_SIZE, BUFFER_MEM, EAGER_LIMIT,
	CC_SCRATCH_BUF, SINGLE_THREAD, WAIT_MODE, POLLING_INTERVAL,
	CSS_INTERRUPT, HINTS_FILTERED, MSG_ENVELOPE_BUF, RETRANSMIT_INTERVAL,
	THREAD_STACKSIZE, ACK_THRESH, IO_BUFFER_SIZE, IO_ERRLOG, REXMIT_BUF_SIZE,
	REXMIT_BUF_CNT, PE_AFFINITY, TASK_AFFINITY
};

struct TaskAffinity {
	// This is the default constructor for TaskAffinity struct
	TaskAffinity():enabled(false),cpu(false),nCpu(0),core(false),nCore(0),mcm(false){}

	bool 	enabled;
	bool 	cpu;
	int 	nCpu;
	bool 	core;
	int 	nCore;
	bool 	mcm;
	vector<int> mcmVals;
};

enum intervalOperator { SUM, MUL, POW };

enum mpiImplementationName {
	GENERIC, OPENMPI, IBM, INTEL
};

/**
 * Stores the parameter information extracted from the configuration file
 * interval: start-step-end
 * name of the MPI parameter
 * values: vector of possible values as strings
 */

struct MPIParameterTP {

	MPIParameterTP():start(0),end(0),step(0),ranged(false),op(SUM) {}

	TaskAffinity taskAffinity;
	mpiType MPIType;
	string  env_var_name;
	vector<string> values;
	long start;
	long end;
	long step;
	bool ranged;
	intervalOperator op;

};


class MPIParametersPlugin: public IPlugin {

	vector<TaskAffinity> taskAffinityVector;	///< Vector made of taskAffinity values
	vector<TuningParameter*> tuningParameters; 	///< Vector of tuning parameters
	mpiImplementationName MPIType; 				///< To differentiate MPI implementations (IBM, OpenMPI, Intel, others)
	vector<MPIParameterTP> conftps; 			///< Vector of configuration parameters parsed from the configuration file
	string exports; 							///< For those cases (others) where parameters are set through environments variables
	string restartCommand;						///< For those cases (IBM, INTEL, OpenMPI) where parameters are set in the application command line
	string appflags;
	string automatic_eager, automatic_buf_mem;	///< To store the names of the eager limit and buf mem parameters when their ranges are automatically determined by the plugin
	bool instrumented_flag;
	map< long, vector<string> > valuesMap; 		///< Maps integers to string values for some parameter

	VariantSpace variantSpace;
	SearchSpace searchSpace;

	ISearchAlgorithm* searchAlgorithm;
//	Application& app;
	Region *tunedRegion;

	void extractTuningParametersFromConfigurationFile();
	public:
	void initialize(DriverContext *context, ScenarioPoolSet *pool_set);
	bool analysisRequired(StrategyRequest** strategy);
	void startTuningStep(void);
	void createScenarios(void);
	void prepareScenarios(void);
	void defineExperiment(int numprocs, bool& analysisRequired, StrategyRequest** strategy);
	bool restartRequired(std::string& env, int& numprocs, std::string& command, bool& is_instrumented);
	bool searchFinished(void);
	void finishTuningStep(void);
	bool tuningFinished(void);
	Advice *getAdvice(void);
	void finalize(void);
	void terminate(void);

	// [TODO]
	// ===================== CHECKSUM ===================== //

	map<long,int> checkSumVector;

	bool isYetInCheckSumVector (long value) {
		return (checkSumVector.count(value)==1);
	}

	float check_sum(string str) {

		float totalSum = 0;

		for (int i = 0; i < str.length(); i++) {
			totalSum += str[i];
		}

		return totalSum;
	}
	// =================== END CHECKSUM =================== //

	const int getNumParameters() const {
	return tuningParameters.size();
	}

	const int getNumValues(int i) const {
	return (tuningParameters.at(i)->getRangeTo() - tuningParameters.at(i)->getRangeFrom()
		   + tuningParameters.at(i)->getRangeStep()) / tuningParameters.at(i)->getRangeStep();
	}

	const string getExportsString() const {
	return exports;
	}

	bool isAutoEagerLimit(vector<MPIParameterTP> &conftps) {
		MPIParameterTP aux;
		for (int i = 0; i < conftps.size(); i++) {
			aux = conftps.at(i);
			if (aux.MPIType == AUTO_EAGER_LIMIT){
				return true;
			}
		}
		return false;
	}

	void deleteEagerLimitAndBufferMem (vector<MPIParameterTP> &conftps) {
		int totalElements = conftps.size();
		for (int i = 0; i < totalElements; i++) {
			if (conftps[i].MPIType == BUFFER_MEM) {
				conftps.erase (conftps.begin() + i); // We have to erase the value in the 'i' position
			}
		}

		totalElements = conftps.size();
		for (int i = 0; i < totalElements; i++) {
			if (conftps[i].MPIType == EAGER_LIMIT) {
				conftps.erase (conftps.begin() + i);
			}
		}
	}

	void deleteConfParameter(mpiType val, map<TuningParameter*,int> &values) {

		for (map<TuningParameter*,int>::iterator it = values.begin(); it != values.end(); ++it) {
			if (it->first->getName() == "bulk_min_msg_size" && val == BULK_MIN_MSG_SIZE) {
				values.erase(it);
			} else if (it->first->getName() == "task_affinity" && val == TASK_AFFINITY) {
				values.erase(it);
			}
		}
	}

	template <typename T>

	string numberToString(T Number)
	{
		stringstream aux;
		aux << Number;
		return aux.str();
	}

	void transformTaskAffinityToStringValues(vector<TaskAffinity> &taskAffinityVector, vector<string> &values) {
		string currentValue;

		for (int i = 0; i < taskAffinityVector.size(); i++) {
			TaskAffinity curr = taskAffinityVector[i];
			//if (curr.enabled) currentValue = "enabled";
		    if (curr.cpu && curr.nCpu == 1) currentValue = "cpu";
		    else if (curr.cpu && curr.nCpu > 1) currentValue = "cpu:" + numberToString(curr.nCpu);
			else if (curr.core && curr.nCore == 1) currentValue = "core";
			else if (curr.core && curr.nCore > 1) currentValue = "core:" + numberToString(curr.nCore);
			else if (curr.mcm && curr.mcmVals.size() == 0) currentValue = "mcm";
			else if (curr.mcm && curr.mcmVals.size() >= 1) {
				currentValue = "mcm:";
				currentValue += numberToString(curr.mcmVals[0]);
				for (int j = 1; j < curr.mcmVals.size(); j++) {
					currentValue += "," + numberToString(curr.mcmVals[j]);
				}
			}
			values.push_back(currentValue);
		}
	}


};

vector<MPIParameterTP> getTuningParameters(const char *filename, mpiImplementationName &MPItype);

#endif
