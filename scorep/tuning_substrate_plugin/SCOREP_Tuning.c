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
#include "scorep_tuning_registered_functions.h"
#include "scorep_tuning_service_management.h"

#include <scorep/SCOREP_SubstratePlugins.h>

/****************************************************************************************
   Initialization / Finalization
 ***************************************************************************************/

static const SCOREP_SubstratePluginCallbacks* callbacks = NULL;
static size_t plugin_id;

extern SCOREP_TuningRegionType* scorep_tuning_region_table;
extern SCOREP_TuningActionType* scorep_tuning_action_table;

static int
initialize( void )
{
    scorep_debug_init();

    scorep_tuning_initialize_region_table();

    scorep_tuning_initialize_action_table();

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, "Initializing tuning substrate\n" );
    int ret = tuning_subsystem_init();

    UTILS_BUG_ON( ret != 0, "Could not initialize Tuning Subsystem." );

// Here we are initializing tuning actions if necessary.
// Now it is done through the tuning plugins.
//    scorep_tuning_map_function_tuning_actions( );

    //TODO: Maybe to add an initialized flag
    return plugin_id;
}


static void
finalize( void )
{
    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, "Finalizing tuning component of ScoreP\n" );

    tuning_subsystem_finalize();

    scorep_tuning_finalize_region_table();

    scorep_tuning_finalize_action_table();

//    scorep_tuning_print_region_table();
//    scorep_tuning_print_action_table();
}


