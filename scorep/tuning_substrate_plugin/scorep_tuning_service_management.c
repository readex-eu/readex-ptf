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
 *
 *  @file
 *
 *
 *  @brief This module handles implementations of tuning control plugins.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <stdio.h>

#include <UTILS_Error.h>
#define SCOREP_DEBUG_MODULE_NAME TUNING
#include <UTILS_Debug.h>

#include "scorep_tuning_table.h"
#include "scorep_tuning_service_management.h"

#define BUFFER_SIZE 512

/** Per plugin library information */
typedef struct scorep_tuning_plugin_struct
{
    /** Info from get_info() */
    SCOREP_Tuning_Plugin_Info info;
    /** Handle (should be closed when finalize) */
    void*                     dlfcn_handle;
    /** Plugin name */
    char*                     plugin_name;
} scorep_tuning_plugin;

/** Static variable to control initialize status of the tuning control plugins source.
 *  If it is 0 it is initialized. */
static int scorep_tuning_plugins_initialize = 1;

/** A number of plugin libraries, [library_nr] */
static scorep_tuning_plugin* scorep_tuning_plugin_handles;

/** Number of used plugins */
static uint32_t num_plugins;

/** Contains the separator of tuning parameter control plugin names. */
static char* tuning_parameter_control_plugins_separator = NULL;

#define SCOREP_TUNING_PLUGINS_SEPARATOR ","

/* *********************************************************************
 * Macros
 **********************************************************************/

/* *INDENT-OFF* */


/* *INDENT-ON* */

/* *********************************************************************
 * Service management
 **********************************************************************/

/** @brief  Initialize tuning control plugins source.
 *
 *  During initialization respective tuning actions are read from plugins
 *  and registered with the tuning substrate.
 *
 *  @return It returns SCOREP_SUCCESS if successful,
 *          otherwise an error code will be reported.
 */
