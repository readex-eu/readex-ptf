#include "PCAPSpeedupPlugin.h"
#include "search_common.h"
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>




static bool withParallelRegions = false;
static bool withUserRegion      = true;

/**
 * custom compare function for sorting the pair of vectors
 **/
static bool pairCompare( const std::pair<string, double> firstPair,
                         const std::pair<string, double> secondPair ) {
    return firstPair.second > secondPair.second;
}


/** Calculates the metric to be minimized. */
static double objectiveExecTime( int scenario_id, ScenarioResultsPool* srp ) {
    std::list<MetaProperty> properties = srp->getScenarioResultsByID( scenario_id );
    return properties.front().getSeverity();
}


PCAPSpeedupPlugin::PCAPSpeedupPlugin() : app( Application::instance() ) {
}


/*
 * Parse options passed to the pcap plugin
 */
void PCAPSpeedupPlugin::parse_opts( int argc, char* argv[] ) {
    const struct option long_opts[] = {
        { "pcap-config", required_argument, 0, 'W' },
        0
    };

    optind = 1;
    while( optind < argc ) {
        int index = -1;
        opterr = 0;
        int result = getopt_long( argc, argv, "", long_opts, &index );

        if( result == -1 ) {
            psc_errmsg( "Error parsing command line parameters.\n" );
        }

        switch( result ) {
        case 'W':
            has_pcap_config  = true;
            pcap_config_file = optarg;
            break;
        default:
            psc_errmsg( "Unrecognized option passed to the compiler flags plugin.\n" );
            break;
        }
    }
}


/*
 * Parse configuration file
 */
