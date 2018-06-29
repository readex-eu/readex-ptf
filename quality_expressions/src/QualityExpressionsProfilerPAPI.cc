/**
   @file    QualityExpressionsProfilerPAPI.cc
   @ingroup QualityExpressionProfilerPAPI
   @brief   PAPI Event manager
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
#include <errno.h>
#include <string.h>
#include <exception>
#include <sstream>
#include <list>
#include <papi.h>
#include "quality-expressions/QualityExpressionsProfilerPAPI.h"
#include "quality-expressions/QualExprSemanticPAPI.h"

namespace quality_expressions_ns {
/**
   @class QualExprProfilerPapi
   @brief Internal Management of PAPI profiling metrics.
   @ingroup QualityExpressionProfilerPAPI

   This class performs the management of the PAPI profiling metrics for all other modules.
   It is in charge also of interfacing the internal management of the PAPI quality expressions semantic and namespace.
   This class is hidden from all other modules.

   All operations related to the PAPI library should be managed in or through that class.
 */
class QualExprProfilerPapi : private QualExprSemaphore {
public:
    typedef std::list<listen_event_func_t> listenerList_T;
    typedef long long long64_papi_t;

    /**
       @class Exception
       @brief Hold an exception with a description.
     */
    class Exception : public std::exception, public std::string {
public:
        /* Constructor */ Exception( const char* message ) throw( ) : std::string( message ) {
        }

        /* Constructor */ Exception( const std::string& message ) throw( ) : std::string( message ) {
        }

        /* Destructor */ ~Exception( void ) throw( ) {
        }
        virtual const char* what() const throw( )                     {
            return c_str();
        }
    };

public:
    /* Destructor */ ~QualExprProfilerPapi( void ) {
    }

public:         // -- Global Event Management API
    const QualExprSemanticNamespace& registerListener( listen_event_func_t listener );

    void unregisterListener( listen_event_func_t listener );

    static QualExprProfilerPapi* getProfiler( void );

public:         // -- Profiling service for API
    bool isAvailable( enum qualexpr_event_papi_t semantic );

    void addSemantic( enum qualexpr_event_papi_t semantic ) throw( Exception );

    void reset( void ) throw( Exception );

    void startCounters( void );

    void stopCounters( void );

    enum qualexpr_kind_papi_t eventKind( enum qualexpr_event_papi_t semantic );

private:
    /* Constructor */ QualExprProfilerPapi( void );                             //!< Constructor is only available via the getProfiler() method.
    void propagateEvent( struct profiling_event_t* event );

    long64_papi_t getInfo( enum qualexpr_event_papi_t semantic ) throw ( QualExprProfilerPapi::Exception );

private:        // -- PAPI util functions.
    static void processError( const std::string& message,
                              int                retVal ) throw ( QualExprProfilerPapi::Exception );

    static int papiCounterNumber( enum qualexpr_event_papi_t semantic ) throw ( QualExprProfilerPapi::Exception );

private:                                                          /* ---- */
    int                                     m_papiEventSet;       //!< PAPI event set for counter activation.
    PAPI_hw_info_t                          m_papi_hardware_info; //!< PAPI machine description descriptor.
    unsigned int                            m_eidCursor;          //!< Atomic counter for the generation of unique event IDs.
    listenerList_T                          m_listenerList;       //!< The list of listeners for the profiler.
    QualExprSemanticNamespacePAPI*          m_semanticNamespace;  //!< The quality expression namespace built for the PAPI profiler.
    std::vector<enum qualexpr_event_papi_t> m_counterList;        //!< The list of counters active in the PAPI event set.
    std::vector<enum qualexpr_event_papi_t> m_infoList;           //!< The list of active static PAPI metrics extracted from the hardware info descriptor.
};

/** @brief PAPI Profiler constructor
    The QualExprProfilerPapi class initialization is in charge of operating the necessary initialization of the PAPI library.
    A PAPI event set - the descriptor holding all active PAPI counters - and the machine description descriptor are also created for future usage.
 */
