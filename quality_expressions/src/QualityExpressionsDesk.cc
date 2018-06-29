/**
   @file    QualityExpressionsDesk.cc
   @ingroup QualityExpressionCore
   @brief   Quality Expression desk implementation
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
#include "quality-expressions/QualityExpressionsDesk.h"
#include "quality-expressions/QualityExpressionsProfilerLocal.h"
#include "quality-expressions/QualityExpressionsProfilerPAPI.h"
#include "qualexpr-evaluator/QualExprManager.h"

extern "C" {
/** @brief Global initialization.
 */
void QualExprDesk_globalInit( void ) {
    QualityExpressionsDesk::getGlobalManager();
}

/** @brief Append a new quality expression indexed by a metric ID.
    @param metric the metric ID to associated with the quality expression
    @param expression the quality expression
 */
int QualExprDesk_addCounter( int   metric,
                             char* expression ) {
    try {
        pthread_t               tid  = pthread_self();
        QualityExpressionsDesk* desk = QualityExpressionsDesk::getGlobalManager();
        desk->addCounter( tid, QualityExpressionEntry( metric, QualityExpression( expression ) ) );
    }
    catch( QualityExpressionsDesk::Exception e ) {
        fprintf( stderr, "Internal quality expression error: %s\n", e.what() );
    }
    return 1;   // OK
}

/** @brief Get the value of a quality expression by a metric ID.
    @param metric the metric ID to associated with the quality expression
    @return the current value of the quality expression associated to the metric
 */
long long QualExprDesk_getLongCounter( int metric ) {
    long long result = 0;
    try {
        pthread_t               tid  = pthread_self();
        QualityExpressionsDesk* desk = QualityExpressionsDesk::getGlobalManager();
        result = desk->getLongCounter( tid, metric );
    }
    catch( QualityExpressionsDesk::Exception e ) {
        fprintf( stderr, "Internal quality expression error: %s\n", e.what() );
    }
    return result;
}

/** @brief Reset to 0 all quality expression values.
    @return 1 in case of success.
 */
int QualExprDesk_resetCounters( void ) {
    try {
        pthread_t               tid  = pthread_self();
        QualityExpressionsDesk* desk = QualityExpressionsDesk::getGlobalManager();
        desk->resetCounters( tid );
    }
    catch( QualityExpressionsDesk::Exception e ) {
        fprintf( stderr, "Internal quality expression error: %s\n", e.what() );
    }
    return 1;   // OK
}

/** @brief Remove all quality expressions.
    @return 1 in case of success.
 */
int QualExprDesk_removeCounters( void ) {
    try {
        pthread_t               tid  = pthread_self();
        QualityExpressionsDesk* desk = QualityExpressionsDesk::getGlobalManager();
        desk->removeCounters( tid );
    }
    catch( QualityExpressionsDesk::Exception e ) {
        fprintf( stderr, "Internal quality expression error: %s\n", e.what() );
    }
    return 1;   // OK
}

/** @brief Start measurements.
    @return 1 in case of success.
 */
int QualExprDesk_startMeasures( void ) {
    try {
        pthread_t               tid  = pthread_self();
        QualityExpressionsDesk* desk = QualityExpressionsDesk::getGlobalManager();
        desk->enableMeasures( tid );
    }
    catch( QualityExpressionsDesk::Exception e ) {
        fprintf( stderr, "Internal quality expression error: %s\n", e.what() );
    }
    return 1;   // OK
}

/** @brief Stop measurements.
    @return 1 in case of success.
 */
int QualExprDesk_stopMeasures( void ) {
    try {
        pthread_t               tid  = pthread_self();
        QualityExpressionsDesk* desk = QualityExpressionsDesk::getGlobalManager();
        desk->disableMeasures( tid );
    }
    catch( QualityExpressionsDesk::Exception e ) {
        fprintf( stderr, "Internal quality expression error: %s\n", e.what() );
    }
    return 1;   // OK
}

/** @brief Private desk call-back function for profiling backend events.
    @param event the generated event.
 */
static void launchEvent( profiling_event_t* event ) {
    try {
        QualityExpressionsDesk* desk = QualityExpressionsDesk::getGlobalManager();
        desk->event( event );
    }
    catch( QualityExpressionsDesk::Exception e ) {
        fprintf( stderr, "Event error: %s\n", e.what() );
    }
}
}

/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */

using namespace quality_expressions_core;

/** @brief Desk initialization
    Shall be executed once application wide.
    Creates a manager instance, and launch the registration of profiling backend
 */
