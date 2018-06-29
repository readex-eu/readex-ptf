/**
 @file    PCAPPlugin.cc
 @ingroup PCAPPlugin
 @brief   PCAP Plugin
 @author  Umbreen Sabir
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

#include "search_common.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <map>




#ifdef CURVE_FITTING
#include <gsl/gsl_multifit.h>
#include "Brent.h"
#endif

#include "PCAPPlugin.h"

bool withParallelRegions = false;
bool withUserRegion = true;

#ifdef CURVE_FITTING
using namespace brent;
int compute_minimum( double* ptr ); //Helper function to compute minima of a function

void test_glomin_all();//Helper function for computing minima

void
test_glomin_one( double a,
        double b,
        double c,
        double m,
        double e,
        double t,
        double f( double x ),
        string title );//Helper functions

double func( double x );//Helper functions

double coeff[ 3 ] =
{   0.0, 0.0, 0.0};
#endif

static const struct option long_opts[] =
{
{ "pcap-config", required_argument, 0, 'W' }, 0 };

double Objective_Function_EDP1(int scenario_id, ScenarioResultsPool* srp)
{
    std::list<MetaProperty> properties = srp->getScenarioResultsByID(
            scenario_id);
    std::list<MetaProperty> base_properties = srp->getScenarioResultsByID(0);
    double EDP1 = 1.0;
    double base_valueEDP1 = 1.0;

    for (std::list<MetaProperty>::iterator iterator = base_properties.begin(),
            end = base_properties.end(); iterator != end; ++iterator)
    {
        base_valueEDP1 *= (iterator)->getSeverity();
    }

    for (std::list<MetaProperty>::iterator iterator = properties.begin(), end =
            properties.end(); iterator != end; ++iterator)
    {
        EDP1 *= iterator->getSeverity();
    }

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotuneSearch), "EDP1: %f;\n", EDP1);
    //return EDP1 / base_valueEDP1;
    return EDP1;
}

double Objective_Function_EDP2(int scenario_id, ScenarioResultsPool* srp)
{
    std::list<MetaProperty> properties = srp->getScenarioResultsByID(
            scenario_id);
    std::list<MetaProperty> base_properties = srp->getScenarioResultsByID(0);
    double EDP2 = 1.0;
    double base_valueEDP2 = 1.0;

    for (std::list<MetaProperty>::iterator iterator = base_properties.begin(),
            end = base_properties.end(); iterator != end; ++iterator)
    {
        base_valueEDP2 *= (iterator)->getSeverity();

        if ((iterator)->getName() == "ExecTime")
        {
            base_valueEDP2 *= (iterator)->getSeverity();
        }
    }

    for (std::list<MetaProperty>::iterator iterator = properties.begin(), end =
            properties.end(); iterator != end; ++iterator)
    {
        EDP2 *= (iterator)->getSeverity();

        if ((iterator)->getName() == "ExecTime")
        {
            EDP2 *= (iterator)->getSeverity();
        }
    }

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotuneSearch), "EDP2: %f;\n", EDP2);
    //return EDP2 / base_valueEDP2;
    return EDP2;
}

double Objective_Function_EDP3(int scenario_id, ScenarioResultsPool* srp)
{
    std::list<MetaProperty> properties = srp->getScenarioResultsByID(
            scenario_id);
    std::list<MetaProperty> base_properties = srp->getScenarioResultsByID(0);
    double EDP3 = 1.0;
    double base_valueEDP3 = 1.0;

    for (std::list<MetaProperty>::iterator iterator = base_properties.begin(),
            end = base_properties.end(); iterator != end; ++iterator)
    {
        base_valueEDP3 *= iterator->getSeverity();
        if ((iterator)->getName() == "ExecTime")
        {
            base_valueEDP3 *= (iterator)->getSeverity();
            base_valueEDP3 *= (iterator)->getSeverity();
        }
    }

    for (std::list<MetaProperty>::iterator iterator = properties.begin(), end =
            properties.end(); iterator != end; ++iterator)
    {
        EDP3 *= (iterator)->getSeverity();

        if ((iterator)->getName() == "ExecTime")
        {
            EDP3 *= (iterator)->getSeverity();
            EDP3 *= (iterator)->getSeverity();
        }
    }

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotuneSearch), "EDP3: %f;\n", EDP3);
    //return EDP3 / base_valueEDP3;
    return EDP3;
}

PCAPPlugin::PCAPPlugin() :
        app(Application::instance())
{
}

/*
 * Parse options passed to the pcap plugin
 */
void PCAPPlugin::parse_opts(int argc, char* argv[])
{
    optind = 1;
    while (optind < argc)
    {
        int index = -1;
        opterr = 0;
        int result = getopt_long(argc, argv, "", long_opts, &index);

        if (result == -1)
        {
            psc_errmsg("Error parsing command line parameters.\n");
        }

        switch (result)
        {
        case 'W':
            has_pcap_config = true;
            pcap_config_file = optarg;
            break;
        default:
            psc_errmsg(
                    "Unrecognized option passed to the compiler flags plugin.\n");
            break;
        }
    }
}

/*
 * Parse configuration file
 */
