/**
   @file    regsrv_client.cc
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

#include <string>
using std::string;


#include "stringutil.h"

#include "regsrv_client.h"
#include "protocol.h"

/**
 * @brief Brief description
 *
 * @ingroup RegistryServer
 */
int RegServClient::open() {
    const int bufsize = 400;
    char      welcome[ bufsize ];

    sprintf( welcome, MSG_WELCOME, get_hostname().c_str(), get_port(),
             xstr( PERISCOPE_VERSION ), PERISCOPE_BUILD_DATE );
    write_line( sock_, welcome );

    return reactor()->register_handler( this, ACE_Event_Handler::READ_MASK );
}

/**
 * @brief Brief description
 *
 * @ingroup RegistryServer
 */
int RegServClient::handle_input( ACE_HANDLE h ) {
    int       i, ret;
    const int bufsize = 400;
    char      buf[ bufsize ];

    std::string::size_type pos;
    std::string            line;
    std::string            command;

    while( 1 ) {
        ret = read_line( sock_, buf, bufsize );
        if( ret > 0 ) {
            line = buf;
            pos  = get_token( line, 0, " ", command );

            if( !strcasecmp( command.c_str(), CMD_QUIT ) ) {
                on_quit( sock_, line, pos );
                break;
            }
            if( !strcasecmp( command.c_str(), CMD_HELP ) ) {
                on_help( sock_, line, pos );
                continue;
            }

            if( !strcasecmp( command.c_str(), CMD_CREATE ) ) {
                on_create( sock_, line, pos );
                continue;
            }

            if( !strcasecmp( command.c_str(), CMD_SEARCH ) ) {
                on_search( sock_, line, pos );
                continue;
            }

            if( !strcasecmp( command.c_str(), CMD_DELETE ) ||
                !strcasecmp( command.c_str(), CMD_DELETE_SHORT ) ) {
                on_delete( sock_, line, pos );
                continue;
            }

            if( !strcasecmp( command.c_str(), CMD_LIST ) ) {
                on_list( sock_, line, pos );
                continue;
            }

            if( !strcasecmp( command.c_str(), CMD_SHOW ) ) {
                on_show( sock_, line, pos );
                continue;
            }
            // string storage and retrieval
            if( !strcasecmp( command.c_str(), CMD_STR_ADD ) ) {
                on_add_string( sock_, line, pos );
                continue;
            }

            if( !strcasecmp( command.c_str(), CMD_STR_GET ) ) {
                on_get_string( sock_, line, pos );
                continue;
            }

            if( !strcasecmp( command.c_str(), CMD_STR_DEL ) ) {
                on_delete_string( sock_, line, pos );
                continue;
            }

            if( !strcasecmp( command.c_str(), CMD_STR_LIST ) ) {
                on_list_strings( sock_, line, pos );
                continue;
            }

            if( !strcasecmp( command.c_str(), CMD_STR_COUNT ) ) {
                on_count_strings( sock_, line, pos );
                continue;
            }

            if( !strcasecmp( command.c_str(), CMD_CLEAN ) ) {
                on_clean( sock_, line, pos );
                continue;
            }

            if( !strcasecmp( command.c_str(), CMD_CHANGE ) ) {
                on_change( sock_, line, pos );
                continue;
            }
            on_unknown( sock_, line, pos );
        }
        else {
            fprintf( stderr, "Client disconnected unexpectedly\n" );
            break;
        }
    }

    handle_close( get_handle(), ACE_Event_Handler::READ_MASK );
    return 0;
}

/**
 * @brief Brief description
 *
 * @ingroup RegistryServer
 */
int RegServClient::handle_output( ACE_HANDLE h ) {
    return 0;
}

/**
 * @brief Brief description
 *
 * @ingroup RegistryServer
 */
int RegServClient::handle_close( ACE_HANDLE       h,
                                 ACE_Reactor_Mask mask ) {
    if( mask == ACE_Event_Handler::WRITE_MASK ) {
        return 0;
    }

    mask = ACE_Event_Handler::ALL_EVENTS_MASK | ACE_Event_Handler::DONT_CALL;

    reactor()->remove_handler( this, mask );
    sock_.close();

    return 0;
}

/**
 * @brief Brief description
 *
 * @ingroup RegistryServer
 */
void RegServClient::write_line( ACE_SOCK_Stream& sock,
                                char*            buf ) {
    sock.send( buf, strlen( buf ) );
}