int tuning_subsystem_init( void )
{
    char* scorep_tunings_plugins = getenv( "SCOREP_TUNING_PLUGINS" );
    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, " initialize tuning control plugins source." );
    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, " list of plugins = %s\n", scorep_tunings_plugins );
    if ( scorep_tunings_plugins == NULL )
    {
        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, "No tuning control plugin found" );
        return 0;
    }

    tuning_parameter_control_plugins_separator = getenv( "SCOREP_TUNING_PLUGINS_SEP" );
    if ( tuning_parameter_control_plugins_separator == NULL )
    {
         tuning_parameter_control_plugins_separator = SCOREP_TUNING_PLUGINS_SEPARATOR;
    }

    /* Working copy of environment variable content */
    char* env_var_content;
    /* Pointer to single character of tuning specification string */
    char* token;

    /* Buffer to extend plugin name with .so suffix */
    char buffer[ BUFFER_SIZE ];
    /* Handle from dlopen() */
    void* handle;
    /* Information about plugin */
    SCOREP_Tuning_Plugin_Info info;
    /* String with error message if there is a problem while dlopen() */
    char* dl_lib_error;
    /* Tuning action info */
    SCOREP_Tuning_Action_Info* tuning_action_info = NULL;

    /* List of plugin names */
    char** plugins = NULL;

    /* Use union to get rid of compiler warning */
    union
    {
        void*                     void_ptr;
        SCOREP_Tuning_Plugin_Info ( * function )( void );
    } get_info;

    /* Number of selected plugins */
    uint32_t num_selected_plugins = 0;

    /* Read content of environment variable */
    int env_var_content_len = strlen( scorep_tunings_plugins );
    env_var_content = malloc( ( env_var_content_len + 1 ) * sizeof( char ) );
    env_var_content = strcpy( env_var_content, scorep_tunings_plugins );

    /* Return if environment variable is empty */
    if ( strlen( env_var_content ) == 0 )
    {
        free( env_var_content );
        return 0;
    }

    /* Read plugin names from specification string */
    token = strtok( env_var_content, tuning_parameter_control_plugins_separator );
    while ( token )
    {
        int is_new_plugin = 1;

        /* Avoid loading the same plugin multiple times */
        for ( uint32_t i = 0; i < num_selected_plugins; i++ )
        {
            if ( strcmp( plugins[ i ], token ) == 0 )
            {
                /* This plugin was already specified */
                is_new_plugin = 0;
                break;
            }
        }

        if ( is_new_plugin == 1 )
        {
            num_selected_plugins++;
            /* Allocate the plugin name buffer */
            plugins = realloc( plugins, num_selected_plugins * sizeof( char* ) );
            UTILS_BUG_ON( plugins == NULL, "Out of memory." );
            /* Copy the content to the buffer */
            int token_len = strlen( token );
            plugins[ num_selected_plugins - 1 ] = malloc( ( token_len + 1 ) * sizeof( char ) );
            plugins[ num_selected_plugins - 1 ] = strcpy( plugins[ num_selected_plugins - 1 ], token );

        }

        /* Handle next plugin */
        token = strtok( NULL, tuning_parameter_control_plugins_separator );
    }
    free( env_var_content );

    /* Go through all plugins */
    for ( uint32_t i = 0; i < num_selected_plugins; i++ )
    {
        char* current_plugin_name = plugins[ i ];

        /* Load it from LD_LIBRARY_PATH*/
        snprintf( buffer, BUFFER_SIZE, "lib%s.so", current_plugin_name );

        /* Now use dlopen to load dynamic library
         * RTLD_NOW: all undefined symbols in the library are resolved
         *           before dlopen() returns, if this cannot be done,
         *           an error is returned. */
        handle = dlopen( buffer, RTLD_NOW );

        /* If it is not valid */
        dl_lib_error = dlerror();
        if ( dl_lib_error != NULL )
        {
            UTILS_WARNING( "Could not open tuning control plugin %s. Error message was: %s",
                           current_plugin_name,
                           dl_lib_error );
            continue;
        }

        /* Now get the address where the
         * info symbol is loaded into memory */
        snprintf( buffer, BUFFER_SIZE, "SCOREP_TuningPlugin_%s_get_info", current_plugin_name );
        get_info.void_ptr = dlsym( handle, buffer );
        dl_lib_error      = dlerror();
        if ( dl_lib_error != NULL )
        {
            UTILS_WARNING( "Could not find symbol 'get_info' of tuning control plugin %s. Error message was: %s",
                           current_plugin_name,
                           dl_lib_error );
            dlclose( handle );
            continue;
        }

        /* Call get_info function of plugin */
        info = get_info.function();

        if ( info.plugin_version > SCOREP_TUNING_PLUGIN_VERSION )
        {
            UTILS_WARNING( "Incompatible version of tuning control plugin detected. You may experience problems. (Version of %s plugin %u, support up to version %u)\n",
                           current_plugin_name,
                           info.plugin_version,
                           SCOREP_TUNING_PLUGIN_VERSION );
        }

        if ( info.initialize == NULL )
        {
            UTILS_WARNING( "Initialization function not implemented in plugin %s\n", current_plugin_name );
            /* Try loading next */
            continue;
        }

        if ( info.finalize == NULL )
        {
            UTILS_WARNING( "Finalization function not implemented in plugin %s\n", current_plugin_name );
            /* Try loading next */
            continue;
        }

        num_plugins++;

        scorep_tuning_plugin_handles =
            realloc( scorep_tuning_plugin_handles,
                     num_plugins * sizeof( scorep_tuning_plugin ) );
        UTILS_BUG_ON( scorep_tuning_plugin_handles == NULL, "Out of memory." );

        scorep_tuning_plugin* current_plugin = &scorep_tuning_plugin_handles[ num_plugins - 1 ];

        /* Clear out current plugin */
        memset( current_plugin, 0, sizeof( scorep_tuning_plugin ) );

        /* Add handle (should be closed in the end) */
        current_plugin->dlfcn_handle = handle;

        /* Store the info object of the plugin */
        current_plugin->info = info;

        /* Store the name of the plugin */
        int current_plugin_name_len = strlen( current_plugin_name ) + 1;
        current_plugin->plugin_name = malloc( current_plugin_name_len * sizeof( char ) );
        current_plugin->plugin_name = strcpy( current_plugin->plugin_name, current_plugin_name );

        /* Initialize plugin */
        if ( current_plugin->info.initialize() )
        {
            UTILS_WARNING( "Error while initializing plugin %s, initialization returned != 0\n", current_plugin_name );
            continue;
        }

        tuning_action_info = current_plugin->info.get_tuning_info();
        if ( tuning_action_info == NULL )
        {
            UTILS_WARNING( "Error while loading tuning actions from plugin %s, initialization returned != 0\n", current_plugin_name );
            continue;
        }
        else
        {
            uint32_t tai = 0;
            while ( tuning_action_info[ tai ].name != NULL )
            {
                scorep_tuning_action_table_fill_entry( tuning_action_info[ tai ].name,
                                                       tuning_action_info[ tai ].kind,
                                                       tuning_action_info[ tai ].restoreValueFlag,
                                                       tuning_action_info[ tai ].enterRegionVariablePtr,
                                                       tuning_action_info[ tai ].exitRegionVariablePtr,
                                                       tuning_action_info[ tai ].enterRegionFunctionPtr,
                                                       tuning_action_info[ tai ].exitRegionFunctionPtr,
                                                       tuning_action_info[ tai ].validationEndRegionFunctionPtr,
                                                       SCOREP_LANGUAGE_C );
                tai++;
            }
        }
    }

    for ( uint32_t i = 0; i < num_selected_plugins; i++ )
    {
        free( plugins[ i ] );
    }
    free( plugins );

    /* ************************************************************** */

    scorep_tuning_plugins_initialize = 0;

    return 0;
}

/** @brief Service finalization.
 */
int
tuning_subsystem_finalize( void )
{
    /* Call only, if previously initialized */
    if ( !scorep_tuning_plugins_initialize )
    {
        for ( uint32_t j = 0; j < num_plugins; j++ )
        {
            /* Call finalization function of plugin */
            scorep_tuning_plugin_handles[ j ].info.finalize();

            /* Free resources */
            free( scorep_tuning_plugin_handles[ j ].plugin_name );

            dlclose( scorep_tuning_plugin_handles[ j ].dlfcn_handle );
        }
        free( scorep_tuning_plugin_handles );

        /* Set initialization flag */
        scorep_tuning_plugins_initialize = 1;
        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, " finalize tuning subsystem." );
    }
    return 0;
}