void PCAPPlugin::parseConfigFile()
{
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "PCAPPlugin: call to parseConfigFile()\n");
    ifstream inputFile(pcap_config_file.c_str());
    if (inputFile.is_open())
    {
        string line;
        while (getline(inputFile, line))
        {
            // if line starts with a comment, ignore the line
            trim(line);
            if (line.length() > 2 && line.substr(0, 2) == "//")
            {
                continue;
            }
            if (line.find('=') == std::string::npos)
            {
                continue;
            }

            boost::char_separator<char> sep("=");
            boost::tokenizer<boost::char_separator<char> > tokens(line, sep);
            int tokenNo = 0;
            string key = "", value = "";
            BOOST_FOREACH( const string &t, tokens ){
            if( tokenNo == 0 )
            {
                key = t;
            }
            else
            {
                value = t;
                break;
            }
            tokenNo++;
        }

        // now we have the key and value, try to make sense of it
            trim(key);
            trim(value);
            if (value[value.length() - 1] == ';')
            {
                value = value.substr(0, value.length() - 1);
            }

            if (key.empty() || value.empty())
            {
                continue;
            }
            // ignore comments if any
            if (value.find("//") != std::string::npos)
            {
                value = value.substr(0, value.find("//"));
            }

            if (value[0] == '\"' && value[value.length() - 1] == '\"')
            {
                if (value.length() >= 3)
                {
                    value = value.substr(1, value.length() - 2);
                } else
                {
                    continue;
                }
            }

            if (key == "search_algorithm")
            {
                // such if-else structures will make sure that invalid values are not set
                if (value == "gde3")
                {
                    searchAlgorithmName = "gde3";
                    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                            "PCAPPlugin: search algorithm to be loaded changed to gde3\n");
                } else if (value == "individual")
                {
                    searchAlgorithmName = "individual";
                    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                            "PCAPPlugin: search algorithm to be loaded changed to individual\n");
                } else if (value == "random")
                {
                    searchAlgorithmName = "random";
                    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                            "PCAPPlugin: search algorithm to be loaded changed to random\n");
                } else if (value == "exhaustive")
                {
                    searchAlgorithmName = "exhaustive";
                    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                            "PCAPPlugin: search algorithm to be loaded changed to exhaustive\n");
                } else
                {
                    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                            "PCAPPlugin: Invalid options for search algorithm. Valid options are \"exhaustive\",\"gde3\",\"individual\" & \"random\".\n");
                }
            } else if (key == "parallelRegions")
            {
                if (value == "true")
                {
                    withParallelRegions = true;
                    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                            "PCAPPlugin: parallel regions will be used for tuning\n");
                } else if (value == "false")
                {
                    withParallelRegions = false;
                    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                            "PCAPPlugin: parallel regions will NOT be used for tuning\n");
                } else
                {
                    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                            "PCAPPlugin: Invalid options for parallel regions. Valid options are \"true\" or \"false\"\n");
                }
            } else if (key == "userRegion")
            {
                if (value == "true")
                {
                    withUserRegion = true;
                    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                            "PCAPPlugin: User region will be used for tuning\n");
                } else if (value == "false")
                {
                    withUserRegion = false;
                    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                            "PCAPPlugin: User region will NOT be used for tuning\n");
                } else
                {
                    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                            "PCAPPlugin: Invalid options for user regions. Valid options are \"true\" or \"false\"\n");
                }
            } else if (key == "minRegionsToConsider")
            {
                try
                {
                    int noRegions = boost::lexical_cast<int>(value);
                    if (noRegions > 0)
                    {
                        minNoRegions = noRegions;
                        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                                "PCAPPlugin: At least %d parallel regions will be considered for tuning\n",
                                minNoRegions);
                    } else
                    {
                        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                                "PCAPPlugin: Invalid option for minRegionsToConsider. Valid option is a integer value > 0\n",
                                minNoRegions);
                    }
                } catch (const boost::bad_lexical_cast&)
                {
                    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                            "Invalid option for noRegionsToConsider\n");
                    continue;
                }
            } else if (key == "maxRegionsToConsider")
            {
                try
                {
                    int noRegions = boost::lexical_cast<int>(value);
                    if (noRegions > 0)
                    {
                        maxNoRegions = noRegions;
                        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                                "PCAPPlugin: At most %d parallel regions will be considered for tuning\n",
                                maxNoRegions);
                    } else
                    {
                        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                                "PCAPPlugin: Invalid option for maxRegionsToConsider. Valid option is a integer value > 0\n",
                                minNoRegions);
                    }
                } catch (const boost::bad_lexical_cast&)
                {
                    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                            "Invalid option for noRegionsToConsider\n");
                    continue;
                }
            } else if (key == "cutoff1")
            {
                try
                {
                    double cutoff = boost::lexical_cast<double>(value);
                    if (cutoff > 0 && cutoff < 1)
                    {
                        firstCutOff = cutoff;
                        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                                "PCAPPlugin: firstCutOff set to %.2lf.\n",
                                firstCutOff);
                        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                                "PCAPPlugin: parallel regions with exec time >= %.2lf will be considered for tuning until max limit is reached.\n",
                                firstCutOff);
                    } else
                    {
                        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                                "PCAPPlugin: Invalid value for cutoff1. Valid option is a float value between 0 and 1.\n");
                    }
                } catch (const boost::bad_lexical_cast&)
                {
                    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                            "invalid option for cutoff1\n");
                    continue;
                }
            } else if (key == "cutoff2")
            {
                try
                {
                    double cutoff = boost::lexical_cast<double>(value);
                    if (cutoff > 0 && cutoff < 1)
                    {
                        secondCutOff = cutoff;
                        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                                "PCAPPlugin: secondCutOff set to %.2lf.\n",
                                secondCutOff);
                        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                                "PCAPPlugin: parallel regions with exec time >= %.2lf will be considered depending on number of regions already considered.\n",
                                secondCutOff);
                    } else
                    {
                        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                                "PCAPPlugin: Invalid value for cutoff2. Valid option is a float value between 0 and 1.\n");
                    }
                } catch (const boost::bad_lexical_cast&)
                {
                    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                            "invalid option for cutoff2\n");
                    continue;
                }
            }
        }

        // check for no of regions consistency
        if (maxNoRegions < minNoRegions)
        {
            psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                    "PCAPPlugin: Maximum no of regions (%d) cannot be less than minimum no of regions (%d).\n",
                    maxNoRegions, minNoRegions);
            maxNoRegions = minNoRegions;
            psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                    "PCAPPlugin: Setting maximum no of regions to be equal as minimum no of regions (%d).\n",
                    minNoRegions);
        }

        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                "PCAPPlugin: completed reading the configuration file.\n");
    } else
    {
        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                "PCAPPlugin: could not open configuration file.\n");
    }
}

