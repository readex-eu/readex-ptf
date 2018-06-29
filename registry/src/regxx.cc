/**
   @file	regxx.cc
   @brief   Registry Service API
   @author	Karl Fuerlinger
   @verbatim
   Revision:       $Revision$
   Revision date:  $Date$
   Committed by:   $Author$

   This file is part of the Periscope performance measurement tool.
   See http://www.lrr.in.tum.de/periscope for details.

   Copyright (c) 2005-2011, Technische Universitaet Muenchen, Germany
   See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#include "registry.h"
#include "regxx.h"
#include "protocol.h"

#include "psc_errmsg.h"
#include "psc_config.h"
#include "regsrv.h"
#include "config.h"

RegistryService::RegistryService( std::string host,
                                  int         port,
                                  bool        startReg )
    : MAXQUERY( 8192 ) {
    /// Start the registry server
    if( startReg && port ) {
        start_registry_server( host.c_str(), port );
        sleep( 5 );
        psc_dbgmsg( 2, "RegistryService(): Started.\n" );

        if( psc_config_open() ) {
            char reg_host[ 2000 ];
            psc_config_reghost( reg_host, 2000 );

            reghost_ = reg_host;
            regport_ = psc_config_regport();
            psc_config_close();
        }
        else {
            psc_errmsg( "RegistryService(): Error opening config file, cannot continue.\n" );
            exit( 1 );
        }
    }
    else {
        regport_ = port;
        reghost_ = host;
    }
    reg_ = 0;
}

int RegistryService::add_entry( EntryData& data ) {
    int  entry_id;
    bool havetoclose = false;

    if( !reg_ ) {
        reg_        = open_registry( reghost_.c_str(), regport_ );
        havetoclose = true;
    }

    if( !reg_ ) {
        //psc_errmsg( "Unable to open registry at %s:%d\n",
        //            reghost_.c_str(), regport_ );
        return -1;
    }

    entry_id = registry_create_entry( reg_, data.app.c_str(), data.site.c_str(),
                                      data.mach.c_str(), data.node.c_str(), data.port, data.pid,
                                      data.comp.c_str(), data.tag.c_str() );

    if( entry_id == 0 ) {
        if( havetoclose ) {
            close_registry( reg_ );
            reg_ = 0;
        }
        return -1;
    }

    if( havetoclose ) {
        close_registry( reg_ );
        reg_ = 0;
    }
    return entry_id;
}

int RegistryService::delete_entry( int entry ) {
    int  ret;
    bool havetoclose = false;

    if( !reg_ ) {
        reg_        = open_registry( reghost_.c_str(), regport_ );
        havetoclose = true;
    }
    if( !reg_ ) {
        //psc_errmsg( "Unable to open registry at %s:%d\n",
        //            reghost_.c_str(), regport_ );
        return -1;
    }

    ret = registry_delete_entry( reg_, entry );

    if( !ret ) {
        //psc_errmsg( "Failed to delete registry entry\n" );

        if( havetoclose ) {
            close_registry( reg_ );
            reg_ = 0;
        }
        return -1;
    }

    if( havetoclose ) {
        close_registry( reg_ );
        reg_ = 0;
    }
    return 0;
}

int RegistryService::get_entry( EntryData& data,
                                int        id,
                                bool       withstrings ) {
    int  ret;
    bool havetoclose = false;

    if( !reg_ ) {
        reg_        = open_registry( reghost_.c_str(), regport_ );
        havetoclose = true;
    }
    if( !reg_ ) {
        //psc_errmsg( "Unable to open registry at %s:%d\n",
        //reghost_.c_str(), regport_ );
        return -1;
    }

    char* app, * site, * mach, * node, * comp, * tag;
    int   port, pid;

    app = site = mach = node = comp = tag = 0;

    ret = registry_fillin( reg_, id, &app, &site, &mach, &node, &port, &pid,
                           &comp, &tag );

//  fprintf(stderr, "Get entry %d\n", id);

    if( ret == 0 ) {
        //psc_errmsg( "Error opening entry id=%d\n", id );

        if( havetoclose ) {
            close_registry( reg_ );
            reg_ = 0;
        }
        return -1;
    }

    data.app  = app;
    data.site = site;
    data.mach = mach;
    data.node = node;
    data.port = port;
    data.pid  = pid;
    data.comp = comp;
    data.tag  = tag;
    data.id   = id;

    int  n, j;
    char buf[ 512 ];

    if( withstrings ) {
        n = registry_count_strings( reg_, id );

        for( j = 1; j <= n; j++ ) {
            registry_get_string_at( reg_, id, j, buf, 512 );
            data.strings.push_back( buf );
        }
    }

    if( havetoclose ) {
        close_registry( reg_ );
        reg_ = 0;
    }

    return 0;
}

int RegistryService::query_entries( std::list<EntryData>& result,
                                    EntryData&            query,
                                    bool                  withstrings ) {
    bool havetoclose = false;
    char line[ BUFSIZE ];
    char buf[ BUFSIZE ];
    char s[ BUFSIZE ];
    char n[ BUFSIZE ];
    char m[ BUFSIZE ];
    char a[ BUFSIZE ];
    char c[ BUFSIZE ];
    char t[ BUFSIZE ];
    int  po = 0;
    int  pi = 0;
    int  id = 0;

    if( !reg_ ) {
        reg_        = open_registry( reghost_.c_str(), regport_ );
        havetoclose = true;
    }

    if( !reg_ ) {
        return -1;
    }

    const char* app, * site, * mach, * node, * comp, * tag;
    int         port, pid;

    app  = query.app.size() ? query.app.c_str() : 0;
    site = query.site.size() ? query.site.c_str() : 0;
    mach = query.mach.size() ? query.mach.c_str() : 0;
    node = query.node.size() ? query.node.c_str() : 0;
    comp = query.comp.size() ? query.comp.c_str() : 0;
    tag  = query.tag.size() ? query.tag.c_str() : 0;
    port = ( query.port > 0 ) ? query.port : 0;
    pid  = ( query.pid > 0 ) ? query.pid : 0;

    r_info entries[ MAXQUERY ];

    int nentries;

    sprintf( line, "%s ", CMD_SEARCH );

    if( app ) {
        sprintf( buf, "app=\"%s\" ", app );
        strcat( line, buf );
    }

    if( site ) {
        sprintf( buf, "site=\"%s\" ", site );
        strcat( line, buf );
    }

    if( mach ) {
        sprintf( buf, "mach=\"%s\" ", mach );
        strcat( line, buf );
    }

    if( node ) {
        sprintf( buf, "node=\"%s\" ", node );
        strcat( line, buf );
    }

    if( port > 0 ) {
        sprintf( buf, "port=%d ", port );
        strcat( line, buf );
    }

    if( pid > 0 ) {
        sprintf( buf, "pid=%d ", pid );
        strcat( line, buf );
    }

    if( comp ) {
        sprintf( buf, "comp=\"%s\"", comp );
        strcat( line, buf );
    }

    if( tag ) {
        sprintf( buf, "tag=\"%s\"", tag );
        strcat( line, buf );
    }

    strcat( line, "\n" );

    socket_write_line( reg_->sock_, line );

    socket_read_line( reg_->sock_, buf, BUFSIZE );

    if( !strcmp( buf, MSG_SEARCH_SUCCESS ) ) {
        if( havetoclose ) {
            close_registry( reg_ );
            reg_ = 0;
        }
        return 0;
    }
    socket_read_line( reg_->sock_, buf, BUFSIZE );

    /* no matches found */
    if( !strcmp( buf, STR_END_OF_MULTILINE ) ) {
        if( havetoclose ) {
            close_registry( reg_ );
            reg_ = 0;
        }
        return 0;
    }

    do {
        EntryData queryresult;

        sscanf( buf, STR_ENTRYDATA, &id, a, s, m, n, &po, &pi, c, t );
        //printf("got next  %d, %s, %s, %s, %s, %d, %d, %s, %s\n", id, a, s, m, n, po, pi, c, t);
        queryresult.app  = a;
        queryresult.site = s;
        queryresult.mach = m;
        queryresult.id   = id;
        queryresult.node = n;
        queryresult.port = po;
        queryresult.pid  = pi;
        queryresult.tag  = t;
        queryresult.comp = c;

        result.push_back( queryresult );

        socket_read_line( reg_->sock_, buf, BUFSIZE );
    }
    while( strcmp( buf, STR_END_OF_MULTILINE ) );

    if( havetoclose ) {
        close_registry( reg_ );
        reg_ = 0;
    }
    return 0;
}