void PCAPSpeedupPlugin::parseConfigFile() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: call to parseConfigFile()\n" );
    ifstream inputFile( pcap_config_file.c_str() );
    if( inputFile.is_open() ) {
        string line;
        while( getline( inputFile, line ) ) {
            // if line starts with a comment, ignore the line
            boost::algorithm::trim( line );
            if( line.length() > 2 && line.substr( 0, 2 ) == "//" )
                continue;
            if( line.find( '=' ) == std::string::npos )
                continue;


            boost::char_separator<char>                    sep( "=" );
            boost::tokenizer<boost::char_separator<char> > tokens( line, sep );
            int                                            tokenNo = 0;
            string                                         key     = "", value = "";
            BOOST_FOREACH( const string &t, tokens ) {
                if( tokenNo == 0 ) {
                    key = t;
                }
                else {
                    value = t;
                    break;
                }
                tokenNo++;
            }


            // now we have the key and value, try to make sense of it
            boost::algorithm::trim( key );
            boost::algorithm::trim( value );
            if( value[ value.length() - 1 ] == ';' ) {
                value =  value.substr( 0, value.length() - 1 );
            }

            if( key.empty() || value.empty() ) {
                continue;
            }
            // ignore comments if any
            if( value.find( "//" ) != std::string::npos ) {
                value = value.substr( 0, value.find( "//" ) );
            }

            if( value[ 0 ] == '\"' && value[ value.length() - 1 ] == '\"' ) {
                if( value.length() >= 3 ) {
                    value = value.substr( 1, value.length() - 2 );
                }
                else {
                    continue;
                }
            }

            if( key == "search_algorithm" ) {
                // such if-else structures will make sure that invalid values are not set
                if( value == "gde3" ) {
                    searchAlgorithmName = "gde3";
                    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: search algorithm to be loaded changed to gde3\n" );
                }
                else if( value == "individual" ) {
                    searchAlgorithmName = "individual";
                    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: search algorithm to be loaded changed to individual\n" );
                }
                else if( value == "random" ) {
                    searchAlgorithmName = "random";
                    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: search algorithm to be loaded changed to random\n" );
                }
                else if( value == "exhaustive" ) {
                    searchAlgorithmName = "exhaustive";
                    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: search algorithm to be loaded changed to exhaustive\n" );
                }
                else {
                    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: Invalid options for search algorithm. Valid options are \"exhaustive\",\"gde3\",\"individual\" & \"random\".\n" );
                }
            }
            else if( key == "parallelRegions" ) {
                if( value == "true" ) {
                    withParallelRegions = true;
                    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: parallel regions will be used for tuning\n" );
                }
                else if( value == "false" ) {
                    withParallelRegions = false;
                    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: parallel regions will NOT be used for tuning\n" );
                }
                else {
                    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: Invalid options for parallel regions. Valid options are \"true\" or \"false\"\n" );
                }
            }
            else if( key == "userRegion" ) {
                if( value == "true" ) {
                    withUserRegion = true;
                    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: User region will be used for tuning\n" );
                }
                else if( value == "false" ) {
                    withUserRegion = false;
                    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: User region will NOT be used for tuning\n" );
                }
                else {
                    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: Invalid options for user regions. Valid options are \"true\" or \"false\"\n" );
                }
            }
            else if( key == "minRegionsToConsider" ) {
                try{
                    int noRegions = boost::lexical_cast<int>( value );
                    if( noRegions > 0 ) {
                        minNoRegions = noRegions;
                        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: At least %d parallel regions will be considered for tuning\n", minNoRegions );
                    }
                    else {
                        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: Invalid option for minRegionsToConsider. Valid option is a integer value > 0\n", minNoRegions );
                    }
                }
                catch( const boost::bad_lexical_cast& ) {
                    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Invalid option for noRegionsToConsider\n" );
                    continue;
                }
            }
            else if( key == "maxRegionsToConsider" ) {
                try{
                    int noRegions = boost::lexical_cast<int>( value );
                    if( noRegions > 0 ) {
                        maxNoRegions = noRegions;
                        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: At most %d parallel regions will be considered for tuning\n", maxNoRegions );
                    }
                    else {
                        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: Invalid option for maxRegionsToConsider. Valid option is a integer value > 0\n", minNoRegions );
                    }
                }
                catch( const boost::bad_lexical_cast& ) {
                    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Invalid option for noRegionsToConsider\n" );
                    continue;
                }
            }
            else if( key == "cutoff1" ) {
                try{
                    double cutoff = boost::lexical_cast<double>( value );
                    if( cutoff > 0 && cutoff < 1 ) {
                        firstCutOff = cutoff;
                        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: firstCutOff set to %.2lf.\n", firstCutOff );
                        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: parallel regions with exec time >= %.2lf will be considered for tuning until max limit is reached.\n", firstCutOff );
                    }
                    else {
                        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: Invalid value for cutoff1. Valid option is a float value between 0 and 1.\n" );
                    }
                }
                catch( const boost::bad_lexical_cast& ) {
                    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "invalid option for cutoff1\n" );
                    continue;
                }
            }
            else if( key == "cutoff2" ) {
                try{
                    double cutoff = boost::lexical_cast<double>( value );
                    if( cutoff > 0 && cutoff < 1 ) {
                        secondCutOff = cutoff;
                        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: secondCutOff set to %.2lf.\n", secondCutOff );
                        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: parallel regions with exec time >= %.2lf will be considered depending on number of regions already considered.\n", secondCutOff );
                    }
                    else {
                        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: Invalid value for cutoff2. Valid option is a float value between 0 and 1.\n" );
                    }
                }
                catch( const boost::bad_lexical_cast& ) {
                    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "invalid option for cutoff2\n" );
                    continue;
                }
            }
        }

        // check for no of regions consistency
        if( maxNoRegions < minNoRegions ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: Maximum no of regions (%d) cannot be less than minimum no of regions (%d).\n", maxNoRegions, minNoRegions );
            maxNoRegions = minNoRegions;
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: Setting maximum no of regions to be equal as minimum no of regions (%d).\n", minNoRegions );
        }

        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: completed reading the configuration file.\n" );
    }
    else {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: could not open configuration file.\n" );
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
void PCAPSpeedupPlugin::initialize( DriverContext*   context,
                                    ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: call to initialize()\n" );
    this->context  = context;
    this->pool_set = pool_set;

    has_pcap_config = false;
    parse_opts( context->getArgc(), context->getArgv() );

    if( app.get_regions().empty() )
        psc_abort( "PCAPSpeedupPlugin : No Region found. Exiting.\n" );

    numberOfThreadsTP = new TuningParameter();
    numberOfThreadsTP->setId( 0 );
    numberOfThreadsTP->setName( "NUMTHREADS" );
    numberOfThreadsTP->setPluginType( PCAP );
    numberOfThreadsTP->setRange( 1, context->getOmpnumthreads(), 1 );
    Restriction* r = new Restriction();
    for( int i = 1; i <= context->getOmpnumthreads(); i = i * 2 ) {
        r->addElement( i );
    }
    r->setRegion( NULL );
    r->setType( 2 );
    numberOfThreadsTP->setRestriction( r );
    numberOfThreadsTP->setRuntimeActionType( TUNING_ACTION_FUNCTION_POINTER );

    string results = numberOfThreadsTP->toString();
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: Before exiting to initialize, string is: %s\n\n", results.c_str() );

    // initializing variables to default value
    //  for preanalysis & region selection
    firstTuningStep     = true;
    firstCutOff         = 0.2;
    secondCutOff        = 0.1;
    minNoRegions        = 3;
    maxNoRegions        = 3;
    searchAlgorithmName = "exhaustive";

    if( has_pcap_config )
        parseConfigFile(); // might override some defaults
}


/*
 * Defines a pre-analysis strategy.
 *
 * Found properties of the pre-analysis strategy are stored in the arp.
 *
 */
bool PCAPSpeedupPlugin::analysisRequired( StrategyRequest** strategy ) {
    StrategyRequestGeneralInfo*         analysisStrategyGeneralInfo = new StrategyRequestGeneralInfo;
    std::map<string, Region*>::iterator region;

    analysisStrategyGeneralInfo->strategy_name     = "ConfigAnalysis";
    analysisStrategyGeneralInfo->pedantic          = 0;
    analysisStrategyGeneralInfo->delay_phases      = 0;
    analysisStrategyGeneralInfo->delay_seconds     = 0;
    analysisStrategyGeneralInfo->analysis_duration = 1;

    PropertyRequest* req = new PropertyRequest();

    req->addPropertyID( EXECTIME );
    for( region = code_region_candidates.begin(); region != code_region_candidates.end(); region++ ) {
        req->addRegion( region->second );
    }
    req->addAllProcesses();

    std::list<PropertyRequest*>* reqList = new list<PropertyRequest*>;
    reqList->push_back( req );

    *strategy = new StrategyRequest( reqList, analysisStrategyGeneralInfo );
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
void PCAPSpeedupPlugin::startTuningStep() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: call to startTuningStep()\n" );
    std::list<Region*> code_regions;
    code_regions = appl->get_regions();
    std::list<Region*>::iterator region;

    // iterating over all regions & adding to code_region_candidates for preanalysis
    int parallel_regions = 0, parallel_do_regions = 0;
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Searching all available regions...\n" );
    for( region = code_regions.begin(); region != code_regions.end(); region++ ) {
        /*
         * Only parallel regions should be considered for tuning, since we can not set num threads for
         * individual for loops in the same parallel section
         */
        if( ( *region )->get_type() == PARALLEL_REGION ) {
            if( ( *region )->get_type() == PARALLEL_REGION ) {
                parallel_regions++;
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Parallel region found:\n" );
            }
            else {
                parallel_do_regions++;
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Parallel do region found:\n" );
            }
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "\tFile name: %s\n", ( *region )->getFileName().c_str() );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "\tFirst line: %d\n", ( *region )->getFirstLine() );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "\tLast line: %d\n", ( *region )->getLastLine() );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "\tMatcher key: %s\n", ( *region )->getIdForPropertyMatching().c_str() );
            code_region_candidates[ ( *region )->getIdForPropertyMatching() ] = *region;
        }
    }
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAP PLUGIN: found %d parallel & parallel_do regions.\n", ( parallel_regions + parallel_do_regions ) );

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "obtain getSearchInstance\n" );
    int    major, minor;
    string name, description;

    char const* selected_search = getenv( "PSC_SEARCH_ALGORITHM" );
    if( selected_search != NULL ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "User specified search algorithm: %s\n", selected_search );
        string selected_search_string = string( selected_search );
        context->loadSearchAlgorithm( selected_search_string, &major, &minor, &name, &description );
        searchAlgorithm = context->getSearchAlgorithmInstance( selected_search_string );
    }
    else {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Selecting default search algorithm: %s\n ", searchAlgorithmName.c_str() );
        context->loadSearchAlgorithm( searchAlgorithmName, &major, &minor, &name, &description );
        searchAlgorithm = context->getSearchAlgorithmInstance( searchAlgorithmName );
        searchAlgorithm->addObjectiveFunction( new TimeObjective("") );
    }

    if( searchAlgorithm != NULL ) {
        print_loaded_search( major, minor, name, description );
        searchAlgorithm->initialize( context, pool_set );
    }
    else {
        perror( "NULL pointer in searchAlgorithm\n" );
        throw PTF_PLUGIN_ERROR( NULL_REFERENCE );
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "searchAlgorithm instance obtained\n" );
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
void PCAPSpeedupPlugin::createScenarios() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: call to createScenarios()\n" );
    if( !searchAlgorithm )
        psc_abort("Error: The search algorithm is set to a nullptr.");

    // while createScenarios may be called multiple times, we should add the searchspace only once
    if( firstTuningStep ) {
        firstTuningStep = false;

        if( withParallelRegions ) {
            // sorting and arranging prenanalysis result
            list<MetaProperty> found_properties = pool_set->arp->getPreAnalysisProperties( 0 );
            double             totalExecTime    = 0;

            BOOST_FOREACH( MetaProperty& property, found_properties ) {
                regionExectimeMap.push_back( make_pair( property.getRegionId(), property.getSeverity() ) );
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                            "PCAPSpeedupPlugin: Severity (%f) in region: %d added to vector\n",
                            property.getSeverity(), static_cast<int>( property.getRegionType() ) );
                totalExecTime += property.getSeverity();
            }

            // sort the vector according to decreasing execution time
            std::sort( regionExectimeMap.begin(), regionExectimeMap.end(), pairCompare );

            /*
             *  Using heuristic to find out important regions
             *  - first add regions with >= firstCutOff % (e.g. 20%) exectime unitl maxNoRegions
             *  - if no of regions are less than minNoRegions, add regions with  with
             *    exectime < firstCutoff & >=secondCutOff % (e.g. between 20% & 10%) exectime
             */

            int noRegionsAdded = 0;
            // initially collect regions that are above first cutoff
            for( int i = 0; i < regionExectimeMap.size(); i++ ) {
                double execTime = regionExectimeMap[ i ].second;
                if( execTime < ( firstCutOff * totalExecTime ) ) {
                    break;
                }

                noRegionsAdded++;
                selected_region = code_region_candidates[ regionExectimeMap[ i ].first ];
                addRegionToSearchAlgorithm( regionExectimeMap[ i ].second );
                if( noRegionsAdded == maxNoRegions ) {
                    break;
                }
            }

            if( noRegionsAdded < minNoRegions ) { // utilize second cutoff
                for( int i = 0; i < regionExectimeMap.size(); i++ ) {
                    double execTime = regionExectimeMap[ i ].second;
                    if( execTime < ( secondCutOff * totalExecTime ) ) {
                        break;
                    }

                    if( execTime >= ( firstCutOff * totalExecTime ) ) {
                        continue;
                    }

                    noRegionsAdded++;
                    selected_region = code_region_candidates[ regionExectimeMap[ i ].first ];
                    addRegionToSearchAlgorithm( regionExectimeMap[ i ].second );
                    if( noRegionsAdded == minNoRegions ) {
                        break;
                    }
                }
            }

            // if no regions added, add first few regions in decreasing order of their execTime
            if( noRegionsAdded == 0 ) {
                for( int i = 0; i < regionExectimeMap.size(); i++ ) {
                    noRegionsAdded++;
                    selected_region = code_region_candidates[ regionExectimeMap[ 0 ].first ];
                    addRegionToSearchAlgorithm( regionExectimeMap[ 0 ].second );
                    if( noRegionsAdded == minNoRegions ) {
                        break;
                    }
                }
            }
        }

        if( !withParallelRegions || withUserRegion ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                        "PCAPSpeedupPlugin: Creating search space for the USER region\n" );
            SearchSpace*  searchspace  = new SearchSpace();
            VariantSpace* variantSpace = new VariantSpace();
            variantSpace->addTuningParameter( numberOfThreadsTP );
            searchspace->setVariantSpace( variantSpace );
            searchspace->addRegion( app.get_phase_region() );
            searchAlgorithm->addSearchSpace( searchspace );
        }
    }

    searchAlgorithm->createScenarios();
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: call to createScenarios() completed\n" );
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
void PCAPSpeedupPlugin::prepareScenarios() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: call to prepareScenarios()\n" );

    if( !pool_set->csp->empty() ) {
        Scenario* scenario;
        scenario = pool_set->csp->pop();
        pool_set->psp->push( scenario );
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
void PCAPSpeedupPlugin::defineExperiment( int               numprocs,
                                          bool&             analysisRequired,
                                          StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: call to defineExperiment()\n" );

    int i = 0;
    Scenario* scenario;
    for( ; !pool_set->psp->empty() && i < numprocs; i++ ) {
        scenario = pool_set->psp->pop();

        std::list<PropertyRequest*>* propertyRequestList = new list<PropertyRequest*>();
        std::list<int>*              propertyIds         = new list<int>;

        propertyIds->push_back( EXECTIME );
        propertyRequestList->push_back( new PropertyRequest( propertyIds ) );
        scenario->setPropertyRequests( propertyRequestList );
        scenario->setTunedRegion( app.get_phase_region() );

        pool_set->esp->push( scenario );
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: Added %d scenario in the experiment.\n", i );
    StrategyRequestGeneralInfo* strategyRequestGeneralInfo = new StrategyRequestGeneralInfo;

    strategyRequestGeneralInfo->strategy_name     = "OMP";
    strategyRequestGeneralInfo->pedantic          = 1;
    strategyRequestGeneralInfo->delay_phases      = 0;
    strategyRequestGeneralInfo->delay_seconds     = 0;
    strategyRequestGeneralInfo->analysis_duration = 1;

    *strategy = NULL;
}


/*
 * Return true if a restart of the application is required for the next experiment,
 * false otherwise.
 */
bool PCAPSpeedupPlugin::restartRequired( std::string& env,
                                         int&         numprocs,
                                         std::string& command,
                                         bool&        is_instrumented ) {
    return false; // restart not required to change the number of threads
}


/*
 * Return true if if the current search iteration is finished (by either the plugin itself or as
 * a result of the search algorithm), false otherwise.
 */
bool PCAPSpeedupPlugin::searchFinished() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: call to searchFinished()\n" );
    return searchAlgorithm->searchFinished();
}


/*
 * Final operation of a tuning step.
 *
 * If any post-processing is necessary before entering the next tuning iteration, it is to be
 * done here.
 */
void PCAPSpeedupPlugin::finishTuningStep() {
}


/*
 * Returns true if the plugin finished the tuning process, false otherwise.
 */
bool PCAPSpeedupPlugin::tuningFinished() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: call to tuningFinished()\n" );
    return true;
}


