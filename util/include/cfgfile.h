/**
   @file	cfgfile.c
   @brief   Header with access functions for a config file
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

#ifndef CFGFILE_H_INCLUDED
#define CFGFILE_H_INCLUDED


#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
    char* key;
    char* val;
}
cfg_item;


typedef struct {
    cfg_item* table;
    size_t    size;
    size_t    free;
}
cfg_table;


cfg_table* config_open( char* fname );

void config_write( const char* fname,
                   cfg_table*  tab );

void config_close( cfg_table* tab );

char* config_query( cfg_table*  tab,
                    const char* s );
char* config_update( cfg_table*  tab,
                     const char* key,
                     const char* newValue );



#ifdef __cplusplus
}
#endif


#endif /* CFGFILE_H_INCLUDED */
