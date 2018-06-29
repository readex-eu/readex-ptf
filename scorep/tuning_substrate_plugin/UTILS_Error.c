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
 * @file UTILS_Error.c
 *
 * Mimics work of Score-P's error module
 * TODO: If this ever becomes the substrate in Score-P instead of a plugin this should be removed.
 */

// #include <config.h>

#include "UTILS_Error.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>

void
UTILS_Error_Handler( const char*    file,
                     const uint64_t line,
                     const char*    msgFormatString,
                     ... )
{
    size_t msg_format_string_length = msgFormatString ?
                                      strlen( msgFormatString ) : 0;

    fprintf( stderr,
             "[Score-P] %s:%" PRIu64 ": %s",
             file,
             line,
             msg_format_string_length ? ": " : "\n" );

    if ( msg_format_string_length )
    {
        va_list va;
        va_start( va, msgFormatString );
        vfprintf( stderr, msgFormatString, va );
        fprintf( stderr, "\n" );
        va_end( va );
    }
}

void
UTILS_Error_Abort( const char*    file,
                   const uint64_t line,
                   const char*    msgFormatString,
                   ... )
{
    size_t msg_format_string_length = msgFormatString ?
                                      strlen( msgFormatString ) : 0;

    fprintf( stderr,
             "[Score-P] %s:%" PRIu64 ": %s",
             file,
             line,
             msg_format_string_length ? ": " : "\n" );

    if ( msg_format_string_length )
    {
        va_list va;
        va_start( va, msgFormatString );
        vfprintf( stderr, msgFormatString, va );
        fprintf( stderr, "\n" );
        va_end( va );
    }

    abort();
}
