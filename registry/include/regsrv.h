/**
   @file	regsrv.h
   @brief   Registry server header
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

#ifndef REGSRV_H_INCLUDED
#define REGSRV_H_INCLUDED

#include <string>
#include <vector>
#include <list>
#include <map>

#include "ace/Reactor.h"
#include "ace/SOCK_Acceptor.h"

class RegEntry {
private:
    std::vector<std::string> strings_;

public:
    RegEntry() {
    }

    std::string app;
    std::string site;
    std::string mach;
    std::string node;
    int         port;
    int         pid;
    std::string comp;
    std::string tag;

    const char* c_str() {
        char buf[ 512 ];
        sprintf( buf, "app=\"%s\" "
                 "site=\"%s\" "
                 "mach=\"%s\" "
                 "node=\"%s\" "
                 "pid=%d "
                 "comp=\"%s\" "
                 "tag=\"%s\"\n", app.c_str(), site.c_str(), mach.c_str(),
                 node.c_str(), pid, comp.c_str(), tag.c_str() );
        return strdup( buf );
    }

    int store_string( std::string& str ) {
        strings_.push_back( str );
        return strings_.size();
    }

    int get_string_at( int id, std::string& str ) {
        if( id < 1 || id > strings_.size() ) {
            return 0;
        }

        str = strings_[ id - 1 ];
        return 1;
    }

    int delete_string_at( int id ) {
        if( id < 1 || id > strings_.size() ) {
            return 0;
        }

        strings_.erase( strings_.begin() + ( id - 1 ) );
        return 1;
    }

    int count_strings() {
        return strings_.size();
    }
};

class RegServ : public ACE_Event_Handler {
public:
    std::map<int, RegEntry*> reg_data_;

private:
    std::string hostname_;
    int         port_;
    int         next_id_;

protected:
    ACE_SOCK_Acceptor acceptor_;

public:
    RegServ() {
        next_id_ = 1;
    }

    ~RegServ() {
        handle_close( ACE_INVALID_HANDLE, 0 );
    }

    int open( ACE_INET_Addr& addr );

    ACE_HANDLE get_handle() const {
        return this->acceptor_.get_handle();
    }

    int handle_input( ACE_HANDLE fd = ACE_INVALID_HANDLE );

    int handle_close( ACE_HANDLE       h,
                      ACE_Reactor_Mask m );

    RegEntry* get_entry( int id ) {
        return reg_data_[ id ];
    }

    void set_entry( int id, RegEntry* entry ) {
        reg_data_[ id ] = entry;
    }

    int unique_id() {
        return next_id_++;
    }

    int get_port() {
        return port_;
    }

    std::string get_hostname() {
        return hostname_;
    }
};

#endif // REGSRV_H_INCLUDED