/*
 * Initialize the plugin's data structures.
 *
 * The tuning parameter list needs to be created.
 *
 * Search algorithms are loaded here when required. This can be done as follows:
 *
 *    searchAlgorithm = loadSearchAlgorithm("name");
 *
 * where "name" refers to one of the available search algorithms (currently only exhaustive).
 *
 */
void PCAPPlugin::initialize(DriverContext* context, ScenarioPoolSet* pool_set)
{
    perror("PCAPPlugin: call to initialize()\n");
    this->context = context;
    this->pool_set = pool_set;

    has_pcap_config = false;
    parse_opts(context->getArgc(), context->getArgv());

    number_of_cores = context->getOmpnumthreads();

    if (app.get_regions().empty())
    {
        perror("PCAPPlugin : No Region found. Exiting.\n");
        abort();
    }

    TuningParameter*         numberOfThreadsTP;
    TuningParameter*         scheduleType;
    TuningParameter*         scheduleChunkSize;

    numberOfThreadsTP = new TuningParameter();
    numberOfThreadsTP->setId(0);
    numberOfThreadsTP->setName("NUMTHREADS");
    numberOfThreadsTP->setPluginType(PCAP);

#ifdef CURVE_FITTING
    VectorRangeRestriction* r = new VectorRangeRestriction();
    r->addElement( 1 );
    r->addElement( 8 );
    r->addElement( 16 );
    r->setRegion( NULL );
    printf( "\nIncluded Curve fitting code for this executable\n" );
    numberOfThreadsTP->setVectorRangeRestriction( r );
#else
    //printf("<<<<NUMBER OF THREADS>>>> %d \n", context->getOmpnumthreads());fflush(stdout);
    numberOfThreadsTP->setRange(1, context->getOmpnumthreads(), 1);
//    Restriction* r = new Restriction();
//    for( int i = 1; i <= context->getOmpnumthreads(); i = i * 2 ) {
//        r->addElement( i );
//    }
//    r->setRegion( NULL );
//    r->setType( 2 );
//    numberOfThreadsTP->setRestriction( r );
#endif
    numberOfThreadsTP->setRuntimeActionType(TUNING_ACTION_FUNCTION_POINTER);
    tuningParameters.push_back(numberOfThreadsTP);

    scheduleType = new TuningParameter();
    scheduleType->setId(10003);
    scheduleType->setName("SCHEDULE_TYPE");
    scheduleType->setPluginType(PCAP);
    scheduleType->setRange(1, 4, 1); // 1=static, 2=Dynamic, 3=Guided, 4 = auto
    scheduleType->setRuntimeActionType(TUNING_ACTION_FUNCTION_POINTER);
//    scheduleType->setRestriction( r );
    tuningParameters.push_back(scheduleType);

    Restriction* scheduleChunkSizeRestriction = new Restriction();
    scheduleChunkSizeRestriction->addElement(0);
    scheduleChunkSizeRestriction->addElement(1);
    scheduleChunkSizeRestriction->addElement(4);
    scheduleChunkSizeRestriction->addElement(16);
    scheduleChunkSizeRestriction->addElement(64);
    scheduleChunkSizeRestriction->addElement(256);
    scheduleChunkSizeRestriction->setRegion(NULL);
    scheduleChunkSizeRestriction->setType( 2 );

    scheduleChunkSize = new TuningParameter();
    scheduleChunkSize->setId(10004);
    scheduleChunkSize->setName("SCHEDULE_CHUNK_SIZE");
    scheduleChunkSize->setPluginType(PCAP);
    scheduleChunkSize->setRestriction(scheduleChunkSizeRestriction);
    scheduleChunkSize->setRuntimeActionType(TUNING_ACTION_FUNCTION_POINTER);
    tuningParameters.push_back(scheduleChunkSize);

    string results = numberOfThreadsTP->toString();
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "PCAPPlugin: Before exiting to initialize, string is: %s\n\n",
            results.c_str());

    tuningStep = 2;

    // initializing variables to default value
    //  for preanalysis & region selection
    firstTuningStep = true;
    firstCutOff = 0.2;
    secondCutOff = 0.1;
    minNoRegions = 3;
    maxNoRegions = 3;
    searchAlgorithmName = "exhaustive";

    if (has_pcap_config)
    {
        parseConfigFile(); // might override some defaults
    }
}

/*
 * Defines a pre-analysis strategy.
 *
 * Found properties of the pre-analysis strategy are stored in the arp.
 *
 */
bool PCAPPlugin::analysisRequired(StrategyRequest** strategy)
{
    StrategyRequestGeneralInfo* analysisStrategyGeneralInfo =
            new StrategyRequestGeneralInfo;
    std::map<string, Region*>::iterator region;

    if (firstTuningStep)
    {
        return false;
    }
    analysisStrategyGeneralInfo->strategy_name = "ConfigAnalysis";
    analysisStrategyGeneralInfo->pedantic = 0;
    analysisStrategyGeneralInfo->delay_phases = 0;
    analysisStrategyGeneralInfo->delay_seconds = 0;
    analysisStrategyGeneralInfo->analysis_duration = 1;

    PropertyRequest* req = new PropertyRequest();

    req->addPropertyID(EXECTIME);
    for (region = code_region_candidates.begin();
            region != code_region_candidates.end(); region++)
    {
        req->addRegion(region->second);
    }
    req->addRegion(app.get_phase_region());
    req->addAllProcesses();

    std::list<PropertyRequest*>* reqList = new list<PropertyRequest*>;
    reqList->push_back(req);

    *strategy = new StrategyRequest(reqList, analysisStrategyGeneralInfo);
    return true;
}

