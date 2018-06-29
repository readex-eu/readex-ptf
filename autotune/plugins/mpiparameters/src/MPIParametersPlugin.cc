/**
   @file    MPIParametersPlugin.cc
   @ingroup MPIParametersPlugin
   @brief   MPI Parameters Plugin (for PTF 2)
   @author  Toni Pimenta, Gertvjola Saveta, Eduardo César, Anna Sikora, Alberto Olmo
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

#include "IndividualSearch.h"
#include "RandomSearch.h"
#include "GDE3Search.h"
#include "LoadSearch.h"

#include "MPIParametersPlugin.h"
#include <sstream>

using std::list;
using std::map;


/**
* @brief Extracting Tuning Parameters from the Configuration File
* @ingroup MPIParametersPlugin
*/
void MPIParametersPlugin::extractTuningParametersFromConfigurationFile() {
  // Should get filename from env var.
  char const *param_spec_file = getenv("PSC_PARAM_SPEC_FILE");

  if (param_spec_file != NULL)
    conftps = getTuningParameters(param_spec_file, MPIType);

  else {
    conftps = getTuningParameters("./param_spec.conf", MPIType);
  }

  if (conftps.empty()) {
      psc_errmsg("Using specification file: %s\n", param_spec_file);
      psc_errmsg("No tuning parameters found in specification file (or unable to open file). Set PSC_PARAM_SPEC_FILE to correct path.\n" );
      psc_abort("Exiting.\n");
  }
}

/**
 * @brief Initializes the MPIParameters tuning plugin.
 * @ingroup MPIParametersPlugin
 *
 * The tuning space is created. Here the configuration file is loaded and the
 * corresponding tuning parameters  are generated from the specified MPI
 * parameters. Then, the search strategy is chosen and the corresponding search
 * algorithm is instantiated.
 * In case the user indicates in the configuration file that the eager limit and buffer mem
 * parameters range of values are to be automatically determined by the plugin, the names of the
 * parameters are stored in automatic_eager and automatic_buf_mem members. Their corresponding tuning
 * parameters will be determined later after calling a Periscope pre-analysis.
 *
 * @param context a pointer to a context for a plugin
 * @param pool_set a pointer to a set of pools for a plugin
 *
 */
void MPIParametersPlugin::initialize(DriverContext *context, ScenarioPoolSet *pool_set) {

  // We declare mpisearch variable so as to store the
  // value of the introduced search if necessary.
  MPIParameterTP  mpisearch;
  int ind_keep = 0;
  int gde_pop_sz = 0;
  int scount = 2;
  int taskAffinityVectorInitialized = 0;

  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: call to initialize()\n");
  this->context = context;
  this->pool_set = pool_set;

  extractTuningParametersFromConfigurationFile();

  Application& app = Application::instance();

  list<Region*> regions = app.get_regions();

  if( regions.empty() && !context->applUninstrumented() ) {
      psc_abort( "MPIParametersPlugin: No code regions are known! Exiting.\n" );
  }

  Region *pr = app.get_phase_region();

  if (pr == NULL) {
    pr = new Region();
    RegionIdent ri;

    ri.file_id = -1;
    ri.rfl = -1;
    pr->set_ident(ri);
  } else {
    cout << "Printing region (phase):" << endl;
    pr->print();
  }
  tunedRegion=pr;

  if (isAutoEagerLimit(conftps)) {
	  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: Found AUTO_EAGER_LIMIT, deleting possible EAGER_LIMIT and BUFFER_MEM options.\n");
	  automatic_eager = "eager_limit";
	  automatic_buf_mem = "buffer_mem";
	  // Elminamos de la lista los valores actuales introducidos por el usuario de EAGER_LIMIT y buffer_mem
	  deleteEagerLimitAndBufferMem(conftps);
  }

  // Check if there exists any TASK_AFFINITY parameter
  // If so, then we accumulate them in a vector.
  for (int i = 0; i < conftps.size(); i++) {
	MPIParameterTP mpitp;
	mpitp = conftps.at(i);
	if (mpitp.MPIType == TASK_AFFINITY) {
		taskAffinityVector.push_back(mpitp.taskAffinity);
		taskAffinityVectorInitialized = 1;
	}
  }

  // We use this loop in order to take each one of the parameters
  // that appeared in the configuration file.
  for (int i = 0; i < conftps.size(); i++) {

	// We use an auxiliary variable to go over all of the
	// recognised parameters.
 	MPIParameterTP mpitp;
	mpitp = conftps.at(i);

	if (mpitp.MPIType == TASK_AFFINITY && taskAffinityVectorInitialized) {
		mpitp.start = 0;
		mpitp.end = taskAffinityVector.size() - 1;
		mpitp.step = 1;
		mpitp.ranged = true;
		// We change the flag now so as not to initialize this parameter in
		// each iteration.
		taskAffinityVectorInitialized = 0;
	} else if (mpitp.MPIType == TASK_AFFINITY && !taskAffinityVectorInitialized) {
		continue; // If there are more than one TASK_AFFINITY parameters, continue.
	}

	if(mpitp.MPIType == SEARCHALG){
		mpisearch = mpitp;
		continue;
	}
	if (mpitp.MPIType == INDKEEP){
		ind_keep = atoi(mpitp.values.at(0).c_str());
		continue;
	}
	if (mpitp.MPIType == GDE3POPSZ){
		gde_pop_sz = atoi(mpitp.values.at(0).c_str());
		continue;
	}
	if (mpitp.MPIType == SAMPCOUNT){
		scount = atoi(mpitp.values.at(0).c_str());
		continue;
	}
	if (mpitp.MPIType == AUTO_EAGER_LIMIT) {
		continue;
	}

    TuningParameter* tp;

    tp = new TuningParameter();
    tp->setId(i);
    tp->setName(mpitp.env_var_name.c_str());
    tp->setPluginType(MPI);
    tp->setRuntimeActionType(TUNING_ACTION_NONE);
    tp->setRange(mpitp.start, mpitp.end, mpitp.step);
    Restriction* res = new Restriction();
    res->setRegion(pr);
    res->setRegionDefined(true);
    tp->setRestriction(res);

    tuningParameters.push_back(tp);

    if (mpitp.ranged) {
    	std::vector<string> values;
    	if (tp->getName() == "task_affinity") {
    		transformTaskAffinityToStringValues(taskAffinityVector, values);
    	}
      	valuesMap.insert(make_pair(tp->getId(), values));
    } else {
      	valuesMap.insert(make_pair(tp->getId(), mpitp.values));
    }
  }

  if  (active_dbgLevel(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins)) > 0) {
    printf("\nAll tuning parameters created\n=======================\n");
    for(int i = 0; i<tuningParameters.size(); i++) {
      string str = tuningParameters[i]->toString();
      cout << str <<  "=====================" << endl;
    }
  }

  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "obtain getSearchInstance\n");

  int major, minor;
  string name, description;
  string selected_search_string;

  if (!mpisearch.values.empty()) {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "Selecting search algorithm: %s\n", mpisearch.values.at(0).c_str());
    selected_search_string = string(mpisearch.values.at(0));
    context->loadSearchAlgorithm(selected_search_string, &major, &minor, &name, &description);
    searchAlgorithm = context->getSearchAlgorithmInstance(selected_search_string);
    if (selected_search_string.compare("individual") == 0) {
    	IndividualSearch *alg = reinterpret_cast<IndividualSearch *>(searchAlgorithm);
    	if (ind_keep) alg->setScenariosToKeep(ind_keep);
	}
    if (selected_search_string.compare("random") == 0) {
	    RandomSearch *alg = reinterpret_cast<RandomSearch *>(searchAlgorithm);
	    alg->setSampleCount(scount);
	}
    if (selected_search_string.compare("gde3") == 0) {
    	GDE3Search *alg = reinterpret_cast<GDE3Search *>(searchAlgorithm);
    	if (gde_pop_sz) alg->setPopulationSize(gde_pop_sz);
    }
    if (selected_search_string.compare("load") == 0) {
    	LoadSearch *alg = reinterpret_cast<LoadSearch *>(searchAlgorithm);
    }
  } else {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "Selecting default search algorithm: exhaustive\n");
    context->loadSearchAlgorithm("exhaustive", &major, &minor, &name, &description);
    searchAlgorithm = context->getSearchAlgorithmInstance("exhaustive");
  }

  if (searchAlgorithm != NULL) {
	  print_loaded_search( major, minor, name, description );
	  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "Before Search algorithm init\n");
	  searchAlgorithm->initialize(context, pool_set);
	  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "After Search algorithm init\n");
  } else {
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),"NULL pointer in searchAlgorithm\n");
    throw PTF_PLUGIN_ERROR(NULL_REFERENCE);
  }

  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "searchAlgorithm instance obtained\n");

}

