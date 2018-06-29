/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2011,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2011, 2013, 2015,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2011,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2011,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2011, 2014,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2011, 2015-2016,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */

#ifndef SCOREP_USER_TUNING_FUNCTIONS_H
#define SCOREP_USER_TUNING_FUNCTIONS_H

/**
    @file       SCOREP_UserTuning_Functions.h
    @ingroup    SCOREP_UserTuning_External

    This File contains the function definitions which are called from the user manual
    instrumentation.

    @note The user should not insert calls to this functions directly, but
    use the macros provided in SCOREP_User.h.
 */


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
    @defgroup SCOREP_User_External External visisble functions of the Score-P user adapter
    @ingroup SCOREP_User

    This module contains all external visible items of the user adapter except the
    macros for the manual user instrumentation.

    @note We strongly recommend not to insert
    calls to this functions for instrumentation, but use the provided macros instead.

    @{
 */

/* **************************************************************************************
 * Tuning functions
 * *************************************************************************************/

/**
    Generates an event that maps a tuning parameter into a variable.
 */
void
SCOREP_User_TuningMapTuningParameterToVariable( char* tuningParameterName,
                                                int*  variantSelector,
                                                int   restoreValueFlag );

/**
    Generates an event that maps a tuning parameter into a function.
 */
void
SCOREP_User_TuningMapTuningParameterToFunction( char*    tuningParameterName,
                                                void ( * variantSelector )( int,
                                                                            int* ),
                                                int      restoreValueFlag );



#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

/** @} */

#endif /* SCOREP_USER_TUNING_FUNCTIONS_H */