/*
   int RegServCleint::my_read(ACE_SOCK_Stream &sock, char *ptr)
   {
   if (read_cnt <= 0) {
    again:
      if ( (read_cnt = read(sock, read_buf, sizeof(read_buf))) < 0) {
        if (errno == EINTR)
          goto again;
        return (-1);
      } else if (read_cnt == 0)
        return (0);
        read_ptr = read_buf;
   }

   read_cnt--;
 * ptr = *read_ptr++;
   return (1);

   }
 */

/**
 * @brief Brief description
 *
 * @ingroup RegistryServer
 */
int RegServClient::read_line( ACE_SOCK_Stream& sock,
                              char*            bufptr,
                              int              len ) {
    /*
       char *bufx = bufptr;
       static char *bp;
       static int cnt = 0;
       static char b[ 1500 ];
       char c;

       while(--len > 0) {
       if (--cnt <= 0) {
        cnt = sock.recv(b, sizeof( b ), 0);
        if (cnt < 0) {
          if ( errno == EINTR )
            continue;
          return -1;
        }
        if ( cnt == 0 )
          return 0;
        bp = b;
       }
       c = *bp++;
     * bufptr++ = c;
       if ( c == '\n' ) {
     * bufptr = '\0';
        return bufptr - bufx;
       }
       }
       errno = EMSGSIZE;
       return -1;
     */

    ssize_t n, rc;
    char    c;

    for( n = 0; n < len; ) {
        rc = sock.recv( &c, 1 );

        switch( rc ) {
        case 1:
            if( c == '\r' ) {
                continue;
            }
            if( c == '\n' ) {
                *bufptr = 0;
                n++;
                return n;
            }

            *bufptr++ = c;
            n++;

            break;
        case 0:
            return n;
        default:
            perror( "read_line" );
            return -1;
        }
    }
    return n;
}

/**
 * @brief Brief description
 *
 * @ingroup RegistryServer
 */
void RegServClient::on_quit( ACE_SOCK_Stream&       sock,
                             std::string&           line,
                             std::string::size_type pos ) {
    char buf[ 400 ];
    sprintf( buf, MSG_QUIT,
             get_hostname().c_str(), get_port() );
    write_line( sock, buf );
}

/**
 * @brief Brief description
 *
 * @ingroup RegistryServer
 */
void RegServClient::on_unknown( ACE_SOCK_Stream&       sock,
                                std::string&           line,
                                std::string::size_type pos ) {
    char   buf[ 400 ];
    string command;

    get_token( line, 0, " ", command );
    sprintf( buf, MSG_UNKNOWN_COMMAND, command.c_str() );
    write_line( sock, buf );
}

/**
 * @brief Brief description
 *
 * @ingroup RegistryServer
 */
void RegServClient::on_help( ACE_SOCK_Stream&       sock,
                             std::string&           line,
                             std::string::size_type pos ) {
    char buf[ 400 ];
    sprintf( buf, MSG_HELP );
    write_line( sock, buf );

    sprintf( buf, "%10s %s\n", CMD_QUIT, DESCR_QUIT );
    write_line( sock, buf );

    sprintf( buf, "%10s %s\n", CMD_HELP, DESCR_HELP );
    write_line( sock, buf );

    sprintf( buf, "%10s %s\n", CMD_CREATE, DESCR_CREATE );
    write_line( sock, buf );

    sprintf( buf, "%10s %s\n", CMD_SEARCH, DESCR_SEARCH );
    write_line( sock, buf );

    sprintf( buf, "%10s %s\n", CMD_CHANGE, DESCR_CHANGE );
    write_line( sock, buf );

    sprintf( buf, "%10s %s\n", CMD_DELETE, DESCR_DELETE );
    write_line( sock, buf );

    sprintf( buf, "%10s %s\n", CMD_LIST, DESCR_LIST );
    write_line( sock, buf );

    sprintf( buf, "%10s %s\n", CMD_SHOW, DESCR_SHOW );
    write_line( sock, buf );

    sprintf( buf, "%10s %s\n", CMD_CLEAN, DESCR_CLEAN );
    write_line( sock, buf );

    sprintf( buf, "%10s %s\n", CMD_STR_ADD, DESCR_STR_ADD );
    write_line( sock, buf );

    sprintf( buf, "%10s %s\n", CMD_STR_GET, DESCR_STR_GET );
    write_line( sock, buf );

    sprintf( buf, "%10s %s\n", CMD_STR_DEL, DESCR_STR_DEL );
    write_line( sock, buf );

    sprintf( buf, "%10s %s\n", CMD_STR_LIST, DESCR_STR_LIST );
    write_line( sock, buf );

    sprintf( buf, "%10s %s\n", CMD_STR_COUNT, DESCR_STR_COUNT );
    write_line( sock, buf );

    sprintf( buf, "%s\n", STR_END_OF_MULTILINE );
    write_line( sock, buf );
}

