/**
   @file	sockets.h
   @brief   Functions for working with sockets header
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

#ifndef SOCKETS_H_INCLUDED
#define SOCKETS_H_INCLUDED

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PSC_10MS_DELAY \
    struct timespec req, rem; \
    req.tv_sec  = 0; \
    req.tv_nsec = 10000000L; \
    nanosleep( &req, &rem );

#ifdef __cplusplus
extern "C" {
#endif


/*
 * creates a TCP socket(), bind()s it to the given port and
 * executes listen(), returs the created socket if successful,
 * -1 otherwise
 */
int socket_server_startup( int port );

/*
 * creates a TCP socket(), bind()s it to the given port and
 * executes listen(),
 *
 * retries for specified retries with specified step
 *
 * returns the created socket if successful and the port in init_port,
 * -1 otherwise
 */
int socket_server_startup_retry( int* init_port,
                                 int  retries,
                                 int  step );


/*
 * clean up
 */
int socket_server_shutdown( int sock );


void socket_close( int sock );



/*
 * waits for a client's connection on the socket
 * specified by sock and returns the a socket through wich
 * to communicate with the new client
 */
int socket_server_accept_client( int sock );


/*
 * used by clients to connecto to a service
 * on hostname:port
 */
int socket_client_connect( char* hostname,
                           int   port );

int socket_client_connect_retry( char* hostname,
                                 int   port,
                                 int   retries );


int socket_blockread( int   sock,
                      char* ptr,
                      int   size );
int socket_read_line( int   sock,
                      char* str,
                      int   maxlen );
void socket_write_line( int         sock,
                        const char* str );


#ifdef __cplusplus
}
#endif

#endif /* SOCKETS_H_INCLUDED */
