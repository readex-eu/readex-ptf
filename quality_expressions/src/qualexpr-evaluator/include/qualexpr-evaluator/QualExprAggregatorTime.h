/**
   @file    QualExprAggregatorTime.h
   @ingroup QualityExpressionEvaluation
   @brief   Evaluation of quality expressions - aggregator timing evaluation
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

#ifndef QUALEXP_AGGREGATOR_TIME_H_
#define QUALEXP_AGGREGATOR_TIME_H_

#include "qualexpr-evaluator/QualExprAggregator.h"
#include "qualexpr-evaluator/QualExprAggregatorNamespace.h"

namespace quality_expressions_core {
// -- Some predefined types for aggregators.
typedef long long long64_t;

/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */

/**
 * @defgroup QualExprAggregatorTime Time aggregators
 * Group of aggregator operating on the execution time.
 * @ingroup QualityExpressionEvaluation
 */

/**
   @class QualExprAggregatorTime
   @brief Base class for timing aggregators
   @ingroup QualExprAggregatorTime
 */
class QualExprAggregatorTime : public QualExprAggregatorEvalBasic<long64_t>{
protected:
    /* Constructor */ QualExprAggregatorTime( size_t id, bool r = false ) :
        QualExprAggregatorEvalBasic<long64_t>( id ), m_previousEid( 0 ), m_previousTimeStamp( 0 ) {
    }

public:
    static void registerToAggregatorNS( QualExprAggregatorNamespace& aggregNs );                                    //!< Record all aggregators in the group in the namespace.

protected:
    unsigned int m_previousEid;                                 //!< Previous event ID (for relating start/stop events of the same time).
    long64_t     m_previousTimeStamp;                           //!< Timestamp of the previous event.
};

/**
   @class QualExprAggregatorTimeAverage
   @brief Average aggregation of event times.
   @ingroup QualExprAggregatorTime
 */
class QualExprAggregatorTimeAverage : public QualExprAggregatorTime {
public:
    /* Constructor */ QualExprAggregatorTimeAverage( size_t id ) : QualExprAggregatorTime( id ) {
        reset();
    }

    /* Constructor */ QualExprAggregatorTimeAverage( QualExprAggregatorNamespace& aggregNs ) : QualExprAggregatorTime( 0 ) {
        aggregNs.registerNewAggregator( '~', this );
    }

    /* Destructor */ virtual ~QualExprAggregatorTimeAverage( void ) {
    }

public:
    virtual void display( const std::string& indent,
                          std::stringstream& s ) const;

    virtual void processEvent( const QualExprEvent& event );                                                                                                                                 //!< Aggregate the given event.

    virtual long64_t evaluate( void ) const {
        return average();
    }                                                                                                                                                                           //!< Aggregator evaluation.

    virtual const char* name( void ) const {
        return "~time";
    }                                                                                                                                                                           //!< Aggregator fully qualified name.

    virtual const char* description( void ) const {
        return "Average time";
    }                                                                                                                                                                           //!< Aggregator description.

    virtual QualExprAggregator* build( size_t id ) const {
        return new QualExprAggregatorTimeAverage( id );
    }                                                                                                                                                                           //!< Auto-constructor.
};

/**
   @class QualExprAggregatorTimeMax
   @brief Maximum of all event times.
   @ingroup QualExprAggregatorTime
 */
class QualExprAggregatorTimeMax : public QualExprAggregatorTime {
public:
    /* Constructor */ QualExprAggregatorTimeMax( size_t id ) : QualExprAggregatorTime( id ) {
        reset();
    }

    /* Constructor */ QualExprAggregatorTimeMax( QualExprAggregatorNamespace& aggregNs ) : QualExprAggregatorTime( 0 ) {
        aggregNs.registerNewAggregator( '+', this );
    }

    /* Destructor */ virtual ~QualExprAggregatorTimeMax( void ) {
    }

public:
    virtual void display( const std::string& indent,
                          std::stringstream& s ) const;

    virtual void processEvent( const QualExprEvent& event );                                                                                                                         //!< Aggregate the given event.

    virtual long64_t evaluate( void ) const {
        return value();
    }                                                                                                                                                                   //!< Aggregator evaluation.

    virtual const char* name( void ) const {
        return "+time";
    }                                                                                                                                                                   //!< Aggregator fully qualified name.

    virtual const char* description( void ) const {
        return "Maximum time";
    }                                                                                                                                                                   //!< Aggregator description.

    virtual QualExprAggregator* build( size_t id ) const {
        return new QualExprAggregatorTimeMax( id );
    }                                                                                                                                                                   //!< Auto-constructor.
};

/**
   @class QualExprAggregatorTimeMin
   @brief Minimum of all event times.
   @ingroup QualExprAggregatorTime
 */
class QualExprAggregatorTimeMin : public QualExprAggregatorTime {
public:
    /* Constructor */ QualExprAggregatorTimeMin( size_t id ) : QualExprAggregatorTime( id ) {
        reset();
    }

    /* Constructor */ QualExprAggregatorTimeMin( QualExprAggregatorNamespace& aggregNs ) : QualExprAggregatorTime( 0 ) {
        aggregNs.registerNewAggregator( '-', this );
    }

    /* Destructor */ virtual ~QualExprAggregatorTimeMin( void ) {
    }

public:
    virtual void display( const std::string& indent,
                          std::stringstream& s ) const;

    virtual void processEvent( const QualExprEvent& event );                                                                                                                         //!< Aggregate the given event.

    virtual long64_t evaluate( void ) const {
        return value();
    }                                                                                                                                                                   //!< Aggregator evaluation.

    virtual const char* name( void ) const {
        return "-time";
    }                                                                                                                                                                   //!< Aggregator fully qualified name.

    virtual const char* description( void ) const {
        return "Minimum time";
    }                                                                                                                                                                   //!< Aggregator description.

    virtual QualExprAggregator* build( size_t id ) const {
        return new QualExprAggregatorTimeMin( id );
    }                                                                                                                                                                   //!< Auto-constructor.

    virtual void reset( void ) {
        m_value = -1;
        m_count = 0;
    }                                                                                                                                                                   //!< Reset the aggregator state.
};

/**
   @class QualExprAggregatorTimeSum
   @brief Direct aggregation of event times.
   @ingroup QualExprAggregatorTime
 */
class QualExprAggregatorTimeSum : public QualExprAggregatorTime {
public:
    /* Constructor */ QualExprAggregatorTimeSum( size_t id ) : QualExprAggregatorTime( id ) {
        reset();
    }

    /* Constructor */ QualExprAggregatorTimeSum( QualExprAggregatorNamespace& aggregNs ) : QualExprAggregatorTime( 0 ) {
        aggregNs.registerNewAggregator( '|', this );
    }

    /* Destructor */ virtual ~QualExprAggregatorTimeSum( void ) {
    }

public:
    virtual void display( const std::string& indent,
                          std::stringstream& s ) const;

    virtual void processEvent( const QualExprEvent& event );                                                                                                                         //!< Aggregate the given event.

    virtual long64_t evaluate( void ) const {
        return value();
    }                                                                                                                                                                   //!< Aggregator evaluation.

    virtual const char* name( void ) const {
        return "|time";
    }                                                                                                                                                                   //!< Aggregator fully qualified name.

    virtual const char* description( void ) const {
        return "Accumulated time";
    }                                                                                                                                                                   //!< Aggregator description.

    virtual QualExprAggregator* build( size_t id ) const {
        return new QualExprAggregatorTimeSum( id );
    }                                                                                                                                                                   //!< Auto-constructor.
};
}

#endif