/**
 * @brief Defines a pre-analysis strategy.
 *
 * As far as PTF 2 does not allows for pre-analysis this function has been set to return false always
 *
 * @ingroup MPIParametersPlugin
 * @return False if the user does not indicates to use the automatic eager limit range determination
 * and true otherwise.
 */
bool MPIParametersPlugin::analysisRequired(StrategyRequest** strategy) {

	// (11-04-2016) EC: By the moment the pre-analysis is switch-off because it seems that it is not working in PTF 2.
	return false;

	if ( context->applUninstrumented()) return false;

	if ( !automatic_eager.empty() ){
		list<PropertyRequest*>* reqList = new list<PropertyRequest *>;
		StrategyRequestGeneralInfo* analysisStrategyGeneralInfo = new StrategyRequestGeneralInfo;

		analysisStrategyGeneralInfo->strategy_name = "ConfigAnalysis";
		analysisStrategyGeneralInfo->pedantic = true;
		analysisStrategyGeneralInfo->delay_phases = 0;
		analysisStrategyGeneralInfo->delay_seconds = 0;
		analysisStrategyGeneralInfo->analysis_duration = 1;

		PropertyRequest *req = new PropertyRequest();
		req->addPropertyID(EAGERLIMITDEPENDENT);
        if (!context->applUninstrumented()) req->addRegion(appl->get_main_region());
       	req->addAllProcesses();

		reqList->push_back(req);

		*strategy = new StrategyRequest(reqList, analysisStrategyGeneralInfo);
		return true;
	}
	return false;
}

/**
 * @brief Operations to be done at the start of a tuning step.
 *
 * @ingroup MPIParametersPlugin
 *
 */
void MPIParametersPlugin::startTuningStep(void) {
  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: call to startTuningStep() \n");
  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: Create a VariantSpace from the tuning parameters.\n");
  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: Search prepared.\n");
}

/**
 * @brief The Created Scenario Pool (csp) is populated here.
 *
 * If the user has not indicated to automatically determine the range of values for the eager limit
 * and buffer mem parameters this method simply creates the variant space using the tuning parameters
 * created at @see initialize and makes the search algorithm to create the scenarios.
 * If, on the contrary, the user has indicated to automatically determine the range of values for
 * the eager limit and buffer mem parameters then the auto_eager_limit option then
 * the range for the eager limit is set to 1K-64K with step 1K (there is no pre-analysis available).
 * This is because PTF 2 does not supports pre-analysis.
 * Finally, once the eager limit range has been determined, the plugin uses the expression
 * limit_buf_mem = 2*limit_eager_limit for determining the limits of the buffer mem parameter and also
 * its step.
 * Once the ranges of both parameters have been determined, the corresponding tuning parameters are
 * created and added to the set of tuning parameters defined at @see initialize and the scenarios
 * created in the search algorithm.
 *
 * @ingroup MPIParametersPlugin
 */