/* Constructor */ QualExprProfilerPapi::QualExprProfilerPapi( void ) : QualExprSemaphore(), m_papiEventSet( PAPI_NULL ), m_eidCursor( 0 ), m_listenerList(), m_semanticNamespace( NULL ) {
    int initDone = PAPI_is_initialized();
    if( initDone == PAPI_NOT_INITED ) {
        int err = PAPI_library_init( PAPI_VER_CURRENT );
        if( err != PAPI_VER_CURRENT ) {
            processError( "PAPI library version mismatch", err );
        }
        err = PAPI_thread_init( pthread_self );
        if( err != PAPI_OK ) {
            processError( "PAPI thread initialization error", err );
        }
    }

    int err = PAPI_create_eventset( &m_papiEventSet );
    if( err != PAPI_OK ) {
        processError( "Creating event sets", err );
    }

    const PAPI_hw_info_t* hw_info = PAPI_get_hardware_info();
    if( hw_info ) {
        memcpy( ( void* )&m_papi_hardware_info, ( const void* )hw_info, sizeof( m_papi_hardware_info ) );
    }
    else {
        processError( "Could not get papi hardware info", err );
    }
}

/** @brief Return the Profiler descriptor that must be unique application wide.
 */
QualExprProfilerPapi* QualExprProfilerPapi::getProfiler( void ) {
    static QualExprProfilerPapi* g_profiler = NULL;
    if( !g_profiler ) {
        g_profiler = new QualExprProfilerPapi();
    }
    return g_profiler;
}

/** @brief Propate an event to all listeners.
    @param currentEvent the event to propate
 */
void QualExprProfilerPapi::propagateEvent( struct profiling_event_t* currentEvent ) {
    for( listenerList_T::const_iterator ite = m_listenerList.begin(); ite != m_listenerList.end(); ite++ ) {
        ( *ite )( currentEvent );
    }
}

/** @brief Register a listener for all events generated by this profiling manager.
    @param listener the callback that will be used to propagate events to that listener.
    @return the semantic namespace for the PAPI profiler.
 */
const QualExprSemanticNamespace& QualExprProfilerPapi::registerListener( listen_event_func_t listener ) {
    m_listenerList.push_back( listener );
    if( !m_semanticNamespace ) {
        m_semanticNamespace = new QualExprSemanticNamespacePAPI;
    }
    return *m_semanticNamespace;
}

/** @brief Unregister an event listener.
    @param listener the callback to be removed
 */
void QualExprProfilerPapi::unregisterListener( listen_event_func_t listener ) {
    m_listenerList.remove( listener );
}

/** @brief Check and process a PAPI error.
    @param message the error message
    @param retVal the error code to check
 */
void QualExprProfilerPapi::processError( const std::string& message,
                                         int                retVal ) throw ( QualExprProfilerPapi::Exception ) {
    std::stringstream s;
    s << "[quality expressions][papi] " << message;
    if( retVal == PAPI_ESYS ) {
        s << ": PAPI system error : " << strerror( errno );
        throw ( Exception( s.str() ) );
    }
    else if( retVal < 0 ) {
        s << ": PAPI error : " << PAPI_strerror( retVal );
        throw ( Exception( s.str() ) );
    }
    else {
        s << ": PAPI unexpected error : " << PAPI_strerror( retVal );
        throw ( Exception( s.str() ) );
    }
}

/** @brief Compute the PAPI counter associated to a PAPI event semantic
    @param semantic the PAPI event semantic
    @return the PAPI counter
 */
