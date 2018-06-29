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

/**
 * @file       SCOREP_TuningTypes.h
 *
 * @brief Types used by metric service.
 */


#ifndef SCOREP_TUNING_TYPES_H_
#define SCOREP_TUNING_TYPES_H_

/* *********************************************************************
 * Type definitions
 **********************************************************************/

typedef enum SCOREP_TuningActionKind
{
    SCOREP_TUNING_VARIABLE = 0,
    SCOREP_TUNING_FUNCTION,
    SCOREP_TUNING_UNDEFINED
} SCOREP_TuningActionKind;

/**
 * Language of the functions registered
 *
 * Determines whether the language passes parameters by-value or by-reference.
 *
 */
typedef enum SCOREP_Language
{
    SCOREP_LANGUAGE_C,
    SCOREP_LANGUAGE_FORTRAN
} SCOREP_Language;

/**
 *
 * Data about tuning parameter
 *
 */
typedef struct SCOREP_Tuning_Action_Info
{
    char*                   name;                     /**< Tuning action name */
    SCOREP_TuningActionKind kind;                     /**< Type of tuning action (variable/function)*/
    unsigned int*           enterRegionVariablePtr;   /**< Variable updated at enter region event */
    unsigned int*           exitRegionVariablePtr;    /**< Variable updated at exit region event */
    void ( * enterRegionFunctionPtr )( int,
                                       int* );        /**< Function called at enter region event */
    void ( * exitRegionFunctionPtr )( int,
                                      int* );         /**< Function called at exit region event */
    void ( * validationEndRegionFunctionPtr )( int ); /**< Function called to validate numerical stability at exit region event */
    int             restoreValueFlag;                 /**< Restore the previous value at region exit event */
    SCOREP_Language languageType;                     /**< Language of the function tuning parameter */
} SCOREP_Tuning_Action_Info;

#endif /* SCOREP_TUNING_TYPES_H_ */