/**
 * @brief Brief description
 *
 * @ingroup RegistryServer
 */
void RegServClient::on_create( ACE_SOCK_Stream&       sock,
                               std::string&           line,
                               std::string::size_type pos ) {
    char                        buf[ 400 ];
    string                      key, value;
    string                      command;
    std::pair< string, string > mypair;

    RegEntry* entry = new RegEntry();

    do {
        pos   = get_key_value_pair( line, pos, mypair );
        key   = mypair.first;
        value = mypair.second;

        if( key == "" ) {
            continue;
        }

        if( key == "app" ) {
            entry->app = value;
            continue;
        }
        if( key == "site" ) {
            entry->site = value;
            continue;
        }
        if( key == "mach" ) {
            entry->mach = value;
            continue;
        }
        if( key == "node" ) {
            entry->node = value;
            continue;
        }
        if( key == "port" ) {
            entry->port = atoi( value.c_str() );
            continue;
        }
        if( key == "pid" ) {
            entry->pid = atoi( value.c_str() );
            continue;
        }
        if( key == "comp" ) {
            entry->comp = value;
            continue;
        }
        if( key == "tag" ) {
            entry->tag = value;
            continue;
        }

        sprintf( buf, MSG_CREATE_BAD_FORMAT );
        write_line( sock, buf );
        return;
    }
    while( pos != string::npos );

    // check if we have the information we need:
    if( entry->app == "" || entry->site == "" || entry->mach == "" ||
        entry->node == "" || entry->port < 0 ||
        entry->pid == 0 || entry->comp == "" || entry->tag == "" ) {
        sprintf( buf, MSG_CREATE_INCOMPLETE );
        write_line( sock, buf );
        delete entry;
        return;
    }

    int id = serv_->unique_id();
    serv_->set_entry( id, entry );

#ifdef EBUG
    fprintf( stdout, "Added new entry[%d]: %s", id, entry->c_str() );
    fflush( stdout );
#endif

    sprintf( buf, MSG_CREATE_SUCCESS, id );
    write_line( sock, buf );
}

/**
 * @brief Brief description
 *
 * @ingroup RegistryServer
 */
void RegServClient::on_list( ACE_SOCK_Stream&       sock,
                             std::string&           line,
                             std::string::size_type pos ) {
    char buf[ 400 ];

    std::map< int, RegEntry* >::iterator it;
    sprintf( buf, MSG_LIST_SUCCESS, ( int )serv_->reg_data_.size() );
    write_line( sock, buf );

    for( it = serv_->reg_data_.begin(); it != serv_->reg_data_.end(); it++ ) {
        sprintf( buf, STR_ENTRYDATA, it->first,
                 it->second->app.c_str(),
                 it->second->site.c_str(),
                 it->second->mach.c_str(),
                 it->second->node.c_str(),
                 it->second->port,
                 it->second->pid,
                 it->second->comp.c_str(),
                 it->second->tag.c_str() );
        write_line( sock, buf );
    }

    sprintf( buf, "%s\n", STR_END_OF_MULTILINE );
    write_line( sock, buf );
}

/**
 * @brief Brief description
 *
 * @ingroup RegistryServer
 */
void RegServClient::on_show( ACE_SOCK_Stream&       sock,
                             std::string&           line,
                             std::string::size_type pos ) {
    int                                  id = 0;
    string                               tok;
    char                                 buf[ 400 ];
    std::map< int, RegEntry* >::iterator it;

    get_token( line, pos, " ", tok );
    id = atoi( tok.c_str() );

    if( ( it = serv_->reg_data_.find( id ) ) == serv_->reg_data_.end() ) {
        sprintf( buf, MSG_SHOW_NOT_FOUND );
        write_line( sock, buf );
    }
    else {
        sprintf( buf, MSG_SHOW_SUCCESS, id );
        write_line( sock, buf );
        // write entry
        sprintf( buf, STR_ENTRYDATA, it->first,
                 it->second->app.c_str(),
                 it->second->site.c_str(),
                 it->second->mach.c_str(),
                 it->second->node.c_str(),
                 it->second->port,
                 it->second->pid,
                 it->second->comp.c_str(),
                 it->second->tag.c_str() );
        write_line( sock, buf );
    }
}

