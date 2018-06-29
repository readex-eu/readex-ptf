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


#ifndef SCOREP_TUNING_TABLE_H
#define SCOREP_TUNING_TABLE_H

/**
 * @file
 *
 */

// #include <SCOREP_ErrorCodes.h>
#include "scorep_tuning_stack.h"
#include <scorep/SCOREP_TuningTypes.h>
#include <scorep/SCOREP_PublicTypes.h>


typedef enum SCOREP_TuningReturnCodeStruct
{
    SCOREP_TUNING_FOUND = 0,
    SCOREP_TUNING_NOTFOUND
} SCOREP_TuningReturnCodeType;

typedef struct SCOREP_TuningActionStruct
{
    char*                   name;                     /**< Tuning action name */
    SCOREP_TuningActionKind kind;                     /**< Type of tuning action (variable/function)*/
    unsigned int*           enterRegionVariablePtr;   /**< Variable updated at enter region event */
    unsigned int*           exitRegionVariablePtr;    /**< Variable updated at exit region event */
    void ( * enterRegionFunctionPtr )( int,
                                       int* );        /**< Function called at enter region event */
    void ( * exitRegionFunctionPtr )( int,
                                      int* );         /**< Function called at exit region event */
//TODO: Not implemented, should we implement it or remove?
    void ( * validationEndRegionFunctionPtr )( int ); /**< Function called to validate numerical stability at exit region event */
    int               restoreValueFlag;               /**< Restore the previous value at region exit event */
    SCOREP_StackType* restoreValueStack;              /**< Stack used to keep the previous values of the tuning parameter */
    SCOREP_Language   languageType;                   /**< Language of the function tuning parameter */
} SCOREP_TuningActionType;

typedef struct SCOREP_TuningParameterStruct
{
    SCOREP_TuningActionKind tuningActionKind;
    int                     tuningParameterValue;
    uint16_t                tuningActionListIndex;
} SCOREP_TuningParameterType;

typedef struct SCOREP_TuningRegionStruct
{
    uint32_t                    regionId;
    SCOREP_TuningParameterType* tuningActions;
    uint16_t                    max_tuning_action_entries;
    uint16_t                    next_free_tuning_action_entry;
} SCOREP_TuningRegionType;

typedef struct SCOREP_OATuningActionStruct
{
    uint32_t                regionId;
    SCOREP_TuningActionKind kind;
    int                     value;
    uint16_t                locationInTable;
} SCOREP_OATuningActionType;

typedef void ( * SCOREP_Tuning_FunctionCb )( int );

void
scorep_tuning_initialize_region_table( void );

void
scorep_tuning_initialize_action_table( void );

void
scorep_tuning_finalize_region_table( void );

void
scorep_tuning_finalize_action_table( void );

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
                                       SCOREP_Language         languageType );

uint16_t
scorep_tuning_region_table_find_region_or_add( uint32_t regionId );

SCOREP_TuningReturnCodeType
scorep_tuning_region_table_find_region( uint16_t* location,
                                        uint32_t  regionId );

uint16_t
scorep_tuning_action_table_find_action( char* name );

void
scorep_tuning_region_table_find_action_or_add( uint16_t                regionLocation,
                                               SCOREP_TuningActionKind actionType,
                                               uint16_t                locationInTuningActionTable,
                                               int                     tuningParameterValue );


//TODO: Goes into the user adapter?
void
scorep_tuning_map_tuning_parameter_to_variable( char*         tuningParameterName,
                                                unsigned int* variantSelector,
                                                int           restoreValueFlag );

void
scorep_tuning_map_tuning_parameter_to_function( char*           tuningParameterName,
                                                void ( *        variantSelector )( int,
                                                                                   int* ),
                                                int             restoreValueFlag,
                                                SCOREP_Language languageType );


void
scorep_tuning_print_region_table( void );

void
scorep_tuning_print_action_table( void );

#endif /* SCOREP_TUNING_TABLE_H */
