/**
   @file    QualExprProfiler.h
   @ingroup QualityExpressionProfilerInternal
   @brief   Quality expression profilers - headers
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

#ifndef QUALEXPR_PROFILER_H_
#define QUALEXPR_PROFILER_H_

#include <string>

#include "quality-expressions/QualityExpressionsProfiler.h"

/** @defgroup QualityExpressionProfilerInternal Quality expressions internal profiling tools
 *  @ingroup QualityExpressionCore
 */

namespace quality_expressions_core {
/**
   @class QualExprException
   @brief Basic interface for standard exceptions.
   @ingroup QualityExpressionProfilerInternal
 */
class QualExprException : public std::exception, public std::string {
public:
    /* Constructor */ QualExprException( const char* message ) throw( ) : std::string( message ) {
    }

    /* Constructor */ QualExprException( const std::string& message ) throw( ) : std::string( message ) {
    }

    /* Destructor */ ~QualExprException( void ) throw( ) {
    }

    virtual const char* what() const throw( )                     {
        return c_str();
    }
};

/**
   @class QualExprTimer
   @brief Interface for timers.
   @ingroup QualityExpressionProfilerInternal
 */
class QualExprTimer {
protected:
    /* Constructor */ QualExprTimer( void ) {
    }

    /* Constructor */ virtual ~QualExprTimer( void ) {
    }

public:
    virtual double timestamp( void ) = 0;
};

/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
} // /quality_expressions

#include "qualexpr-profiler/QualExprProfilerSystem.h"
#include "qualexpr-profiler/QualExprProfilerEvents.h"

#endif
