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


#ifndef UTILS_ERROR_H 
#define UTILS_ERROR_H 

/**
 * @file scorep_tuning_debug.h
 *
 * Mimics work of Score-P's error module
 * TODO: If this ever becomes the substrate in Score-P instead of a plugin this should be removed.
 */

#include <stddef.h>
#include <stdint.h>

/**
 * Emit a warning.
 */
#define UTILS_WARNING( ... ) \
    UTILS_Error_Handler( __FILE__, \
                         __LINE__, \
                         __VA_ARGS__ )


///**
// * Delegation error handler function, which is used by the prep UTILS_ERROR to
// * to avert that external programmers use the function pointer directly.
// *
// * @param function        : Name of the function where the error appeared
// * @param file            : Name of the source-code file where the error appeared
// * @param line            : Line number in the source-code where the error appeared
// * @param errorCode       : Error Code
// * @param msgFormatString : Format string like it is used at the printf family.
// *
// * @returns Should return the ErrorCode
// * @ingroup UTILS_Exception_module
// */
//#define UTILS_Error_Handler PACKAGE_MANGLE_NAME( UTILS_Error_Handler )
//PACKAGE_ErrorCode
void
UTILS_Error_Handler( const char*       file,
                     uint64_t          line,
                     const char*       msgFormatString,
                     ... );

/**
 * @def UTILS_FATAL
 *
 * Indicates a fatal condition. The program will abort immediately.
 *
 */
#define UTILS_FATAL( ... ) \
    UTILS_Error_Abort( __FILE__, \
                       __LINE__, \
                       __VA_ARGS__ )

/**
 * @def UTILS_BUG_ON
 *
 * Same as UTILS_BUG but with an condition.
 *
 */
#define UTILS_BUG_ON( expression, ... ) \
    ( !( expression ) ? ( void )0 :     \
      UTILS_Error_Abort( __FILE__,      \
                         __LINE__,      \
                         "Bug '" #expression "': " __VA_ARGS__ ) )


/**
 * This function implements the UTILS_ASSERT, UTILS_FATAL, UTILS_BUG, UTILS_BUG_ON macro.
 * It prints an error message and aborts the program. Do not insert calls to this function
 * directly, but use the macros instead.
 *  @param fileName   The file name of the file which contains the source code where the
 *                    message was created.
 *  @param line       The line number of the source code line where the debug message
 *                    was created.
 *  @param functionName  A string containing the name of the function where the debug
 *                       message was called.
 *  @param messageFormatString A printf-like format string.
 */
//#define UTILS_Error_Abort PACKAGE_MANGLE_NAME( UTILS_Error_Abort )
void
UTILS_Error_Abort( const char* fileName,
                   uint64_t    line,
                   const char* messageFormatString,
                   ... ) __attribute__( ( noreturn ) );


void
scorep_init_debug( void );

#endif /* UTILS_ERROR_H */
