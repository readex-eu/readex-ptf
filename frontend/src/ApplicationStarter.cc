#include "ApplicationStarter.h"
#include "frontend.h"

extern int  application_pid;
extern char user_specified_environment[ 5000 ];

using namespace std;




static std::string get_filename_from_region( const std::string& region ) {
    size_t      start, stop;
    std::string filename;

    start = region.find( "f: " );
    stop  = region.find( ",", start + 3 );
    if( region.find_last_of( "/", stop ) != string::npos ) {
        start = region.find_last_of( "/", stop );
        start = start - 2;
    }
    filename = region.substr( start + 3, stop - start - 3 );
    return filename;
}


static std::string get_rfl_from_region( const std::string& region ) {
    size_t      start, stop;
    std::string rfl;

    start = region.find( "r: " );
    stop  = region.find( "\n", start + 3 );
    rfl   = region.substr( start + 3, stop - start - 3 );

    return rfl;
}


ApplicationStarter::ApplicationStarter(): pluginHandle(NULL), processes(0) { }


ApplicationStarter::~ApplicationStarter() {
    if( pluginHandle )
        dlclose(pluginHandle);
}


ApplicationStarter::StarterPluginFunction ApplicationStarter::loadPluginFunction( const char* name ) {
    if( !pluginHandle ) {
        pluginHandle = dlopen("libptfstarter.so", RTLD_LAZY | RTLD_LOCAL);
        char* error;
        if( (error = dlerror()) ) {
            psc_errmsg("%s\n", error);
            abort();
        }
    }

    StarterPluginFunction fn = (StarterPluginFunction) dlsym(pluginHandle, name);
    if( dlerror() )
        return NULL;

    return fn;
}


void ApplicationStarter::startApplication( bool isFirstStart ) {
    StarterPluginFunction cmdfn = loadPluginFunction("StarterPlugin_appStartCmd");
    if( !cmdfn )
        psc_abort("Could not load function StarterPlugin_appStartCmd.\n");

    string appstart_cmd = cmdfn(user_specified_environment);

    if (applUninstrumented()) {
        psc_dbgmsg( 1,
                    "Starting non-instrumented application in interactive partition: >%s<\n",
                    appstart_cmd.c_str());
        system(appstart_cmd.c_str());
    }
    else {
        RegistryService* regsrv = fe->get_registry();

        if (!isFirstStart) {
            // Set time-out to the maximum value and call re-instrumentation procedure on restart
            fe->set_global_timeout( 429496729 );
            instrumentRequiredRegions();
        }

        application_pid = fork();
        if (application_pid == 0) {
            // Child process starts application
            psc_dbgmsg( 1,
                        "Starting application in interactive partition: >%s<\n",
                        appstart_cmd.c_str());
            int retVal = system(appstart_cmd.c_str());
            exit( retVal );
        }
        else if (application_pid < 0) {
            psc_errmsg("Error forking child process!\n");
            abort();
        }

        for( int rank = 0; rank < processes; rank++ ) {
            idmap_f[ rank ] = isFirstStart ? - 1 : idmap_t[ rank ];
            idmap_t[ rank ] = -1;
        }

        EntryData query;

        bool startup_finished       = false;
        int  num_startup_components = processes;

        if (isFirstStart && num_startup_components == 0) {
            num_startup_components = 1;
        }

        psc_dbgmsg(5, "waiting for startup of %d components...\n", num_startup_components);
        while( !startup_finished ) {
            query.app = fe->get_appname();
            query.tag = "none";

            processList.clear();

            if( regsrv->query_entries( processList, query, false ) == -1 ) {
                psc_errmsg( "Error querying registry for application\n" );
                abort();
            }

            psc_dbgmsg( 1, "components started up: %d (of %d)\n",
                        processList.size(), num_startup_components );
            startup_finished = ( processList.size() >= num_startup_components );

            int child_status;

            // Child finished but components not started -> process was terminated or error occurred
            if (waitpid(application_pid, &child_status, WNOHANG) == application_pid && !startup_finished) {
                abort();
            }

            if (!startup_finished) {
                sleep(1);
            }
        }

        std::list<EntryData>::iterator entryit;
        for (entryit = processList.begin(); entryit != processList.end(); entryit++) {
            idmap_t[entryit->pid - 1] = entryit->id;
        }

        for( int i = 0; i < processes; i++ ) {
            if( idmap_t[ i ] == -1 ) {
                psc_errmsg( "Error in collecting ids of processes. Registry entry for rank %d not found.\n",
                            i );
                abort();
            }
        }
    }
}


