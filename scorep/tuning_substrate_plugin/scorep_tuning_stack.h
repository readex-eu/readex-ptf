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


#ifndef SCOREP_TUNING_STACK_H
#define SCOREP_TUNING_STACK_H

/**
 * @file scorep_tuning_stack.h
 *
 */

#include <scorep/SCOREP_TuningTypes.h>
#include <stddef.h>

typedef struct SCOREP_StackType
{
    size_t size;
    size_t maxSize;
    int*   value;
} SCOREP_StackType;

void
push2Stack( SCOREP_StackType*,
            int );

int
popFromStack( SCOREP_StackType* );

SCOREP_StackType*
initStack();

void
finalizeStack( SCOREP_StackType* );

void
printStack( SCOREP_StackType* stack );

#endif /* SCOREP_TUNING_STACK_H */
