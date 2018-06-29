/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2012,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2012,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2012, 2014-2015,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2012,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2014,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2012,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2012, 2015-2016,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 *
 */

/**
 *  @file
 *
 *  This file contains the implementation of user adapter functions concerning
 *  regions.
 */

#include <SCOREP_Tuning_Functions.h>


void
SCOREP_User_TuningMapTuningParameterToVariable( char* tuningParameterName,
                                                int*  variantSelector,
                                                int   restoreValueFlag )
{
    SCOREP_Tuning_MapTuningParameterToVariable( tuningParameterName, variantSelector, restoreValueFlag );
}


void
SCOREP_User_TuningMapTuningParameterToFunction( char*    tuningParameterName,
                                                void ( * variantSelector )( int,
                                                                            int* ),
                                                int      restoreValueFlag )
{
    SCOREP_Tuning_MapTuningParameterToFunction( tuningParameterName, variantSelector, restoreValueFlag, SCOREP_LANGUAGE_C );
}