/*
 * Prints to the screen (and to a file, where necessary) the tuning advice.
 */
Advice* PCAPSpeedupPlugin::getAdvice() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: call to getAdvice()\n" );
    if( !searchAlgorithm )
        psc_abort("Error: The search algorithm is set to a nullptr.");

    Scenario* scenario;
    for( int scenario_id = 0; scenario_id < pool_set->fsp->size(); scenario_id++ ) {
        scenario = ( *pool_set->fsp->getScenarios() )[ scenario_id ];
        std::list<MetaProperty> properties = pool_set->srp->getScenarioResultsByID( scenario_id );

        BOOST_FOREACH( MetaProperty& prop, properties )
            scenario->addResult( "Time", prop.getSeverity() );
    }

    writePCAPresults();

    Scenario* best_scenario = ( *pool_set->fsp->getScenarios() )[ searchAlgorithm->getOptimum() ];
    if( !best_scenario )
        psc_abort( "Error: Best scenario cannot be NULL at this point." );

    return new Advice( getName(), best_scenario, searchAlgorithm->getSearchPath(), "Time", pool_set->fsp->getScenarios() );
}


/*
 * Finalize the plugin normally.
 *
 * Remove any allocated memory, objects, file descriptors, etc.
 *
 */
void PCAPSpeedupPlugin::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: call to finalize()\n" );
    terminate();
}


