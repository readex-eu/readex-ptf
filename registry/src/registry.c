/**
   @file    registry.c
   @brief   Registry API
   @author  Karl Fuerlinger
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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "registry.h"
#include "protocol.h"
#include "strutil.h"


/**
 * @brief Opens connection to the registry server and initialize data structures
 *
 * @ingroup RegistryServer
 */
registry* open_registry( const char* hostname,
                         int         port ) {
    registry* reg = ( registry* )malloc( sizeof( registry ) );
    char      buf[ BUFSIZE ];


    reg->hostname_ = strdup( hostname );
    reg->port_     = port;

    reg->sock_ = socket_client_connect_retry( reg->hostname_, reg->port_, 10 );

    if( reg->sock_ < 0 ) {
        if( reg->hostname_ ) {
            free( reg->hostname_ );
        }
        free( reg );
        return 0;
    }

    socket_read_line( reg->sock_, buf, BUFSIZE );

    if( strncmp( buf, PREFIX_SUCCESS, strlen( PREFIX_SUCCESS ) ) ) {
        /* something went wrong */
        socket_close( reg->sock_ );
        if( reg->hostname_ ) {
            free( reg->hostname_ );
        }
        free( reg );
        return 0;
    }

    return reg;
}

/**
 * @brief Closes the connection to the registry server
 *
 * @ingroup RegistryServer
 */
int close_registry( registry* reg ) {
    int  ret;
    char buf[ BUFSIZE ];

    sprintf( buf, "%s\n", CMD_QUIT );
    socket_write_line( reg->sock_, buf );

    socket_read_line( reg->sock_, buf, BUFSIZE );
    if( strncmp( buf, PREFIX_SUCCESS, strlen( PREFIX_SUCCESS ) ) ) {
        ret = 0;
    }
    else {
        ret = 1;
    }

    socket_close( reg->sock_ );
    if( reg->hostname_ ) {
        free( reg->hostname_ );
    }
    free( reg );

    return ret;
}

/**
 * @brief Creates an registry entry
 *
 * @ingroup RegistryServer
 */
int registry_create_entry( registry*   reg,
                           const char* app,
                           const char* site,
                           const char* mach,
                           const char* node,
                           int         port,
                           int         pid,
                           const char* comp,
                           const char* tag ) {
    int  id = 0;
    char buf[ BUFSIZE ];
    int  n = 0;

    sprintf( buf, "%s "
             "app=\"%s\" "
             "site=\"%s\" "
             "mach=\"%s\" "
             "node=\"%s\" "
             "port=%d "
             "pid=%d "
             "comp=\"%s\" "
             "tag=\"%s\"\n",
             CMD_CREATE, app, site, mach, node, port, pid, comp, tag );
    socket_write_line( reg->sock_, buf );
    socket_read_line( reg->sock_, buf, BUFSIZE );
    n = sscanf( buf, MSG_CREATE_SUCCESS, &id );
    if( n < 1 ) {
        return 0;
    }

    return id;
}

/**
 * @brief Changes specified registry entry
 *
 * @ingroup RegistryServer
 */
int registry_change_entry( registry*   reg,
                           int         entid,
                           const char* app,
                           const char* site,
                           const char* mach,
                           const char* node,
                           int         port,
                           int         pid,
                           const char* comp,
                           const char* tag ) {
    char buf[ BUFSIZE ];
    char line[ BUFSIZE ];
    int  id, n;

    sprintf( line, "%s %d ", CMD_CHANGE, entid );

    if( app ) {
        sprintf( buf, "app=\"%s\" ", app );
        strcat( line, buf );
    }

    if( site ) {
        sprintf( buf, "site=\"%s\" ", site );
        strcat( line, buf );
    }

    if( mach ) {
        sprintf( buf, "mach=\"%s\" ", mach );
        strcat( line, buf );
    }

    if( node ) {
        sprintf( buf, "node=\"%s\" ", node );
        strcat( line, buf );
    }

    if( port > 0 ) {
        sprintf( buf, "port=%d ", port );
        strcat( line, buf );
    }

    if( pid > 0 ) {
        sprintf( buf, "pid=%d ", pid );
        strcat( line, buf );
    }

    if( comp ) {
        sprintf( buf, "comp=\"%s\"", comp );
        strcat( line, buf );
    }

    if( tag ) {
        sprintf( buf, "tag=\"%s\"", tag );
        strcat( line, buf );
    }


    strcat( line, "\n" );

    socket_write_line( reg->sock_, line );
    socket_read_line( reg->sock_, line, BUFSIZE );

    n = sscanf( buf, MSG_CREATE_SUCCESS, &id );
    if( n < 1 ) {
        return 0;
    }

    return id;
}

