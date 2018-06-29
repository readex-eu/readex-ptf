/**
   @file    DriverContext.h
   @ingroup Autotune
   @brief   Tuning Parameter header
   @author  Isaias Compres
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2015, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
    @endverbatim
 */

#ifndef DRIVERCONTEXT_H_
#define DRIVERCONTEXT_H_

#include "ISearchAlgorithm.h"
#include "IPlugin.h"
#include "config.h"
#include <dlfcn.h>
#include <getopt.h>

class IPlugin;
class ISearchAlgorithm;

#include <time.h>
#include <string>
using namespace std;

template <typename T>
struct loadable_entry {
    int    load_count;
    int    reference_count;
    void*  libhandle;
    T*     (* getInstance)( void );
    int    (* getVersionMajor)( void );
    int    (* getVersionMinor)( void );
    string (* getName)( void );
    string (* getShortSummary)( void );
};

struct timer_entry {
    timer_entry() {
    }

    timer_entry( void( *call_back_in )( double current_time ), double delay_in,
                 double initial_time_in ) : call_back( call_back_in ), delay( delay_in ), initial_time( initial_time_in ) {
    }

    void   (* call_back)( double current_time );
    double delay;
    double initial_time;
};

class DriverContext {
private:
    static map<string, loadable_entry<ISearchAlgorithm> > searchalgorithm;
    static map<string, loadable_entry<IPlugin> >          plugin;

    map<void ( * )( double ), timer_entry> timer;
    string                                 app_name;
    int                                    argc;
    char**                                 argv;
    bool                                   instrumented;
    int                                    ompnumthreads;
    int                                    mpinumprocs;
public:
    int search_step;
    int tuning_step;
    int experiment_count;

    DriverContext();

    DriverContext( const DriverContext& context );

    virtual ~DriverContext();

    string toString( int    indent,
                     string indentation_character );

    void register_timer( void   ( * call_back )( double current_time ),
                         double delay );

    double get_elapsed_time( void ( * call_back )( double current_time ) );

    map<void ( * )( double ), timer_entry>* get_timers();

    void loadSearchAlgorithm( string  search_name,
                              int*    major,
                              int*    minor,
                              string* name,
                              string* description );

    ISearchAlgorithm* getSearchAlgorithmInstance( string search_name );

    void unloadSearchAlgorithms( void );

    void loadPlugin( string  plugin_name,
                     int*    major,
                     int*    minor,
                     string* name,
                     string* description );

    IPlugin* getTuningPluginInstance( string plugin_name );

    void unloadPlugins( void );

    void setArgsFromString( list<string> parameters, char* exec_name ) {
        this->argc = parameters.size() + 1;
        this->argv = ( char** )malloc( ( this->argc + 1 ) * sizeof *this->argv );
        int size = strlen( exec_name ) + 1;
        this->argv[ 0 ] = ( char* )malloc( size * sizeof( char ) );
        strcpy( this->argv[ 0 ], exec_name );
        if( !parameters.empty() ) {
            std::list<string>::const_iterator iterator;
            int                               i = 1;
            for( iterator = parameters.begin(); iterator != parameters.end(); iterator++ ) {
                size            = strlen( ( *iterator ).c_str() ) + 1;
                this->argv[ i ] = ( char* )malloc( size * sizeof( char ) );
                strcpy( this->argv[ i ], ( *iterator ).c_str() );
                i++;
            }
        }
    }

    char** getArgv() {
        return argv;
    }

    int getArgc() {
        return argc;
    }

    void setApplicationName( string path ) {
        app_name = path;
    }

    string getApplicationName() {
        return app_name;
    }

    void setApplInstrumented( bool is_instrumented ) {
        instrumented = is_instrumented;
    }

    bool applInstrumented() {
        return instrumented;
    }

    bool applUninstrumented() {
        return !instrumented;
    }

    void setOmpnumthreads( int threads ) {
        ompnumthreads = threads;
    }

    int getOmpnumthreads() {
        return ompnumthreads;
    }

    void setMPINumProcs( int processes ) {
        mpinumprocs = processes;
    }

    int getMPINumProcs() {
        return mpinumprocs;
    }

