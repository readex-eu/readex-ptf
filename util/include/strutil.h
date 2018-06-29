/**
   @file	strutil.h
   @brief	String manipulation routines header
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


#ifndef STRUTIL_H_INCLUDED
#define STRUTIL_H_INCLUDED


typedef struct {
    char* key;
    char* val;
}
kv_pair;


char* mystrndup( const char* str,
                 size_t      len );

static inline size_t skip_ws( const char* str, size_t pos );

static inline size_t next_token( const char* str, size_t pos,
                                 const char* delim, char** token );

size_t make_key_value_pairs( const char* str,
                             kv_pair**   kv );


#endif /* STRUTIL_H_INCLUDED */