void
enter_region( struct SCOREP_Location* location,
              uint64_t                timestamp,
              SCOREP_RegionHandle     regionHandle,
              uint64_t*               metricValues )
{
//    fprintf( stderr, "enter_region: Applying tuning action enter for region %s (%s/%d) of type %d (%d) file %s begin line %d and end line %d\n",
//             callbacks->SCOREP_RegionHandle_GetName( regionHandle ),
//             callbacks->SCOREP_RegionHandle_GetCanonicalName( regionHandle ),
//             callbacks->SCOREP_RegionHandle_GetId( regionHandle ),
//             callbacks->SCOREP_RegionHandle_GetType( regionHandle ),
//             callbacks->SCOREP_RegionHandle_GetParadigmType( regionHandle ),
//             callbacks->SCOREP_RegionHandle_GetFileName( regionHandle ),
//             callbacks->SCOREP_RegionHandle_GetBeginLine( regionHandle ),
//             callbacks->SCOREP_RegionHandle_GetEndLine( regionHandle ) );

    UTILS_BUG_ON( callbacks == 0, "SCORE-P internal callbacks not set." );
    const char* regionName = callbacks->SCOREP_RegionHandle_GetName( regionHandle );
    uint32_t    regionId   = callbacks->SCOREP_RegionHandle_GetId( regionHandle );

    uint16_t i; //uint16_t regionIndex;

    if ( scorep_tuning_region_table_find_region( &i, regionId ) == SCOREP_TUNING_FOUND )
    {
        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, "Region %s found on location %u\n", regionName, i );
        //scorep_tuning_print_region_table( );
        //scorep_tuning_print_action_table();
        for ( uint16_t j = 0; j < scorep_tuning_region_table[ i ].next_free_tuning_action_entry; j++ )
        {
            uint16_t k = scorep_tuning_region_table[ i ].tuningActions[ j ].tuningActionListIndex; // Index of the tuning action in the table
            if ( scorep_tuning_action_table[ k ].kind == SCOREP_TUNING_VARIABLE )
            {
                if ( scorep_tuning_action_table[ k ].enterRegionVariablePtr )
                {
                    if ( scorep_tuning_action_table[ k ].restoreValueFlag )
                    {
                        if ( scorep_tuning_action_table[ k ].restoreValueStack )
                        {
                            push2Stack( scorep_tuning_action_table[ k ].restoreValueStack, *( scorep_tuning_action_table[ k ].enterRegionVariablePtr ) );
                        }
                        else
                        {
                            UTILS_WARNING( "enter_region: Restore value stack is NULL\n" );
                        }
                    }
/* Apply the tuning action */
//                    printf("enter_region: This is the place where I set the variable with value = %d!\n", scorep_tuning_region_table[ i ].tuningActions[ j ].tuningParameterValue);
                    *( scorep_tuning_action_table[ k ].enterRegionVariablePtr ) = scorep_tuning_region_table[ i ].tuningActions[ j ].tuningParameterValue;
                    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, "Variable %p set to %d for tuning action %d in region %s(%d)\n",
                                        scorep_tuning_action_table[ k ].enterRegionVariablePtr,
                                        *( scorep_tuning_action_table[ k ].enterRegionVariablePtr ),
                                        j, regionName, regionId );
                }
//                if ( scorep_tuning_action_table[ k ].exitRegionVariablePtr )
//                {
///* Apply the tuning action */
//                    printf("enter_region: This is the place where I set the variable with value = %d!\n", scorep_tuning_region_table[ i ].tuningActions[ j ].tuningParameterValue);
//                    *( scorep_tuning_action_table[ k ].exitRegionVariablePtr ) = scorep_tuning_region_table[ i ].tuningActions[ j ].tuningParameterValue;
//                    printf("Variable %p set to %d\n", scorep_tuning_action_table[ k ].enterRegionVariablePtr,
//                           *( scorep_tuning_action_table[ k ].enterRegionVariablePtr ));
//                }
            }
            else if ( scorep_tuning_action_table[ k ].kind == SCOREP_TUNING_FUNCTION )
            {
                if ( scorep_tuning_action_table[ k ].enterRegionFunctionPtr )
                {
                    if ( scorep_tuning_action_table[ k ].languageType == SCOREP_LANGUAGE_C )
                    {
                        int old;
/* Apply the tuning action */
                        scorep_tuning_action_table[ k ].enterRegionFunctionPtr( scorep_tuning_region_table[ i ].tuningActions[ j ].tuningParameterValue, &old );
                        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, " Function %p(%d) for tuning action %d in region %s(%d)\n",
                                            scorep_tuning_action_table[ k ].enterRegionFunctionPtr,
                                            scorep_tuning_region_table[ i ].tuningActions[ j ].tuningParameterValue,
                                            j, regionName, regionId );

// TODO: Fetch the old value from the function
                        if ( scorep_tuning_action_table[ k ].restoreValueFlag )
                        {
                            if ( scorep_tuning_action_table[ k ].restoreValueStack )
                            {
//TODO: Store the old value on the stack
                                push2Stack( scorep_tuning_action_table[ k ].restoreValueStack, old );
                            }
                            else
                            {
                                UTILS_WARNING( "enter_region: Restore value stack is NULL\n" );
                            }
                        }
                    }
                    else if ( scorep_tuning_action_table[ k ].languageType == SCOREP_LANGUAGE_FORTRAN )
                    {
                        int* old;
                        union
                        {
                            int  value;
                            int* value_ptr;
                        } union_value;

                        union
                        {
                            int*  value_ptr;
                            int** value_ptrptr;
                        } union_old;

                        union_value.value_ptr  = &scorep_tuning_region_table[ i ].tuningActions[ j ].tuningParameterValue;
                        union_old.value_ptrptr = &old;
/* Apply the tuning action */
                        scorep_tuning_action_table[ k ].enterRegionFunctionPtr( union_value.value, union_old.value_ptr );
                        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, "Function %p(%d) for tuning action %d in region %s(%d)\n",
                                            scorep_tuning_action_table[ k ].enterRegionFunctionPtr,
                                            scorep_tuning_region_table[ i ].tuningActions[ j ].tuningParameterValue,
                                            j, regionName, regionId );

// TODO: Fetch the old value from the function
                        if ( scorep_tuning_action_table[ k ].restoreValueFlag )
                        {
                            if ( scorep_tuning_action_table[ k ].restoreValueStack )
                            {
//TODO: Store the old value on the stack
                                push2Stack( scorep_tuning_action_table[ k ].restoreValueStack, *old );
                            }
                            else
                            {
                                UTILS_WARNING( "enter_region: Restore value stack is NULL\n" );
                            }
                        }
                    }
                    else
                    {
                        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, "Undefined language for function tuning action\n" );
                    }
                }
