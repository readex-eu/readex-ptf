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
 * @file       SCOREP_TuningPlugins.h
 *
 * @brief Tuning plugin definitions.
 */


#ifndef SCOREP_TUNING_PLUGINS_H_
#define SCOREP_TUNING_PLUGINS_H_


/**
 * The developer of a tuning plugin should provide a README file which
 * explains how to compile, install and use the plugin. In particular,
 * all supported tuning actions should be described in the README file.
 *
 * Each tuning plugin has to include <tt>SCOREP_TuningPlugin.h</tt>
 * and implement 'get_info' function. Therefore, use the
 * SCOREP_TUNING_PLUGIN_ENTRY macro and provide the name of the plugin
 * library as the argument. For example, the tuning plugin libexample.so
 * should use SCOREP_TUNING_PLUGIN_ENTRY( example ). The 'get_info'
 * function returns a SCOREP_Tuning_Plugin_Info data structure which
 * contains information about the plugin and its tuning actions.
 *
 * Mandatory functions
 *
 * @ initialize
 * This function is called once per process event. It registers tuning
 * actions If all requirements are met, data structures used by the
 * plugin can be initialized within this function.
 *
 * @ finalize
 * This function is called once per process event. It unregisters tuning
 * actions and internal infrastructure of a tuning plugin (e.g., free allocated
 * resources).
 *
 * @ plugin_version
 * Should be set to SCOREP_TUNING_PLUGIN_VERSION
 *
 */

#include <stdint.h>
#include <scorep/SCOREP_TuningTypes.h>

/** Current version of Score-P tuning plugin interface */
#define SCOREP_TUNING_PLUGIN_VERSION 0

#ifdef __cplusplus
#   define EXTERN extern "C"
#else
#   define EXTERN extern
#endif

/** Macro used for implementation of the 'get_info' function */
#define SCOREP_TUNING_PLUGIN_ENTRY( _name ) \
    EXTERN SCOREP_Tuning_Plugin_Info \
    SCOREP_TuningPlugin_ ## _name ## _get_info( void )


/* *********************************************************************
 * Type definitions
 **********************************************************************/

/**
 * Information describing the tuning plugin.
 *
 */
typedef struct SCOREP_Tuning_Plugin_Info
{
    /*
     * For all plugins
     */

    /** Should be set to SCOREP_PLUGIN_VERSION
     *  (needed for back- and forward compatibility)
     */
    uint32_t plugin_version;

    /** This functions is called once per process to initialize
     *  tuning plugin. It should return 0 if successful.
     *
     *  @return 0 if successful.
     */
    int32_t ( * initialize )( void );

    /** This functions is called once per process to finalize
     *  tuning plugin.
     */
    void ( * finalize )( void );

    /** This datastructure returns data about tuning actions and is null
     * terminated.
     *
     *  @return Meta data (called properties) about this metric plugin.
     */
    SCOREP_Tuning_Action_Info* ( *get_tuning_info )( void );

    /** Some space for future stuff, should be zeroed */
    uint64_t reserved[ 100 ];
} SCOREP_Tuning_Plugin_Info;


#endif /* SCOREP_TUNING_PLUGINS_H_ */
