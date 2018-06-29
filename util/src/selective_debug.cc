/**
   @file    psc_errmsg.c
   @brief   Periscope debugging and logging output
   @author  Karl Fuerlinger
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
#include <cstdlib>
#include <string>
#include <map>
#include <list>

#include "selective_debug.h"

using std::string;


std::map <int, int> dbgLevels;

void print_dbgLevelsDefs() {
    for( int i = 0; i < number_dbgLevels; i++ ) {
        fprintf( stderr, "   %s\n", dbgLevelsDefs[ i ] );
    }
}

static int dbgLevelToInt( const string& level ) {
    for( int i = 0; i < number_dbgLevels; i++ ) {
        if( level.compare( dbgLevelsDefs[ i ] ) == 0 ) {
            return 1000 + i;
        }
    }

    psc_errmsg( "ERROR: %s is no debug level. Supported are: \n", level.c_str() );
    print_dbgLevelsDefs();
    return -1;
}

static void add_dbgLevel( const string& level ) {
    int i = dbgLevelToInt( level );

    if( i < 0 ) {
        return;
    }
    dbgLevels[ i ] = 1;

    if( i == AutotuneAll ) {  // if AutotuneAll, set all autotune levels and frontend state-machine
        psc_dbgmsg( 1, "Enabling selective debug for AutotuneAll\n" );
        dbgLevels[ dbgLevelToInt( "AutotunePlugins" ) ]       = 1;
        dbgLevels[ dbgLevelToInt( "AutotuneSearch" ) ]        = 1;
        dbgLevels[ dbgLevelToInt( "AutotuneAgentStrategy" ) ] = 1;
        dbgLevels[ dbgLevelToInt( "FrontendStateMachines" ) ] = 1;
    }

    else if( i == QualityExpressions ) {
        setenv( "QUALITY_EXPRESSION_VERBOSITY", "1", 1 );
    }
    else if( i == QualityExpressionEvents ) {
        setenv( "QUALITY_EXPRESSION_VERBOSITY", "2", 1 );
    }
}

void handle_dbgLevelList( const string& dbgList ) {
    size_t start, stop;
    string level;

    start = 0;
    while( start != string::npos ) {
        stop  = dbgList.find( ",", start );
        level = dbgList.substr( start, stop - start );
        if( stop == string::npos ) {
            start = stop;
        }
        else {
            start = stop + 1;
        }
        psc_dbgmsg( 6, "Found request for debugging level: %s\n", level.c_str() );
        add_dbgLevel( level );
//		stop=dbgList.find(",",start);
    }
}

extern "C" int active_dbgLevel( int i ) {
    if( i < 0 ) {
        return 0;
    }

    if( dbgLevels.find( i ) == dbgLevels.end() ) {
        return 0;
    }
    else {
        return 1;
    }
}