void ApplicationStarter::runApplication() {
    processes = fe->get_mpinumprocs();
    if( opts.has_apprun )
        ApplicationStarter::startApplication(true);
}


void ApplicationStarter::rerunApplication() {
    startApplication(false);

    //send mapping info to child agents
    std::map< std::string, AgentInfo >* child_agents = fe->get_child_agents();
    std::map< std::string, AgentInfo >::iterator it;
    for( it = ( *child_agents ).begin(); it != ( *child_agents ).end(); it++ ) {
        AgentInfo& ai = it->second;
        if( ai.status != AgentInfo::CONNECTED ) {
            if( fe->connect_to_child( &ai ) == -1 ) {
                psc_errmsg( "Error connecting to child at %s:%d\n",
                            ai.hostname.c_str(), ai.port );
            }
            else {
                ai.handler->reinit( processes, idmap_f, idmap_t );
            }
        }
    }
}


void ApplicationStarter::runAnalysisAgent( AgentDetails* agent ) {
    if( !opts.has_phase ) {
        psc_abort( "The name of the phase region was not provided!\n" );
    }

    std::stringstream common_command;

    StarterPluginFunction cmdfn = loadPluginFunction("StarterPlugin_agentStartPrefix");

    // Test whether we have a plugin which gives us the prefix.
    // The prefix has to respect the agent host. Additionally, the plugin may
    // add a debug tool like gdb to obtain stack traces.
    if (cmdfn == NULL) {
        const char*       cwd = getenv( "PWD" );

        if( opts.has_force_localhost ) {
            common_command <<  "cd "  <<  cwd  <<  ";";
        } else {
            common_command <<  "ssh "  <<  agent->host  <<  " \"cd "  <<  cwd  <<  "\";";
        }
    } else {
        common_command << cmdfn(agent->host);
    }

    //AAgent command
    common_command << xstr(_AAGENT_EXEC)
                   << " --tag=" << agent->tag
                   << " --parent=" << agent->parent
                   << " --phase=" << opts.phase_string
                   << " --port=" << agent->port
                   << " --appname=" << fe->get_appname()
                   << " --id=" << agent->applIds
                   << " --mpinumprocs=" << fe->get_mpinumprocs()
                   << " --ompnumthreads=" << fe->get_ompnumthreads();

    if( opts.has_strategy ) {
        common_command << " --strategy=" << opts.strategy;
    }

    if( opts.has_configurationfile ) {
        common_command << " --config-file=" << opts.configfile_string;
    }

    if( opts.has_delay ) {
        common_command << " --delay=" << opts.delay_string;
    }

    if( opts.has_iterations ) {
        common_command << " --iterations=" << opts.iterations_string;
    }

    if( opts.has_registry ) {
        common_command << " --registry=" << opts.reg_string;
    }

    if( opts.has_timeout ) {
        common_command << " --timeout=" << opts.timeout_string;
    }

    common_command << " --debug=" << psc_dbg_level;

    if( opts.has_inst ) {
        common_command << " --inst=" << opts.inst_string;
    }

    if( opts.has_selectivedebug ) {
        common_command << " --selective-debug=" << opts.selectivedebug_string;
    }

    if( opts.has_plugin ) {
        common_command << " --tune=" << opts.plugin;
    }
    const std::string command_str = common_command.str();

    // START analysis agent..
    psc_dbgmsg( 1, "Starting AAgent (%s): %s\n", agent->tag, command_str.c_str() );
    if( fork() == 0 ) {
        int retVal = system( command_str.c_str() );
        _exit( retVal );
    }
}


