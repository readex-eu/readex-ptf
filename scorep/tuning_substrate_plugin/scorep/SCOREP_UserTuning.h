/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2011,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2011, 2014-2015,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2011,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2011, 2013-2014,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2011, 2014-2015,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2011, 2015-2016,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

#ifndef SCOREP_TUNING_USER_H
#define SCOREP_TUNING_USER_H

/**
    @file       SCOREP_UserTuning.h
    @ingroup    SCOREP_UserTuning

    @brief This file contains the interface for the manual user instrumentation.
 */

/* Guarded because it declares variables in every file where it is included. */
#ifdef SCOREP_USER_ENABLE
#include <scorep/SCOREP_UserTuning_Functions.h>
#endif

/**
    @defgroup SCOREP_User Score-P User Adapter

    The user adapter provides a set of macros for user manual instrumentation. The macros
    are inserted in the source code and call functions of the Score-P runtime system.
    The user should avoid calling the Score-P runtime functions directly.

    For every macro, two definitions are provided: The first one inserts calls to the
    Score-P runtime system, the second definitions resolve to nothing. Which
    implementation is used,
    depends on the definition of SCOREP_USER_ENABLE. If SCOREP_USER_ENABLE is defined, the
    macros resolve to calls to the Score-P runtime system. If SCOREP_USER_ENABLE is
    undefined,
    the user instrumentation is removed by the preprocessor. This flag SCOREP_USER_ENABLE
    should be set through the instrumentation wrapper tool automatically if user
    manual instrumentation is enabled.

    Every source file which is instrumented must include a header file
    with the Score-P user instrumentation header. For C/C++ programs,
    the header file is 'scorep/SCOREP_User.h', for Fortran files,
    'scorep/SCOREP_User.inc' must be included. Because the Fortran
    compilers cannot expand macros, the Fortran source code must be
    preprocessed by a C or C++ preprocessor, to include the headers
    and expand the macros. Which Fortran files are passed to the
    preprocessor depends on the suffix.  Usually, suffixes .f and .f90
    are not preprocessed, .F and .F90 files are preprocessed. However,
    this may depend on the used compiler.

   @{
 */

/* **************************************************************************************
 * Documentation for region enclosing macros
 * *************************************************************************************/

/**
    @name Macros for region instrumentation
    @{
 */


/**
    @def SCOREP_USER_TUNING_MAP_VARIABLE(handle)
    This macro maps a variable used as a tuning parameter.
    TODO: This description has to be appended.

    @param handle The handle of the region to be started. This handle must be declared
                  using SCOREP_USER_REGION_DEFINE or SCOREP_USER_GLOBAL_REGION_DEFINE before.

    C/C++ example:
    @code
    void myfunc()
    {
      SCOREP_USER_REGION_DEFINE( my_region_handle )

      // do something

      SCOREP_USER_TUNING_MAP_VARIABLE( my_region_handle )
    }
    @endcode

    Fortran example:
    @code
    program myProg
      SCOREP_USER_REGION_DEFINE( my_region_handle )
      ! more declarations

      SCOREP_USER_TUNING_MAP_VARIABLE( my_region_handle )

    end program myProg
    @endcode
 */


/**
    @def SCOREP_USER_TUNING_MAP_FUNCTION(handle)
    This macro maps a function used as a tuning parameter.
    TODO This description has to be appended.

    @param handle The handle of the region to be started. This handle must be declared
                  using SCOREP_USER_REGION_DEFINE or SCOREP_USER_GLOBAL_REGION_DEFINE before.

    C/C++ example:
    @code
    void myfunc()
    {
      SCOREP_USER_REGION_DEFINE( my_region_handle )

      // do something

      SCOREP_USER_TUNING_MAP_FUNCTION( my_region_handle )
    }
    @endcode

    Fortran example:
    @code
    program myProg
      SCOREP_USER_REGION_DEFINE( my_region_handle )
      ! more declarations

      SCOREP_USER_TUNING_MAP_FUNCTION( my_region_handle )

    end program myProg
    @endcode
 */
/**@}*/

/* **************************************************************************************
 * Region enclosing macros
 * *************************************************************************************/
/* Empty define for SCOREP_USER_FUNC_DEFINE to allow documentation of the macro and
   let it disappear in C/C++ codes */
#define SCOREP_USER_FUNC_DEFINE()

#ifdef SCOREP_USER_ENABLE

#define SCOREP_USER_TUNING_MAP_VARIABLE( tuningParameterName, variantSelector, restoreValueFlag ) \
    SCOREP_User_TuningMapTuningParameterToVariable( tuningParameterName, variantSelector, restoreValueFlag );

#define SCOREP_USER_TUNING_MAP_FUNCTION( tuningParameterName, variantSelector, restoreValueFlag ) \
    SCOREP_User_TuningMapTuningParameterToFunction( tuningParameterName, variantSelector, restoreValueFlag );

#else // SCOREP_USER_ENABLE

#define SCOREP_USER_TUNING_MAP_VARIABLE( tuningParameterName, variantSelector, restoreValueFlag )
#define SCOREP_USER_TUNING_MAP_FUNCTION( tuningParameterName, variantSelector, restoreValueFlag )

#endif // SCOREP_USER_ENABLE

/** @} */

#endif /* SCOREP_TUNING_USER_H */
