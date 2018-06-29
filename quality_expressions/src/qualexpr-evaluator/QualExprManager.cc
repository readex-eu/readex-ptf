/**
   @file    QualExprManager.cc
   @ingroup QualityExpressionCore
   @brief   Quality Expression manager implementation
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

#include "qualexpr-evaluator/QualExprManager.h"
#include "quality-expressions/QualityExpressionsProfilerLocal.h"

namespace quality_expressions_core {
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */

/** @brief Quality Expression Manager initialization

    Initiliaze a global semaphore and a quality expression evaluator
    for the current thread.
 */
/* Constructor */ QualExprManager::QualExprManager( void ) throw( ) :
    QualExprSemaphore(), m_state( S_OFF ), m_timer(), m_eventBuilder( m_timer ) {
    evaluateVerbosityLevel();
}

/** @brief Quality Expression Manager destruction
    Warning: the clear method must be called before.
 */
/* Destructor */ QualExprManager::~QualExprManager( void ) {
}

/** @brief Quality expression evaluator Initializations
 */
void QualExprManager::init( void ) {
    lock();
    if( m_state == S_OFF ) {
        pthread_t          tid       = pthread_self();
        QualExprEvaluator* evaluator = new QualExprEvaluator();
        if( !evaluator ) {
            unlock();
            throw( Exception( "internal allocation error" ) );
        }
        registerAllSemanticNamespaces( *evaluator );
        m_evaluators[ tid ] = evaluator;
        display( *evaluator );
        m_state = S_REGISTERED;
    }
    unlock();
}

/** @brief Manager cleanup
 */
void QualExprManager::clear( void ) {
    lock();
    if( m_state == S_REGISTERED ) {
        for( size_t index = 0; index < m_expressionList.size(); index++ ) {
            delete m_expressionList[ index ];
        }
        m_expressionList.clear();
        EvaluatorSet_T::iterator ite;
        for( ite = m_evaluators.begin(); ite != m_evaluators.end(); ite++ ) {
            QualExprEvaluator* evaluator = ite->second;
            delete evaluator;
        }
        m_evaluators.clear();
        m_state = S_OFF;
    }
    unlock();
}

/** @brief Read the verbosity level from an environment variable.
 */
void QualExprManager::evaluateVerbosityLevel( void ) {
    const char* env = getenv( QUALEXPR_VERBOSITY_ENVNAME );
    if( env ) {
        m_debugLevel = ( enum debug_level_t )atoi( env );
    }
    else {
        m_debugLevel = D_OFF;
    }
}

/** @brief Log a message to the standard error (check verbosity level).

    @param message text to display
    @param o_header indentation or header text
 */
void QualExprManager::log( const std::string& message,
                           std::string*       o_header ) const throw( ) {
    if( m_debugLevel >= D_ON ) {
        if( o_header ) {
            if( o_header->empty() ) {
                std::stringstream s;
                s <<  "[quality expressions][" << getpid() << "][" << pthread_self() % 0xFF << "] ";
                *o_header = s.str();
            }
            std::cerr << *o_header << message;
        }
        else {
            std::cerr << "[quality expressions][" << getpid() << "][" << pthread_self() % 0xFF << "] " << message << std::endl;
        }
    }
}

/** @brief Display information and the data structures (check verbosity level).
 */
void QualExprManager::display( const QualExprEvaluator& evaluator ) const throw( )      {
    if( m_debugLevel >= D_ON ) {
        std::string header;
        log( "Library version ", &header );
        std::cerr << TAG << std::endl;

        if( m_debugLevel >= D_FULL ) {
            log( "available measures", &header );
            std::stringstream s;
            header =  "\n" + header + " *";
            s << header << "Semantic Namespaces: ";
            evaluator.displaySemantics( header, s );
            s << header << "Aggregator Namespaces: ";
            evaluator.displayAggregator( header, s );
            std::cerr << s.str() << std::endl;
        }
    }
}

/** @brief Return the dynamic quality expression evaluator object.
    @param tid the posix thread id the quality evaluator evaluator is attached to
 */
