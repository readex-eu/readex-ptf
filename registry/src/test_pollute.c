/**
   @file	test_pollute.c
   @brief   Test app to fill the registry with garbage
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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


#include "registry.h"
#include "regclient.h"


int main( int   argc,
          char* argv[] ) {
    char      hostname[ 100 ];
    int       port;
    registry* reg;
    int       i;
    int       entry_id;
    int       string_id;

    char strbuf[ 200 ];
    int  len;

    port = 31337;
    gethostname( hostname, 100 );

    for( i = 0; i < 100; i++ ) {
        reg = open_registry( hostname, port );
        if( !reg ) {
            fprintf( stderr, "Cannot open registry at %s:%d\n", hostname, port );
            exit( 1 );
        }

        entry_id = registry_create_entry( reg, "Hybrid LU", "TU 'Muenchen'",
                                          "Infinicluster", hostname, 1, 1, "ENDOMON", "none" );
        if( entry_id == 0 ) {
            fprintf( stderr, "Failed to create registry entry\n" );
        }

        string_id = registry_store_string( reg, entry_id, "asldjfasldj" );
        if( !string_id ) {
            fprintf( stderr, "Failed to add string to registry\n" );
        }
        else {
            fprintf( stderr, "Stored string at entry %d\n", string_id );
        }

        close_registry( reg );
    }

    return 0;
}