/**
 * @brief Brief description
 *
 * @ingroup RegistryServer
 */
void RegServClient::on_delete( ACE_SOCK_Stream&       sock,
                               std::string&           line,
                               std::string::size_type pos ) {
    int                                  id = 0;
    string                               tok;
    char                                 buf[ 400 ];
    std::map< int, RegEntry* >::iterator it;

    get_token( line, pos, " ", tok );
    id = atoi( tok.c_str() );

    if( ( it = serv_->reg_data_.find( id ) ) == serv_->reg_data_.end() ) {
        sprintf( buf, MSG_DELETE_NOT_FOUND );
        write_line( sock, buf );
    }
    else {
#ifdef DEBUG
        fprintf( stdout, "Deleted entry[%d]: %s", it->first, ( ( RegEntry* )it->second )->c_str() );
        fflush( stdout );
#endif

        delete it->second;
        serv_->reg_data_.erase( it );
        sprintf( buf, MSG_DELETE_SUCCESS, id );
        write_line( sock, buf );
    }
}

/**
 * @brief Brief description
 *
 * @ingroup RegistryServer
 */
void RegServClient::on_add_string( ACE_SOCK_Stream&       sock,
                                   std::string&           line,
                                   std::string::size_type pos ) {
    string                               tok;
    char                                 buf[ 400 ];
    int                                  id, strid;
    std::map< int, RegEntry* >::iterator it;

    pos = get_token( line, pos, " ", tok );
    id  = atoi( tok.c_str() );

    pos = strskip_ws( line, pos );
    if( pos == string::npos ) {
        sprintf( buf, MSG_ADDSTR_BAD_FORMAT );
        write_line( sock, buf );
        return;
    }

    if( line[ pos ] == '"' ) {
        pos = get_token( line, pos, "\"", tok );
    }
    else {
        pos = get_token( line, pos, " ", tok );
    }

    if( id == 0 || tok == "" ) {
        sprintf( buf, MSG_ADDSTR_BAD_FORMAT );
        write_line( sock, buf );
        return;
    }

    if( ( it = serv_->reg_data_.find( id ) ) == serv_->reg_data_.end() ) {
        sprintf( buf, MSG_ADDSTR_NOT_FOUND );
        write_line( sock, buf );
    }
    else {
        strid = it->second->store_string( tok );
        sprintf( buf, MSG_ADDSTR_SUCCESS, strid );
        write_line( sock, buf );
    }
}

/**
 * @brief Brief description
 *
 * @ingroup RegistryServer
 */
void RegServClient::on_count_strings( ACE_SOCK_Stream&       sock,
                                      std::string&           line,
                                      std::string::size_type pos ) {
    string                               tok;
    char                                 buf[ 200 ];
    int                                  id, strcount;
    std::map< int, RegEntry* >::iterator it;

    pos = get_token( line, pos, " ", tok );
    id  = atoi( tok.c_str() );

    if( id == 0 ) {
        sprintf( buf, MSG_COUNTSTR_BAD_FORMAT );
        write_line( sock, buf );
        return;
    }

    if( ( it = serv_->reg_data_.find( id ) ) == serv_->reg_data_.end() ) {
        sprintf( buf, MSG_COUNTSTR_NOT_FOUND );
        write_line( sock, buf );
    }
    else {
        strcount = it->second->count_strings();
        sprintf( buf, MSG_COUNTSTR_SUCCESS, strcount, id );
        write_line( sock, buf );
    }
}

/**
 * @brief Brief description
 *
 * @ingroup RegistryServer
 */
void RegServClient::on_get_string( ACE_SOCK_Stream&       sock,
                                   std::string&           line,
                                   std::string::size_type pos ) {
    string                               tok;
    char                                 buf[ 200 ];
    int                                  id, strid;
    string                               str;
    std::map< int, RegEntry* >::iterator it;

    pos = get_token( line, pos, " ", tok );
    id  = atoi( tok.c_str() );

    pos   = get_token( line, pos, " ", tok );
    strid = atoi( tok.c_str() );
    if( id == 0 || strid == 0 ) {
        sprintf( buf, MSG_GETSTR_BAD_FORMAT );
        write_line( sock, buf );
        return;
    }

    if( ( it = serv_->reg_data_.find( id ) ) == serv_->reg_data_.end() ) {
        sprintf( buf, MSG_GETSTR_ENTRY_NOT_FOUND );
        write_line( sock, buf );
        return;
    }

    if( !it->second->get_string_at( strid, str ) ) {
        sprintf( buf, MSG_GETSTR_STRING_NOT_FOUND );
        write_line( sock, buf );
        return;
    }

    sprintf( buf, MSG_GETSTR_SUCCESS, strid, id, ( int )strlen( str.c_str() ) );
    write_line( sock, buf );

    sprintf( buf, "%s\n", str.c_str() );
    write_line( sock, buf );
}