/*
 * Terminate the plugin due to error.
 *
 * Safely remove any allocated memory, objects, file descriptors, etc. This method should
 * be able to be executed safely at any point.
 *
 */
void PCAPSpeedupPlugin::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: call to terminate()\n" );

    if( searchAlgorithm ) {
        searchAlgorithm->finalize();
        delete searchAlgorithm;
    }

    context->unloadSearchAlgorithms();
}


/**
 * @brief Returns an instance of this particular plugin implementation.
 *
 * @ingroup PCAPSpeedupPlugin
 *
 * @return A pointer to an instance of this particular plugin implementation.
 */
IPlugin* getPluginInstance() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: call to getComponentInstance()\n" );
    return new PCAPSpeedupPlugin();
}


/**
 * @brief Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 *
 * @ingroup PCAPSpeedupPlugin
 *
 * @return The major plugin interface version used by the plugin
 */
int getVersionMajor() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: call to getInterfaceVersionMajor()\n" );
    return 1;
}


/**
 * @brief Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 *
 * @ingroup PCAPSpeedupPlugin
 *
 * @return The minor plugin interface version used by the plugin
 *
 */
int getVersionMinor() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: call to getInterfaceVersionMinor()\n" );
    return 0;
}


/**
 * @brief Returns a string with the name of the plugin.
 *
 * @ingroup PCAPSpeedupPlugin
 *
 * @return A string with the name of the plugin
 *
 */
