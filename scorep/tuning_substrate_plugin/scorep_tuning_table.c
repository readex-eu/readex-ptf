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
 * @file
 *
 *
 */

// #include <config.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include <UTILS_Error.h>
#define SCOREP_DEBUG_MODULE_NAME TUNING
#include <UTILS_Debug.h>

// #include <SCOREP_Config.h>

#include "scorep_tuning_table.h"

// #include <SCOREP_Definitions.h>

#define SCOREP_TUNING_MAX_TUNING_ACTION_ENTRIES 4

/****************************************************************************************
   Initialization / Finalization
 ***************************************************************************************/

SCOREP_TuningRegionType* scorep_tuning_region_table;
uint16_t                 scorep_tuning_region_table_max_entries;
uint16_t                 scorep_tuning_region_table_next_free_entry;

SCOREP_TuningActionType* scorep_tuning_action_table;
uint16_t                 scorep_tuning_action_table_max_entries;
uint16_t                 scorep_tuning_action_table_next_free_entry;

void
scorep_tuning_initialize_region_table( void )
{
    /* Initializes the tuning region table */
    scorep_tuning_region_table_max_entries = 16;
    scorep_tuning_region_table             = ( SCOREP_TuningRegionType* )calloc( scorep_tuning_region_table_max_entries, sizeof( SCOREP_TuningRegionType ) );
    assert( scorep_tuning_region_table );
    scorep_tuning_region_table_next_free_entry = 0;

    for ( uint16_t i = 0; i < scorep_tuning_region_table_max_entries; i++ )
    {
        scorep_tuning_region_table[ i ].max_tuning_action_entries = SCOREP_TUNING_MAX_TUNING_ACTION_ENTRIES;
        scorep_tuning_region_table[ i ].tuningActions             = ( SCOREP_TuningParameterType* )calloc( scorep_tuning_region_table[ i ].max_tuning_action_entries, sizeof( SCOREP_TuningParameterType ) );
        assert( scorep_tuning_region_table[ i ].tuningActions );
        scorep_tuning_region_table[ i ].next_free_tuning_action_entry = 0;
    }
}

void
scorep_tuning_initialize_action_table( void )
{
    /* Initializes the tuning action table */
    scorep_tuning_action_table_max_entries = 16;
    scorep_tuning_action_table             = ( SCOREP_TuningActionType* )calloc( scorep_tuning_action_table_max_entries, sizeof( SCOREP_TuningActionType ) );
    assert( scorep_tuning_action_table );
    scorep_tuning_action_table_next_free_entry = 0;
}

void
scorep_tuning_finalize_region_table( void )
{
    for ( uint16_t i = 0; i < scorep_tuning_region_table_max_entries; i++ )
    {
        free( scorep_tuning_region_table[ i ].tuningActions );
    }

    free( scorep_tuning_region_table );
}

void
scorep_tuning_finalize_action_table( void )
{
    for ( uint16_t i = 0; i < scorep_tuning_action_table_next_free_entry; i++ )
    {
        free( scorep_tuning_action_table[ i ].restoreValueStack );
//        finalizeStack ( scorep_tuning_action_table[ i ].restoreValueStack );
        free( scorep_tuning_action_table[ i ].name );
    }

    free( scorep_tuning_action_table );
}

