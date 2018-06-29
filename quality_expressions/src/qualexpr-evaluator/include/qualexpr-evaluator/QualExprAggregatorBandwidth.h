/**
   @file    QualExprAggregatorBandwidth.h
   @ingroup QualityExpressionEvaluation
   @brief   Evaluation of quality expressions - aggregator bandwidth evaluation
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

#ifndef QUALEXP_AGGREGATOR_BANDWIDTH_H_
#define QUALEXP_AGGREGATOR_BANDWIDTH_H_

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
 * @defgroup QualExprAggregatorBandwidth Bandwidth aggregators
 * Group of aggregator operating on the event value supposed to be a size and derived from it a speed (i.e. a bandwidth for a size).
 * @ingroup QualityExpressionEvaluation
 */

/**
   @class QualExprAggregatorBandwidth
   @brief Base class for bandwidth based aggregators
   @ingroup QualExprAggregatorBandwidth
 */
class QualExprAggregatorBandwidth : public QualExprAggregatorEvalBasic<long64_t>{
protected:
    /* Constructor */ QualExprAggregatorBandwidth( size_t id, bool r = false ) :
        QualExprAggregatorEvalBasic<long64_t>( id ) {
    }

public:
    static void registerToAggregatorNS( QualExprAggregatorNamespace& aggregNs );

protected:
    unsigned int m_previousEid;                         //!< Previous event ID (for relating start/stop events of the same time).
    long64_t     m_previousTimeStamp;                   //!< Timestamp of the previous event.
    double       m_currentSize;                         //!< Size of the current event.
};

/**
   @class QualExprAggregatorBandwidthAverage
   @brief Average aggregation of event bandwidth.
   @ingroup QualExprAggregatorBandwidth
 */
class QualExprAggregatorBandwidthAverage : public QualExprAggregatorBandwidth {
public:
    /* Constructor */ QualExprAggregatorBandwidthAverage( size_t id ) : QualExprAggregatorBandwidth( id ) {
        reset();
    }

    /* Constructor */ QualExprAggregatorBandwidthAverage( QualExprAggregatorNamespace& aggregNs ) : QualExprAggregatorBandwidth( 0 ) {
        aggregNs.registerNewAggregator( '~', this );
    }

    /* Destructor */ virtual ~QualExprAggregatorBandwidthAverage( void ) {
    }

public:
    virtual void display( const std::string& indent,
                          std::stringstream& s ) const;

    virtual void processEvent( const QualExprEvent& event );                                                                                                                         //!< Aggregate the given event.

    virtual long64_t evaluate( void ) const {
        return average();
    }                                                                                                                                                                   //!< Aggregator evaluation.

    virtual const char* name( void ) const {
        return "~bw";
    }                                                                                                                                                                   //!< Aggregator fully qualified name.

    virtual const char* description( void ) const {
        return "Average bandwidth";
    }                                                                                                                                                                   //!< Aggregator description.

    virtual QualExprAggregator* build( size_t id ) const {
        return new QualExprAggregatorBandwidthAverage( id );
    }                                                                                                                                                                   //!< Auto-constructor.
};

/**
   @class QualExprAggregatorBandwidthMax
   @brief Maximum of all event bandwidth.
   @ingroup QualExprAggregatorBandwidth
 */
class QualExprAggregatorBandwidthMax : public QualExprAggregatorBandwidth {
public:
    /* Constructor */ QualExprAggregatorBandwidthMax( size_t id ) : QualExprAggregatorBandwidth( id ) {
        reset();
    }

    /* Constructor */ QualExprAggregatorBandwidthMax( QualExprAggregatorNamespace& aggregNs ) : QualExprAggregatorBandwidth( 0 ) {
        aggregNs.registerNewAggregator( '+', this );
    }

    /* Destructor */ virtual ~QualExprAggregatorBandwidthMax( void ) {
    }

public:
    virtual void display( const std::string& indent,
                          std::stringstream& s ) const;

    virtual void processEvent( const QualExprEvent& event );                                                                                                                         //!< Aggregate the given event.

    virtual long64_t evaluate( void ) const {
        return value();
    }                                                                                                                                                                   //!< Aggregator evaluation.

    virtual const char* name( void ) const {
        return "+bw";
    }                                                                                                                                                                   //!< Aggregator fully qualified name.

    virtual const char* description( void ) const {
        return "Max bandwidth";
    }                                                                                                                                                                   //!< Aggregator description.

    virtual QualExprAggregator* build( size_t id ) const {
        return new QualExprAggregatorBandwidthMax( id );
    }                                                                                                                                                                   //!< Auto-constructor.
};

/**
   @class QualExprAggregatorBandwidthMin
   @brief Minimum of all event bandwidth.
   @ingroup QualExprAggregatorBandwidth
 */
class QualExprAggregatorBandwidthMin : public QualExprAggregatorBandwidth {
public:
    /* Constructor */ QualExprAggregatorBandwidthMin( size_t id ) : QualExprAggregatorBandwidth( id ) {
        reset();
    }

    /* Constructor */ QualExprAggregatorBandwidthMin( QualExprAggregatorNamespace& aggregNs ) : QualExprAggregatorBandwidth( 0 ) {
        aggregNs.registerNewAggregator( '-', this );
    }

    /* Destructor */ virtual ~QualExprAggregatorBandwidthMin( void ) {
    }

public:
    virtual void display( const std::string& indent,
                          std::stringstream& s ) const;

    virtual void processEvent( const QualExprEvent& event );                                                                                                                         //!< Aggregate the given event.

    virtual long64_t evaluate( void ) const {
        return value();
    }                                                                                                                                                                   //!< Aggregator evaluation.

    virtual const char* name( void ) const {
        return "-bw";
    }                                                                                                                                                                   //!< Aggregator fully qualified name.

    virtual const char* description( void ) const {
        return "Min bandwidth";
    }                                                                                                                                                                   //!< Aggregator description.

    virtual QualExprAggregator* build( size_t id ) const {
        return new QualExprAggregatorBandwidthMin( id );
    }                                                                                                                                                                   //!< Auto-constructor.

    virtual void reset( void ) {
        m_value = -1;
        m_count = 0;
    }                                                                                                                                                                   //!< Reset the aggregator state.
};
}

#endif