QualExprEvaluator* QualExprManager::getEvaluator( pthread_t tid ) throw( QualExprManager::Exception )                    {
    QualExprEvaluator* evaluator = NULL;

    lock();
    EvaluatorSet_T::iterator evaluatorIte = m_evaluators.find( tid );
    if( evaluatorIte == m_evaluators.end() ) {
        evaluator = new QualExprEvaluator();
        registerAllSemanticNamespaces( *evaluator );
        for( size_t index = 0; index < m_expressionList.size(); index++ ) {
            evaluator->pushMeasure( *m_expressionList[ index ] );
        }

        m_evaluators[ tid ] = evaluator;

        if( m_debugLevel >= D_FULLEVENTS ) {
            std::stringstream msg;
            msg << "New quality expression evaluator for thread id=" << tid;
            log( msg.str() );
        }
    }
    else {
        evaluator = evaluatorIte->second;
    }
    unlock();

    if( !evaluator ) {
        throw( Exception( "internal allocation error" ) );
    }
    return evaluator;
}

/** @brief Foward all semantic namespaces to a given evaluator.
    @param evaluator    the evaluator
 */
void QualExprManager::registerAllSemanticNamespaces( QualExprEvaluator& evaluator ) {
    for( size_t index = 0; index < m_semanticNamespaceList.size(); index++ ) {
        const QualExprSemanticNamespace& semNs = *( ( const QualExprSemanticNamespace* )( m_semanticNamespaceList[ index ] ) );
        evaluator.registerSemanticNamespace( semNs );
    }
}

/** @brief Add globaly a semantic namespace.
    @param ns    the semantic namespace
 */
void QualExprManager::registerSemanticNamespace( semantic_namespace_t ns ) {
    m_semanticNamespaceList.push_back( ns );
    const QualExprSemanticNamespace& semNs = *( ( const QualExprSemanticNamespace* )( ns ) );
    EvaluatorSet_T::iterator         ite;
    for( ite = m_evaluators.begin(); ite != m_evaluators.end(); ite++ ) {
        QualExprEvaluator* evaluator = ite->second;
        evaluator->registerSemanticNamespace( semNs );
    }
}

/** @brief Append a quality expression evaluation request to all threads.
    @param entry the quality expression entry
    @return      the number of expression item registered
 */
size_t QualExprManager::addGlobalCounter( const QualityExpressionEntry& entry ) throw( QualExprManager::Exception )        {
    size_t rcount = 0;
    if( m_state == S_REGISTERED ) {
        m_expressionList.push_back( new QualityExpressionEntry( entry ) );
        for( EvaluatorSet_T::iterator evaluatorIte = m_evaluators.begin(); evaluatorIte != m_evaluators.end(); evaluatorIte++ ) {
            rcount += addCounter( evaluatorIte->first, entry );
        }
    }
    else {
        throw( Exception( "Profiler not initialized or already activated" ) );
    }
    return rcount;
}

/** @brief Append a quality expression evaluation request.
    @param tid   thread id
    @param entry the quality expression entry
    @return      the number of expression item registered
 */
size_t QualExprManager::addCounter( pthread_t                     tid,
                                    const QualityExpressionEntry& entry ) throw( QualExprManager::Exception ) {
    size_t rcount = 0;
    if( m_state == S_REGISTERED ) {
        QualExprEvaluator* evaluator = getEvaluator( tid );
        if( m_debugLevel >= D_ON ) {
            std::stringstream msg;
            msg << "New quality expression: ";
            entry.display( "", msg );
            log( msg.str() );
        }
        try {
            rcount = evaluator->pushMeasure( entry );
        }
        catch( QualExprEvaluator::Exception e ) {
            display( *evaluator );
            throw( Exception( e.what() ) );
        }
    }
    else {
        throw( Exception( "Profiler not initialized or already activated" ) );
    }
    return rcount;
}

/** @brief Get the result of a quality expression evaluation.
    @param tid thread id
    @param id  the quality expression ID
    @return    the value
 */