/* Constructor */ QualityExpressionsDesk::QualityExpressionsDesk( void ) throw( QualityExpressionsDesk::Exception ) :
    m_instance( NULL ) {
    m_instance = new QualExprManager();
    if( !m_instance ) {
        throw( Exception( "could not allocate data" ) );
    }
    try {
        registerWithProfilers();
        m_instance->init();
    }
    catch( QualExprManager::Exception e ) {
        throw( Exception( "error during the quality expression manager initializations" ) );
    }
}

/** @brief Desk destruction
    Shall be called after the application termination (library static destructor calls)
    Release profiling backend and destroy the manager.
 */
QualityExpressionsDesk::~QualityExpressionsDesk( void ) throw( QualityExpressionsDesk::Exception ) {
    try {
        unregisterWithProfilers();
        m_instance->clear();
    }
    catch( QualExprManager::Exception e ) {
        throw( Exception( "error during the quality expression manager initializations" ) );
    }
    delete m_instance;
}

/** @brief Unique access point to the quality expression desk descriptor.
    @return the desk descriptor, unique application wide.
 */
QualityExpressionsDesk* QualityExpressionsDesk::getGlobalManager( void ) throw( QualityExpressionsDesk::Exception )                         {
    static QualityExpressionsDesk* g_desk = NULL;
    if( !g_desk ) {
        g_desk = new QualityExpressionsDesk();
        if( !g_desk ) {
            throw( Exception( "could not allocate data" ) );
        }
    }
    return g_desk;
}

/** @brief Append a quality expression evaluation request.
    @param tid thread id
    @param entry the quality expression entry
    @return the number of expression item registered
 */
size_t QualityExpressionsDesk::addCounter( pthread_t                     tid,
                                           const QualityExpressionEntry& entry ) throw( QualityExpressionsDesk::Exception ) {
    size_t rcount = 0;
    try {
        rcount = m_instance->addGlobalCounter( entry );
    }
    catch( QualExprManager::Exception e ) {
        throw( Exception( e.what() ) );
    }
    return rcount;
}

/** @brief Get the result of a quality expression evaluation.
    @param tid thread id
    @param id  the quality expression ID
    @return    the value
 */
long long QualityExpressionsDesk::getLongCounter( pthread_t             tid,
                                                  QualityExpressionID_T id ) throw( QualityExpressionsDesk::Exception ) {
    long long result = 0;
    try {
        result = m_instance->getLongCounter( tid, id );
    }
    catch( QualExprManager::Exception e ) {
        throw( Exception( e.what() ) );
    }
    return result;
}

/** @brief Reset all quality expressions to their neutral value.
    @param tid   thread id
 */
void QualityExpressionsDesk::resetCounters( pthread_t tid ) throw( QualityExpressionsDesk::Exception )      {
    try {
        m_instance->resetCounters( tid );
    }
    catch( QualExprManager::Exception e ) {
        throw( Exception( e.what() ) );
    }
}

/** @brief Remove a quality expression evaluation request.
    @param tid   thread id
 */
void QualityExpressionsDesk::removeCounters( pthread_t tid ) throw( QualityExpressionsDesk::Exception )      {
    try {
        m_instance->removeAllCounters();
        qualExpr_papi_reset();
    }
    catch( QualExprManager::Exception e ) {
        throw( Exception( e.what() ) );
    }
}

/** @brief Enable measurements.
    @param tid   thread id
 */
void QualityExpressionsDesk::enableMeasures( pthread_t tid ) {
    try {
        m_instance->enableMeasures( tid );
    }
    catch( QualExprManager::Exception e ) {
        throw( Exception( e.what() ) );
    }
}

/** @brief Disable measurements.
    @param tid   thread id
 */
void QualityExpressionsDesk::disableMeasures( pthread_t tid ) {
    try {
        m_instance->disableMeasures( tid );
    }
    catch( QualExprManager::Exception e ) {
        throw( Exception( e.what() ) );
    }
}

/** @brief Record this manager as an event listener for foreign profilers.
 */
bool QualityExpressionsDesk::registerWithProfilers( void ) throw( QualityExpressionsDesk::Exception )      {
    // ... register launchEvent ...
    semantic_namespace_t localNameSpace = qualExpr_registerListener( launchEvent );
    semantic_namespace_t papiNameSpace  = qualExpr_papi_registerListener( launchEvent );

    m_instance->registerSemanticNamespace( localNameSpace );
    m_instance->registerSemanticNamespace( papiNameSpace );
    return true;
}

/** @brief Remove this manager from registered event listeners in foreign profilers.
 */
bool QualityExpressionsDesk::unregisterWithProfilers( void ) throw( QualityExpressionsDesk::Exception )      {
    // ... un-register launchEvent ...
    qualExpr_papi_unregisterListener( launchEvent );
    qualExpr_unregisterListener( launchEvent );
    return true;
}

void QualityExpressionsDesk::event( profiling_event_t* event ) throw( )      {
    m_instance->event( event );
}
