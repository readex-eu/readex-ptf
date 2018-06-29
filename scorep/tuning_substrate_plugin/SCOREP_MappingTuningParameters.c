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
 * @file
 *
 * This file contains the implementation of Tuning action functions
 */

#include <UTILS_Debug.h>

#include <scorep_tuning_table.h>

void
SCOREP_Tuning_MapTuningParameterToVariable( char* tuningParameterName,
                                            int*  variantSelector,
                                            int   restoreValueFlag )
{
    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, "Mapping tuning parameter %s to variable", tuningParameterName );

//    if ( !SCOREP_IsInitialized() )
//    {
//        SCOREP_InitMeasurement();
//    }
//
//    if ( !SCOREP_IsTuningEnabled() )
//    {
//        return;
//    }

    scorep_tuning_map_tuning_parameter_to_variable( tuningParameterName, variantSelector, restoreValueFlag );
}

void
SCOREP_Tuning_MapTuningParameterToFunction( char*           tuningParameterName,
                                            void ( *        variantSelector )( int,
                                                                               int* ),
                                            int             restoreValueFlag,
                                            SCOREP_Language languageType )
{
    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_TUNING, "Mapping tuning parameter %s to variable" );

//    if ( !SCOREP_IsInitialized() )
//    {
//        SCOREP_InitMeasurement();
//    }
//
//    if ( !SCOREP_IsTuningEnabled() )
//    {
//        return;
//    }

    scorep_tuning_map_tuning_parameter_to_function( tuningParameterName, variantSelector, restoreValueFlag, languageType );
}