void MPIParametersPlugin::createScenarios(void) {
	static bool pass = false;

	psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: call to createScenarios()\n");
	if (searchAlgorithm == NULL) {
		psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),"Search algorithm not instantiated\n");
		throw PTF_PLUGIN_ERROR(NULL_REFERENCE);
	}

	if( !pass ) {
		psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),"First call to Create Scenarios\n");

//		(11-04-2016) EC: The pre-analysis has been switch-off, so if the auto_eager_limit option is used, the values for
//		eager_limit and buff_mem are going to be set in the whole range of eager_limit
		if (!automatic_eager.empty()) {
			unsigned lowlim =0, uplim = 65536, step =1024;
			unsigned bufmemlow, bufmemup, bufmemstep;
			int numprocs = context->getMPINumProcs();

			bufmemlow = 2*numprocs*lowlim;
			bufmemup = 2*numprocs*uplim;
			bufmemstep = 2*numprocs*step;

			int tpid = (tuningParameters.size() >0) ? tuningParameters.back()->getId()+1 : 0;

			TuningParameter *tp = new TuningParameter();

			tp->setId(tpid);
			tp->setName(automatic_eager.c_str());
			tp->setPluginType(MPI);
			tp->setRuntimeActionType(TUNING_ACTION_NONE);
			tp->setRange(lowlim, uplim, step);
			Restriction* res = new Restriction();
			res->setRegion(tunedRegion);
			res->setRegionDefined(true);
			tp->setRestriction(res);

			tuningParameters.push_back(tp);

			std::vector<string> values;
			valuesMap.insert(make_pair(tp->getId(), values));

			tp = new TuningParameter();
			tp->setId(tpid+1);
			tp->setName(automatic_buf_mem.c_str());
			tp->setPluginType(MPI);
			tp->setRuntimeActionType(TUNING_ACTION_NONE);
			tp->setRange(bufmemlow, bufmemup, bufmemstep);
			res = new Restriction();
			res->setRegion(tunedRegion);
			res->setRegionDefined(true);
			tp->setRestriction(res);

			tuningParameters.push_back(tp);

			valuesMap.insert(make_pair(tp->getId(), values));
		}