/**
 * @brief Deletes specified registry entry
 *
 * @ingroup RegistryServer
 */
int registry_delete_entry( registry* reg,
                           int       entid ) {
    char buf[ BUFSIZE ];
    int  id;
    int  n = 0;

    sprintf( buf, "%s %d\n", CMD_DELETE, entid );
    socket_write_line( reg->sock_, buf );

    socket_read_line( reg->sock_, buf, BUFSIZE );
    n = sscanf( buf, MSG_DELETE_SUCCESS, &id );

    if( n < 1 ) {
        return 0;
    }
    return id;
}

/**
 * @brief Opens specific registry entry
 *
 * @ingroup RegistryServer
 */
int registry_open_entry( registry*   reg,
                         const char* app,
                         const char* site,
                         const char* mach,
                         const char* node,
                         int         port,
                         int         pid,
                         const char* comp,
                         const char* tag ) {
    char buf[ BUFSIZE ];

    char s[ BUFSIZE ];
    char n[ BUFSIZE ];
    char m[ BUFSIZE ];
    char a[ BUFSIZE ];
    char c[ BUFSIZE ];
    char t[ BUFSIZE ];
    int  po;
    int  pi;

    int id;

    if( pid < 0 ) {
        pid = 0;
    }

    if( port < 0 ) {
        port = 0;
    }

    sprintf( buf, "%s "
             "app=\"%s\" "
             "site=\"%s\" "
             "mach=\"%s\" "
             "node=\"%s\" "
             "port=%d "
             "pid=%d"
             "\n",
             CMD_SEARCH, app, site, mach, node, port, pid );

    socket_write_line( reg->sock_, buf );

    socket_read_line( reg->sock_, buf, BUFSIZE );
    if( !strcmp( buf, MSG_SEARCH_SUCCESS ) ) {
        return 0;
    }
    socket_read_line( reg->sock_, buf, BUFSIZE );

    /* no matches found */
    if( !strcmp( buf, STR_END_OF_MULTILINE ) ) {
        return 0;
    }

    sscanf( buf, STR_ENTRYDATA, &id, a, s, n, m, &po, &pi, c, t );

    do {
        socket_read_line( reg->sock_, buf, BUFSIZE );
    }
    while( strcmp( buf, STR_END_OF_MULTILINE ) );

    return id;
}

/**
 * @brief Opens specific registry entry
 *
 * @ingroup RegistryServer
 */
