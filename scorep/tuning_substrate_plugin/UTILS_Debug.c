/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2015-2016,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */


/**
 * @file UTILS_Debug.c
 *
 * Mimics work of Score-P's debug module
 * TODO: If this ever becomes the substrate in Score-P instead of a plugin this should be removed.
 */

// #include <config.h>

#include "UTILS_Debug.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>

const char*         SCOREP_TUNING_SUBSTRATE_DEBUG = "SCOREP_TUNING_SUBSTRATE_DEBUG";
static unsigned int scorep_tuning_debug           = 0;


void
scorep_debug_init( void )
{
    if ( getenv( SCOREP_TUNING_SUBSTRATE_DEBUG ) )
    {
        fprintf( stdout, "[Score-P] Active debug module(s): TUNING\n" );
        scorep_tuning_debug = 1;
    }
}

void
UTILS_Debug_Printf( uint64_t    bitMask,
                    const char* file,
                    uint64_t    line,
                    const char* msgFormatString,
                    ... )
{
    if ( !scorep_tuning_debug )
        return;

    size_t msg_format_string_length = msgFormatString ?
                                      strlen( msgFormatString ) : 0;

    fprintf( stdout,
             "[Score-P] %s:%" PRIu64 ": %s",
             file,
             line,
             msg_format_string_length ? ": " : "\n" );

    if ( msg_format_string_length )
    {
        va_list va;
        va_start( va, msgFormatString );
        vfprintf( stdout, msgFormatString, va );
        fprintf( stdout, "\n" );
        va_end( va );
    }
}