//		if (!automatic_eager.empty()) {
//			int mpi_msg_p2p_tot, mpi_msg_p2p_thr;
//			int mpi_msg_freq_2k,mpi_msg_freq_4k, mpi_msg_freq_8k;
//			int mpi_msg_freq_16k, mpi_msg_freq_32k, mpi_msg_freq_64k;
//			int tot_msg_thr;
//
//			map<int, list<MetaProperty> > preAnalysisPropertiesMap;
//			map<int, list<MetaProperty> >::const_iterator propertyMapIter;
//			list<MetaProperty> properties;
//			list<MetaProperty>::iterator property;
//			list<Region*> regions;
//			list<Region*>::iterator reg;
//
//			preAnalysisPropertiesMap = pool_set->arp->getAllPreAnalysisProperties();
//
//			for (propertyMapIter = preAnalysisPropertiesMap.begin(); propertyMapIter != preAnalysisPropertiesMap.end(); propertyMapIter++) {
//				psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParameters Plugin: Getting %d properties from the PreAnalysisPropertiesMap \n",  preAnalysisPropertiesMap.size());
//				properties = propertyMapIter->second;
//				mpi_msg_p2p_tot = mpi_msg_p2p_thr = mpi_msg_freq_2k = mpi_msg_freq_4k = mpi_msg_freq_8k = mpi_msg_freq_16k = mpi_msg_freq_32k = mpi_msg_freq_64k = 0;
//				tot_msg_thr = 0;
//
//				for (property = properties.begin();property != properties.end(); property++) {
//					if (atoi(property->getId().c_str()) == EAGERLIMITDEPENDENT) {
//						mpi_msg_p2p_tot += atoi(property->getExtraInfo().at("mpi_msg_p2p_tot").c_str());
//						mpi_msg_p2p_thr += atoi(property->getExtraInfo().at("mpi_msg_p2p_thr").c_str());
//						mpi_msg_freq_2k += atoi(property->getExtraInfo().at("mpi_msg_freq_2k").c_str());
//						mpi_msg_freq_4k += atoi(property->getExtraInfo().at("mpi_msg_freq_4k").c_str());
//						mpi_msg_freq_8k += atoi(property->getExtraInfo().at("mpi_msg_freq_8k").c_str());
//						mpi_msg_freq_16k += atoi(property->getExtraInfo().at("mpi_msg_freq_16k").c_str());
//						mpi_msg_freq_32k += atoi(property->getExtraInfo().at("mpi_msg_freq_32k").c_str());
//						mpi_msg_freq_64k += atoi(property->getExtraInfo().at("mpi_msg_freq_64k").c_str());
//					}
//				}
//
//				tot_msg_thr = mpi_msg_freq_2k + mpi_msg_freq_4k + mpi_msg_freq_8k + mpi_msg_freq_16k + mpi_msg_freq_32k + mpi_msg_freq_64k;
//
//				if ((mpi_msg_p2p_tot > 0) && (mpi_msg_p2p_thr >= 0.3*mpi_msg_p2p_tot)){
//					//Tuning the eager limit is worthy because volume of bytes communicated within the eager limit range represent 30 or more of the total
//					unsigned lowlim = 1024, uplim = 65536, step = 1024;
//					float prop2k = (1.0*mpi_msg_freq_2k)/tot_msg_thr;
//					float prop4k = (1.0*mpi_msg_freq_4k)/tot_msg_thr;
//					float prop8k = (1.0*mpi_msg_freq_8k)/tot_msg_thr;
//					float prop16k = (1.0*mpi_msg_freq_16k)/tot_msg_thr;
//					float prop32k = (1.0*mpi_msg_freq_32k)/tot_msg_thr;
//					float prop64k = (1.0*mpi_msg_freq_64k)/tot_msg_thr;
//
//					if (prop2k > 0.5){
//						uplim = 2048;
//					}else if (prop4k > 0.5){
//						lowlim = 2048; uplim = 4096;
//					}else if (prop8k > 0.5){
//						lowlim = 4096; uplim = 8192;
//					}else if (prop16k > 0.5){
//						lowlim = 8192; uplim = 16384;
//					}else if (prop32k > 0.5){
//						lowlim = 16384; uplim = 32768;
//					}else if (prop64k > 0.5){
//						lowlim = 32768;
//					}
//
//					if ((prop2k + prop4k) > 0.6){
//						uplim = 4096;
//					}
//					if ((prop4k + prop8k) > 0.6){
//						lowlim = 2048; uplim = 8192;
//					}
//					if ((prop8k + prop16k) > 0.6){
//						lowlim = 4096; uplim = 16384;
//					}
//					if ((prop16k + prop32k) > 0.6){
//						lowlim = 8192; uplim = 32768;
//					}
//					if ((prop32k + prop64k) > 0.6){
//						lowlim = 16384; uplim = 65536;
//					}
//
//					if ((prop2k + prop4k + prop8k) > 0.8){
//						lowlim = 1024; uplim = 8192;
//					}
//					if ((prop4k + prop8k + prop16k) > 0.8){
//						lowlim = 2048; uplim = 16384;
//					}
//					if ((prop8k + prop16k + prop32k) > 0.8){
//						lowlim = 4096; uplim = 32768;
//					}
//					if ((prop16k + prop32k + prop64k) > 0.8){
//						lowlim = 8192; uplim = 65536;
//					}
//
//					if ((prop2k + prop4k + prop8k + prop16k) > 0.9){
//						lowlim = 1024; uplim = 16384;
//					}
//					if ((prop4k + prop8k + prop16k + prop32k) > 0.9){
//						lowlim = 2048; uplim = 32768;
//					}
//					if ((prop8k + prop16k + prop32k + prop64k) > 0.9){
//						lowlim = 4096; uplim = 65536;
//					}
//
//					if ((prop2k + prop4k + prop8k + prop16k + prop32k) > 0.9){
//						lowlim =1024; uplim = 32768;
//					}
//					if ((prop4k + prop8k + prop16k + prop32k + prop64k)>0.9){
//						lowlim =2048; uplim = 65536;
//					}
//
//					unsigned bufmemlow, bufmemup, bufmemstep;
//
//					int numprocs = context->getMPINumProcs();
//
//					bufmemlow = 2*numprocs*lowlim;
//					bufmemup = 2*numprocs*uplim;
//					bufmemstep = 2*numprocs*step;
//
//					int tpid = (tuningParameters.size() >0) ? tuningParameters.back()->getId()+1 : 0;
//
//					TuningParameter *tp = new TuningParameter();
//
//					tp->setId(tpid);
//					tp->setName(automatic_eager.c_str());
//					tp->setPluginType(MPI);
//					tp->setRuntimeActionType(TUNING_ACTION_NONE);
//					tp->setRange(lowlim, uplim, step);
//					Restriction* res = new Restriction();
//					res->setRegion(tunedRegion);
//					res->setRegionDefined(true);
//					tp->setRestriction(res);
//
//					tuningParameters.push_back(tp);
//
//					std::vector<string> values;
//					valuesMap.insert(make_pair(tp->getId(), values));
//
//					tp = new TuningParameter();
//					tp->setId(tpid+1);
//					tp->setName(automatic_buf_mem.c_str());
//					tp->setPluginType(MPI);
//					tp->setRuntimeActionType(TUNING_ACTION_NONE);
//					tp->setRange(bufmemlow, bufmemup, bufmemstep);
//					res = new Restriction();
//					res->setRegion(tunedRegion);
//					res->setRegionDefined(true);
//					tp->setRestriction(res);
//
//					tuningParameters.push_back(tp);
//
//					valuesMap.insert(make_pair(tp->getId(), values));
//
//					if (active_dbgLevel(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins)) > 0) {
//						printf("\nAll tuning parameters created\n=======================\n");
//						for (int i = 0; i < tuningParameters.size(); i++) {
//							string str = tuningParameters[i]->toString();
//							cout << str <<  "=====================" << endl;
//						}
//					}
//				}
//			}
//		}

		for (int i = 0; i<tuningParameters.size(); i++) {
			variantSpace.addTuningParameter(tuningParameters[i]);
		}

		psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: Create a SearchSpace from %d tuning parameters.\n", tuningParameters.size());
		searchSpace.setVariantSpace(&variantSpace);

		searchSpace.addRegion(tunedRegion);

		psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: Add SearchSpace to Search Algorithm.\n");
		searchAlgorithm->addSearchSpace(&searchSpace);
		pass = true;
		psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: Search Algorithm creating scenarios.\n");
		searchAlgorithm->createScenarios();

	} else {
		psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),"Another call to Create Scenarios\n");
		searchAlgorithm->createScenarios();
	}
	psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),"MPIParametersPlugin: Scenarios created\n");
}

