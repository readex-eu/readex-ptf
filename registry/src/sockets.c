/**
   @file	sockets.c
   @brief   Functions for working with sockets
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

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include "config.h"


#include "sockets.h"

/**
 * @brief Start socket server
 *
 * @ingroup RegistryServer
 */
int socket_server_startup( int port ) {
    int sock;
    int yes = 1;

    struct sockaddr_in my_addr;  /* my address information */

    /*
       create a new socket
       socket() returns positive integer on success
     */
    if( ( sock = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 ) {
        perror( "Could not oppen the socket: socket_server_startup::socket()" );
        return -1;
    }

    if( setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( int ) ) < 0 ) {
        perror( "Could not oppen the socket: socket_server_startup::setsockopt()" );
        return -1;
    }

    my_addr.sin_family      = AF_INET;        /* host byte order */
    my_addr.sin_port        = htons( port );  /* short, network byte order */
    my_addr.sin_addr.s_addr = INADDR_ANY;     /* automatically fill with my IP */
    memset( &( my_addr.sin_zero ), '\0', 8 ); /* zero the rest of the struct */

    if( bind( sock, ( struct sockaddr* )&my_addr, sizeof( struct sockaddr ) ) < 0 ) {
        perror( "Could not oppen the socket: socket_server_startup::bind()" );
        return -1;
    }

    if( listen( sock, 1 ) < 0 ) {
        perror( "Could not oppen the socket: socket_server_startup::listen()" );
        return -1;
    }

    return sock;
}


/**
 * @brief Retries to start socket server
 *
 * @ingroup RegistryServer
 */
int socket_server_startup_retry( int* init_port,
                                 int  retries,
                                 int  step ) {
    int sock;
    int yes  = 1;
    int stat = -1;
    int port;

    struct sockaddr_in my_addr; /* my address information */

    /*
       create a new socket
       socket() returns positive interger on success
     */

    for( port = *init_port; port <= *init_port + retries * step && stat == -1; port = port + step ) {
        stat = 0;

        if( ( sock = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 ) {
            if( port + step > *init_port + retries * step ) {
                perror( "socket_server_startup::socket()" );
            }
            stat = -1;
        }
        else {
            if( setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( int ) ) < 0 ) {
                if( port + step > *init_port + retries * step ) {
                    perror( "socket_server_startup::setsockopt()" );
                }
                stat = -1;
            }
            else {
                my_addr.sin_family      = AF_INET;        /* host byte order */
                my_addr.sin_port        = htons( port );  /* short, network byte order */
                my_addr.sin_addr.s_addr = INADDR_ANY;     /* automatically fill with my IP */
                memset( &( my_addr.sin_zero ), '\0', 8 ); /* zero the rest of the struct */

                if( bind( sock, ( struct sockaddr* )&my_addr, sizeof( struct sockaddr ) ) < 0 ) {
                    if( port + step > *init_port + retries * step ) {
                        perror( "socket_server_startup::bind()" );
                    }
                    stat = -1;
                }
                else {
                    if( listen( sock, 1 ) < 0 ) {
                        if( port + step > *init_port + retries * step ) {
                            perror( "socket_server_startup::listen()" );
                        }
                        stat = -1;
                    }
                    else {
                        break;
                    }
                }
            }
        }
    }

    if( stat == -1 ) {
        return -1;
    }
    else {
        *init_port = port;
        return sock;
    }
}

/**
 * @brief Shutdowns socket server
 *
 * @ingroup RegistryServer
 */
int socket_server_shutdown( int sock ) {
    return close( sock );
}

/**
 * @brief Closes socket
 *
 * @ingroup RegistryServer
 */
void socket_close( int sock ) {
    close( sock );
}

/**
 * @brief Accepts socket connection from client
 *
 * @ingroup RegistryServer
 */
int socket_server_accept_client( int sock ) {
    int newsock;

    struct sockaddr_in client_addr; /* client's address information */

    //size_t sin_size;
    socklen_t sin_size;

    sin_size = sizeof( struct sockaddr_in );

    if( ( newsock = accept( sock, ( struct sockaddr* )&client_addr, &sin_size ) ) < 0 ) {
        perror( "socket_server_accept_client::accept()" );
        return -1;
    }

    return newsock;
}

/**
 * @brief Connects to the client
 *
 * @ingroup RegistryServer
 */