void
scorep_tuning_action_table_fill_entry( char*                   name,
                                       SCOREP_TuningActionKind kind,
                                       int                     restore,
                                       unsigned int*           enterRegionVar,
                                       unsigned int*           exitRegionVar,
                                       void ( *                enterRegionFunc )( int,
                                                                                  int* ),
                                       void ( *                exitRegionFunc )( int,
                                                                                 int* ),
                                       void ( *                validationEndRegionFunction )( int ),
                                       SCOREP_Language         languageType )
{
    UTILS_BUG_ON( validationEndRegionFunction != NULL, "Validation of tuning correctness not implemented yet!" );

    uint16_t i = scorep_tuning_action_table_find_action( name );
//Kind already added when the tuning action was received by the parser. Now it is repeated. It should be removed by the parser.

    scorep_tuning_action_table[ i ].kind                           = kind;
    scorep_tuning_action_table[ i ].enterRegionVariablePtr         = enterRegionVar;
    scorep_tuning_action_table[ i ].exitRegionVariablePtr          = exitRegionVar;
    scorep_tuning_action_table[ i ].enterRegionFunctionPtr         = enterRegionFunc;
    scorep_tuning_action_table[ i ].exitRegionFunctionPtr          = exitRegionFunc;
    scorep_tuning_action_table[ i ].validationEndRegionFunctionPtr = validationEndRegionFunction;
    scorep_tuning_action_table[ i ].languageType                   = languageType;
    scorep_tuning_action_table[ i ].restoreValueFlag               = restore;

    if ( !restore )
    {
        scorep_tuning_action_table[ i ].restoreValueStack = NULL;
    }
    else if ( !scorep_tuning_action_table[ i ].restoreValueStack )
    {
        scorep_tuning_action_table[ i ].restoreValueStack = initStack();
    }

//    printf( "scorep_tuning_action_table_fill_entry: Adding tuning action into the table to the place %u, with following information: kind %d, enterRegionVar %p, "
//            "exitRegionVar %p, enterRegionFunc %p, exitRegionFunc %p, validationEndRegionFunction %p, "
//            "restoreValueFlag %d, restoreValueStack %p\n",
//            i, kind, enterRegionVar, exitRegionVar, enterRegionFunc, exitRegionFunc,
//            validationEndRegionFunction, restore, scorep_tuning_action_table[ i ].restoreValueStack );
}

uint16_t
scorep_tuning_region_table_find_region_or_add( uint32_t regionId )
{
    uint16_t i;
    if ( scorep_tuning_region_table_find_region( &i, regionId ) == SCOREP_TUNING_FOUND )
    {
        return i;
    }

    if ( scorep_tuning_region_table_next_free_entry == scorep_tuning_region_table_max_entries )
    {
        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, "Increasing the size of the tuning region table\n" );
        scorep_tuning_region_table_max_entries <<= 1;
        scorep_tuning_region_table               = ( SCOREP_TuningRegionType* )realloc( scorep_tuning_region_table, scorep_tuning_region_table_max_entries * sizeof( SCOREP_TuningRegionType ) );
        assert( scorep_tuning_region_table );

        for ( uint16_t i = scorep_tuning_region_table_next_free_entry; i < scorep_tuning_region_table_max_entries; i++ )
        {
            scorep_tuning_region_table[ i ].max_tuning_action_entries = SCOREP_TUNING_MAX_TUNING_ACTION_ENTRIES;
            scorep_tuning_region_table[ i ].tuningActions             = ( SCOREP_TuningParameterType* )calloc( scorep_tuning_region_table[ i ].max_tuning_action_entries, sizeof( SCOREP_TuningParameterType ) );
            assert( scorep_tuning_region_table[ i ].tuningActions );
            scorep_tuning_region_table[ i ].next_free_tuning_action_entry = 0;
        }
    }

    i                                        = scorep_tuning_region_table_next_free_entry;
    scorep_tuning_region_table[ i ].regionId = regionId;
    scorep_tuning_region_table_next_free_entry++;

    return i;
}

//TODO: As this share the same functionality a macro should be defined with the shared implementation.
SCOREP_TuningReturnCodeType
scorep_tuning_region_table_find_region( uint16_t* location,
                                        uint32_t  regionId )
{
    SCOREP_TuningReturnCodeType ret_code = SCOREP_TUNING_NOTFOUND;
    for ( uint16_t i = 0; i < scorep_tuning_region_table_next_free_entry; i++ )
    {
        if ( scorep_tuning_region_table[ i ].regionId == regionId )
        {
            ret_code  = SCOREP_TUNING_FOUND;
            *location = i;
            break;
        }
    }
    return ret_code;
}