/**
 * @brief Preparatory steps for the scenarios are done here.
 *
 * This method builds the set of flags that will be used for passing to the application
 * the values corresponding to a given scenario (obtained from the csp). In the case of the
 * supported MPI flavors (IBM MPI, OpenMPI, or Intel MPI) the parameters are passed in the
 * application command line using appflags, while, in any other case, they are set through
 * environment variables using exports.
 * For the case of IBM MPI, given that these parameters are specifically parsed, the preparation
 * of scenarios has been optimized:
 * - It is checked whether use_bulk_xfer or pe_affinity are present and if set to no, the associated
 * parameters min_bulk_msg_size and task_affinity are eliminated of the scenario
 * - The checksum of the resulting command line is computed
 * - If the command has not been previously executed the checksumm is stored and the scenario is prepared.
 * - If the command has been previously executed we look for results obtained in the SRP, then we clone the
 * corresponding metaproperty (BTW. It would be nice to have a clone method in this class), and change the scenario id
 * in the ExtraInfo field (the cloned MetaProperty will have the current scenario id in this field)
 * - The cloned MetaProperty is inserted in the SRP and the scenario is inserted in the FSP. In this way
 * we are avoiding the execution of one experiment (which would be identical to one that has already been executed).
 * - The only case in which an experiment will be repeated is when it is the last scenario in the CSP because
 * in this case we have to go all PTF remaining steps to avoid crashing the tool.
 * Finally, a new scenario is pushed into the prepared scenario pool (psp).
 *
 * @ingroup MPIParametersPlugin
 */
void MPIParametersPlugin::prepareScenarios(void) {
  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: call to prepareScenarios()\n");
  int auxVal;
  bool dontPushScenario = true;

  if (!pool_set->csp->empty()) {
    Scenario* scenario;
    ostringstream command;
    scenario = pool_set->csp->pop();
    const list<TuningSpecification*> *ts = scenario->getTuningSpecifications();
    map<TuningParameter*,int> values = ts->front()->getVariant()->getValue();

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: recovered scenario from CSP and values [%d]\n", values.size());
    appflags.clear();
    exports.clear();

    switch (MPIType) {
      case IBM:
    	  while( dontPushScenario ){
				for (map<TuningParameter*,int>::const_iterator it = values.begin(); it != values.end(); ++it) {
					vector<string> current_values = valuesMap.find(it->first->getId())->second;
					string key = it->first->getName();
					mpiType confParameter;
					 if (current_values.size() != 0) {
						 if (current_values.at(it->second) == "no") {
							 // In this section we delete all the values that
							 // have already been marked with a 'no'
							 if (key == "use_bulk_xfer") {
								 confParameter = BULK_MIN_MSG_SIZE;
								 deleteConfParameter(confParameter, values);
							 } else if (key == "pe_affinity") {
								 confParameter = TASK_AFFINITY;
								 deleteConfParameter(confParameter, values);
							}
						}

					 }
				}


				for (map<TuningParameter*,int>::const_iterator it = values.begin(); it != values.end(); ++it) {
				  vector<string> current_values = valuesMap.find(it->first->getId())->second;

				  if (current_values.size() == 0) {
					  command << "-" << it->first->getName() << " " << it->second << " ";
				  } else {
					  command << "-" << it->first->getName() << " " << current_values.at(it->second) << " ";
				  }
				}

			  auxVal = check_sum (command.str());

			  if (isYetInCheckSumVector(auxVal)) {
				 if (pool_set->csp->empty()) break; //si no hay más scenarios creados, tenemos que
													//ejecutarlo aunque esté repetido, por eso no
													//engañamos al srp y el fsp
				/*
				 * 1.- Crear Metaproperty nueva
				 */
				 MetaProperty *newMetaProp = new MetaProperty();
				 /*
				 * 2.- Obtener Metaproperty del scenario ya ejecutado (value correspondiente al key auxval) del
				 * 		srp (scenario results pool)
				 *  Con esto conseguimos el value correspondiente a la key auxVal
				 *  */
				 MetaProperty oldMetaProp = pool_set->srp->getScenarioResultsByID(checkSumVector.find(auxVal)->second).front();
				 /*
				 * 3.- En la nueva Metaproperty copiar todos los elementos de la obtenida (hay métodos para
				 * 		obtener todos los campos en (MetaPropoerty.h Periscope/util)
				 */
				 newMetaProp->setName(oldMetaProp.getName());
				 newMetaProp->setCluster(oldMetaProp.getCluster());
				 newMetaProp->setId(oldMetaProp.getId());
				 newMetaProp->setFileId(oldMetaProp.getFileId());
				 newMetaProp->setFileName(oldMetaProp.getFileName());
				 newMetaProp->setStartPosition(oldMetaProp.getStartPosition());
				 newMetaProp->setConfiguration(oldMetaProp.getConfiguration());
				 if(!context->applUninstrumented()) newMetaProp->setThread(oldMetaProp.getThread());
				 newMetaProp->setRegionType(oldMetaProp.getRegionType());
				 newMetaProp->setRegionId(oldMetaProp.getRegionId());
				 newMetaProp->setSeverity(oldMetaProp.getSeverity());
				 newMetaProp->setConfidence(oldMetaProp.getConfidence());
				 newMetaProp->setPurpose(oldMetaProp.getPurpose());
				 newMetaProp->addExecObjs(oldMetaProp.getExecObjs());

				 /*
				  *
				  * 4.- Uno de los campos es addInfo (se obtiene llamando al método getExtraInfo). Este es un map.
				  * 		Este map debemos recorrerlo insertando cada par llamando a addExtraInfo siempre que el key
				  * 		no sea "ScenarioId". En caso de que el key sea "ScenarioId" entonces como valor pondremos
				  * 		el scenario id del scenario que nos saltamos (scenario->getId()).*/

				 map<std::string, std::string> newExtraInfoType;
				 newExtraInfoType = oldMetaProp.getExtraInfo();

				 newExtraInfoType.at("ScenarioID") = numberToString(scenario->getID());
				 for(map<string,string>::const_iterator it = newExtraInfoType.begin(); it != newExtraInfoType.end(); ++it)
					 newMetaProp->addExtraInfo(it->first, it->second);

				 /* 5.- Insertar la metaproperty creada en el srp.
				 */
				 pool_set->srp->push(*newMetaProp, 0);
				//6.- Insertar scenario en el fsp
				 pool_set->fsp->push(scenario);

				 scenario = pool_set->csp->pop();
				 ts = scenario->getTuningSpecifications();
				 values = ts->front()->getVariant()->getValue();
				 command.str(std::string());
				 command.clear();
			  } else {
				  checkSumVector.insert(std::pair<int,int>(auxVal,scenario->getID()));
				  dontPushScenario = false;
			  }
			}
          appflags.append(command.str());
          command.str(std::string());
          command.clear();
        break;
      case OPENMPI:
        for (map<TuningParameter*,int>::const_iterator it = values.begin(); it != values.end(); ++it) {
          vector<string> current_values = valuesMap.find(it->first->getId())->second;
          if (current_values.size() == 0) {
            command << "--mca " << it->first->getName() << " " << it->second <<" ";
            appflags.append(command.str());
          } else {
            command << "--mca " << it->first->getName() << " " << current_values.at(it->second) <<" ";
            appflags.append(command.str());
          }
          command.str(std::string());
          command.clear();
        }
        break;
      case INTEL:
        for (map<TuningParameter*,int>::const_iterator it = values.begin(); it != values.end(); ++it) {
          vector<string> current_values = valuesMap.find(it->first->getId())->second;
          if (current_values.size() == 0) {
            command << "-genv " << it->first->getName() << " " << it->second <<" ";
            appflags.append(command.str());
          } else {
            command << "-genv " << it->first->getName() << " " << current_values.at(it->second) <<" ";
            appflags.append(command.str());
          }
          command.str(std::string());
          command.clear();
        }
        break;

      default:
        for (map<TuningParameter*,int>::const_iterator it = values.begin(); it != values.end(); ++it) {
          vector<string> current_values = valuesMap.find(it->first->getId())->second;
          if (current_values.size() == 0) {
            command << "export " << it->first->getName() << "=" << it->second <<"; ";
            exports.append(command.str());
          } else {
            command << "export " << it->first->getName() << "=" << current_values.at(it->second) <<"; ";
            exports.append(command.str());
          }
          command.str(std::string());
          command.clear();
        }
        break;
    }

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: export environment string = %s\n", appflags.c_str());

    pool_set->psp->push(scenario);
  }
}