int socket_client_connect( char* hostname,
                           int   port ) {
    int                sock;
    struct sockaddr_in pin;
    struct hostent*    hp;

#ifndef _BGP_PORT
    if( ( hp = gethostbyname( hostname ) ) == 0 ) {
        perror( "socket_client_connect::gethostbyname()" );
        return -1;
    }

    /* fill in the socket structure with host information */
    memset( &pin, 0, sizeof( pin ) );
    pin.sin_family      = AF_INET;
    pin.sin_addr.s_addr = ( ( struct in_addr* )( hp->h_addr ) )->s_addr;
    pin.sin_port        = htons( port );

    /* grab an Internet domain socket */
    if( ( sock = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
        perror( "socket_client_connect::socket()" );
        return -1;
    }

    /* connect to PORT on HOST */
    if( connect( sock, ( struct sockaddr* )&pin, sizeof( pin ) ) == -1 ) {
        perror( "socket_client_connect::connect()" );
        return -1;
    }
#else

    sock = socket( AF_INET, SOCK_STREAM, 0 );

    if( sock < 0 ) {
        error( "ERROR opening socket" );
    }

    hp = gethostbyname( hostname );
    if( hp == NULL ) {
        fprintf( stderr, "ERROR, no such host\n" );
        exit( 0 );
    }

    bzero( ( char* )&pin, sizeof( pin ) );

    pin.sin_family = AF_INET;

    bcopy( ( char* )hp->h_addr, ( char* )&pin.sin_addr.s_addr, hp->h_length );

    pin.sin_port = htons( port );


    if( connect( sock, ( struct sockaddr* )&pin, sizeof( pin ) ) < 0 ) {
        perror( "socket_client_connect::connect()" );
        return -1;
    }
#endif

    return sock;
}

/**
 * @brief Retries to reconnect to the client
 *
 * @ingroup RegistryServer
 */
int socket_client_connect_retry( char* hostname,
                                 int   port,
                                 int   retries ) {
    int                sock;
    struct sockaddr_in pin;
    struct hostent*    hp;
    int                success, i;

    success = -1;
    for( i = 0; i < retries && success == -1; i++ ) {
        // TODO make sure this retry call is not necessary at all by inverting the connection setup order -IC
        // TODO may need to insert a 1 second sleep here, when not running in fast mode -IC
        PSC_10MS_DELAY;

        success = 0;
        if( ( hp = gethostbyname( hostname ) ) == 0 ) {
            if( i == retries - 1 ) {
                perror( "socket_client_connect::gethostbyname()" );
            }
            success = -1;
        }
        else {
            /* fill in the socket structure with host information */
            memset( &pin, 0, sizeof( pin ) );
            pin.sin_family      = AF_INET;
            pin.sin_addr.s_addr = ( ( struct in_addr* )( hp->h_addr ) )->s_addr;
            pin.sin_port        = htons( port );

            /* grab an Internet domain socket */
            if( ( sock = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
                if( i == retries - 1 ) {
                    perror( "socket_client_connect::socket()" );
                }
                success = -1;
            }
            else {
                /* connect to PORT on HOST */
                if( connect( sock, ( struct sockaddr* )&pin, sizeof( pin ) ) == -1 ) {
                    if( i == retries - 1 ) {
                        perror( "socket_client_connect::connect()" );
                    }
                    success = -1;
                }
            }
        }
    }
    if( success == -1 ) {
        sock = -1;
    }
    return sock;
}

/**
 * @brief Writes line to the socket
 *
 * @ingroup RegistryServer
 */
void socket_write_line( int         sock,
                        const char* str ) {
    int retVal = write( sock, str, strlen( str ) );
}


static int   read_cnt;
static char* read_ptr;
static char  read_buf[ 1000 ];

/**
 * @brief Reads from the socket
 *
 * @ingroup RegistryServer
 */
int socket_my_read( int   fd,
                    char* ptr ) {
    if( read_cnt <= 0 ) {
again:
        if( ( read_cnt = read( fd, read_buf, sizeof( read_buf ) ) ) < 0 ) {
            if( errno == EINTR ) {
                goto again;
            }
            return -1;
        }
        else if( read_cnt == 0 ) {
            return 0;
        }
        read_ptr = read_buf;
    }

    read_cnt--;
    *ptr = *read_ptr++;
    return 1;
}

/**
 * @brief Reads block from the socket
 *
 * @ingroup RegistryServer
 */
int socket_blockread( int   sock,
                      char* ptr,
                      int   size ) {
    int  n, rc;
    char c;

    for( n = 1; n <= size; n++ ) {
        rc = socket_my_read( sock, &c );
        if( rc == 1 ) {
            *ptr++ = c;
        }
        else if( rc == 0 ) {
            // TODO may have to add a 1 second sleep when operating in slow mode
            // TODO remove this delay once the startup order is reversed for fast mode -IC
            PSC_10MS_DELAY;
        }
        else {
            return -1;
        }
    }
    return n - 1;
}

/**
 * @brief Reads line from the socket
 *
 * @ingroup RegistryServer
 */
int socket_read_line( int   sock,
                      char* str,
                      int   maxlen ) {
    int  n, rc;
    char c, * ptr;

    ptr = str;
    for( n = 1; n < maxlen; n++ ) {
        if( ( rc = socket_my_read( sock, &c ) ) == 1 ) {
            if( c  == '\n' ) {
                break;  /* newline is stored, like fgets() */
            }
            *ptr++ = c;
        }
        else if( rc == 0 ) {
            *ptr = 0;
            return n - 1; /* EOF, n - 1 bytes were read */
        }
        else {
            return -1;    /* error, errno set by read() */
        }
    }

    *ptr = 0;    /* null terminate like fgets() */

    return n;


    /*
       22 ssize_t
       23 readline(int fd, void *vptr, size_t maxlen)
       24 {
       25     ssize_t n, rc;
       26     char    c, *ptr;

       27     ptr = vptr;
       28     for (n = 1; n < maxlen; n++) {
       29         if ((rc = my_read(fd, &c)) == 1) {
       30             *ptr++ = c;
       31             if (c  == '\n')
       32                 break;          // newline is stored, like fgets()
       33 } else if (rc == 0) {
       34             *ptr = 0;
       35             return (n - 1);     // EOF, n - 1 bytes were read
       36 } else
       37             return (-1);        // error, errno set by read()
       38 }

       39     *ptr  = 0;                  // null terminate like fgets()
       40     return (n);
       41 }

       42 ssize_t
       43 readlinebuf(void **vptrptr)
       44 {
       45     if (read_cnt)
       46         *vptrptr = read_ptr;
       47     return (read_cnt);
       48 }
     */

    /*
       int  n, rc;
       char c;

       for ( n = 0; n < maxlen; )
       {
           rc = read( sock, &c, 1 );

           switch( rc )
           case 1:
               if ( c == '\r' ) // telnet  hack
                   continue;
               if ( c == '\n' )
               {
       *str = 0;
                   n++;
                   return n;
               }

       *str++ = c;
               n++;
               break;
           case 0:
               return n;
           default:
               perror( "read_line" );
               return -1;
       }
          return n;
     */
}