int QualExprProfilerPapi::papiCounterNumber( enum qualexpr_event_papi_t semantic ) throw ( QualExprProfilerPapi::Exception )     {
    static const int g_papiEventTable[] = {
        PAPI_NULL,                              //!< List Base.
#define QUALEXPRSEMANTIC_PAPI_DEF( DefName, KIND ) DefName,
#define QUALEXPRSEMANTIC_PAPI_DEF_ONLY_PRESET
#include "quality-expressions/QualityExpressionsProfilerPAPI.h"
#undef QUALEXPRSEMANTIC_PAPI_DEF_ONLY_PRESET
#undef QUALEXPRSEMANTIC_PAPI_DEF
        PAPI_NULL                               //!< List end.
    };

    if( semantic <= QE_PROFILER_PAPI_PRESET_BASE || semantic >= QE_PROFILER_PAPI_NOMORE ) {
        throw ( Exception( "Illegal papi event" ) );
    }
    return g_papiEventTable[ semantic - QE_PROFILER_PAPI_PRESET_BASE ];
}

/** @brief Compute the kind of PAPI metric (static info, or counter) of a PAPI event semantic
    @param semantic the PAPI event semantic
    @return the metric kind
 */
enum qualexpr_kind_papi_t QualExprProfilerPapi::eventKind( enum qualexpr_event_papi_t semantic ) {
    static const enum qualexpr_kind_papi_t g_papiKindTable[] = {
        PAPI_KIND_UNDEF,                        //!< List Base.
#define QUALEXPRSEMANTIC_PAPI_DEF( DefName, KIND ) PAPI_KIND_ ## KIND,
#include "quality-expressions/QualityExpressionsProfilerPAPI.h"
#undef QUALEXPRSEMANTIC_PAPI_DEF
        PAPI_KIND_UNDEF                         //!< List end.
    };

    if( semantic <= QE_PROFILER_PAPI_BASE || semantic >= QE_PROFILER_PAPI_NOMORE ) {
        throw ( Exception( "Illegal papi event" ) );
    }
    return g_papiKindTable[ semantic - QE_PROFILER_PAPI_BASE ];
}

/** @brief Compute the information associated to a static papi hardware info
    @param semantic the PAPI event semantic
    @return the metric value
 */
QualExprProfilerPapi::long64_papi_t QualExprProfilerPapi::getInfo( enum qualexpr_event_papi_t semantic ) throw ( QualExprProfilerPapi::Exception )                                     {
    long64_papi_t result = 0;
    switch( semantic ) {
    case QE_PROFILER_PAPI_INF_CORES:
        result = m_papi_hardware_info.cores; /* */ break;
    case QE_PROFILER_PAPI_INF_CPU_MAX_MHZ:
        result = m_papi_hardware_info.cpu_max_mhz; /* */ break;
    case QE_PROFILER_PAPI_INF_CPU_MIN_MHZ:
        result = m_papi_hardware_info.cpu_min_mhz; /* */ break;
    case QE_PROFILER_PAPI_INF_CPUID_FAMILY:
        result = m_papi_hardware_info.cpuid_family; /* */ break;
    case QE_PROFILER_PAPI_INF_CPUID_MODEL:
        result = m_papi_hardware_info.cpuid_model; /* */ break;
    case QE_PROFILER_PAPI_INF_CPUID_STEPPING:
        result = m_papi_hardware_info.cpuid_stepping; /* */ break;
    case QE_PROFILER_PAPI_INF_CPU_MODEL:
        result = m_papi_hardware_info.model; /* */ break;
    case QE_PROFILER_PAPI_INF_NCPU:
        result = m_papi_hardware_info.ncpu; /* */ break;
    case QE_PROFILER_PAPI_INF_CPU_NNODES:
        result = m_papi_hardware_info.nnodes; /* */ break;
    case QE_PROFILER_PAPI_INF_CPU_REVISION:
        result = m_papi_hardware_info.revision; /* */ break;
    case QE_PROFILER_PAPI_INF_CPU_SOCKETS:
        result = m_papi_hardware_info.sockets; /* */ break;
    case QE_PROFILER_PAPI_INF_CPU_THREADS:
        result = m_papi_hardware_info.threads; /* */ break;
    case QE_PROFILER_PAPI_INF_TOTALCPUS:
        result = m_papi_hardware_info.totalcpus; /* */ break;
    case QE_PROFILER_PAPI_INF_VENDOR:
        result = m_papi_hardware_info.vendor; /* */ break;
    default:
        result = 0; /* */ break;
    }
    return result;
}

