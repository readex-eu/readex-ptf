/**
   @file    QualExprManager.h
   @ingroup QualityExpressionCore
   @brief   Quality Expression manager headers
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

#ifndef QUALEXPRMANAGER_H_
#define QUALEXPRMANAGER_H_

#include <string>
#include <map>

#include "quality-expressions/QualityExpressionsProfilerSystem.h"
#include "qualexpr-evaluator/QualExprEvaluator.h"
#include "qualexpr-profiler/QualExprProfiler.h"

namespace quality_expressions_core {
/** @def   QUALEXPR_VERBOSITY_ENVNAME
    @ingroup QualityExpressionCore
    @brief Verbosity integer environment variable
    @sa    debug_level_t
 */
#define QUALEXPR_VERBOSITY_ENVNAME "QUALITY_EXPRESSION_VERBOSITY"

/** @brief Define the debug verbosity level.
    @ingroup QualityExpressionCore
    @sa    QUALEXPR_VERBOSITY_ENVNAME
 */
enum debug_level_t {
    D_QUIET      = -1,                          //!< Remove error messages.
    D_OFF        = 0,                           //!< Default, no verbosity.
    D_ON         = 1,                           //!< Basic verbosity: what is done.
    D_FULL       = 2,                           //!< Even move verbosity: what is done on what object except events.
    D_FULLEVENTS = 3                            //!< Full verbosity: what is done on what object even events.
};

/**
   @class QualExprManager
   @brief Manage all quality expressions for all threads.

   This class provide a general support for the quality expression
   evaluation with the support of multi-threading.

   @ingroup QualityExpressionCore
 */
class QualExprManager : private QualExprSemaphore {
public:
    enum profiler_state_t { S_OFF = 0, S_REGISTERED, S_ON };                            //!< Define the possible profiler states.

    class Exception : public QualExprException {
public:
        /* Constructor */ Exception( const char* message ) throw( ) : QualExprException( message ) {
        }

        /* Constructor */ Exception( const std::string& message ) throw( ) : QualExprException( message ) {
        }

        /* Destructor */ ~Exception( void ) throw( ) {
        }
    };

public:                 // Class Definitions
    /* Constructor */ QualExprManager( void ) throw( );
    /* Destructor */ ~QualExprManager( void );

    void init( void );                                                                                       //!< Quality expression evaluator Initializations.

    void clear( void );                                                                                      //!< Clear all data structures.

    void log( const std::string& message,
              std::string*       o_header = NULL ) const throw( );                                      //!< Log a message to the standard error (check verbosity level).

    void registerSemanticNamespace( semantic_namespace_t ns );                                               //!< Add globaly a semantic namespace.

    size_t addGlobalCounter( const QualityExpressionEntry& entry ) throw( Exception );                                 //!< Append globally a quality expresion evaluation request.

    size_t addCounter( pthread_t                     tid,
                       const QualityExpressionEntry& entry ) throw( Exception );                                //!< Append a quality expresion evaluation request.

    long long getLongCounter( pthread_t             tid,
                              QualityExpressionID_T id ) throw( Exception );                                    //!< Retrieve the current quality expresion evaluation value.

    void removeAllCounters( void ) throw( Exception );                                                               //!< Remove all quality expresion evaluators.

    void removeCounters( pthread_t tid ) throw( Exception );                                                         //!< Remove all quality expresions.

    void resetCounters( pthread_t tid ) throw( Exception );                                                          //!< Reset all quality expresions.

    void enableMeasures( pthread_t tid );  //!< Enable measurements.

    void disableMeasures( pthread_t tid ); //!< Disable measurements.

public:                               // Profiling system API
    double timestamp( void ) throw( )        {
        return m_timer.timestamp();
    }

    void event( profiling_event_t* event ) throw( );                    //!< Handle an event for quality expressions.

private:                                                           // Internal functions
    QualExprEvaluator* getEvaluator( pthread_t tid ) throw( Exception );              //!< Return the dynamic quality expression evaluator object.

    void evaluateVerbosityLevel( void );                                //!< Read the verbosity level from an environment variable.

    void display( const QualExprEvaluator& evaluator ) const throw( );  //!< Display information and the data structures (check verbosity level).

    void registerAllSemanticNamespaces( QualExprEvaluator& evaluator ); //!< Add all semantic namespaces to a fresh evaluator.

private:                                                           // Data types
    typedef std::map<pthread_t, QualExprEvaluator*> EvaluatorSet_T;

private:                                                          // Data structures
    enum debug_level_t                   m_debugLevel;            //!< Current verbosity level.
    enum profiler_state_t                m_state;                 //!< Status of the profiling system.
    QualExprTimerStdUnix                 m_timer;                 //!< Global tic-tac.
    EvaluatorSet_T                       m_evaluators;            //!< Quality expression evaluators / thread.
    QualExprEventBuilder                 m_eventBuilder;          //!< Convert basic events into event descriptors.
    std::vector<semantic_namespace_t>    m_semanticNamespaceList; //!< List of profiler semantic namespaces.
    std::vector<QualityExpressionEntry*> m_expressionList;        //!< List of quality expression entries given.
};
}
#endif /* QUALEXPRMANAGER_H_ */
