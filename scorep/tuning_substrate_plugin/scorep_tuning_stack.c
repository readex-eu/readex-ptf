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
 * @file
 *
 *
 */

// #include <config.h>

#include "scorep_tuning_stack.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include <UTILS_Error.h>

void
push2Stack( SCOREP_StackType* pStack,
            int               value )
{
    int* temp;
    if ( pStack->size == pStack->maxSize )
    {
        pStack->maxSize <<= 1; // Binary multiply with 2
        temp              = ( int* )realloc( pStack->value, pStack->maxSize * sizeof( int ) );
        assert( temp );
        printf( "Stack: Size = %d\n", ( int )pStack->maxSize );

//TODO: Replace with UTILS_* call
        if ( pStack->value != NULL )
        {
            pStack->value = temp;
        }
        else
        {
            free( temp );
            UTILS_FATAL( "Stack could not reallocate memory\n" );
        }
    }

    pStack->value[ pStack->size++ ] = value;
}

int
popFromStack( SCOREP_StackType* pStack )
{
    UTILS_BUG_ON( pStack->size == 0, "Stack underflow\n" );

    return pStack->value[ --pStack->size ];
}

SCOREP_StackType*
initStack( void )
{
    SCOREP_StackType* stack;
    stack = ( SCOREP_StackType* )malloc( sizeof( SCOREP_StackType ) );
    assert( stack );
    stack->size    = 0;
    stack->maxSize = 16;
    stack->value   = ( int* )malloc( stack->maxSize * sizeof( int ) );
    assert( stack->value );

    return stack;
}

void
finalizeStack( SCOREP_StackType* stack )
{
    free( stack->value );
    free( stack );
}

void
printStack( SCOREP_StackType* stack )
{
    int i;
    for ( i = 0; i < stack->size; i++ )
    {
        printf( "printStack: Element[%d] = %d\n", i, stack->value[ i ] );
    }
}