void ApplicationStarter::runHlAgent( AgentDetails* agent ) {
    std::stringstream command_string;

    StarterPluginFunction cmdfn = loadPluginFunction("StarterPlugin_agentStartPrefix");

    // Test whether we have a plugin which gives us the prefix.
    if (cmdfn == NULL) {
        if (!opts.has_force_localhost) {
            command_string <<  "ssh "  <<  agent->host  <<  " ";
        }
    } else {
        command_string << cmdfn(agent->host);
    }

    command_string <<  xstr( _HLAGENT_EXEC );
    command_string <<  " --tag="  <<  agent->tag;
    command_string <<  " --parent="  <<  agent->parent;
    command_string <<  " --appname="  <<  fe->get_appname();
    command_string <<  " --port="  <<  agent->port;
    command_string <<  " --child="  <<  agent->children;
    if( opts.has_registry )
        command_string <<  " --registry="  <<  opts.reg_string;
    if( opts.has_dontcluster )
        command_string <<  " --dontcluster";
    if( opts.has_timeout )
        command_string <<  " --timeout="  <<  atoi(opts.timeout_string);
    command_string <<  " --debug="  <<  psc_dbg_level;
    if( opts.has_selectivedebug )
        command_string <<  " --selective-debug="  <<  opts.selectivedebug_string;

    // START high-level agent..
    const std::string command = command_string.str();
    psc_dbgmsg( 1, "Starting HLAgent (%s): %s\n", agent->tag, command.c_str() );

    if( fork() == 0 ) {
        int retVal = system( command.c_str() );
        exit( retVal );
    }
}


void ApplicationStarter::printAgentHierarchy( LevelInfo* levels ) const {
    LevelInfo* level   = levels;
    int        levelNr = 1;
    while( level != NULL ) {
        AgentDetails* agent = level->agents;
        psc_dbgmsg( 1, "Agents on level %d: %d\n", levelNr++,
                    level->numberOfAgents );
        while( agent != NULL ) {
            psc_dbgmsg( 1,
                        "%s %s Port: %s; Host: %s; Parent: %s; ApplIds: %s; Children: %s;\n",
                        ( level->prevLevel == NULL ) ? "[AA]" : "[HLA]", agent->tag,
                        agent->port, agent->host, agent->parent, agent->applIds,
                        ( strlen( agent->children ) ? agent->children : "none" ) );
            agent = agent->nextAgent;
        }
        level = level->prevLevel;
    }
}