/**
 * @brief Populate the Experiment Scenario Pool (esp) for the next experiment.
 *
 * This is the final step before the experiments are executed. Scenarios are simply moved from the
 * psp to the esp.
 *
 * @ingroup MPIParametersPlugin
 * @param numprocs Not used.
 * @param analysisRequired Set to false.
 * @param strategy No analysis strategy requested.

 */
void  MPIParametersPlugin::defineExperiment(int numprocs, bool& analysisRequired, StrategyRequest** strategy) {
  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: call to defineExperiment()\n");

  if (!pool_set->psp->empty()) {
    Scenario* scenario;
    scenario = pool_set->psp->pop();
    const list<TuningSpecification*> *ts = scenario->getTuningSpecifications();
    if (ts->size() != 1) {
      perror("Can't currently handle multiple tuning specs\n");
      throw 0;
    }

    if (!context->applUninstrumented()) {
		list<unsigned int> *ranks= new list<unsigned int>;
		list<PropertyRequest*> *pr_in = new list<PropertyRequest*>();
		list<int> *prop_list = new list<int>;
		ranks->push_back(0);
		prop_list->push_back(EXECTIME);
		pr_in->push_back(new PropertyRequest(prop_list, ranks));
		scenario->setPropertyRequests(pr_in);
		scenario->setTunedRegion(tunedRegion);
    } else
    	scenario->setSingleTunedRegionWithPropertyRank(NULL, EXECTIME, 0);


    pool_set->esp->push(scenario);
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: Added 1 scenario to the experiment.\n");
  }
}

/**
 * @brief Return true if a restart of the application is required for the next experiment,
 * false otherwise.
 *
 *
 *
 * @ingroup MPIParametersPlugin
 * @param env Used for setting the MPI parameters through environment variables when
 * unsupported flavours are used.
 * @param numprocs not changed (the number of processes is not changed)
 * @param command for indicating the application command line invocation using the flags build at
 * @see prepareScenarios (for the supported MPI implementations)
 * @param is_instrumented
 *
 */
bool MPIParametersPlugin::restartRequired(std::string& env, int& numprocs, std::string& command, bool& is_instrumented) {
  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: call to restartRequired()\n");

  if (restartCommand.empty()) restartCommand.assign(command);
  cout << " ====================================================================== " << endl;
  cout << " Restart command: " << restartCommand << endl;
  cout << " App flags: " << appflags << endl;
  cout << " ====================================================================== " << endl;
  switch (MPIType) {
    case IBM:
      command.assign(restartCommand);
      command.append(" ");
      command.append(appflags);
      break;
    case OPENMPI:
		  command.assign(appflags);
		  command.append(" ");
		  command.append(restartCommand);

      break;
    case INTEL:
//      [TODO]: is this incorrect?
		  command.assign(appflags);
		  command.append(" ");
		  command.append(restartCommand);
      break;
    default:
      env.assign(exports);
      break;
  }
  appflags.clear();
  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: outgoing env: %s\n", env.c_str());
  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: outgoing command: %s\n", command.c_str());
  is_instrumented = !context->applUninstrumented();
  return true;
}

/**
 * @brief Returns whether the search algorithm is finished.
 * Return true if if the current search iteration is finished (by either the plugin itself or as
 * a result of the search algorithm), false otherwise.
 *
 * @ingroup MPIParameters Plugin
 */
bool MPIParametersPlugin::searchFinished(void) {
  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: call to searchFinished()\n");

  return searchAlgorithm->searchFinished();
}