//TODO: Check what to do with this
//                if ( scorep_tuning_action_table[ k ].exitRegionFunctionPtr )
//                {
///* Apply the tuning action */
//                    printf("enter_region: This is the place where I call the function with the variable set to = %d!\n", scorep_tuning_region_table[ i ].tuningActions[ j ].tuningParameterValue );
//                    scorep_tuning_action_table[ k ].exitRegionFunctionPtr( scorep_tuning_region_table[ i ].tuningActions[ j ].tuningParameterValue );
//                    printf("Function %p(%d)\n", scorep_tuning_action_table[ k ].exitRegionFunctionPtr,
//                           scorep_tuning_region_table[ i ].tuningActions[ j ].tuningParameterValue );
//                }
            }
        }
    }
    else
    {
//        UTILS_WARNING( "enter_region: region %s with Id %u not found! Tuning action will not be applied!\n", regionName, regionId );
//        SCOREP_Tuning_PrintActionTable();
    }
}


static void
exit_region( struct SCOREP_Location* location,
             uint64_t                timestamp,
             SCOREP_RegionHandle     regionHandle,
             uint64_t*               metricValues )
{
//    fprintf( stderr, "exit_region: Applying tuning action exit for region %s (%s/%d) of type %d (%d) file %s begin line %d and end line %d\n",
//             callbacks->SCOREP_RegionHandle_GetName( regionHandle ),
//             callbacks->SCOREP_RegionHandle_GetCanonicalName( regionHandle ),
//             callbacks->SCOREP_RegionHandle_GetId( regionHandle ),
//             callbacks->SCOREP_RegionHandle_GetType( regionHandle ),
//             callbacks->SCOREP_RegionHandle_GetParadigmType( regionHandle ),
//             callbacks->SCOREP_RegionHandle_GetFileName( regionHandle ),
//             callbacks->SCOREP_RegionHandle_GetBeginLine( regionHandle ),
//             callbacks->SCOREP_RegionHandle_GetEndLine( regionHandle ) );

    UTILS_BUG_ON( callbacks == 0, "SCORE-P internal callbacks not set." );
    const char* regionName = callbacks->SCOREP_RegionHandle_GetName( regionHandle );
    uint32_t    regionId   = callbacks->SCOREP_RegionHandle_GetId( regionHandle );

    uint16_t i;

    if ( scorep_tuning_region_table_find_region( &i, regionId ) == SCOREP_TUNING_FOUND )
    {
        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, "Region %s found on location %u\n", regionName, i );
        for ( uint16_t j = 0; j < scorep_tuning_region_table[ i ].next_free_tuning_action_entry; j++ )
        {
            uint16_t k = scorep_tuning_region_table[ i ].tuningActions[ j ].tuningActionListIndex; // Index of the tuning action in the table

            if ( scorep_tuning_action_table[ k ].kind == SCOREP_TUNING_VARIABLE )
            {
                if ( scorep_tuning_action_table[ k ].enterRegionVariablePtr )
                {
                    if ( scorep_tuning_action_table[ k ].restoreValueFlag )
                    {
                        if ( scorep_tuning_action_table[ k ].restoreValueStack )
                        {
                            *( scorep_tuning_action_table[ k ].enterRegionVariablePtr ) = popFromStack( scorep_tuning_action_table[ k ].restoreValueStack );
                            UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, "Variable %p(%d) restored from stack for tuning action %d in region %s(%d)!\n",
                                                scorep_tuning_action_table[ k ].enterRegionVariablePtr,
                                                *( scorep_tuning_action_table[ k ].enterRegionVariablePtr ),
                                                j, regionName, regionId );
                        }
                        else
                        {
                            UTILS_WARNING( "exit_region: Restore value stack is NULL\n" );
                        }
                    }
                }
//TODO: Check what to do with this
//                if ( scorep_tuning_action_table[ k ].exitRegionVariablePtr )
//                {
//                    /* Apply the tuning action */
//                    scorep_tuning_action_table[ k ].exitRegionVariablePtr( scorep_tuning_region_table[ i ].tuningActions[ j ].tuningParameterValue );
//                    printf("Variable %p set to %d\n", scorep_tuning_action_table[ k ].exitRegionVariablePtr,
//                           *( scorep_tuning_action_table[ k ].enterRegionVariablePtr ));
//                }
            }
            else if ( scorep_tuning_action_table[ k ].kind == SCOREP_TUNING_FUNCTION )
            {
                if ( scorep_tuning_action_table[ k ].enterRegionFunctionPtr )
                {
                    if ( scorep_tuning_action_table[ k ].languageType == SCOREP_LANGUAGE_C )
                    {
                        int old;
// Store the old value
                        if ( scorep_tuning_action_table[ k ].restoreValueFlag )
                        {
                            if ( scorep_tuning_action_table[ k ].restoreValueStack )
                            {
//TODO: Pop the old value from the stack
//TODO: Call the function with the value from the stack
                                unsigned int value = popFromStack( scorep_tuning_action_table[ k ].restoreValueStack );
                                scorep_tuning_action_table[ k ].enterRegionFunctionPtr( value, &old );
                                UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, "Function %p(%d) restored from stack for tuning action %d in region %s(%d)!\n",
                                                    scorep_tuning_action_table[ k ].enterRegionFunctionPtr,
                                                    value,
                                                    j, regionName, regionId );
                            }
                            else
                            {
                                UTILS_WARNING( "exit_region: Restore value stack is NULL\n" );
                            }
                        }
/* Apply the tuning action */
//                    scorep_tuning_action_table[ k ].enterRegionFunctionPtr( scorep_tuning_region_table[ i ].tuningActions[ j ].tuningParameterValue );
//                    printf("Function %p(%d)\n", scorep_tuning_action_table[ k ].enterRegionFunctionPtr,
//                           scorep_tuning_region_table[ i ].tuningActions[ j ].tuningParameterValue );
                    }
                    else if ( scorep_tuning_action_table[ k ].languageType == SCOREP_LANGUAGE_FORTRAN )
                    {
                        int* old;
                        // Store the old value
                        if ( scorep_tuning_action_table[ k ].restoreValueFlag )
                        {
                            if ( scorep_tuning_action_table[ k ].restoreValueStack )
                            {
                                int value = popFromStack( scorep_tuning_action_table[ k ].restoreValueStack );

                                union
                                {
                                    int  value;
                                    int* value_ptr;
                                } union_value;

                                union
                                {
                                    int*  value_ptr;
                                    int** value_ptrptr;
                                } union_old;

                                union_value.value_ptr  = &value;
                                union_old.value_ptrptr = &old;
                                //TODO: Pop the old value from the stack
                                //TODO: Call the function with the value from the stack
                                scorep_tuning_action_table[ k ].enterRegionFunctionPtr( union_value.value, union_old.value_ptr );
                                UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, "Function %p(%d) restored from stack for tuning action %d in region %s(%d)!\n",
                                                    scorep_tuning_action_table[ k ].enterRegionFunctionPtr,
                                                    value,
                                                    j, regionName, regionId );
                            }
                            else
                            {
                                UTILS_WARNING( "exit_region: Restore value stack is NULL\n" );
                            }
                        }
/* Apply the tuning action */
//                    scorep_tuning_action_table[ k ].enterRegionFunctionPtr( scorep_tuning_region_table[ i ].tuningActions[ j ].tuningParameterValue );
//                    printf("Function %p(%d)\n", scorep_tuning_action_table[ k ].enterRegionFunctionPtr,
//                           scorep_tuning_region_table[ i ].tuningActions[ j ].tuningParameterValue );
                    }
                }
