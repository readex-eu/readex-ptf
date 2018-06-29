/**
   @file    QualExprProfilerEvents.h
   @ingroup QualityExpressionProfilerInternal
   @brief   Quality expression profilers - event builder
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

#ifndef QUALEXPR_PROFILER_EVENTS_H_
#define QUALEXPR_PROFILER_EVENTS_H_

#include <string>

namespace quality_expressions_core {
/**
   @class QualExprEvent
   @brief Basic class for event or counters.
   @ingroup QualityExpressionProfilerInternal
 */
class QualExprEvent {
protected:
    friend class QualExprEventBuilder;
    /* Constructor */ QualExprEvent( void ) {
    }

    /* Constructor */ QualExprEvent( event_state_t state, unsigned int semanticId ) :
        m_timestamp( 0 ), m_value( 0 ), m_state( state ), m_eid( 0 ), m_semanticId( semanticId ) {
    }

    /* Destructor */ virtual ~QualExprEvent( void ) {
    }

public:
    double        m_timestamp;                          //!< Time stamp of the event.
    long long     m_value;                              //!< Value of the counter or event (size, count, ...).
    event_state_t m_state;                              //!< Event state (start/stop/wait).
    unsigned int  m_eid;                                //!< Event unique ID - mandatory for intealeaved events.
    unsigned int  m_semanticId;                         //!< Event semantic ID - defined globally.

    virtual void display( const std::string& indent,
                          std::stringstream& s ) const;
};

/**
   @class QualExprEventBuilder
   @brief Basic class for the generation of event or counters objects.
   @ingroup QualityExpressionProfilerInternal
 */
class QualExprEventBuilder {
public:
    class Exception : public QualExprException {
public:
        /* Constructor */ Exception( const char* message ) throw( ) : QualExprException( message ) {
        }

        /* Destructor */ ~Exception( void ) throw( ) {
        }
    };

public:
    /* Constructor */ QualExprEventBuilder( QualExprTimer& timer ) : m_timer( timer ) {
    }

    /* Destructor */ ~QualExprEventBuilder( void ) {
    }

    const QualExprEvent& pushEvent( profiling_event_t* event );                                      //!< Build and push a new event descriptor.

    void popEventSequence( const QualExprEvent& event );                             //!< Pop the event sequence.

    const QualExprEvent& pushEvent_singleThread( profiling_event_t* event );                         //!< Build and push a new event descriptor. Not thread safe.

    void popEventSequence_singleThread( const QualExprEvent& event );                //!< Pop the event sequence. Not thread safe.

protected:
    QualExprTimer& m_timer;                                             //!< Timestamp service. */
    QualExprEvent  m_event;                                             //!< For a faster thread safe service. */
};

/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
} // /quality_expressions

#endif