int RegistryService::change_entry( EntryData& data,
                                   int        id ) {
    int  ret;
    bool havetoclose = false;

    if( !reg_ ) {
        reg_        = open_registry( reghost_.c_str(), regport_ );
        havetoclose = true;
    }
    if( !reg_ ) {
        //psc_errmsg( "Unable to open registry at %s:%d\n",
        //reghost_.c_str(), regport_ );
        return -1;
    }

    ret = registry_change_entry( reg_, id, data.app.c_str(), data.site.c_str(),
                                 data.mach.c_str(), data.node.c_str(), data.port, data.pid,
                                 data.comp.c_str(), data.tag.c_str() );

    if( ret == 0 ) {
        //psc_errmsg( "Error opening entry id=%d\n", id );

        if( havetoclose ) {
            close_registry( reg_ );
            reg_ = 0;
        }
        return -1;
    }

    if( havetoclose ) {
        close_registry( reg_ );
        reg_ = 0;
    }

    return 0;
}

void RegistryService::start_registry_server( const char* hostname,
                                             int         port ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( HierarchySetup ), "Forking a registry process in regxx.cc...\n" );
    pid_t pid = fork();
    if( pid == 0 ) {
        /*
           RegServ serv;
           ACE_INET_Addr listen_addr(port);
           serv.reactor(ACE_Reactor::instance());

           port = serv.open(listen_addr);
           if (port == -1)
           {
                psc_errmsg("Error starting server on %d\n", port);
                exit(1);
           } else printf("PSC_REGISTRY=%s:%d\n", listen_addr.get_host_name(),
                        listen_addr.get_port_number());
           ACE_Reactor::instance()->run_reactor_event_loop();
         */

        // Executed by the child process
        char cmd[ 100 ], strPort[ 10 ];
        int  retVal;

        psc_dbgmsg( 6, "Trying to start registry service on %s:%d in PID %d\n", hostname, port, pid );
        sprintf( cmd, "psc_regsrv %d", port );
        retVal = system( cmd );

        //sprintf(strPort, "%d", port);
        //retVal = execl("/bin/sh", "sh", "-c", "psc_regsrv", "psc_regsrv", strPort, (char *) 0);
        psc_dbgmsg( 6, "Started registry service on %s:%d\n", hostname, port );

        // retVal = system( cmd );
        // Terminate the child process
        _exit( retVal );        // call kernel level cleanup and not the user level one!
    }
    else if( pid < 0 ) {
        psc_errmsg( "Error forking child process!\n" );
    }
    else {
        registryPID = pid;
        psc_dbgmsg( 6, "Started registry with pid %d\n", registryPID );
    }
}

void RegistryService::stop_registry_server() {
    psc_dbgmsg( 6, "Stopping registry service with pid %d\n", registryPID );

    kill( registryPID, SIGTERM );     // the shell that started the registry
    kill( registryPID + 1, SIGTERM ); // the real registry
}
