/**
   @file    test_query.cc
   @brief   Test app to query all entries from the registry
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

#include "regxx.h"
#include <stdio.h>


int main( int   argc,
          char* argv[] ) {
    RegistryService regsrv( argv[ 1 ], atoi( argv[ 2 ] ) );

    RegistryService::EntryData                        query;
    std::list< RegistryService::EntryData >           result;
    std::list< RegistryService::EntryData >::iterator entryit;

    /*
       regsrv.foo( result, query );
     */

    if( regsrv.query_entries( result, query ) == -1 ) {
        fprintf( stderr, "Error querying registry for application\n" );
        exit( 1 );
    }

    for( entryit = result.begin(); entryit != result.end(); entryit++ ) {
        printf( "%d\n", entryit->id );
    }
}
