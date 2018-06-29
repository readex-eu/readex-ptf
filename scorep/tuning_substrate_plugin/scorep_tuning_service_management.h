/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2015-2016,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2015-2016,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */


#ifndef SCOREP_TUNING_SERVICE_MANAGEMENT_H_
#define SCOREP_TUNING_SERVICE_MANAGEMENT_H_

/**
 * @file
 *
 */


#include "scorep/SCOREP_TuningPlugins.h"
#include "scorep/SCOREP_TuningTypes.h"

int
tuning_subsystem_init( void );

int
tuning_subsystem_finalize( void );



#endif /* SCOREP_TUNING_SERVICE_MANAGEMENT_H_ */
