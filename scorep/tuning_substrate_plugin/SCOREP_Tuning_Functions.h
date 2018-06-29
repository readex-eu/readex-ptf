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


#ifndef SCOREP_TUNING_FUNCTIONS_H
#define SCOREP_TUNING_FUNCTIONS_H


/**
 * @file
 *
 * @brief   Declarations of Tuning module user interface.
 *
 */

#include <scorep/SCOREP_TuningTypes.h>

/**
 * Description of the function
 */

void
SCOREP_Tuning_MapTuningParameterToVariable( char* tuningParameterName,
                                            int*  variantSelector,
                                            int   restoreValueFlag );

void
SCOREP_Tuning_MapTuningParameterToFunction( char* tuningParameterName,
                                            void ( * variantSelector )( int, int* ),
                                            int restoreValueFlag,
                                            SCOREP_Language languageType );

#endif /* SCOREP_TUNING_FUNCTIONS_H */