//TODO: Check what to do with this
//                if ( scorep_tuning_action_table[ k ].exitRegionFunctionPtr )
//                {
///* Apply the tuning action */
//                    printf("exit_region: This is the place where I call the function with the variable set to = %d!\n", scorep_tuning_region_table[ i ].tuningActions[ j ].tuningParameterValue );
//                    scorep_tuning_action_table[ k ].exitRegionFunctionPtr( scorep_tuning_region_table[ i ].tuningActions[ j ].tuningParameterValue );
//                    printf("Function %p(%d)\n", scorep_tuning_action_table[ k ].exitRegionFunctionPtr,
//                           scorep_tuning_region_table[ i ].tuningActions[ j ].tuningParameterValue );
//                }
            }
        }
    }
    else
    {
//        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, "exit_region: region %s with Id %u not found! Tuning action will not be applied!\n", regionName, regionId );
//        SCOREP_Tuning_PrintActionTable();
    }
}

static void
add_tuning_action( uint32_t regionId,
                   uint8_t  kind,
                   char*    name,
                   int      value )
{
    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING,
                        "Adding tuning action: %s for regionId %d of type %d = %d\n",
                        name, regionId, kind, value );

    uint16_t i               = scorep_tuning_region_table_find_region_or_add( regionId );
    uint16_t locationInTable = scorep_tuning_action_table_find_action( name );
    scorep_tuning_region_table_find_action_or_add( i, kind, locationInTable, value );
}