    template <typename T>
    void loadComponent( map<string, loadable_entry<T> >* entries,
                        char*                            location,
                        string                           libname,
                        int*                             major,
                        int*                             minor,
                        string*                          name,
                        string*                          description ) {
        if( entries->find( libname ) == entries->end() ) {        // not yet loaded
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                        "Component \"%s\" not yet loaded; loading...\n", libname.c_str() );

            dlerror();
            ( *entries )[ libname ].libhandle = dlopen( location, RTLD_LAZY | RTLD_LOCAL );

            char* error;
            if( ( error = dlerror() ) != NULL ) {
                psc_errmsg( "%s\n", error );
                throw 0;
            }
            if( !( *entries )[ libname ].libhandle ) {
                perror( "could not load component\n" );
                throw 0;
            }

            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "about to load %s.getVersionMajor\n", libname.c_str() );
            dlerror();
            ( *entries )[ libname ].getVersionMajor = ( int ( * )( void ) )dlsym( ( *entries )[ libname ].libhandle,
                                                                                  "getVersionMajor" );
            if( ( error = dlerror() ) != NULL ) {
                psc_errmsg( "%s\n", error );
                throw 0;
            }
            if( !( ( *entries )[ libname ].getVersionMajor ) ) {
                perror( "could not load getVersionMajor\n" );
                throw 0;
            }
            *major = ( *( *entries )[ libname ].getVersionMajor )();

            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "about to load %s.getVersionMinor\n", libname.c_str() );
            dlerror();
            ( *entries )[ libname ].getVersionMinor = ( int ( * )( void ) )dlsym( ( *entries )[ libname ].libhandle,
                                                                                  "getVersionMinor" );
            if( ( error = dlerror() ) != NULL ) {
                psc_errmsg( "%s\n", error );
                throw 0;
            }
            if( !( ( *entries )[ libname ].getVersionMinor ) ) {
                perror( "could not load getVersionMinor\n" );
                throw 0;
            }
            *minor = ( *( *entries )[ libname ].getVersionMinor )();

            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "about to load %s.getName\n", libname.c_str() );
            dlerror();
            ( *entries )[ libname ].getName = ( string ( * )( void ) )dlsym( ( *entries )[ libname ].libhandle, "getName" );
            if( ( error = dlerror() ) != NULL ) {
                psc_errmsg( "%s\n", error );
                throw 0;
            }
            if( !( ( *entries )[ libname ].getName ) ) {
                perror( "could not load getName\n" );
                throw 0;
            }
            *name = ( *( *entries )[ libname ].getName )();

            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "about to load %s.getShortSummary\n", libname.c_str() );
            dlerror();
            ( *entries )[ libname ].getShortSummary = ( string ( * )( void ) )dlsym( ( *entries )[ libname ].libhandle, "getShortSummary" );
            if( ( error = dlerror() ) != NULL ) {
                psc_errmsg( "%s\n", error );
                throw 0;
            }
            if( !( ( *entries )[ libname ].getShortSummary ) ) {
                perror( "could not load getShortSummary\n" );
                throw 0;
            }
            *description                            = ( *( *entries )[ libname ].getShortSummary )();
            ( *entries )[ libname ].getInstance     = NULL;
            ( *entries )[ libname ].load_count      = 1;
            ( *entries )[ libname ].reference_count = 0;
        }
        else {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ),
                        "Component \"%s\" was already loaded; incrementing its load count...\n", libname.c_str() );
            ( *entries )[ libname ].load_count++;
        }
    }

    template <typename T>
    T* getComponentInstance( map<string, loadable_entry<T> >* entries,
                             string                           libname,
                             string                           method ) {
        char* error;
        if( ( *entries )[ libname ].getInstance == NULL ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "about to load %s.%s...\n", libname.c_str(), method.c_str() );
            dlerror();
            ( *entries )[ libname ].getInstance = ( T * ( * )( void ) )dlsym( ( *entries )[ libname ].libhandle,
                                                                              method.c_str() );
            if( ( error = dlerror() ) != NULL ) {
                psc_errmsg( "%s\n", error );
                throw 0;
            }
            if( !( ( *entries )[ libname ].getInstance ) ) {
                psc_errmsg( "could not load %s\n", method.c_str() );
                throw 0;
            }
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "%s.%s loaded.\n", libname.c_str(), method.c_str() );
        }
        else {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "%s.%s was already loaded.\n", libname.c_str(), method.c_str() );
        }

        ( *entries )[ libname ].reference_count++;
        return ( *( *entries )[ libname ].getInstance )();
    }

    template <typename T>
    void unloadComponents( map<string, loadable_entry<T> >* entries ) {
        typename map<string, loadable_entry<T> >::iterator iter;
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Attempting to unload %d components.\n", entries->size() );
        for( iter = entries->begin(); iter != entries->end(); iter++ ) {
            if( iter->second.load_count < 0 ) {
                psc_errmsg( "Negative (%d) load count for component: %s.\nUnrecoverable error, aborting!\n",
                            iter->second.load_count, iter->first.c_str() );
                throw 0;
            }
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Component %s has been loaded %d times.\n",
                        iter->first.c_str(), iter->second.load_count );
            if( --iter->second.load_count == 0 ) {
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Component %s's load count reached zero; unloading...\n",
                            iter->first.c_str() );
                unloadComponent( iter->second.libhandle );
                entries->erase( iter );
            }
            else {
                psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "Component %s's load count was decremented to %d.\n",
                            iter->first.c_str(), iter->second.load_count );
            }
        }
    }

    void unloadComponent( void* handle ) {
        if( handle ) {
            if( dlclose( handle ) ) {
                psc_errmsg( "ERROR: non-zero return code in dlclose\n" );
            }
        }
        else {
            psc_errmsg( "ERROR: attepnt to unload NULL handle.\n" );
        }
    }
};

#endif /* DRIVERCONTEXT_H_ */