std::string getName() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: call to getName()\n" );
    return "PCAP plugin";
}


/**
 * @brief Returns a string with a short description of the plugin.
 *
 * @ingroup PCAPSpeedupPlugin
 *
 * @return A string with a short description of the plugin
 *
 */
std::string getShortSummary() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: call to getShortSummary()\n" );
    return "Tunes the Energy Delay Product of OMP applications.";
}


void PCAPSpeedupPlugin::addRegionToSearchAlgorithm(double  execTime) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "PCAPSpeedupPlugin: selected region with: runtime: %f; ID: %s; file: %s; first line: %d; last line: %d;\n", execTime, selected_region->getIdForPropertyMatching().c_str(), selected_region->getFileName().c_str(), selected_region->getFirstLine(), selected_region->getLastLine() );

    SearchSpace*  searchspace  = new SearchSpace();
    VariantSpace* variantSpace = new VariantSpace();

    variantSpace->addTuningParameter( numberOfThreadsTP );
    searchspace->setVariantSpace( variantSpace );

    selected_region->print();
    printf( "\n" );
    searchspace->addRegion( selected_region );
    searchAlgorithm->addSearchSpace( searchspace );
}


void PCAPSpeedupPlugin::writePCAPresults() {
    const int num_scenarios = pool_set->fsp->size();

    std::ostringstream result_oss;
    result_oss << "\nAutoTune Results:" << std::endl;
    result_oss << "-----------------------\n\n";

    result_oss << "\nAll Results:\n";
    result_oss << "Scenario                            |   Threads   |   Execution Time\n";
    std::ofstream result_file( "pcap_results.txt" );
    result_file << "Scenario ID;Threads;Time\n";
    for( int scenario_id = 0; scenario_id < num_scenarios; ++scenario_id ) {
        Scenario* scenario = ( *pool_set->fsp->getScenarios() )[ scenario_id ];
        result_oss <<  std::setw( 3 )  <<  scenario_id  <<  "    ";
        result_file << scenario_id << "; ";

        BOOST_FOREACH( TuningSpecification* ts, *scenario->getTuningSpecifications() ) {
            typedef std::map<TuningParameter*, int>::const_iterator Iterator;
            int nthr = 0;

            // the variant is a list of parameters and values
            for( Iterator iter = ts->getVariant()->getValue().begin(); iter != ts->getVariant()->getValue().end(); ++iter )
                nthr = iter->second;

            BOOST_FOREACH( const std::string& region_iter, *ts->getVariantContext().context_union.entity_list ) {
                std::ostringstream temp;
                temp <<  "("  <<  region_iter  <<  "):";
                std::string tempStr = temp.str();
                result_oss <<  std::setw( 24 )  <<  std::left  <<  tempStr  <<  std::setw( 3 )  <<  std::right  <<  nthr  <<  "  ";
                result_file <<  tempStr;
            }
        }

        result_file << ";";

        typedef std::map<string, double>::const_iterator Iterator;
        std::map<string, double> results = scenario->getResults();
        for( Iterator i = results.begin(); i != results.end(); i++ ) {
            result_oss << std::setw( 14 ) <<  std::right  << i->second;
            result_file << i->second << ";";
        }

        result_oss << std::endl;
        result_file << std::endl;
    }
    result_oss << std::endl;

    // print best scenario
    Scenario* best_scenario = ( *pool_set->fsp->getScenarios() )[ searchAlgorithm->getOptimum() ];
    if( !best_scenario )
        psc_abort( "Error: Best scenario cannot be NULL at this point." );
    assert( best_scenario );
    result_oss << "=> Best scenario: " << best_scenario->getID() << std::endl;

    // print all the generated output
    std::cout << result_oss.str();
}