/** @brief Check if a standard PAPI counter is available for the local machine
    @param semantic the PAPI event semantic
    @return true if the metric is available
 */
bool QualExprProfilerPapi::isAvailable( enum qualexpr_event_papi_t semantic ) {
    bool                      isAvailable = false;
    enum qualexpr_kind_papi_t kind        = eventKind( semantic );
    if( kind == PAPI_KIND_INFO ) {
        isAvailable = true;
    }
    else if( kind == PAPI_KIND_PRESET ) {
        int               papi_counter = papiCounterNumber( semantic );
        PAPI_event_info_t info;
        int               err = PAPI_get_event_info( papi_counter, &info );
        if( err != PAPI_OK ) {
            char eventName[ PAPI_MAX_STR_LEN ];
            PAPI_event_code_to_name( papi_counter, eventName );
            std::stringstream s;
            s << "Failed to get info on PAPI counter " << eventName;
            processError( s.str(), err );
        }
        else {
            isAvailable = ( info.count > 0 );
        }
    }
    return isAvailable;
}

/** @brief Append a PAPI metric to the list of active profiling metrics
    @param semantic the PAPI event semantic
 */
void QualExprProfilerPapi::addSemantic( enum qualexpr_event_papi_t semantic ) throw( QualExprProfilerPapi::Exception )      {
    enum qualexpr_kind_papi_t kind = eventKind( semantic );
    if( kind == PAPI_KIND_INFO ) {
        bool found = false;
        lock();
        for( size_t index = 0; !found && ( index < m_infoList.size() ); index++ ) {
            found = ( m_infoList[ index ] == semantic );
        }
        if( !found ) {
            m_infoList.push_back( semantic );
        }
        unlock();
    }
    else if( kind == PAPI_KIND_PRESET ) {
        int  err          = PAPI_OK;
        bool found        = false;
        int  papi_counter = papiCounterNumber( semantic );
        lock();
        for( size_t index = 0; !found && ( index < m_counterList.size() ); index++ ) {
            found = ( m_counterList[ index ] == semantic );
        }
        if( !found ) {
            m_counterList.push_back( semantic );
            err = PAPI_add_event( m_papiEventSet, papi_counter );
        }
        unlock();
        if( err != PAPI_OK ) {
            char eventName[ PAPI_MAX_STR_LEN ];
            PAPI_event_code_to_name( papi_counter, eventName );
            std::stringstream s;
            s << "Failed adding PAPI counter " << eventName;
            processError( s.str(), err );
        }
    }
    else {
        throw ( Exception( "Illegal papi event" ) );
    }
}

/** @brief Remove all PAPI counters and metrics active for profiling
 */
void QualExprProfilerPapi::reset( void ) throw( QualExprProfilerPapi::Exception )      {
    int err = PAPI_OK;
    lock();
    m_infoList.clear();
    if( !m_counterList.empty() ) {
        m_counterList.clear();
        err = PAPI_reset( m_papiEventSet );
    }
    unlock();
    if( err != PAPI_OK ) {
        processError( "Failed removing PAPI counter", err );
    }
}

/** @brief Start the active PAPI counters and generate an event for all counters and metrics
    The event value is set with the current counter value.
 */
void QualExprProfilerPapi::startCounters( void ) {
    struct profiling_event_t  event        = { D_COUNTER, QE_PROFILER_PAPI_BASE };
    struct profiling_event_t* currentEvent = ( struct profiling_event_t* )malloc( sizeof( struct profiling_event_t ) );
    *currentEvent = event;
    for( size_t index = 0; index < m_infoList.size(); index++ ) {
        unsigned int eid = atomic_add<unsigned int> ( m_eidCursor, 1 );
        currentEvent->m_eid        = eid;
        currentEvent->m_semanticId = m_infoList[ index ];
        currentEvent->m_value      = ( long long )getInfo( m_infoList[ index ] );
        propagateEvent( currentEvent );
    }
    free( currentEvent );

    if( !m_counterList.empty() ) {
        lock();
        int err = PAPI_start( m_papiEventSet );
        unlock();
        if( err != PAPI_OK ) {
            processError( "Failed starting PAPI counter", err );
        }
    }
}