long long QualExprManager::getLongCounter( pthread_t             tid,
                                           QualityExpressionID_T id ) throw( QualExprManager::Exception ) {
    if( m_state == S_REGISTERED ) {
        QualExprEvaluator* evaluator = getEvaluator( tid );
        long long          result    = 0;
        try {
            result = evaluator->getLongCounter( id );
        }
        catch( QualExprEvaluator::Exception e ) {
            display( *evaluator );
            throw( Exception( e.what() ) );
        }
        if( m_debugLevel >= D_FULLEVENTS ) {
            std::stringstream msg;
            msg << "Fetch quality expression (id:" << id << ") = " << result;
            log( msg.str() );
        }
        return result;
    }
    else {
        throw( Exception( "Profiler not initialized or already activated" ) );
    }
}

/** @brief Remove all quality expression evaluators.
 */
void QualExprManager::removeAllCounters( void ) throw( QualExprManager::Exception )      {
    if( m_state == S_REGISTERED ) {
        for( EvaluatorSet_T::iterator evaluatorIte = m_evaluators.begin(); evaluatorIte != m_evaluators.end(); evaluatorIte++ ) {
            removeCounters( evaluatorIte->first );
        }
        for( size_t index = 0; index < m_expressionList.size(); index++ ) {
            delete m_expressionList[ index ];
        }
        m_expressionList.clear();
    }
    else {
        throw( Exception( "Profiler not initialized or already activated" ) );
    }
}

/** @brief Remove a quality expression evaluation request.
    @param tid   thread id
 */
void QualExprManager::removeCounters( pthread_t tid ) throw( QualExprManager::Exception )      {
    if( m_state == S_REGISTERED ) {
        QualExprEvaluator* evaluator = getEvaluator( tid );

        if( m_debugLevel >= D_FULL ) {
            std::string header;
            log( "Remove quality expression measures:", &header );

            std::stringstream s;
            header =  "\n" + header + " *";
            s << header;
            evaluator->display( header, s );
            std::cerr << s.str() << std::endl;
        }

        evaluator->clearMeasures();
    }
    else {
        throw( Exception( "Profiler not initialized or already activated" ) );
    }
}

/** @brief Reset all quality expresions to their neutral value.
    @param tid   thread id
 */
void QualExprManager::resetCounters( pthread_t tid ) throw( QualExprManager::Exception )      {
    if( m_state == S_REGISTERED ) {
        QualExprEvaluator* evaluator = getEvaluator( tid );

        if( m_debugLevel >= D_FULL ) {
            std::string header;
            log( "Reset quality expression measures:", &header );

            std::stringstream s;
            header =  "\n" + header + " ";
            s << header;
            evaluator->display( header, s );
            std::cerr << s.str() << std::endl;
        }

        evaluator->resetMeasures();
    }
    else {
        throw( Exception( "Profiler not initialized or already activated" ) );
    }
}

/** @brief Enable measurements.
    @param tid   thread id
 */
void QualExprManager::enableMeasures( pthread_t tid ) {
    if( m_state == S_REGISTERED || m_state == S_ON ) {
        m_state = S_ON;
    }
    else {
        throw( Exception( "Profiler not initialized" ) );
    }

    EvaluatorSet_T::iterator ite;
    for( ite = m_evaluators.begin(); ite != m_evaluators.end(); ite++ ) {
        QualExprEvaluator* evaluator = ite->second;
        evaluator->consolidate();
    }
}

/** @brief Disable measurements.
    @param tid   thread id
 */
void QualExprManager::disableMeasures( pthread_t tid ) {
    if( m_state == S_ON ) {
        m_state = S_REGISTERED;
    }
    else {
        throw( Exception( "Profiler not activated" ) );
    }
}

/** @brief Handle an event for quality expressions.
 */
void QualExprManager::event( profiling_event_t* event ) throw( )      {
    if( event && m_state == S_ON ) {
        pthread_t          tid       = pthread_self();
        QualExprEvaluator* evaluator = getEvaluator( tid );

        lock();
        const QualExprEvent& eventSem = m_eventBuilder.pushEvent_singleThread( event );
        evaluator->evaluateEvent( eventSem );
        m_eventBuilder.popEventSequence_singleThread( eventSem );
        unlock();
    }
}
} // namespace quality_expressions_core