/*
 * Operations to be done at the start of a tuning step.
 *
 * The tuning parameters that were created at initialization are used to create a variant space.
 * The variant space and the regions are then used to create a search space that is passed to the
 * search algorithm.
 *
 */
void PCAPPlugin::startTuningStep(void)
{
    switch (tuningStep)
    {
    case 1:
        startTuningStep1SpeedupAnalysis();
        break;
    case 2:
        startTuningStep2EnergyTuning();
        break;
    }
}

/*
 * The Created Scenario Pool (csp) is populated here.
 *
 * The scenarios need to be created and added to the first pool. To create the scenarios, a
 * search algorithm can be used or it can be done directly by the plugin.
 *
 * After this step, the Periscope will verify that scenarios were added to the csp.
 *
 */
void PCAPPlugin::createScenarios(void)
{
    switch (tuningStep)
    {
    case 1:
        createScenarios1SpeedupAnalysis();
        break;
    case 2:
        createScenarios2EnergyTuning();
        break;
    }
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "PCAPPlugin: call to createScenarios() completed\n");
}

/*
 * Preparatory steps for the scenarios are done here.
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
 */
void PCAPPlugin::prepareScenarios(void)
{
    switch (tuningStep)
    {
    case 1:
        prepareScenarios1SpeedupAnalysis();
        break;
    case 2:
        prepareScenarios2EnergyTuning();
        break;
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

void PCAPPlugin::defineExperiment(int numprocs, bool& analysisRequired,
        StrategyRequest** strategy)
{
    switch (tuningStep)
    {
    case 1:
        defineExperiment1SpeedupAnalysis(numprocs, analysisRequired, strategy);
        break;
    case 2:
        defineExperiment2EnergyTuning(numprocs, analysisRequired, strategy);
        break;
    }
}

/*
 * Return true if a restart of the application is required for the next experiment,
 * false otherwise.
 */
bool PCAPPlugin::restartRequired(std::string& env, int& numprocs,
        std::string& command, bool& is_instrumented)
{
    // *new_process_count = numberOfThreads;
    // *is_instrumented = true;
    return false; // restart not required to change the number of threads
}

/*
 * Return true if if the current search iteration is finished (by either the plugin itself or as
 * a result of the search algorithm), false otherwise.
 */
bool PCAPPlugin::searchFinished(void)
{
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "PCAPPlugin: call to searchFinished()\n");
    return searchAlgorithm->searchFinished();
}

/*
 * Final operation of a tuning step.
 *
 * If any post-processing is necessary before entering the next tuning iteration, it is to be
 * done here.
 */
void PCAPPlugin::finishTuningStep(void)
{
    switch (tuningStep)
    {
    case 1:
        finishTuningStep1SpeedupAnalysis();
        break;
    case 2:
        finishTuningStep2EnergyTuning();
        break;
    }
}

/*
 * Returns true if the plugin finished the tuning process, false otherwise.
 */
bool PCAPPlugin::tuningFinished(void)
{
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "PCAPPlugin: call to tuningFinished()\n");

    if (tuningStep > 2)
    {
        return true;
    } else
    {
        return true;
    }
}

/*
 * Prints to the screen (and to a file, where necessary) the tuning advice.
 */
Advice* PCAPPlugin::getAdvice(void)
{
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "PCAPPlugin: call to getAdvice()\n");

    if (searchAlgorithm == NULL)
    {
        throw PTF_PLUGIN_ERROR(NULL_REFERENCE);
    }
    Scenario* scenario;
    const std::list<TuningSpecification*>* tuningSpecList;
    std::list<TuningSpecification*>::const_iterator tuningSpecIter;
    TuningSpecification* tuningSpec;
    map<TuningParameter*, int> values;
    int variant;
    int Num_Scenarios = pool_set->fsp->size();
    int err;
    double EDP, EDP2, EDP3;

#ifdef CURVE_FITTING
    double coefficients[ 3 ];
    double xi = 0.0, yi = 0.0, zi = 0.0, chisq = 0.0;

    gsl_matrix* X = gsl_matrix_alloc( 3, 3 );
    gsl_vector* y = gsl_vector_alloc( 3 );
    gsl_vector* z = gsl_vector_alloc( 3 );
    gsl_vector* c = gsl_vector_alloc( 3 );
    gsl_matrix* cov = gsl_matrix_alloc( 3, 3 );
#endif

    for (int scenario_id = 0; scenario_id < Num_Scenarios; scenario_id++)
    {
        scenario = (*pool_set->fsp->getScenarios())[scenario_id];
        EDP = Objective_Function_EDP1(scenario_id, pool_set->srp);
        EDP2 = Objective_Function_EDP2(scenario_id, pool_set->srp);
        EDP3 = Objective_Function_EDP3(scenario_id, pool_set->srp);
        scenario->addResult("EDP", EDP);
        scenario->addResult("EDP2", EDP2);
        scenario->addResult("EDP3", EDP3);
        list<MetaProperty> properties = pool_set->srp->getScenarioResultsByID(
                scenario_id);
        for (std::list<MetaProperty>::iterator iterator = properties.begin(),
                end = properties.end(); iterator != end; ++iterator)
        {
//            if( atoi( iterator->getId().c_str() ) == ENERGY_CONSUMPTION ) {
            if (atoi(iterator->getId().c_str())
                    == ENERGY_CONSUMPTION)
            {
                scenario->addResult("Energy", iterator->getSeverity());
            }
            if (atoi(iterator->getId().c_str()) == EXECTIME)
            {
                scenario->addResult("Time", iterator->getSeverity());
            }
        }
    }

    writePCAPresults();

    Scenario* best_scenario =
            (*pool_set->fsp->getScenarios())[searchAlgorithm->getOptimum()];
    if (best_scenario == NULL)
    {
        psc_abort("Error: Best scenario cannot be NULL at this point.");
    }

    return new Advice(getName(), best_scenario,
            searchAlgorithm->getSearchPath(), "Time",
            pool_set->fsp->getScenarios());
}

