/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2015-2016,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */


#ifndef UTILS_DEBUG_H
#define UTILS_DEBUG_H

/**
 * @file scorep_tuning_debug.h
 *
 * Mimics work of Score-P's debug module
 * TODO: If this ever becomes the substrate in Score-P instead of a plugin this should be removed.
 */

#include <stddef.h>
#include <stdint.h>

// TODO: Find a way to generate this.
#define SCOREP_DEBUG_TUNING 1

#define UTILS_DEBUG_PRINTF( debugLevel, ... ) \
    UTILS_Debug_Printf( \
        debugLevel, \
        __FILE__, \
        __LINE__, \
        __VA_ARGS__ )


/**
 * Function implementation called by @ref UTILS_DEBUG_PRINTF. It prints a debug message
 * in the given debug level. Furthermore, it provides the function name, file name and
 * line number.
 * @param bitMask    The debug level which must be enabled to print out the message.
 * @param function   A string containing the name of the function where the debug messages
 *                   was called.
 * @param file       The file name of the file which contains the source code where the
 *                   message was created.
 * @param line       The line number of the source code line where the debug message
 *                   was created.
 * @param msgFormatString A format string followed by the parameters defined in
 *                        the format string. The format string and the
 *                        parameters have the same syntax like in the POSIX
 *                        printf function.
 */
//#define UTILS_Debug_Printf PACKAGE_MANGLE_NAME( UTILS_Debug_Printf )
void
UTILS_Debug_Printf( uint64_t    bitMask,
                    const char* file,
                    uint64_t    line,
//                    const char* function,
                    const char* msgFormatString,
                    ... );

void
scorep_debug_init( void );

#endif /* UTILS_DEBUG_H */