/**
 * @brief Brief description
 *
 * @ingroup RegistryServer
 */
void RegServClient::on_delete_string( ACE_SOCK_Stream&       sock,
                                      std::string&           line,
                                      std::string::size_type pos ) {
    string                               tok;
    char                                 buf[ 200 ];
    int                                  id, strid;
    string                               str;
    std::map< int, RegEntry* >::iterator it;

    pos = get_token( line, pos, " ", tok );
    id  = atoi( tok.c_str() );

    pos   = get_token( line, pos, " ", tok );
    strid = atoi( tok.c_str() );
    if( id == 0 || strid == 0 ) {
        sprintf( buf, MSG_DELSTR_BAD_FORMAT );
        write_line( sock, buf );
        return;
    }

    if( ( it = serv_->reg_data_.find( id ) ) == serv_->reg_data_.end() ) {
        sprintf( buf, MSG_DELSTR_ENTRY_NOT_FOUND );
        write_line( sock, buf );
        return;
    }

    if( !it->second->delete_string_at( strid ) ) {
        sprintf( buf, MSG_DELSTR_STRING_NOT_FOUND );
        write_line( sock, buf );
        return;
    }

    sprintf( buf, MSG_DELSTR_SUCCESS, strid, id );
    write_line( sock, buf );
}

/**
 * @brief Brief description
 *
 * @ingroup RegistryServer
 */
void RegServClient::on_search( ACE_SOCK_Stream&       sock,
                               std::string&           line,
                               std::string::size_type pos ) {
    char                                 buf[ 200 ];
    string                               key, value;
    string                               command;
    std::pair< string, string >          mypair;
    std::map< int, RegEntry* >::iterator it;
    RegEntry                             entry;

    entry.pid  = 0;
    entry.port = 0;

    do {
        pos   = get_key_value_pair( line, pos, mypair );
        key   = mypair.first;
        value = mypair.second;

        if( key == "" ) {
            continue;
        }

        if( key == "app" ) {
            entry.app = value;
            continue;
        }
        if( key == "site" ) {
            entry.site = value;
            continue;
        }
        if( key == "mach" ) {
            entry.mach = value;
            continue;
        }
        if( key == "node" ) {
            entry.node = value;
            continue;
        }
        if( key == "port" ) {
            entry.port = atoi( value.c_str() );
            continue;
        }
        if( key == "pid" ) {
            entry.pid = atoi( value.c_str() );
            continue;
        }
        if( key == "comp" ) {
            entry.comp = value;
            continue;
        }
        if( key == "tag" ) {
            entry.tag = value;
            continue;
        }

        sprintf( buf, MSG_SEARCH_BAD_FORMAT );
        write_line( sock, buf );
        return;
    }
    while( pos != string::npos );


    sprintf( buf, MSG_SEARCH_SUCCESS );
    write_line( sock, buf );

    for( it = serv_->reg_data_.begin(); it != serv_->reg_data_.end(); it++ ) {
        if( ( entry.app == ""  || entry.app == it->second->app ) &&
            ( entry.site == "" || entry.site == it->second->site ) &&
            ( entry.mach == "" || entry.mach == it->second->mach ) &&
            ( entry.node == "" || entry.node == it->second->node ) &&
            ( entry.port == 0  || entry.port == it->second->port ) &&
            ( entry.pid == 0   || entry.pid == it->second->pid ) &&
            ( entry.comp == "" || entry.comp == it->second->comp ) &&
            ( entry.tag == ""  || entry.tag == it->second->tag ) ) {
            sprintf( buf, STR_ENTRYDATA, it->first,
                     it->second->app.c_str(),
                     it->second->site.c_str(),
                     it->second->mach.c_str(),
                     it->second->node.c_str(),
                     it->second->port,
                     it->second->pid,
                     it->second->comp.c_str(),
                     it->second->tag.c_str() );
            write_line( sock, buf );
        }
    }

    sprintf( buf, "%s\n", STR_END_OF_MULTILINE );
    write_line( sock, buf );
}