/**
 * @brief Final operation of a tuning step.
 *
 * If any post-processing is necessary before entering the next tuning iteration, it is to be
 * done here.
 *
 * @ingroup MPIParametersPlugin
 */
void MPIParametersPlugin::finishTuningStep(void) {
  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: call to processResults()\n");

}

/**
 * @brief Returns true if the plugin finished the tuning process, false otherwise.
 * @ingroup MPIParametersPlugin
 * @return True
 */
bool MPIParametersPlugin::tuningFinished(void) {
  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: call to tuningFinished()\n");

  return true;
}

/**
 * @brief Prints to the screen (and to a file, where necessary) the tuning advice.
 *
 * Prints the best combination of the flags, as well as the results for all the tested combinations.
 *
 * @ingroup MPIParametersPlugin
 */
Advice *MPIParametersPlugin::getAdvice(void) {
  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: call to getAdvice()\n");
  if (searchAlgorithm == NULL) {
    throw PTF_PLUGIN_ERROR(NULL_REFERENCE);
  }

// Storing scenario execution time into the file
/******************************************************************************/
  ofstream outfile;
  std::stringstream filename;
  const char* separator = "";
//  filename << "results_MPIParameters" << (int) getpid() << ".csv";
//  filename << "results_MPIParameters.csv";
  outfile.open("results_MPIParameters.csv", ios_base::app);
/******************************************************************************/

  cout << "\n\nFound best scenario: " << searchAlgorithm->getOptimum() << endl;
  cout << "Parameter combination: \n" << endl;

  const list<TuningSpecification*> *ts = pool_set->fsp->getTuningSpecificationByScenarioID(
      searchAlgorithm->getOptimum());
  map<TuningParameter*, int> values = ts->front()->getVariant()->getValue();
  string appflags, exports;
  ostringstream command;

  switch (MPIType) {
  case IBM:
// ======================== This can be inserted into a function ===================================
	for (map<TuningParameter*,int>::const_iterator it = values.begin(); it != values.end(); ++it) {
		vector<string> current_values = valuesMap.find(it->first->getId())->second;
		string key = it->first->getName();
		mpiType confParameter;
		 if (current_values.size() != 0) {
			 //cout << current_values.at(it->second) << endl;
			 if (current_values.at(it->second) == "no") {
				 // In this section we delete all the values that
				 // have already been marked with a 'no'
				 if (key == "use_bulk_xfer") {
					 confParameter = BULK_MIN_MSG_SIZE;
					 deleteConfParameter(confParameter, values);
				 } else if (key == "pe_affinity") {
					 confParameter = TASK_AFFINITY;
					 deleteConfParameter(confParameter, values);
				}
			 }

		}
	}
	// ==================================================================================================
    for (map<TuningParameter*, int>::const_iterator it = values.begin(); it != values.end(); ++it) {
      vector<string> current_values = valuesMap.find(it->first->getId())->second;
      if (current_values.size() == 0) {
        command << "-" << it->first->getName() << " " << it->second << " ";
        appflags.append(command.str());
      } else {
        command << "-" << it->first->getName() << " " << current_values.at(it->second) << " ";
        appflags.append(command.str());
      }
      command.str(std::string());
      command.clear();
    }
    cout << appflags;
    break;
  case OPENMPI:
    for (map<TuningParameter*, int>::const_iterator it = values.begin(); it != values.end(); ++it) {
      vector<string> current_values = valuesMap.find(it->first->getId())->second;
      if (current_values.size() == 0) {
        command << "--mca " << it->first->getName() << " " << it->second << " ";
        appflags.append(command.str());
      } else {
        command << "--mca " << it->first->getName() << " " << current_values.at(it->second) << " ";
        appflags.append(command.str());
      }
      command.str(std::string());
      command.clear();
    }
    cout << appflags;
    break;
  case INTEL:
    for (map<TuningParameter*, int>::const_iterator it = values.begin(); it != values.end(); ++it) {
      vector<string> current_values = valuesMap.find(it->first->getId())->second;
      if (current_values.size() == 0) {
        command << "-genv " << it->first->getName() << " " << it->second << " ";
        appflags.append(command.str());
      } else {
        command << "-genv " << it->first->getName() << " " << current_values.at(it->second) << " ";
        appflags.append(command.str());
      }
      command.str(std::string());
      command.clear();
    }
    cout << appflags;
  default:
    for (map<TuningParameter*, int>::const_iterator it = values.begin(); it != values.end(); ++it) {
      vector<string> current_values = valuesMap.find(it->first->getId())->second;
      if (current_values.size() == 0) {
        command << "export " << it->first->getName() << "=" << it->second << "; ";
        exports.append(command.str());
      } else {
        command << "export " << it->first->getName() << "=" << current_values.at(it->second) << "; ";
        exports.append(command.str());
      }
      command.str(std::string());
      command.clear();
    }
    cout << exports;
    break;
  }
  appflags.clear();

  cout << "\n\nAll Results:\n";
  cout << "Scenario  |   Runtime    | Flags\n";
  for (int scenario_id = 0; scenario_id < pool_set->fsp->size(); scenario_id++) {
    if (!pool_set->srp->getScenarioResultsByID(scenario_id).empty()) {
      printf("%-10d|", scenario_id);
      list<MetaProperty> properties = pool_set->srp->getScenarioResultsByID(scenario_id);
      for (std::list<MetaProperty>::iterator iterator = properties.begin(), end = properties.end(); iterator != end;
          ++iterator) {
        printf("\t%f | ", iterator->getSeverity());
        outfile << separator << iterator->getSeverity();
        separator = "; ";
      }

      const list<TuningSpecification*> *ts = pool_set->fsp->getTuningSpecificationByScenarioID(scenario_id);
      map<TuningParameter*, int> values = ts->front()->getVariant()->getValue();

      switch (MPIType) {
      case IBM:
  		// ======================== This can be inserted into a function ===================================
  		for (map<TuningParameter*,int>::const_iterator it = values.begin(); it != values.end(); ++it) {
  			vector<string> current_values = valuesMap.find(it->first->getId())->second;
  				string key = it->first->getName();
  			mpiType confParameter;
  			   if (current_values.size() != 0) {
  				 if (current_values.at(it->second) == "no") {

  					 if (key == "use_bulk_xfer") {
  						 confParameter = BULK_MIN_MSG_SIZE;
  						 deleteConfParameter(confParameter, values);
  					 } else if (key == "pe_affinity") {
  						 confParameter = TASK_AFFINITY;
  						 deleteConfParameter(confParameter, values);
  					 }
  				 }

  			  }
  		}
  		// ==================================================================================================
        for (map<TuningParameter*, int>::const_iterator it = values.begin(); it != values.end(); ++it) {
          vector<string> current_values = valuesMap.find(it->first->getId())->second;
          if (current_values.size() == 0) {
        	  // [TODO]: =================================================================
            command << "-" << it->first->getName() << " " << it->second << " ";
            appflags.append(command.str());
          } else {
        	  // [TODO]: =================================================================
            command << "-" << it->first->getName() << " " << current_values.at(it->second) << " ";
            appflags.append(command.str());
          }
          command.str(std::string());
          command.clear();
        }
        cout << appflags;
        break;
      case OPENMPI:

        for (map<TuningParameter*, int>::const_iterator it = values.begin(); it != values.end(); ++it) {
          vector<string> current_values = valuesMap.find(it->first->getId())->second;
          if (current_values.size() == 0) {
            command << "--mca " << it->first->getName() << " " << it->second << " ";
            appflags.append(command.str());
          } else {
            command << "--mca " << it->first->getName() << " " << current_values.at(it->second) << " ";
            appflags.append(command.str());
          }
          command.str(std::string());
          command.clear();
        }
        cout << appflags;
        break;
      case INTEL:
        for (map<TuningParameter*, int>::const_iterator it = values.begin(); it != values.end(); ++it) {
          vector<string> current_values = valuesMap.find(it->first->getId())->second;
          if (current_values.size() == 0) {
            command << "-genv " << it->first->getName() << " " << it->second << " ";
            appflags.append(command.str());
          } else {
            command << "-genv " << it->first->getName() << " " << current_values.at(it->second) << " ";
            appflags.append(command.str());
          }
          command.str(std::string());
          command.clear();
        }
        cout << appflags;
        break;
      default:
        for (map<TuningParameter*, int>::const_iterator it = values.begin(); it != values.end(); ++it) {
          vector<string> current_values = valuesMap.find(it->first->getId())->second;
          if (current_values.size() == 0) {
            command << "export " << it->first->getName() << "=" << it->second << "; ";
            exports.append(command.str());
          } else {
            command << "export " << it->first->getName() << "=" << current_values.at(it->second) << "; ";
            exports.append(command.str());
          }
          command.str(std::string());
          command.clear();
        }
        cout << exports;
        break;

      }
      printf("\n");
      appflags.clear();
    }
  }

  outfile << endl;

  return new Advice(getName(), (*pool_set->fsp->getScenarios())[searchAlgorithm->getOptimum()],
                    searchAlgorithm->getSearchPath(), "Time", pool_set->fsp->getScenarios());
}

