/**
   @file    startup.h
   @deprecated
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

#ifndef STARTUP_H_
#define STARTUP_H_

#include <string>
#include <list>
#include <unistd.h>
#include "psc_peerserver.h"

extern "C" { int gethostname( char*  name,
                              size_t len ); }

#include "../registry/registry.h"

class StartupManager {
    // TODO append _ to all private members in this class
    std::string  reg_host;
    unsigned int reg_port;
    std::string  my_host;

    registry*  reg;
    PeerServer peerserver_;

public:

    StartupManager( char* reghost, unsigned int regport ) {
        char myhostname[ 200 ];
        ::gethostname( myhostname, 200 );

        reg_host = std::string( reghost );
        reg_port = regport;
        reg      = 0;
        my_host  = myhostname;
    }

    int start_peerserver();

    int startup_first( char* exec );

    int startup_local( std::list<int>& ids );


    registry* open_registry();

    void close_registry();
};

#endif // STARTUP_H_
