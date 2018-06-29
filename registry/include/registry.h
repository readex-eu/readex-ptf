/**
   @file    registry.h
   @brief   Registry API header
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

// Create a Registry group
/// @defgroup RegistryServer Registry Server


#ifndef REGISTRY_H_INCLUDED
#define REGISTRY_H_INCLUDED

typedef struct {
    int   id;
    char* app;    // the application name
    char* site;   // the name of the site
    char* mach;   // the name of the machine
    char* node;   // the name of the node
    int   port;   // the port number
    int   pid;    // the process id
    char* comp;   // the component
    char* tag;
}r_info;


#define BUFSIZE 512



#ifdef __cplusplus
extern "C"
{
#endif

#include "sockets.h"

typedef struct {
    char* hostname_;
    int   port_;
    int   sock_;
}
registry;


/*
 * open connection to the registry server,
 * initialize data-structures
 */
registry* open_registry( const char* hostname,
                         int         port );


/*
 * close the connection to the registry server
 * returns 1 on success, 0 on failure
 */
int close_registry( registry* reg );


/*
 * create an entry in the registry which doesn't contain data initially,
 * returns the entry's positive ID on success 0 on failure
 */
int registry_create_entry( registry*   reg,
                           const char* app,
                           const char* site,
                           const char* mach,
                           const char* node,
                           int         port,
                           int         pid,
                           const char* comp,
                           const char* tag );


/*
 * change an entry in the registry which doesn't contain data initially,
 * returns the entry's positive ID on success 0 on failure
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
                           const char* tag );

/*
 * opens an existing entry
 * returns the entry's positive ID on success 0 on failure
 */
int registry_open_entry( registry*   reg,
                         const char* app,
                         const char* site,
                         const char* mach,
                         const char* node,
                         int         port,
                         int         pid,
                         const char* comp,
                         const char* tag );


/*
 *
 *
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
                     char**    tag );


int registry_query( registry*   reg,
                    const char* app,
                    const char* site,
                    const char* mach,
                    const char* node,
                    int         port,
                    int         pid,
                    const char* comp,
                    const char* tag,
                    r_info*     entries );



/*
 * deletes the entry from the registry (including all data)
 * returns 1 on success, 0 on failure
 */
int registry_delete_entry( registry* reg,
                           int       entid );


/*
 * stores a copy of the string pointed to by text in the registry at entry
 * returns the positive string id on success, 0 on failure
 */
int registry_store_string( registry*   reg,
                           int         entid,
                           const char* text );


/*
 * returns the number of strings contained in the entry,
 * returns -1 on error
 */
int registry_count_strings( registry* reg,
                            int       entid );


/*
 * up to maxlen characters of the string entry at 'pos' are copied to
 * returns the number of copied characters on success, -1 on error
 */
int registry_get_string_at( registry* reg,
                            int       entid,
                            int       pos,
                            char*     text,
                            int       maxlen );


/* missing: query routine(s) */




#ifdef __cplusplus
}
#endif


#endif /* REGISTRY_H_INCLUDED */
