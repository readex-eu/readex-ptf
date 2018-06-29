/**
   @file    QualityExpressionsProfilerLocal.h
   @ingroup QualityExpressionProfilerLocal
   @brief   Quality Expression profiling headers for local events
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

#ifndef QUALEXPRSEMANTIC_LOCAL_DEF

#ifndef QUALITYEXPRESSION_PROFILERLOCAL_H_
#define QUALITYEXPRESSION_PROFILERLOCAL_H_

#include "quality-expressions/QualityExpressionsProfiler.h"

/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */

/**
 * @defgroup QualityExpressionProfilerLocal Local profiling system
 * Minimum profiling system provided with the quality expression library.
 * @ingroup QualityExpressionProfiler
 */

/** @def QE_PROFILER_LOCAL_BASE
    @brief Define the starting range for the semantic.
    @ingroup QualityExpressionProfilerLocal
 */
#define QE_PROFILER_LOCAL_BASE          0x10000

/** @def QE_PROFILER_LOCAL_MASK
    @brief Define the mask valid for the port range local semantic.
    Note that at this stage, we never expect to have more than 255 event types for this profiler
    @ingroup QualityExpressionProfilerLocal
 */
#define QE_PROFILER_LOCAL_MASK          0xFF

/** @brief Define the list of local events.
    @ingroup QualityExpressionProfilerLocal
 */
typedef enum qualexpr_event_local_t {
    QE_PROFILER_LOCAL_UNDEFINED = QE_PROFILER_LOCAL_BASE,       //!< Default event undefined.

#define QUALEXPRSEMANTIC_LOCAL_DEF( DefName, matchSemanticExpression )  \
    QE_PROFILER_LOCAL_ ## DefName,
#include "quality-expressions/QualityExpressionsProfilerLocal.h"
#undef QUALEXPRSEMANTIC_LOCAL_DEF
    QE_PROFILER_LOCAL_NOMORE                                    //!< List end.
} qualexpr_event_local_t;

#ifdef __cplusplus
extern "C" {
#endif

semantic_namespace_t qualExpr_registerListener( listen_event_func_t listener );

void qualExpr_unregisterListener( listen_event_func_t listener );

struct profiling_event_t* qualExpr_startEvent( enum qualexpr_event_local_t semantic,
                                               long long                   value );

void qualExpr_stopEvent( struct profiling_event_t* eventStarted,
                         long long                 value );

void* qualExpr_startSection( void );

void qualExpr_stopSection( void* handler );

#ifdef __cplusplus
}
#endif

#endif // /QUALITYEXPRESSION_PROFILERLOCAL_H_

#else  // --------------------- List section -------------------------------
QUALEXPRSEMANTIC_LOCAL_DEF( RegionExecution,                     ( sem == QE_PROFILER_LOCAL_RegionExecution ) )                             //!< Delimit a profiling region.
QUALEXPRSEMANTIC_LOCAL_DEF( SectionExecution,                    ( sem == QE_PROFILER_LOCAL_SectionExecution ) )                            //!< Delimit a profiling section.
#endif