/*
 * Prints to the screen (and to a file, where necessary) the tuning advice.
 */
void PCAPPlugin::curveFitting(void)
{
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "PCAPPlugin: call to curveFitting()\n");

    Scenario* scenario;
    const std::list<TuningSpecification*>* tuningSpecList;
    std::list<TuningSpecification*>::const_iterator tuningSpecIter;
    TuningSpecification* tuningSpec;
    map<TuningParameter*, int> values;
    int variant;
    int Num_Scenarios = pool_set->fsp->size();
    int err;

#ifdef CURVE_FITTING
    double coefficients[ 3 ];
    double xi = 0.0, yi = 0.0, zi = 0.0, chisq = 0.0;

    gsl_matrix* X = gsl_matrix_alloc( 3, 3 );
    gsl_vector* y = gsl_vector_alloc( 3 );
    gsl_vector* z = gsl_vector_alloc( 3 );
    gsl_vector* c = gsl_vector_alloc( 3 );
    gsl_matrix* cov = gsl_matrix_alloc( 3, 3 );

    for( int scenario_id = 0; scenario_id < Num_Scenarios; scenario_id++ )
    {
        scenario = ( *pool_set->fsp->getScenarios() )[ scenario_id ];
        double yi;
        if( getResults( "EDP", &yi ) )
        {
            gsl_vector_set( y, scenario_id, yi );
            gsl_vector_set( z, scenario_id, yi );
        }
        else
        {
            printf( "\nError in curve fitting. No EDP found in scenario %d\n", scenario_id );
        }
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "\nPCAPPlugin: Data points(y): (%lf)\n", yi );

        tuningSpecList = scenario->getTuningSpecifications();
        for( tuningSpecIter = tuningSpecList->begin(); tuningSpecIter != tuningSpecList->end(); tuningSpecIter++ )
        {
            tuningSpec = ( *tuningSpecIter );
            values = tuningSpec->getVariant()->getValue();
            for( map<TuningParameter*, int>::iterator i = values.begin(); i != values.end(); i++ )
            {
                variant = i->second;
                xi = variant;
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPPlugin: map<TuningParameter*, int>[2nd part]: %lf, ", xi );
            }
        }

        gsl_matrix_set( X, scenario_id, 0, 1.0 );
        gsl_matrix_set( X, scenario_id, 1, xi );
        gsl_matrix_set( X, scenario_id, 2, xi * xi );
    }

    gsl_multifit_linear_workspace* work = gsl_multifit_linear_alloc( 3, 3 );
    gsl_multifit_linear( X, y, c, cov, &chisq, work );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPPlugin: # best fit: Y = %g + %g X + %g X^2\n",
            coefficients[ 0 ] = gsl_vector_get( c, 0 ), coefficients[ 1 ] = gsl_vector_get( c, 1 ), coefficients[ 2 ] = gsl_vector_get( c, 2 ) );
    err = compute_minimum( coefficients );
    gsl_multifit_linear( X, z, c, cov, &chisq, work );
    gsl_multifit_linear_free( work );

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPPlugin: # best fit[with normalized EDP values]: Y = %g + %g X + %g X^2\n",
            coefficients[ 0 ] = gsl_vector_get( c, 0 ), coefficients[ 1 ] = gsl_vector_get( c, 1 ), coefficients[ 2 ] = gsl_vector_get( c, 2 ) );

    gsl_matrix_free( X );
    gsl_vector_free( y );
    gsl_vector_free( c );
    gsl_matrix_free( cov );

    err = compute_minimum( coefficients );

    if( err != 0 )
    {
        printf( "\nError computing minima for the fitted curve\n" );
    }
#endif
}

/*
 * Finalize the plugin normally.
 *
 * Remove any allocated memory, objects, file descriptors, etc.
 *
 */
void PCAPPlugin::finalize()
{
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "PCAPPlugin: call to finalize()\n");
    terminate();
}

/*
 * Terminate the plugin due to error.
 *
 * Safely remove any allocated memory, objects, file descriptors, etc. This method should
 * be able to be executed safely at any point.
 *
 */
void PCAPPlugin::terminate()
{
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "PCAPPlugin: call to terminate()\n");

    if (searchAlgorithm)
    {
        searchAlgorithm->finalize();
        delete searchAlgorithm;
    }
    context->unloadSearchAlgorithms();
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
 * @ingroup PCAPPlugin
 *
 * @return A pointer to an instance of this particular plugin implementation.
 */
IPlugin* getPluginInstance(void)
{
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "PCAPPlugin: call to getComponentInstance()\n");

    return new PCAPPlugin();
}

/**
 * @brief Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 *
 * @ingroup PCAPPlugin
 *
 * @return The major plugin interface version used by the plugin
 */
int getVersionMajor(void)
{
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "PCAPPlugin: call to getInterfaceVersionMajor()\n");

    return 1;
}

/**
 * @brief Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 *
 * @ingroup PCAPPlugin
 *
 * @return The minor plugin interface version used by the plugin
 *
 */
int getVersionMinor(void)
{
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "PCAPPlugin: call to getInterfaceVersionMinor()\n");

    return 0;
}

/**
 * @brief Returns a string with the name of the plugin.
 *
 * @ingroup PCAPPlugin
 *
 * @return A string with the name of the plugin
 *
 */
string getName(void)
{
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "PCAPPlugin: call to getName()\n");

    return "PCAP plugin";
}

/**
 * @brief Returns a string with a short description of the plugin.
 *
 * @ingroup PCAPPlugin
 *
 * @return A string with a short description of the plugin
 *
 */