static void
assign_id( size_t id )
{
    plugin_id = id;
}

//static void
//init_mpp( void )
//{
//}

static void
create_location( const struct SCOREP_Location* location,
                 const struct SCOREP_Location* parentLocation )
{
}

//static void
//activate_cpu_location( const struct SCOREP_Location* location,
//                       const struct SCOREP_Location* parentLocation,
//                       uint32_t                      forkSequenceCount )
//{
//}
//
//static void
//deactivate_cpu_location( const struct SCOREP_Location* location,
//                         const struct SCOREP_Location* parentLocation )
//{
//}

static void
delete_location( const struct SCOREP_Location* location )
{
}

//static void
//pre_unify( void )
//{
//}
//
//static void
//assign_global_id( struct SCOREP_Location* location )
//{
//}
//
//static void
//write_data( void )
//{
//}

static void
core_task_create( const struct SCOREP_Location* location,
                  SCOREP_TaskHandle             taskHandle )
{
}

static void
core_task_complete( const struct SCOREP_Location* location,
                    SCOREP_TaskHandle             taskHandle )
{
}

//static void
//new_definition_handle( SCOREP_AnyHandle  handle,
//                       SCOREP_HandleType type )
//{
//}

static uint32_t
get_event_functions( SCOREP_Substrates_Mode        mode,
                     SCOREP_Substrates_Callback** functions )
{
    SCOREP_Substrates_Callback* registerd_functions       =
            ( SCOREP_Substrates_Callback* ) calloc( SCOREP_SUBSTRATES_NUM_EVENTS, sizeof( SCOREP_Substrates_Callback ) );
    registerd_functions[ SCOREP_EVENT_ENTER_REGION ]      = ( SCOREP_Substrates_Callback ) enter_region;
    registerd_functions[ SCOREP_EVENT_EXIT_REGION ]       = ( SCOREP_Substrates_Callback ) exit_region;
    registerd_functions[ SCOREP_EVENT_ADD_TUNING_ACTION ] = ( SCOREP_Substrates_Callback ) add_tuning_action;
    *functions                                            = registerd_functions;
    return SCOREP_SUBSTRATES_NUM_EVENTS;
}

/* Receive callbacks from Score-P */
static void
set_callbacks( const SCOREP_SubstratePluginCallbacks* incoming_callbacks,
               size_t                                 size )
{
    assert( sizeof( SCOREP_SubstratePluginCallbacks ) <= size );
    callbacks = incoming_callbacks;
}

SCOREP_SUBSTRATE_PLUGIN_ENTRY( tuning )
{
    SCOREP_SubstratePluginInfo info;
    memset( &info, 0, sizeof( SCOREP_SubstratePluginInfo ) );

    //override initialize and finalize
    info.plugin_version      = SCOREP_SUBSTRATE_PLUGIN_VERSION;
    info.init                = initialize;
    info.assign_id           = assign_id;
    //info.init_mpp            = init_mpp;
    info.finalize            = finalize;
    info.create_location     = create_location;
    //info.activate_cpu_location   = activate_cpu_location;
    //info.deactivate_cpu_location = deactivate_cpu_location;
    info.delete_location     = delete_location;
    //info.pre_unify           = pre_unify;
    //info.assign_global_id    = assign_global_id;
    //info.write_data          = write_data;
    info.core_task_create    = core_task_create;
    info.core_task_complete  = core_task_complete;
    //info.new_definition_handle = new_definition_handle;
    info.get_event_functions = get_event_functions;
    info.set_callbacks       = set_callbacks;

    return info;
}