ApplicationStarter::LevelInfo* ApplicationStarter::computeAgentHierarchy() {
    std::list<EntryData>::iterator                      entryit;
    std::list<int>::iterator                            idit;
    std::map< std::string, std::list< int > >           apphosts;
    std::map< std::string, std::list< int > >::iterator apphostit;
    char                                                hostname[ 100 ];
    gethostname( hostname, 100 );
    LevelInfo*    levels, * level, * masterLevel;
    AgentDetails* agent;
    int           nextPort = fe->get_local_port() + 1;

    psc_dbgmsg( 3, "Identified %d matching entries\n", processList.size() );

    for( entryit = processList.begin(); entryit != processList.end();
         entryit++ ) {
        apphosts[ entryit->node ].push_back( entryit->id );
    }

    int clusterSize = 128;
    if( opts.has_maxcluster ) {
        psc_dbgmsg( 3, "Maxcluster in interactive startup %s\n",
                    opts.maxcluster_string );
        clusterSize = atoi( opts.maxcluster_string );
    }

    levels = ( LevelInfo* )calloc( 1, sizeof( LevelInfo ) );
    level  = levels;

    // Create an agent description
    agent = ( AgentDetails* )calloc( 1, sizeof( AgentDetails ) );
    sprintf( agent->port, "%d", nextPort++ );
    strcat( agent->host, hostname );
    //determine analysis agents
    int   entryCount = 0;
    char* placement  = agent->applIds;
    *placement = 0;

    level->agents         = agent;
    level->numberOfAgents = 1;

    if( opts.has_force_localhost ) {
        for( entryit = processList.begin(); entryit != processList.end();
             entryit++ ) {
            entryCount++;
            //Split list of application entries into clusters of give size
            if( entryCount > clusterSize ) {
                //new analysis agent
                agent->nextAgent = ( AgentDetails* )calloc( 1, sizeof( AgentDetails ) );
                agent            = agent->nextAgent;
                sprintf( agent->port, "%d", nextPort++ );
                strcat( agent->host, hostname );

                // set the pointer to the string that will store the appl ids and init the mem
                placement  = agent->applIds;
                *placement = 0;

                level->numberOfAgents++;
                entryCount = 1;

                // add the current app id to the new agent
                sprintf( strchr( placement, 0 ), "%d", entryit->id );
            }
            else {
                if( entryCount > 1 ) {
                    strcat( placement, "," );
                }
                sprintf( strchr( placement, 0 ), "%d", entryit->id );
            }
        }
    }
    else {
        for( apphostit = apphosts.begin(); apphostit != apphosts.end();
             apphostit++ ) {
            psc_dbgmsg( 3, "Processing node %s\n", apphostit->first.c_str() );

            if( apphostit != apphosts.begin() ) {
                // create new agent
                agent->nextAgent = ( AgentDetails* )calloc( 1, sizeof( AgentDetails ) );
                agent            = agent->nextAgent;
                sprintf( agent->port, "%d", nextPort++ );
                strcat( agent->host, hostname );

                // set the pointer to the string that will store the appl ids and init the mem
                placement  = agent->applIds;
                *placement = 0;

                level->numberOfAgents++;
                entryCount = 0;
            }

            // Loop over all process entries for the current node!
            for( idit = ( *apphostit ).second.begin();
                 idit != ( *apphostit ).second.end(); idit++ ) {
                entryCount++;
                //Split list of application entries into clusters of give size
                if( entryCount > clusterSize ) {
                    //new analysis agent
                    agent->nextAgent = ( AgentDetails* )calloc( 1, sizeof( AgentDetails ) );
                    agent            = agent->nextAgent;
                    sprintf( agent->port, "%d", nextPort++ );
                    strcat( agent->host, hostname );

                    // set the pointer to the string that will store the appl ids and init the mem
                    placement  = agent->applIds;
                    *placement = 0;

                    level->numberOfAgents++;
                    entryCount = 1;

                    // add the current app id to the new agent
                    sprintf( strchr( placement, 0 ), "%d", ( *idit ) );
                }
                else {
                    if( entryCount > 1 ) {
                        strcat( placement, "," );
                    }
                    sprintf( strchr( placement, 0 ), "%d", ( *idit ) );
                }
            }
        }
    }

    int maxfan = fe->get_maxfan();
    //Create levels of high level agents

    while( level->numberOfAgents > 1 ) {
        int numberOfAgentsInPrevLevel = level->numberOfAgents;

        // Create the next hierarchy level in the list of levels
        level->nextLevel            = ( LevelInfo* )calloc( 1, sizeof( LevelInfo ) );
        level->nextLevel->prevLevel = level;
        level                       = level->nextLevel;

        // HLA
        agent = ( AgentDetails* )calloc( 1, sizeof( AgentDetails ) );
        sprintf( agent->port, "%d", nextPort++ );
        strcat( agent->host, hostname );
        level->agents         = agent;
        level->numberOfAgents = 1;

        // if maxfan is 1, then create only 1 HL for all agents
        if( maxfan == 1 ) {
            break;
        }

        for( int i = 1; i < ( numberOfAgentsInPrevLevel + maxfan - 1 ) / maxfan;
             i++ ) {
            agent->nextAgent = ( AgentDetails* )calloc( 1, sizeof( AgentDetails ) );
            agent            = agent->nextAgent;
            sprintf( agent->port, "%d", nextPort );
            nextPort++;
            strcat( agent->host, hostname );
            level->numberOfAgents++;
        }
    }
    masterLevel = level;
    char tagStr[ 100 ], parentStr[ 100 ], childrenTags[ 4000 ];

    sprintf( tagStr, "fe[%d]:0", fe->own_id() );
    strcat( level->agents->tag, tagStr );
    *childrenTags = 0;
    sprintf( parentStr, "%s[%d]", fe->get_local_tag().c_str(), fe->own_id() );
    strcat( level->agents->parent, parentStr );
    while( level->prevLevel != NULL ) {
        AgentDetails* child, * parent;
        int           childrenCnt = 0;
        parent = level->agents;            // List of agents on this level
        child  = level->prevLevel->agents; // Children of the current level (agents on the next lower level)

        while( child != NULL && parent != NULL ) {
            sprintf( parentStr, "%s", parent->tag );
            strcat( child->parent, parentStr );

            sprintf( tagStr, "%s:%d", parent->tag, childrenCnt );
            strcat( child->tag, tagStr );

            strcat( childrenTags, tagStr );
            childrenCnt++;
            if( maxfan > 1 && childrenCnt == maxfan ) {
                childrenCnt = 0;
                strcat( parent->children, childrenTags );
                *childrenTags = 0;
                parent        = parent->nextAgent;
            }
            else {
                if( child != NULL ) {
                    strcat( childrenTags, "," );
                }
            }
            child = child->nextAgent;
        }

        if( parent != NULL ) {
            strcat( parent->children, childrenTags );
        }
        *childrenTags = 0;
        level         = level->prevLevel;
    }
    return masterLevel;
}