string getShortSummary(void)
{
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "PCAPPlugin: call to getShortSummary()\n");

    return "Tunes the Energy Delay Product of OMP applications.";
}

void PCAPPlugin::startTuningStep2EnergyTuning(void)
{
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "PCAPPlugin: call to startTuningStep()\n");
    std::list<Region*> code_regions;
    code_regions = appl->get_regions();
    std::list<Region*>::iterator region;

    // iterating over all regions & adding to code_region_candidates for preanalysis
    int parallel_regions = 0, parallel_do_regions = 0;
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "Searching all available regions...\n");
    for (region = code_regions.begin(); region != code_regions.end(); region++)
    {
        //if ((*region)->get_type() == PARALLEL_REGION || (*region)->get_type() == DO_REGION) {
        /*
         * Only parallel regions should be considered for tuning, since we can not set num threads for
         * individual for loops in the same parallel section
         * */
        if ((*region)->get_type() == PARALLEL_REGION)
        {
            if ((*region)->get_type() == PARALLEL_REGION)
            {
                parallel_regions++;
                psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                        "Parallel region found:\n");
            } else
            {
                parallel_do_regions++;
                psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                        "Parallel do region found:\n");
            }
            psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                    "\tFile name: %s\n", (*region)->getFileName().c_str());
            psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                    "\tFirst line: %d\n", (*region)->getFirstLine());
            psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                    "\tLast line: %d\n", (*region)->getLastLine());
            psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                    "\tMatcher key: %s\n",
                    (*region)->getIdForPropertyMatching().c_str());
            code_region_candidates[(*region)->getIdForPropertyMatching()] =
                    *region;
        }
    }
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "PCAP PLUGIN: found %d parallel & parallel_do regions.\n",
            (parallel_regions + parallel_do_regions));

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "obtain getSearchInstance\n");
    int major, minor;
    string name, description;

    char const* selected_search = getenv("PSC_SEARCH_ALGORITHM");
    if (selected_search != NULL)
    {
        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                "User specified search algorithm: %s\n", selected_search);
        string selected_search_string = string(selected_search);
        context->loadSearchAlgorithm(selected_search_string, &major, &minor,
                &name, &description);
        searchAlgorithm = context->getSearchAlgorithmInstance(
                selected_search_string);
    } else
    {
        psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                "Selecting default search algorithm: %s\n ",
                searchAlgorithmName.c_str());
        context->loadSearchAlgorithm(searchAlgorithmName, &major, &minor, &name,
                &description);
        searchAlgorithm = context->getSearchAlgorithmInstance(
                searchAlgorithmName);

        searchAlgorithm->addObjectiveFunction(new EDPObjective(""));
    }

    if (searchAlgorithm != NULL)
    {
        print_loaded_search(major, minor, name, description);
        searchAlgorithm->initialize(context, pool_set);
    } else
    {
        perror("NULL pointer in searchAlgorithm\n");
        throw PTF_PLUGIN_ERROR(NULL_REFERENCE);
    }

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "searchAlgorithm instance obtained\n");
}
/**
 * custom compare function for sorting the pair of vectors
 **/
bool pairCompare(const std::pair<string, double> firstPair,
        const std::pair<string, double> secondPair)
{
    return firstPair.second > secondPair.second;
}

void PCAPPlugin::createScenarios2EnergyTuning(void)
{
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "PCAPPlugin: call to createScenarios()\n");

    if (searchAlgorithm == NULL)
    {
        perror("Search algorithm not instantiated\n");
        throw PTF_PLUGIN_ERROR(NULL_REFERENCE);
    }

    // while createScenarios may be called multiple times, we should add the searchspace only once
    if (firstTuningStep)
    {
        firstTuningStep = false;

        if (withParallelRegions)
        {
            // sorting and arranging prenanalysis result
            list<MetaProperty>::iterator property;
            list<MetaProperty> found_properties =
                    pool_set->arp->getPreAnalysisProperties(0);
            double severity = 0;
            double totalExecTime = 0;

            for (property = found_properties.begin();
                    property != found_properties.end(); property++)
            {
                regionExectimeMap.push_back(
                        make_pair(property->getRegionId(),
                                property->getSeverity()));
                psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                        "PCAPPlugin: Severity (%f) in region: %d added to vector\n",
                        property->getSeverity(),
                        static_cast<int>(property->getRegionType()));
                totalExecTime += property->getSeverity();
            }

            // sort the vector according to decreasing execution time
            std::sort(regionExectimeMap.begin(), regionExectimeMap.end(),
                    pairCompare);

            /*
             *  Using heuristic to find out important regions
             *  - first add regions with >= firstCutOff % (e.g. 20%) exectime unitl maxNoRegions
             *  - if no of regions are less than minNoRegions, add regions with  with
             *    exectime < firstCutoff & >=secondCutOff % (e.g. between 20% & 10%) exectime
             */

            int noRegionsAdded = 0;
            // initially collect regions that are above first cutoff
            for (int i = 0; i < regionExectimeMap.size(); i++)
            {
                double execTime = regionExectimeMap[i].second;
                if (execTime < (firstCutOff * totalExecTime))
                {
                    break;
                }

                noRegionsAdded++;
                selected_region =
                        code_region_candidates[regionExectimeMap[i].first];
                addRegionToSearchAlgorithm(selected_region,
                        regionExectimeMap[i].second);
                if (noRegionsAdded == maxNoRegions)
                {
                    break;
                }
            }

            if (noRegionsAdded < minNoRegions)
            { // utilize second cutoff
                for (int i = 0; i < regionExectimeMap.size(); i++)
                {
                    double execTime = regionExectimeMap[i].second;
                    if (execTime < (secondCutOff * totalExecTime))
                    {
                        break;
                    }

                    if (execTime >= (firstCutOff * totalExecTime))
                    {
                        continue;
                    }

                    noRegionsAdded++;
                    selected_region =
                            code_region_candidates[regionExectimeMap[i].first];
                    addRegionToSearchAlgorithm(selected_region,
                            regionExectimeMap[i].second);
                    if (noRegionsAdded == minNoRegions)
                    {
                        break;
                    }
                }
            }

            // if no regions added, add first few regions in decreasing order of their execTime
            if (noRegionsAdded == 0)
            {
                for (int i = 0; i < regionExectimeMap.size(); i++)
                {
                    noRegionsAdded++;
                    selected_region =
                            code_region_candidates[regionExectimeMap[0].first];
                    addRegionToSearchAlgorithm(selected_region,
                            regionExectimeMap[0].second);
                    if (noRegionsAdded == minNoRegions)
                    {
                        break;
                    }
                }
            }
        }

        if (!withParallelRegions || withUserRegion)
        {
            psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
                    "PCAPPlugin: Creating search space for the USER region\n");
            SearchSpace* searchspace = new SearchSpace();
            VariantSpace* variantSpace = new VariantSpace();

            for(auto& tuningParameter : tuningParameters)
            {
                variantSpace->addTuningParameter(tuningParameter);
            }

            searchspace->setVariantSpace(variantSpace);
            searchspace->addRegion(app.get_phase_region());
            searchAlgorithm->addSearchSpace(searchspace);
        }
    }

    searchAlgorithm->createScenarios();
}