/**
 * @brief Finalizes the plugin.
 * @ingroup MPIParametersPlugin
 *
 */
void MPIParametersPlugin::finalize() {
  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: call to finalize()\n");

  terminate();
}

/**
 * @brief Terminate the plugin due to error.
 *
 * Safely remove any allocated memory, objects, file descriptors, etc. This method should
 * be able to be executed safely at any point.
 *
 * @ingroup MPIParametersPlugin
 */
void MPIParametersPlugin::terminate() {
  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: call to terminate()\n");
  if (searchAlgorithm) {
    searchAlgorithm->finalize();
    delete searchAlgorithm;
  }
  context->unloadSearchAlgorithms();
}

/**
 * The following are C functions that belong to the Plugin Management Interface, and not to the
 * IPlugin class. Currently the interface includes information and instance creation functions.
 *
 * These are defined in PluginManagement.h .
 */

/**
 * @brief Returns Plugin's instance.

 * Returns an instance of this particular plugin implementation.
 * Typically, a simple return with new is enough. For example:
 *
 * @ingroup MPIParametersPlugin
 * @return new MPIParametersPlugin();
 */
IPlugin* getPluginInstance(void) {
  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: call to getPluginInstance()\n");

  return new MPIParametersPlugin();
}

/**
 * @brief Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 * @ingroup MPIParametersPlugin
 * @return 1
 */
int getVersionMajor(void) {
  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: call to getInterfaceVersionMajor()\n");

  return MPIPARAMETERS_VERSION_MAJOR;
}

/**
 * @brief Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 * @ingroup MPIParametersPlugin
 * @return 0
 */
int getVersionMinor(void) {
  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: call to getInterfaceVersionMinor()\n");

  return MPIPARAMETERS_VERSION_MINOR;
}

/**
 * @brief Returns a string with the name of the plugin.
 * @ingroup MPIParametersPlugin
 *
 * @return A string with the name of the plugin.
 */
string getName(void) {
  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: call to getName()\n");

  return "MPI Parameter Plugin";
}

/**
 * @brief Returns a string with a short description of the plugin.
 * @ingroup MPIParametersPlugin
 *
 * @return A string with a short description of the plugin.
 */
string getShortSummary(void) {
  psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins), "MPIParametersPlugin: call to getShortSummary()\n");

  return "Finds optimal combination of a subset of MPI library parameters based on a goal metric (execution time)";
}