int registry_query( registry*   reg,
                    const char* app,
                    const char* site,
                    const char* mach,
                    const char* node,
                    int         port,
                    int         pid,
                    const char* comp,
                    const char* tag,
                    r_info*     entries ) {
    char line[ BUFSIZE ];
    char buf[ BUFSIZE ];

    char s[ BUFSIZE ];
    char n[ BUFSIZE ];
    char m[ BUFSIZE ];
    char a[ BUFSIZE ];
    char c[ BUFSIZE ];
    char t[ BUFSIZE ];
    int  po = 0;
    int  pi = 0;

    int i;
    int id      = 0;
    int num_ids = 0;
    int idtable[ 8192 ];

    *s = 0;
    *n = 0;
    *m = 0;
    *a = 0;
    *c = 0;
    *t = 0;



    sprintf( line, "%s ", CMD_SEARCH );

    if( app ) {
        sprintf( buf, "app=\"%s\" ", app );
        strcat( line, buf );
    }

    if( site ) {
        sprintf( buf, "site=\"%s\" ", site );
        strcat( line, buf );
    }

    if( mach ) {
        sprintf( buf, "mach=\"%s\" ", mach );
        strcat( line, buf );
    }

    if( node ) {
        sprintf( buf, "node=\"%s\" ", node );
        strcat( line, buf );
    }

    if( port > 0 ) {
        sprintf( buf, "port=%d ", port );
        strcat( line, buf );
    }

    if( pid > 0 ) {
        sprintf( buf, "pid=%d ", pid );
        strcat( line, buf );
    }

    if( comp ) {
        sprintf( buf, "comp=\"%s\"", comp );
        strcat( line, buf );
    }

    if( tag ) {
        sprintf( buf, "tag=\"%s\"", tag );
        strcat( line, buf );
    }

    strcat( line, "\n" );

    socket_write_line( reg->sock_, line );

    socket_read_line( reg->sock_, buf, BUFSIZE );

    if( !strcmp( buf, MSG_SEARCH_SUCCESS ) ) {
        return 0;
    }
    socket_read_line( reg->sock_, buf, BUFSIZE );


    /* no matches found */
    if( !strcmp( buf, STR_END_OF_MULTILINE ) ) {
        return 0;
    }


    num_ids = 0;
    do {
        printf( "get next %d:>%s<\n", num_ids, buf );
        sscanf( buf, STR_ENTRYDATA, &id, a, s, m, n, &po, &pi, c, t );
        printf( "got next %d, %d, %s, %s, %s, %s, %d, %d, %s, %s\n", num_ids, id, a, s, m, n, po, pi, c, t );
        entries[ num_ids ].id   = id;
        entries[ num_ids ].port = po;
        entries[ num_ids ].pid  = pi;
        entries[ num_ids ].app  = ( char* )calloc( 1, strlen( a ) + 1 );
        strcpy( entries[ num_ids ].app, a );
        entries[ num_ids ].site = ( char* )calloc( 1, strlen( s ) + 1 );
        strcpy( entries[ num_ids ].site, s );
        entries[ num_ids ].mach = ( char* )calloc( 1, strlen( m ) + 1 );
        strcpy( entries[ num_ids ].mach, m );
        entries[ num_ids ].node = ( char* )calloc( 1, strlen( n ) + 1 );
        strcpy( entries[ num_ids ].node, n );
        entries[ num_ids ].comp = ( char* )calloc( 1, strlen( c ) + 1 );
        strcpy( entries[ num_ids ].comp, c );
        entries[ num_ids ].tag = ( char* )calloc( 1, strlen( t ) + 1 );
        strcpy( entries[ num_ids ].tag, t );


//    entries[num_ids].app="hallo";
//    printf("Assignments done\n");
//    entries[num_ids].site=s;
//    entries[num_ids].mach=m;
//    entries[num_ids].node=n;
//    entries[num_ids].port=po;
//    entries[num_ids].pid=pi;
//    entries[num_ids].comp=c;
//    entries[num_ids].tag=t;
        printf( "Assignments done\n" );

        num_ids++;

        socket_read_line( reg->sock_, buf, BUFSIZE );
    }
    while( strcmp( buf, STR_END_OF_MULTILINE ) );
//
//  (*entries)=malloc(sizeof(int)*num_ids);
//
//  for(i=0; i<num_ids; i++) {
//    (*entries)[i]=idtable[i];
//  }

    return num_ids;
}

/**
 * @brief Stores string in registry entry
 *
 * @ingroup RegistryServer
 */
int registry_store_string( registry*   reg,
                           int         entid,
                           const char* text ) {
    char buf[ BUFSIZE ];
    int  id;
    int  n = 0;

    sprintf( buf, "%s %d \"%s\"\n", CMD_STR_ADD, entid, text );
    socket_write_line( reg->sock_, buf );

    socket_read_line( reg->sock_, buf, BUFSIZE );
    n = sscanf( buf, MSG_ADDSTR_SUCCESS, &id );
    if( n < 1 ) {
        return 0;
    }

    return id;
}

/**
 * @brief Returns number of strings in specific registry entry
 *
 * @ingroup RegistryServer
 */
