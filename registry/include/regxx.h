/**
   @file	regxx.h
   @brief   Registry Service API header
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

#ifndef REGXX_H_INCLUDED
#define REGXX_H_INCLUDED

#include <string>
#include <list>

#include "registry.h"
#include "regsrv.h"

struct EntryData {
    int         id;
    std::string app;             // the application name
    std::string site;            // the name of the site
    std::string mach;            // the name of the machine
    std::string node;            // the name of the node
    int         port;            // the port number
    int         pid;             // the process id
    std::string comp;            // the component
    std::string tag;

    std::list<std::string> strings;

    EntryData() {
        id = port = pid = 0;
    }
};

class RegistryService {
private:
    std::string reghost_;
    int         regport_;
    const int   MAXQUERY;
    registry*   reg_;
    RegServ*    regServ;
    int         registryPID;

public:
    RegistryService( std::string host = 0,
                     int         port = 0,
                     bool        startReg = false );

    const char* get_reghost() {
        return reghost_.c_str();
    }

    int get_regport() {
        return regport_;
    }

    // return assigned id on success, -1 on failure
    int add_entry( EntryData& data );

    /// detete the specified entry id
    int delete_entry( int entry );

    /// get the entry by the given id
    int get_entry( EntryData& data,
                   int        id,
                   bool       withstrings = false );

    /// change the entry given by id
    int change_entry( EntryData& data,
                      int        id );

    int query_entries( std::list<EntryData>& entries,
                       EntryData&            query,
                       bool                  withstrings = false );

    /// Start a new registry server
    void start_registry_server( const char* hostname,
                                int         port );

    /// Stop the currently running registry server
    void stop_registry_server();
};


#endif // REGXX_H_INCLUDED
