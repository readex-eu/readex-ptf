/**
   @file    autotune_services.cc
   @ingroup Autotune
   @brief   Common functionality used by tuning plugins
   @author  Isaias Compres, Toni Pimenta, Michael Gerndt, Robert Mijakovic
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

#include "autotune_services.h"
#include <boost/algorithm/string/predicate.hpp>

// Should we adapted to Score-P.
/**
 * Extracts tuning parameters.
 */
vector<TuningParameter*>extractTuningParameters( void ) {
    Application&             app = Application::instance();
    vector<TuningParameter*> tps;

    if( app.get_regions().empty() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "[#### AUTOTUNE ####]: No Regions found. Exiting.\n" );
        throw 0;
    }

    TuningParameter*        tp;
    list<Region*>::iterator reg;
    list<Region*>           regions = app.get_regions();
    for( reg = regions.begin(); reg != regions.end(); ++reg ) {
        if( !( *reg )->plugins.empty() ) {
            list<Plugin*>::iterator plugin_iterator;
            list<Plugin*>           plugins = ( *reg )->plugins;
            for( plugin_iterator = plugins.begin(); plugin_iterator != plugins.end(); ++plugin_iterator ) {
                tp = new TuningParameter();
                tp->setId( 0 );
                tp->setName( ( *plugin_iterator )->getTuningActionName() );
                tp->setPluginType( UNKOWN_PLUGIN );
                const runtimeTuningActionType tuningActionType = ( *plugin_iterator )->getTuningActionType();
                //cout << "Tuning action type = " << tuningActionType << endl;
                tp->setRuntimeActionType( tuningActionType );
                tp->setRange( 1, ( *plugin_iterator )->getNumberOfVariants(), 1 );
                Region*      r   = ( *reg );
                Restriction* res = new Restriction();
                res->setRegion( r );
                res->setRegionDefined( true );
                tp->setRestriction( res );
                tps.push_back( tp );
            }
        }
    }

    return tps;
}

void print_loaded_search( int    major,
                          int    minor,
                          string name,
                          string description ) {
    cout << endl << "Search Algorithm: " << name << endl;
    cout << "Version:          " << major << "." << minor << endl;
    cout << "Description:      " << description << endl << endl;
}

void print_loaded_plugin( int    major,
                          int    minor,
                          string name,
                          string description ) {
    cout << endl << "Plugin:           " << name << endl;
    cout << "Version:          " << major << "." << minor << endl;
    cout << "Description:      " << description << endl;
}

list<Region*>extractMPICallRegions( void ) {
    Application& app = Application::instance();
    if( app.get_regions().empty() ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "No regions found. Exiting.\n" );
        throw 0;
    }

    int                     count = 0;
    TuningParameter*        tp;
    list<Region*>::iterator reg;
    list<Region*>           all_regions = app.get_regions();
    list<Region*>           regions;
    for( reg = all_regions.begin(); reg != all_regions.end(); ++reg ) {
        printf( "region: %d; type: %d;\n", count++, ( *reg )->get_type() );
        if( ( *reg )->get_type() == CALL_REGION ) {
            printf( "Found a call region:\n" );
            printf( "Name: %s\n", ( *reg )->get_name().c_str() );

            if( boost::starts_with( ( *reg )->get_name(), "MPI_" ) ) {
                printf( "this is a C mpi call\n" );
                regions.push_back( ( *reg ) );
            }
            else if( boost::starts_with( ( *reg )->get_name(), "mpi_" ) ) {
                printf( "this is a Fortran mpi call\n" );
                regions.push_back( ( *reg ) );
            }
        }
        printf( "\n" );
    }

    return regions;
}

void generate_context_argc_argv( DriverContext* context,
                                 list<string>   parameters,
                                 char*          exec_name ) {
    if( context == NULL ) {
        psc_errmsg( "DriverContext *context == NULL in generate_argc_argv service\n" );
        throw 0;
    }
    context->setArgsFromString( parameters, exec_name );
}
