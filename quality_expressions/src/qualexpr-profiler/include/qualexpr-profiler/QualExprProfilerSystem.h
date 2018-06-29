/**
   @file    QualExprProfilerSystem.h
   @ingroup QualityExpressionProfilerInternal
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

#ifndef QUALEXPR_PROFILER_SYSTEM_H_
#define QUALEXPR_PROFILER_SYSTEM_H_

#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#include <string>

#include "quality-expressions/QualityExpressionsProfiler.h"
#include "qualexpr-profiler/QualExprProfilerSystem.h"

namespace quality_expressions_core {
/**
   @class QualExprTimerStdSysTime
   @brief Provide the standard POSIX timestamp.
   @ingroup QualityExpressionProfilerInternal
 */
class QualExprTimerStdSysTime : public QualExprTimer {
public:
    /* Constructor */ QualExprTimerStdSysTime( void ) {
    }
    /* Constructor */ virtual ~QualExprTimerStdSysTime( void ) {
    }

    virtual double timestamp( void ) {
        struct timeval  chrono;
        struct timezone tz;

        gettimeofday( &chrono, &tz );
        double chrono_value = chrono.tv_sec + chrono.tv_usec / 1000000.0;
        return chrono_value;
    }
};

/**
   @class QualExprTimerStdUnix
   @brief Provide the standard Unix timestamp.
   @ingroup QualityExpressionProfilerInternal
 */
class QualExprTimerStdUnix : public QualExprTimer {
public:         /* WARNING ! The Contructor is not THREAD SAFE ! */
    /* Constructor */ QualExprTimerStdUnix( void ) : m_clock_id( m_start_clock_id ) {
        m_start_clock_id++;
        struct timespec tp;
        tp.tv_sec  = 0;
        tp.tv_nsec = 0;
        /* int error =*/ clock_settime( m_clock_id, &tp );
    }
    /* Constructor */ virtual ~QualExprTimerStdUnix( void ) {
    }

    virtual double timestamp( void ) {
        struct timespec tp;
        /*int error =*/ clock_gettime( m_clock_id, &tp );
        return ( double )( tp.tv_sec )  + ( double )( tp.tv_nsec ) / 1e9;
    }

    static clockid_t m_start_clock_id;
    clockid_t        m_clock_id;
};
} // /quality_expressions

#endif
