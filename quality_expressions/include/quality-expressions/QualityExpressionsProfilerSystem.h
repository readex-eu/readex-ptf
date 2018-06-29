/**
   @file    QualityExpressionsProfilerSystem.h
   @ingroup QualityExpressionProfilerInterface
   @brief   Quality expression profilers - system headers
   @author  Laurent Morin
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.

   @endverbatim
 */

#ifndef QUALITYEXPRESSION_PROFILER_SYSTEM_H_
#define QUALITYEXPRESSION_PROFILER_SYSTEM_H_

#include <stdlib.h>
#include <pthread.h>

/**
   @fn atomic_add
   @brief Interface for built-in atomic increment operation.
   @ingroup QualityExpressionProfilerInterface
 */
template<typename type>
static type atomic_add( type& var, type value ) {
    return __sync_add_and_fetch( &var, value );
}

/**
   @class QualExprSemaphore
   @brief Interface for standard semaphores.
   @ingroup QualityExpressionProfilerInterface
 */
class QualExprSemaphore {
public:
    /* Constructor */ QualExprSemaphore( void ) : m_mutex() {
        initSemaphore();
    }

    /* Destructor */ ~QualExprSemaphore( void ) {
        closeSemaphore();
    }

public:
    void lock( void ) {
        pthread_mutex_lock( &m_mutex );
    }

    void unlock( void ) {
        pthread_mutex_unlock( &m_mutex );
    }

protected:
    void initSemaphore( void ) {
        pthread_mutex_init( &m_mutex, NULL );
    }

    void closeSemaphore( void ) {
        pthread_mutex_destroy( &m_mutex );
    }

private:
    pthread_mutex_t m_mutex;                            //!< Standard Pthread semaphone handler.
};

#endif
