/**
   @file    DriverContext.cc
   @ingroup Autotune
   @brief   Tuning Parameter
   @author  Isaias Compres
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include "DriverContext.h"
map<string, loadable_entry<ISearchAlgorithm> > DriverContext::searchalgorithm;
map<string, loadable_entry<IPlugin> >          DriverContext::plugin;

DriverContext::DriverContext() {
    argv             = NULL;
    search_step      = 0;
    tuning_step      = 0;
    experiment_count = 0;
}

DriverContext::DriverContext( const DriverContext& context ) {
    app_name = context.app_name;
    argc     = context.argc;
    argv     = ( char** )malloc( ( context.argc + 1 ) * sizeof ( *this->argv ) );
    for( int i = 0; i < argc; i++ ) {
        int size  = strlen( context.argv[ i ] ) + 1;
        argv[ i ] = ( char* )malloc( size * sizeof( char ) );
        strcpy( argv[ i ], context.argv[ i ] );
    }
    instrumented                  = context.instrumented;
    ompnumthreads                 = context.ompnumthreads;
    mpinumprocs                   = context.mpinumprocs;
    search_step      = context.search_step;
    tuning_step      = context.tuning_step;
    experiment_count = context.experiment_count;
}

DriverContext::~DriverContext() {
    if( argv ) {
        free( argv );
    }
}


void DriverContext::register_timer( void   ( * call_back )( double current_time ),
                                    double delay ) {
    double      now = psc_wall_time();
    timer_entry entry;
    entry.call_back    = call_back;
    entry.delay        = delay;
    entry.initial_time = now;
    timer[ call_back ] = entry;
}

double DriverContext::get_elapsed_time( void ( * call_back )( double current_time ) ) {
    return psc_wall_time() - timer[ call_back ].initial_time;
}

map<void ( * )( double ), timer_entry>* DriverContext::get_timers( void ) {
    return &timer;
}

void DriverContext::loadSearchAlgorithm( string  libname,
                                         int*    major,
                                         int*    minor,
                                         string* name,
                                         string* description ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "about to load %s search\n", libname.c_str() );
    char search_file[ 1000 ];
    sprintf( search_file, "%s/%s/libptf%s.so", PERISCOPE_SEARCH_ALGORITHMS_DIRECTORY, libname.c_str(), libname.c_str() );
    loadComponent<ISearchAlgorithm>( &searchalgorithm, search_file, libname, major, minor, name, description );
}

void DriverContext::loadPlugin( string  libname,
                                int*    major,
                                int*    minor,
                                string* name,
                                string* description ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "about to load %s plugin\n", libname.c_str() );
    char plugin_file[ 1000 ];
    sprintf( plugin_file, "%s/%s/libptf%s.so", PERISCOPE_PLUGINS_DIRECTORY, libname.c_str(), libname.c_str() );
    loadComponent<IPlugin>( &plugin, plugin_file, libname, major, minor, name, description );
}

ISearchAlgorithm* DriverContext::getSearchAlgorithmInstance( string libname ) {
    return getComponentInstance<ISearchAlgorithm>( &searchalgorithm, libname, "getSearchAlgorithmInstance" );
}

IPlugin* DriverContext::getTuningPluginInstance( string libname ) {
    return getComponentInstance<IPlugin>( &plugin, libname, "getPluginInstance" );
}

void DriverContext::unloadSearchAlgorithms( void ) {
    unloadComponents<ISearchAlgorithm>( &searchalgorithm );
}

void DriverContext::unloadPlugins( void ) {
    unloadComponents<IPlugin>( &plugin );
}

string DriverContext::toString( int    indent,
                                string indentation_character ) {
    string base_indentation;
    for( int i = 0; i < indent; i++ ) {
        base_indentation.append( indentation_character );
    }
    stringstream temp;
    temp << base_indentation << "argc:\n";
    temp << base_indentation << indentation_character << argc << endl;
    temp << base_indentation << "argv:\n";
    for( int i = 0; i < argc; i++ ) {
        temp << base_indentation << indentation_character << string( argv[ i ] ) << "\n";
    }

    return temp.str().c_str();
}