uint16_t
scorep_tuning_action_table_find_action( char* name )
{
    for ( uint16_t i = 0; i < scorep_tuning_action_table_next_free_entry; i++ )
    {
        if ( !strcmp( scorep_tuning_action_table[ i ].name, name ) )
        {
            return i;
        }
    }

    if ( scorep_tuning_action_table_next_free_entry == scorep_tuning_action_table_max_entries )
    {
        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, "Increasing the size of the tuning action table\n" );
        scorep_tuning_action_table_max_entries <<= 1;
        scorep_tuning_action_table               = ( SCOREP_TuningActionType* )realloc( scorep_tuning_action_table, scorep_tuning_action_table_max_entries * sizeof( SCOREP_TuningActionType ) );
        assert( scorep_tuning_action_table );
        for ( uint16_t i = scorep_tuning_action_table_next_free_entry; i < scorep_tuning_action_table_max_entries; i++ )
        {
            scorep_tuning_action_table[ i ].enterRegionFunctionPtr         = NULL;
            scorep_tuning_action_table[ i ].exitRegionFunctionPtr          = NULL;
            scorep_tuning_action_table[ i ].enterRegionVariablePtr         = NULL;
            scorep_tuning_action_table[ i ].exitRegionVariablePtr          = NULL;
            scorep_tuning_action_table[ i ].kind                           = 0;
            scorep_tuning_action_table[ i ].restoreValueFlag               = 0;
            scorep_tuning_action_table[ i ].restoreValueStack              = NULL;
            scorep_tuning_action_table[ i ].validationEndRegionFunctionPtr = NULL;
        }
    }

    scorep_tuning_action_table[ scorep_tuning_action_table_next_free_entry ].name = calloc( 1, strlen( name ) + 1 );
    assert( scorep_tuning_action_table[ scorep_tuning_action_table_next_free_entry ].name );

    strcpy( scorep_tuning_action_table[ scorep_tuning_action_table_next_free_entry ].name, name );

    scorep_tuning_action_table_next_free_entry++;

    return scorep_tuning_action_table_next_free_entry - 1;
}

void
scorep_tuning_region_table_find_action_or_add( uint16_t                regionLocation,
                                               SCOREP_TuningActionKind actionType,
                                               uint16_t                locationInTuningActionTable,
                                               int                     tuningParameterValue )
{
    uint16_t j = scorep_tuning_region_table[ regionLocation ].next_free_tuning_action_entry;

//Find the value in the table
    for ( uint16_t i = 0; i < j; i++ )
    {
        if ( scorep_tuning_region_table[ regionLocation ].tuningActions[ i ].tuningActionListIndex == locationInTuningActionTable )
        {
            scorep_tuning_region_table[ regionLocation ].tuningActions[ i ].tuningActionKind = actionType;
            UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, "Updating the value of tuning action with index %u from value %d to %d\n",
                                locationInTuningActionTable,
                                scorep_tuning_region_table[ regionLocation ].tuningActions[ i ].tuningParameterValue,
                                tuningParameterValue );
            scorep_tuning_region_table[ regionLocation ].tuningActions[ i ].tuningParameterValue = tuningParameterValue;
            return;
        }
    }

//Check is there place to add the tuning action
    if ( j == scorep_tuning_region_table[ regionLocation ].max_tuning_action_entries )
    {
        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, "Increasing the maximum number of tuning actions in the region table\n" );
        scorep_tuning_region_table[ regionLocation ].max_tuning_action_entries <<= 1;
        scorep_tuning_region_table[ regionLocation ].tuningActions               = ( SCOREP_TuningParameterType* )realloc( scorep_tuning_region_table[ regionLocation ].tuningActions, scorep_tuning_region_table[ regionLocation ].max_tuning_action_entries * sizeof( SCOREP_TuningParameterType ) );
        assert( scorep_tuning_region_table[ regionLocation ].tuningActions );
//        for (uint16_t k = j; k < scorep_tuning_region_table[ regionLocation ].max_tuning_action_entries; k++)
//        {
//            scorep_tuning_region_table[ regionLocation ].tuningActions[ k ].tuningActionKind = 0;
//            scorep_tuning_region_table[ regionLocation ].tuningActions[ k ].tuningActionListIndex = 0;
//            scorep_tuning_region_table[ regionLocation ].tuningActions[ k ].tuningParameterValue = 0;
//        }
    }

//Store the value of the tuning action
    scorep_tuning_region_table[ regionLocation ].tuningActions[ j ].tuningActionKind      = actionType;
    scorep_tuning_region_table[ regionLocation ].tuningActions[ j ].tuningActionListIndex = locationInTuningActionTable;
    scorep_tuning_region_table[ regionLocation ].tuningActions[ j ].tuningParameterValue  = tuningParameterValue;

    scorep_tuning_region_table[ regionLocation ].next_free_tuning_action_entry++;

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, "Tuning action entry added for region %d on location %d in the table (kind = %d, listIndex = %u, value = %d)\n",
                        scorep_tuning_region_table[ regionLocation ].regionId,
                        regionLocation,
                        scorep_tuning_region_table[ regionLocation ].tuningActions[ j ].tuningActionKind,
                        scorep_tuning_region_table[ regionLocation ].tuningActions[ j ].tuningActionListIndex,
                        scorep_tuning_region_table[ regionLocation ].tuningActions[ j ].tuningParameterValue );
}