int registry_count_strings( registry* reg,
                            int       entid ) {
    char buf[ BUFSIZE ];
    int  id, strcount;
    int  n = 0;

    sprintf( buf, "%s %d\n", CMD_STR_COUNT, entid );
    socket_write_line( reg->sock_, buf );

    socket_read_line( reg->sock_, buf, BUFSIZE );
    n = sscanf( buf, MSG_COUNTSTR_SUCCESS, &strcount, &id );

    if( n < 2 ) {
        return -1;
    }

    return strcount;
}

/**
 * @brief Fetches length of string in specific registry entry
 *
 * @ingroup RegistryServer
 */
int registry_get_string_at( registry* reg,
                            int       entid,
                            int       pos,
                            char*     text,
                            int       maxlen ) {
    char buf[ BUFSIZE ];
    int  strid, id, size, n = 0, len;

    sprintf( buf, "%s %d %d\n", CMD_STR_GET, entid, pos );
    socket_write_line( reg->sock_, buf );

    socket_read_line( reg->sock_, buf, BUFSIZE );
    n = sscanf( buf, MSG_GETSTR_SUCCESS, &strid, &id, &size );
    if( n < 3 ) {
        return -1;
    }

    len = socket_read_line( reg->sock_, text, maxlen );
    return len;
}

/**
 * @brief Fills registry
 *
 * @ingroup RegistryServer
 */
int registry_fillin( registry* reg,
                     int       entid,
                     char**    app,
                     char**    site,
                     char**    mach,
                     char**    node,
                     int*      port,
                     int*      pid,
                     char**    comp,
                     char**    tag ) {
    char buf[ BUFSIZE ];
    char line[ BUFSIZE ];
    int  id;
    int  p;
    int  i, l = 0;

    char* a, * n, * m, * s, * c, * t;
    int   po;
    int   pi;

    kv_pair* kv;

    sprintf( line, "%s %d\n", CMD_SHOW, entid );
    socket_write_line( reg->sock_, line );

//  fprintf(stderr, "Socket is %d\n", reg->sock_);

    socket_read_line( reg->sock_, line, BUFSIZE );
    l = sscanf( line, MSG_SHOW_SUCCESS, &id );
    if( l < 1 ) {
//    fprintf(stderr, "%s\n", line);
        return 0;
    }

    socket_read_line( reg->sock_, line, BUFSIZE );
    l = sscanf( line, "ID %d %[^\n]", &id, buf );

    a  = s = n = m = c = 0;
    po = -1;
    pi = -1;

    l = make_key_value_pairs( buf, &kv );

    for( i = 0; i < l; i++ ) {
//    fprintf(stderr,  " %s %s \n", kv[i].key, kv[i].val);

        if( !strcmp( kv[ i ].key, "app" ) ) {
            a = kv[ i ].val;
        }

        if( !strcmp( kv[ i ].key, "site" ) ) {
            s = kv[ i ].val;
        }

        if( !strcmp( kv[ i ].key, "node" ) ) {
            n = kv[ i ].val;
        }

        if( !strcmp( kv[ i ].key, "mach" ) ) {
            m = kv[ i ].val;
        }

        if( !strcmp( kv[ i ].key, "comp" ) ) {
            c = kv[ i ].val;
        }

        if( !strcmp( kv[ i ].key, "tag" ) ) {
            t = kv[ i ].val;
        }

        if( !strcmp( kv[ i ].key, "port" ) ) {
            po = atoi( kv[ i ].val );
        }

        if( !strcmp( kv[ i ].key, "pid" ) ) {
            pi = atoi( kv[ i ].val );
        }
    }

//  printf("%s %s %s %s %s %d %d\n", a, s, n, m, c, po, pi);

    if( a ) {
        ( *app ) = strdup( a );
    }

    if( n ) {
        ( *node ) = strdup( n );
    }

    if( m ) {
        ( *mach ) = strdup( m );
    }

    if( c ) {
        ( *comp ) = strdup( c );
    }

    if( s ) {
        ( *site ) = strdup( s );
    }

    if( t ) {
        ( *tag ) = strdup( t );
    }

    if( po > -1 ) {
        ( *port ) = po;
    }

    if( pi > -1 ) {
        ( *pid ) = pi;
    }

    return id;
}
