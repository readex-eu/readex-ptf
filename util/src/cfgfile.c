/**
   @file	cfgfile.c
   @brief   Access functions for a config file
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cfgfile.h"
#include "strutil.h"

/* initial number of entries in the config table */
#define CFG_SIZE_INITIAL   2

/* the amount of entries to increase the table size
   if we run out of free entries */
#define CFG_SIZE_INCREASE  2

/*
 * open the configuration file,
 * read the configuration settings,
 * return 0 on failure
 */
cfg_table* config_open( char* fname ) {
    size_t     bufsize;
    int        len, pos, start;
    char*      buf;
    char*      key, * val;
    FILE*      cfgfile;
    cfg_table* tab;

    cfgfile = fopen( fname, "r" );

    if( !cfgfile ) {
        return 0;
    }

    tab        = malloc( sizeof( cfg_table ) );
    tab->size  = 0;
    tab->free  = CFG_SIZE_INITIAL;
    tab->table = malloc( sizeof( cfg_item ) * CFG_SIZE_INITIAL );

    bufsize = 100;
    buf     = calloc( 1, bufsize );

    while( fgets( buf, bufsize, cfgfile ) ) {
        len = strlen( buf );

        /* account for '\n' in buf */
        if( buf[ len - 1 ] == '\n' ) {
            buf[ len - 1 ] = 0;
            len--;
        }

        /* skip ws */
        start = strspn( buf, " \t" );

        /* skip blank and comment lines */
        if( start == len || ( start < len && buf[ start ] == '#' ) ) {
            continue;
        }

        /* len of first token */
        pos = strcspn( buf + start, " \t=" );

        /* must have key */
        if( !pos ) {
            continue;
        }

        key = mystrndup( buf + start, pos );

        /* again skip ws */
        start += pos;
        if( start >= len ) {
            free( key );
            continue;
        }
        start += strspn( buf + start, " \t" );

        /* we *need* to have a '=' now */
        if( start == len || ( start < len && buf[ start ] != '=' ) ) {
            free( key );
            continue;
        }

        /*again skip ws after the '=' */
        start++;
        start += strspn( buf + start, " \t" );

        /* len of second token */
        pos = strcspn( buf + start, " \t" );

        /* note: we can have an empty val string */
        val = mystrndup( buf + start, pos );

        /* now store the key-value pair */
        if( tab->free == 0 ) {
            tab->free += CFG_SIZE_INCREASE;
            tab->table = realloc( tab->table,
                                  sizeof( cfg_item ) * ( tab->size + CFG_SIZE_INCREASE ) );

            /* DEBUG MSG */
            /* printf("Increasing size of config table\n"); */
        }

        tab->table[ tab->size ].key = key;
        tab->table[ tab->size ].val = val;

        /* DEBUG MSG */
        /* printf("Adding entry %d (key=\"%s\") (value=\"%s\")\n", tab->size, key, val ); */

        ++( tab->size );
        --( tab->free );
    }

    free( buf );
    fclose( cfgfile );
    return tab;
}

void config_write( const char* fname,
                   cfg_table*  tab ) {
    int   i;
    FILE* cfgfile;

    if( ( cfgfile = fopen( fname, "w" ) ) == NULL ) {
        fprintf( stderr, "config_write(): Error opening config file for writing" );
        return;
    }

    for( i = 0; i < tab->size; i++ ) {
        fprintf( cfgfile, "%s = %s\n",  tab->table[ i ].key, tab->table[ i ].val );
    }

    fclose( cfgfile );
}

/*
 * close the configuration file
 */
void config_close( cfg_table* tab ) {
    int i;

    for( i = 0; i < tab->size; i++ ) {
        free( tab->table[ i ].key );
        free( tab->table[ i ].val );
    }
    free( tab->table );
    free( tab );
}

char* config_query( cfg_table*  tab,
                    const char* str ) {
    int i;

    for( i = 0; i < tab->size; i++ ) {
        if( !strcmp( str, tab->table[ i ].key ) ) {
            return tab->table[ i ].val;
        }
    }
    return 0;
}

char* config_update( cfg_table*  tab,
                     const char* key,
                     const char* newValue ) {
    int   i        = 0;
    char* oldValue = 0;

    while( strcmp( key, tab->table[ i ].key ) && ( i < tab->size ) ) {
        i++;
    }

    if( i == tab->size ) {
        // Add a new entry to the table
        if( tab->free == 0 ) {
            tab->free += CFG_SIZE_INCREASE;
            tab->table = realloc( tab->table,
                                  sizeof( cfg_item ) * ( tab->size + CFG_SIZE_INCREASE ) );
        }
        tab->table[ tab->size ].key = strdup( key );
        tab->table[ tab->size ].val = strdup( newValue );
    }
    else {
        // update an existing one
        oldValue            = tab->table[ i ].val;
        tab->table[ i ].val = strdup( newValue );
    }

    return oldValue;
}