static inline char*
parse_tuning_parameter_name( char* parameterName )
{
    uint16_t i         = 0;
    char*    temp_name = NULL;

    while ( parameterName[ i++ ] != '>' )
    {
    }

    temp_name = ( char* )malloc( ( i - 1 ) * sizeof( char ) );
    assert( temp_name );

    strncpy( temp_name, parameterName + sizeof( char ), i - 2 );

    temp_name[ i - 2 ] = '\0';

    return temp_name;
}


void
scorep_tuning_map_tuning_parameter_to_variable( char*         tuningParameterName,
                                                unsigned int* variantSelector,
                                                int           restoreValueFlag )
{
    char* temp_name = parse_tuning_parameter_name( tuningParameterName );

    scorep_tuning_action_table_fill_entry( temp_name, SCOREP_TUNING_VARIABLE, restoreValueFlag, variantSelector, NULL, NULL, NULL, NULL, 0 );

    free( temp_name );
}

void
scorep_tuning_map_tuning_parameter_to_function( char*           tuningParameterName,
                                                void ( *        variantSelector )( int,
                                                                                   int* ),
                                                int             restoreValueFlag,
                                                SCOREP_Language languageType )
{
    char* temp_name = parse_tuning_parameter_name( tuningParameterName );

    scorep_tuning_action_table_fill_entry( temp_name, SCOREP_TUNING_FUNCTION, restoreValueFlag, NULL, NULL, variantSelector, variantSelector, NULL, languageType );

    free( temp_name );
}


void
scorep_tuning_print_region_table( void )
{
    printf( "Tuning region table:\n" );
    for ( uint16_t i = 0; i < scorep_tuning_region_table_next_free_entry; i++ )
    {
        printf( "%u. region entry\n", i );
        printf( "  RegionId: %d\n", scorep_tuning_region_table[ i ].regionId );
        for ( uint16_t j = 0; j < scorep_tuning_region_table[ i ].next_free_tuning_action_entry; j++ )
        {
            printf( "    %u. Tuning action entry\n", j );
            printf( "      Kind: %d\n", scorep_tuning_region_table[ i ].tuningActions[ j ].tuningActionKind );
            printf( "      Table index: %d\n", scorep_tuning_region_table[ i ].tuningActions[ j ].tuningActionListIndex );
            printf( "      Value: %d\n", scorep_tuning_region_table[ i ].tuningActions[ j ].tuningParameterValue );
        }
    }
}

void
scorep_tuning_print_action_table( void )
{
    printf( "Tuning action table:\n" );
    for ( uint16_t i = 0; i < scorep_tuning_action_table_next_free_entry; i++ )
    {
        printf( "%u. entry\n", i );
        printf( "  Kind: %d\n", scorep_tuning_action_table[ i ].kind );
        printf( "  Name: %s\n", scorep_tuning_action_table[ i ].name );
        printf( "  Restore flag: %d\n", scorep_tuning_action_table[ i ].restoreValueFlag );
        if ( scorep_tuning_action_table[ i ].restoreValueFlag != 0 )
        {
            printf( "    Stack address: %p\n", ( void* )scorep_tuning_action_table[ i ].restoreValueStack );
            printf( "    Stack size = %u\n", ( int )scorep_tuning_action_table[ i ].restoreValueStack->size );
            printf( "    Stack max size = %u\n", ( int )scorep_tuning_action_table[ i ].restoreValueStack->maxSize );
            printf( "    Stack top value= %d\n", *( scorep_tuning_action_table[ i ].restoreValueStack->value ) );
        }
        printf( "  Enter region variable address: %p\n", ( void* )scorep_tuning_action_table[ i ].enterRegionVariablePtr );
        printf( "  Exit region variable address: %p\n", ( void* )scorep_tuning_action_table[ i ].exitRegionVariablePtr );
        printf( "  Enter region function address: %p\n", ( void* )scorep_tuning_action_table[ i ].enterRegionFunctionPtr );
        printf( "  Exit region function address: %p\n", ( void* )scorep_tuning_action_table[ i ].exitRegionFunctionPtr );
        printf( "  Validation exit region function address: %p\n", ( void* )scorep_tuning_action_table[ i ].validationEndRegionFunctionPtr );
    }
}