/** @brief Stop the active PAPI counters and generate an event for all counters and metrics
    The event value is set with the current counter value.
 */
void QualExprProfilerPapi::stopCounters( void ) {
    if( !m_counterList.empty() ) {
        long64_papi_t* values = ( long64_papi_t* )malloc( sizeof( long64_papi_t ) * m_counterList.size() );
        lock();
        int err = PAPI_stop( m_papiEventSet, values );
        if( err != PAPI_OK ) {
            processError( "Failed starting PAPI counter", err );
        }
        unlock();

        struct profiling_event_t  event        = { D_COUNTER, QE_PROFILER_PAPI_BASE };
        struct profiling_event_t* currentEvent = ( struct profiling_event_t* )malloc( sizeof( struct profiling_event_t ) );
        *currentEvent = event;
        for( size_t index = 0; index < m_counterList.size(); index++ ) {
            unsigned int eid = atomic_add<unsigned int> ( m_eidCursor, 1 );
            currentEvent->m_eid        = eid;
            currentEvent->m_semanticId = m_counterList[ index ];
            currentEvent->m_value      = ( long long )values[ index ];
            propagateEvent( currentEvent );
        }
        free( currentEvent );
        free( values );
    }
}

extern "C" {
semantic_namespace_t qualExpr_papi_registerListener( listen_event_func_t listener ) {
    return ( semantic_namespace_t )&QualExprProfilerPapi::getProfiler()->registerListener( listener );
}

void qualExpr_papi_unregisterListener( listen_event_func_t listener ) {
    QualExprProfilerPapi::getProfiler()->unregisterListener( listener );
}

void qualExpr_papi_startCounters( void ) {
    try {
        QualExprProfilerPapi::getProfiler()->startCounters();
    }
    catch( QualExprProfilerPapi::Exception e ) {
        fprintf( stderr, "Internal papi error: %s\n", e.what() );
    }
}

void qualExpr_papi_stopCounters( void ) {
    try {
        QualExprProfilerPapi::getProfiler()->stopCounters();
    }
    catch( QualExprProfilerPapi::Exception e ) {
        fprintf( stderr, "Internal papi error: %s\n", e.what() );
    }
}

enum qualexpr_kind_papi_t qualExpr_papi_eventKind( enum qualexpr_event_papi_t semantic ) {
    try {
        return QualExprProfilerPapi::getProfiler()->eventKind( semantic );
    }
    catch( QualExprProfilerPapi::Exception e ) {
        fprintf( stderr, "Internal papi error: %s\n", e.what() );
    }
    return PAPI_KIND_UNDEF;
}

int qualExpr_papi_isAvailable( enum qualexpr_event_papi_t semantic ) {
    try {
        return QualExprProfilerPapi::getProfiler()->isAvailable( semantic );
    }
    catch( QualExprProfilerPapi::Exception e ) {
        fprintf( stderr, "Internal papi error: %s\n", e.what() );
    }
    return false;
}

void qualExpr_papi_addSemantic( enum qualexpr_event_papi_t semantic ) {
    try {
        QualExprProfilerPapi::getProfiler()->addSemantic( semantic );
    }
    catch( QualExprProfilerPapi::Exception e ) {
        fprintf( stderr, "Internal papi error: %s\n", e.what() );
    }
}

void qualExpr_papi_reset( void ) {
    try {
        QualExprProfilerPapi::getProfiler()->reset();
    }
    catch( QualExprProfilerPapi::Exception e ) {
        fprintf( stderr, "Internal papi error: %s\n", e.what() );
    }
}
}
}