void ApplicationStarter::runAgents() {
    // child tag
    char child_tag[200];

    snprintf(child_tag, sizeof(child_tag), "fe[%d]:0", fe->own_id());

    char hostname[100];
    gethostname(hostname, 100);
    fe->add_child_agent( child_tag, hostname, -1 );

    LevelInfo* masterLevel = computeAgentHierarchy();

    printAgentHierarchy( masterLevel );

    LevelInfo* level = masterLevel;
    // for (LevelInfo* level = masterLevel; level != NULL && level->prevLevel != NULL; level = level->prevLevel)
    while( level != NULL && level->prevLevel != NULL ) {
        // for (AgentDetails* agent = level->agents; agent != NULL; agent = agent->nextAgent)
        AgentDetails* agent = level->agents;
        while( agent != NULL ) {
            runHlAgent( agent );
            agent = agent->nextAgent;
        }
        level = level->prevLevel;
    }

    // No need for an if clause here, as we have at least one level.
    for( AgentDetails* agent = level->agents; agent != NULL; agent = agent->nextAgent ) {
        runAnalysisAgent(agent);
    }
}


//TODO: MRI monitor code, should be adapted to Score-P if possible
void ApplicationStarter::instrumentRequiredRegions() {
    bool        requiresMake = false;
    std::string make, src_folder, instr_folder;

    if( opts.has_inst != 1 ) {
        return;
    }

    fe->rm_duplicates_requiredRegionsList();
    fe->print_badRegions();
    fe->print_requiredRegionsList();
    fe->print_requiredRegionsListLastRun();

    if( opts.has_make == 1 ) {
        make = opts.make_string;
    }
    else {
        make = "make";
    }

    if( opts.has_sourcefolder == 1 ) {
        src_folder = opts.sourcefolder_string;
        src_folder.append( "/" );
    }
    else {
        src_folder = "./";
    }

    if( opts.has_instfolder == 1 ) {
        instr_folder = opts.instfolder_string;
        instr_folder.append( "/" );
    }
    else {
        instr_folder = "./inst/";
    }


    if( fe->get_badRegions().size() > 0 && !fe->get_badRegionsRemoved() &&
        ( strcmp( opts.inst_string, "overhead" ) == 0 || strcmp( opts.inst_string, "all_overhead" ) == 0 ) ) {
        size_t                           start, stop;
        std::string                      filename, region, tmp;
        char                             cmnd[ 1000 ];
        std::list < std::string >        bad_regions = fe->get_badRegions();
        std::list<std::string>::iterator itr;

        tmp.erase();
        if( opts.has_config ) {
            tmp.append( opts.config_string );
        }
        else {
            tmp.append( src_folder );
            tmp.append( "psc_inst_config" );
        }
        sprintf( cmnd, "reinstrumentedall.sh %s", tmp.c_str() );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "sending command: %s to the system...\n", cmnd );
        int retVal = system( cmnd );


        for( itr = bad_regions.begin(); itr != bad_regions.end(); itr++ ) {
            stop  = itr->find( ",", 0 );
            start = 2;
            if( itr->find_last_of( "/", stop ) != string::npos ) {
                start = itr->find_last_of( "/", stop );
            }
            filename = itr->substr( start + 1, stop - start - 1 );

            tmp = src_folder;
            tmp.append( filename );
            sprintf( cmnd, "touch %s", tmp.c_str() );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "sending command: %s to the system...\n", cmnd );
            retVal = system( cmnd );

            stop     = itr->find_last_of( ".", stop );
            filename = itr->substr( start + 1, stop - start - 1 );
            filename.append( ".f90" );

            start  = itr->find( "r: ", stop );
            stop   = itr->find( "\n", start + 3 );
            region = itr->substr( start + 3, stop - start - 3 );

            tmp = instr_folder;
            tmp.append( filename );
            sprintf( cmnd, "comment.sh %s %s ", region.c_str(), tmp.c_str() );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "sending command: %s to the system...\n", cmnd );
            retVal = system( cmnd );
        }
        requiresMake = true;
        fe->set_badRegionsRemoved( true );
    }
    else if( strcmp( opts.inst_string, "analysis" ) == 0 ) {   //Analysis instrumentation strategy
        size_t                           start, stop, startln, stopln;
        std::string                      filename, region, path, tmp, make, line;
        char                             cmnd[ 1000 ];
        std::list < std::string >        bad_regions = fe->get_badRegions();
        std::list<std::string>::iterator itr;
        bool                             valid;


        if( !fe->get_allRegionsCommented() ) {
            tmp.erase();
            if( opts.has_config ) {
                tmp.append( opts.config_string );
            }
            else {
                tmp.append( src_folder );
                tmp.append( "psc_inst_config" );
            }

            sprintf( cmnd, "reinstrumentedall.sh %s", tmp.c_str() );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "sending command: %s to the system...\n", cmnd );
            system( cmnd );

            sprintf( cmnd, "commentall.sh %s", instr_folder.c_str() );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "sending command: %s to the system...\n", cmnd );
            system( cmnd );

            sprintf( cmnd, "touchall.sh %s", src_folder.c_str() );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "sending command: %s to the system...\n", cmnd );
            system( cmnd );

            requiresMake = true;
            fe->set_allRegionsCommented( true );
        }

        if( fe->checkForReinstrumentation() ) {       //There are required regions
            requiresMake = true;

            //First, remove the old one
            std::list < std::string > regList = fe->get_requiredRegionsListLastRun();
            for( std::list<std::string>::iterator it = regList.begin(); it != regList.end(); it++ ) {
                std::string region = *it;
                std::string filename, tmp, rfl;

                filename = get_filename_from_region( region );
                rfl      = get_rfl_from_region( region );

                tmp = src_folder;
                tmp.append( filename );
                sprintf( cmnd, "touch %s", tmp.c_str() );
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "sending command: %s to the system...\n", cmnd );
                system( cmnd );

                stop     = filename.find_last_of( "." );
                filename = filename.substr( 0, stop );
                filename.append( ".f90" );

                tmp = instr_folder;
                tmp.append( filename );
                sprintf( cmnd, "comment.sh %s %s", rfl.c_str(), tmp.c_str() );
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "sending command: %s to the system...\n", cmnd );
                system( cmnd );
            }

            //Second, instrument the required regions
            std::list<std::string> requiredRegList = fe->get_requiredRegionsList();
            for( std::list<std::string>::iterator it = requiredRegList.begin(); it != requiredRegList.end(); it++ ) {
                std::string region = *it;
                std::string filename, tmp, rfl;

                if( fe->is_in_badRegions( region ) ) {
                    continue;                     //This region would have too much overhead.
                }

                filename = get_filename_from_region( region );
                rfl      = get_rfl_from_region( region );

                tmp = src_folder;
                tmp.append( filename );
                sprintf( cmnd, "touch %s", tmp.c_str() );
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "sending command: %s to the system...\n", cmnd );
                system( cmnd );

                stop     = filename.find_last_of( "." );
                filename = filename.substr( 0, stop );
                filename.append( ".f90" );

                tmp = instr_folder;
                tmp.append( filename );
                sprintf( cmnd, "uncomment.sh %s %s", rfl.c_str(), tmp.c_str() );
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "sending command: %s to the system...\n", cmnd );

                system( cmnd );
            }
            fe->copy_requiredRegionsToLastRun();
            fe->clear_requiredRegionsList();
        } //New instrumentation required
        fe->clear_requiredRegionsList();
    }     //inst==analysis

    if( requiresMake ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( Autoinstrument ), "sending make command\n" );
        system( make.c_str() );
    }
}
