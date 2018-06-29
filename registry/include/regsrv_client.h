/**
   @file	regsrv_client.h
   @brief   Registry Server-Client Pattern
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

#ifndef REGSRV_CLIENT_H_INCLUDED
#define REGSRV_CLIENT_H_INCLUDED

#include "ace/Reactor.h"
#include "ace/SOCK_Stream.h"
#include "regsrv.h"

class RegServClient : public ACE_Event_Handler {
private:
    RegServ* serv_;

protected:
    ACE_SOCK_Stream sock_;


public:
    RegServClient( RegServ* s ) {
        serv_ = s;
    }

    ACE_SOCK_Stream& peer() {
        return sock_;
    }

    int open();

    ACE_HANDLE get_handle() const {
        return sock_.get_handle();
    }

    int handle_input( ACE_HANDLE fd = ACE_INVALID_HANDLE );

    int handle_output( ACE_HANDLE fd = ACE_INVALID_HANDLE );

    int handle_close( ACE_HANDLE       h,
                      ACE_Reactor_Mask mask );


    std::string get_hostname() {
        return serv_->get_hostname();
    }

    int get_port() {
        return serv_->get_port();
    }

    void write_line( ACE_SOCK_Stream& sock,
                     char*            buf );
    int read_line( ACE_SOCK_Stream& sock,
                   char*            buf,
                   int              maxlen );


    //
    // basic commands
    //

    void on_quit( ACE_SOCK_Stream&       sock,
                  std::string&           line,
                  std::string::size_type pos );

    void on_help( ACE_SOCK_Stream&       sock,
                  std::string&           line,
                  std::string::size_type pos );

    void on_unknown( ACE_SOCK_Stream&       sock,
                     std::string&           line,
                     std::string::size_type pos );

    void on_create( ACE_SOCK_Stream&       sock,
                    std::string&           line,
                    std::string::size_type pos );

    void on_search( ACE_SOCK_Stream&       sock,
                    std::string&           line,
                    std::string::size_type pos );

    void on_delete( ACE_SOCK_Stream&       sock,
                    std::string&           line,
                    std::string::size_type pos );

    void on_list( ACE_SOCK_Stream&       sock,
                  std::string&           line,
                  std::string::size_type pos );

    void on_show( ACE_SOCK_Stream&       sock,
                  std::string&           line,
                  std::string::size_type pos );

    void on_clean( ACE_SOCK_Stream&       sock,
                   std::string&           line,
                   std::string::size_type pos );

    void on_change( ACE_SOCK_Stream&       sock,
                    std::string&           line,
                    std::string::size_type pos );

    //
    // string storage and retrieval
    //
    void on_add_string( ACE_SOCK_Stream&       sock,
                        std::string&           line,
                        std::string::size_type pos );

    void on_get_string( ACE_SOCK_Stream&       sock,
                        std::string&           line,
                        std::string::size_type pos );

    void on_delete_string( ACE_SOCK_Stream&       sock,
                           std::string&           line,
                           std::string::size_type pos );

    void on_list_strings( ACE_SOCK_Stream&       sock,
                          std::string&           line,
                          std::string::size_type pos );

    void on_count_strings( ACE_SOCK_Stream&       sock,
                           std::string&           line,
                           std::string::size_type pos );
};

#endif // REGSRV_CLIENT_H_INCLUDED