void PCAPPlugin::addRegionToSearchAlgorithm(Region* selectedRegion,
        double execTime)
{
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "PCAPPlugin: selected region with: runtime: %f; ID: %s; file: %s; first line: %d; last line: %d;\n",
            execTime, selected_region->getIdForPropertyMatching().c_str(),
            selected_region->getFileName().c_str(),
            selected_region->getFirstLine(), selected_region->getLastLine());

    SearchSpace* searchspace = new SearchSpace();
    VariantSpace* variantSpace = new VariantSpace();

    for(auto& tuningParameter : tuningParameters)
    {
        variantSpace->addTuningParameter(tuningParameter);
    }

    searchspace->setVariantSpace(variantSpace);

    selected_region->print();
    printf("\n");
    searchspace->addRegion(selected_region);
    searchAlgorithm->addSearchSpace(searchspace);
}

void PCAPPlugin::prepareScenarios2EnergyTuning(void)
{
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "PCAPPlugin: call to prepareScenarios()\n");

    if (!pool_set->csp->empty())
    {
        Scenario* scenario;
        scenario = pool_set->csp->pop();
        pool_set->psp->push(scenario);
    }
}

void PCAPPlugin::defineExperiment2EnergyTuning(int numprocs,
        bool& analysisRequired, StrategyRequest** strategy)
{
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "PCAPPlugin: call to defineExperiment()\n");

    int i;
    Scenario* scenario;

    //TODO: We only have one scenario at a time. We should remove this for loop. -RM
    for (i = 0; !pool_set->psp->empty() && i < numprocs; i++)
    {
        scenario = pool_set->psp->pop();

//    const list<TuningSpecification*> *ts = scenario->getTuningSpecifications();
//    if (ts->size() != 1) {
//      perror("PCAPPlugin can't currently handle multiple tuning specs \n");
//      throw 0;
//    }
//    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
//               "setting single rank (0) in scenario with id: %d\n", 0, scenario->getID());
//    ts->front()->setSingleRank(i);

        std::list<PropertyRequest*>* propertyRequestList = new list<PropertyRequest*>();
        list<int>* propertyIds = new list<int>;
        list<unsigned int>* ranks = new list<unsigned int>;

//        propertyIds->push_back( ENERGY_CONSUMPTION );
        propertyIds->push_back(ENERGY_CONSUMPTION);
        propertyIds->push_back(EXECTIME);
        propertyRequestList->push_back(new PropertyRequest(propertyIds));
        scenario->setPropertyRequests(propertyRequestList);
        scenario->setTunedRegion(app.get_phase_region());

        pool_set->esp->push(scenario);
    }

    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "PCAPPlugin: Added %d scenario in the experiment.\n", i);
    StrategyRequestGeneralInfo* strategyRequestGeneralInfo =
            new StrategyRequestGeneralInfo;

    strategyRequestGeneralInfo->strategy_name = "OMP";
    strategyRequestGeneralInfo->pedantic = 1;
    strategyRequestGeneralInfo->delay_phases = 0;
    strategyRequestGeneralInfo->delay_seconds = 0;
    strategyRequestGeneralInfo->analysis_duration = 1;

    *strategy = NULL;
    //*strategy = new StrategyRequest(strategyRequestGeneralInfo);
}

void PCAPPlugin::finishTuningStep2EnergyTuning(void)
{
    psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotunePlugins),
            "PCAPPlugin: call to processResults()\n");

    tuningStep++;
}

#ifdef CURVE_FITTING

int compute_minimum( double* ptr )
{
    timestamp();
    cout << "\n";
    cout << "BRENT_PRB\n";
    cout << "  C++ version\n";
    cout << "  Test the BRENT library.\n";

    coeff[ 0 ] = ptr[ 0 ];
    coeff[ 1 ] = ptr[ 1 ];
    coeff[ 2 ] = ptr[ 2 ];
    cout << "\n" << coeff[ 0 ] << "\n" << coeff[ 1 ] << "\n" << coeff[ 2 ] << "\n";

    test_glomin_all();
    cout << "\n";
    cout << "BRENT_PRB\n";
    cout << "  Normal end of execution.\n";
    cout << "\n";
    timestamp();

    return 0;
}