/**
 * @brief Brief description
 *
 * @ingroup RegistryServer
 */
void RegServClient::on_list_strings( ACE_SOCK_Stream&       sock,
                                     std::string&           line,
                                     std::string::size_type pos ) {
    string                               tok;
    char                                 buf[ 200 ];
    int                                  i, id, strcount;
    std::map< int, RegEntry* >::iterator it;
    std::string                          str;

    pos = get_token( line, pos, " ", tok );
    id  = atoi( tok.c_str() );

    if( id == 0 ) {
        sprintf( buf, MSG_LISTSTR_BAD_FORMAT );
        write_line( sock, buf );
        return;
    }

    if( ( it = serv_->reg_data_.find( id ) ) == serv_->reg_data_.end() ) {
        sprintf( buf, MSG_LISTSTR_NOT_FOUND );
        write_line( sock, buf );
        return;
    }
    else {
        strcount = it->second->count_strings();
        sprintf( buf, MSG_LISTSTR_SUCCESS, strcount, id );
        write_line( sock, buf );
    }

    for( i = 0; i < strcount; i++ ) {
        it->second->get_string_at( i + 1, str );
        sprintf( buf, "%s\n", str.c_str() );
        write_line( sock, buf );
    }

    sprintf( buf, "%s\n", STR_END_OF_MULTILINE );
    write_line( sock, buf );
}

/**
 * @brief Brief description
 *
 * @ingroup RegistryServer
 */
void RegServClient::on_clean( ACE_SOCK_Stream&       sock,
                              std::string&           line,
                              std::string::size_type pos ) {
    char buf[ 200 ];
    int  count = 0;

    std::map< int, RegEntry* >::iterator it;
    for( it = serv_->reg_data_.begin(); it != serv_->reg_data_.end(); it++ ) {
        if( it->second ) {
            delete it->second;
        }

        count++;
        serv_->reg_data_.erase( it );
    }

#ifdef DEBUG
    fprintf( stdout, "Registry cleaned successfully!\n" );
    fflush( stdout );
#endif

    sprintf( buf, MSG_CLEAN_SUCCESS, count );
    write_line( sock, buf );
}

/**
 * @brief Brief description
 *
 * @ingroup RegistryServer
 */
void RegServClient::on_change( ACE_SOCK_Stream&       sock,
                               std::string&           line,
                               std::string::size_type pos ) {
    int                         id = 0;
    string                      tok;
    char                        buf[ 200 ];
    string                      key, value;
    std::pair< string, string > mypair;

    std::map< int, RegEntry* >::iterator it;
    pos = get_token( line, pos, " ", tok );
    id  = atoi( tok.c_str() );


    if( ( it = serv_->reg_data_.find( id ) ) == serv_->reg_data_.end() ) {
        sprintf( buf, MSG_CHANGE_ENTRY_NOT_FOUND );
        write_line( sock, buf );
        return;
    }

    RegEntry entry = *( it->second );


    do {
        pos   = strskip_ws( line, pos );
        pos   = get_key_value_pair( line, pos, mypair );
        key   = mypair.first;
        value = mypair.second;


        if( key == "" ) {
            continue;
        }

        if( key == "app" ) {
            entry.app = value;
            continue;
        }
        if( key == "site" ) {
            entry.site = value;
            continue;
        }
        if( key == "mach" ) {
            entry.mach = value;
            continue;
        }
        if( key == "node" ) {
            entry.node = value;
            continue;
        }
        if( key == "port" ) {
            entry.port = atoi( value.c_str() );
            continue;
        }
        if( key == "pid" ) {
            entry.pid = atoi( value.c_str() );
            continue;
        }
        if( key == "comp" ) {
            entry.comp = value;
            continue;
        }
        if( key == "tag" ) {
            entry.tag = value;
            continue;
        }

        sprintf( buf, MSG_CHANGE_BAD_FORMAT );
        write_line( sock, buf );
        return;
    }
    while( pos != string::npos );

    if( entry.app == "" || entry.site == "" || entry.mach == "" ||
        entry.node == "" || entry.port < 0 ||
        entry.pid == 0 || entry.comp == "" || entry.tag == "" ) {
        sprintf( buf, MSG_CHANGE_INCOMPLETE );
        write_line( sock, buf );
        return;
    }

    ( *it->second ) = entry;

    sprintf( buf, MSG_CHANGE_SUCCESS, id );
    write_line( sock, buf );
}