void test_glomin_all()
{
    double a;
    double b;
    double c;
    double e;
    double m;
    double t;

    cout << "\n";
    cout << "TEST_GLOMIN_ALL\n";
    cout << "  Test the Brent GLOMIN routine, which seeks\n";
    cout << "  a global minimizer of a function F(X)\n";
    cout << "  in an interval [A,B],\n";
    cout << "  given some upper bound M \n";
    cout << "  for the second derivative of F.\n";

    e = sqrt( r8_epsilon() );
    t = sqrt( r8_epsilon() );

    a = 0.0;
    b = 16.0;
    c = ( a + b ) / 2.0;
    m = 3000.0;

    test_glomin_one( a, b, c, m, e, t, func,
            "func" );

    return;
}

void test_glomin_one( double a, double b, double c, double m,
        double e, double t, double f( double x ), string title )
{
    double fa;
    double fb;
    double fx;
    double x;

    fx = glomin( a, b, c, m, e, t, f, x );
    fa = f( a );
    fb = f( b );

    cout << "\n";
    cout << "  " << title << "\n";
    cout << "\n";
    cout << "           A                 X             B\n";
    cout << "         F(A)              F(X)          F(B)\n";
    cout << "  " << setprecision( 6 ) << setw( 14 ) << a
    << "  " << setw( 14 ) << x
    << "  " << setw( 14 ) << b << "\n";
    cout << "  " << setw( 14 ) << fa
    << "  " << setw( 14 ) << fx
    << "  " << setw( 14 ) << fb << "\n";

    return;
}

double func( double x )
{
    double value;
    value = ( pow( x, 2 ) * coeff[ 2 ] ) + ( x * coeff[ 1 ] ) + coeff[ 0 ];
    return value;
}
#endif

void PCAPPlugin::writePCAPresults()
{
    std::stringstream result_oss;
    std::stringstream result_file;
    Scenario* scenario;
    const std::list<TuningSpecification*>* tuningSpecList;
    std::list<TuningSpecification*>::const_iterator tuningSpecIter;
    TuningSpecification* tuningSpec;
    map<TuningParameter*, int> values;
    int variant;
    int Num_Scenarios = pool_set->fsp->size();
    int err;
    double EDP, EDP2, EDP3;

    result_file.precision(12);
    result_oss << "\nAutoTune Results:" << endl;
    result_oss << "-----------------------\n\n";

    result_oss << "\nAll Results:\n";
    result_oss
            << "Scenario | Region | Scheduletype | Threads \t\t"
            "| EDP\t | EDP2\t | EDP^3\t | Energy\t | Execution Time\n";
    result_file << "Scenario ID; Region; Threads; Scheduletype;"
            "ScheduleChunkSize; EDP; EDP2; EDP^3; Energy; Time\n";

    std::vector<double> execution_time;
    std::vector<double> energy;
    for (int scenario_id = 0; scenario_id < Num_Scenarios; scenario_id++)
    {
        scenario = (*pool_set->fsp->getScenarios())[scenario_id];
        result_oss << std::setw(3) << scenario_id << "    ";
        result_file << scenario_id << "; ";

        map<TuningParameter*, int>::iterator iter;
        list<TuningSpecification*>::iterator ts_iter;
        for (ts_iter = scenario->getTuningSpecifications()->begin();
                ts_iter != scenario->getTuningSpecifications()->end();
                ts_iter++)
        {
            int nthr, scheduleType, scheduleChunkSize ;
            // the variant is a list of parameters and values
            for (iter = (*ts_iter)->getVariant()->getValue().begin();
                    iter != (*ts_iter)->getVariant()->getValue().end(); iter++)
            {
//                printf("=====================================================================\n\n");
//                printf("Tuning Parameter %s has value %d\n",iter->first->getName(), iter->second);
//                printf("=====================================================================\n\n");
                if (iter->first->getName() == "NUMTHREADS")
                {
                    nthr = iter->second;
                } else if (iter->first->getName() == "SCHEDULE_TYPE")
                {
                    scheduleType = iter->second;
                } else if (iter->first->getName() == "SCHEDULE_CHUNK_SIZE")
                {
                    scheduleChunkSize = iter->second;
                }
            }

            list<string>::iterator region_iter;
            for (region_iter =
                    (*ts_iter)->getVariantContext().context_union.entity_list->begin();
                    region_iter
                            != (*ts_iter)->getVariantContext().context_union.entity_list->end();
                    region_iter++)
            {
                std::stringstream temp;
                temp << "(" << (*region_iter) << "):";
                result_oss << std::setw(24) << std::left << temp.str()
                        << std::setw(3) << std::right << nthr << "  "
                        << scheduleType << "  " << scheduleChunkSize;

                result_file << temp.str() << ";"
                        << nthr << ";"
                        << scheduleType<< ";"
                        << scheduleChunkSize;

            }
        }

        result_file << ";";

        map<string, double> results = scenario->getResults();
        map<string, double>::iterator res_iter;
        int count = 0;

        for (res_iter = results.begin(); res_iter != results.end(); res_iter++)
        {
            if (count == 3)
            {
                energy.push_back(res_iter->second);
            }
            if (count == 4)
            {
                execution_time.push_back(res_iter->second);
            }
            count++;
            result_oss << std::setw(14) << std::right << res_iter->second;
            result_file << res_iter->second << ";";
        }
        result_oss << endl;
        result_file << endl;
    }
    result_oss << endl;

    std::cout << result_oss.str();
    ofstream results_file;

    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);

    std::stringstream file_name;
    file_name << "pcap_results_date_";
    file_name << tm.tm_year + 1990 << "_" << tm.tm_mon + 1 << "_" << tm.tm_mday;
    file_name << "_time_";
    file_name << tm.tm_hour << "_" << tm.tm_min << "_" << tm.tm_sec;
    file_name << ".csv";

    results_file.open(file_name.str());
    results_file.precision(12);
    results_file << result_file.str();
    results_file.close();
}
