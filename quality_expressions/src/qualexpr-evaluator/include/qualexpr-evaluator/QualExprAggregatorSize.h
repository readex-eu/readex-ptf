/**
   @file    QualExprAggregatorSize.h
   @ingroup QualityExpressionEvaluation
   @brief   Evaluation of quality expressions - aggregator size evaluation
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

#ifndef QUALEXP_AGGREGATOR_SIZE_H_
#define QUALEXP_AGGREGATOR_SIZE_H_

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
 * @defgroup QualExprAggregatorSize Size aggregators
 * Group of aggregator operating on the event value supposed to be a size.
 * @ingroup QualityExpressionEvaluation
 */

/**
   @class QualExprAggregatorSize
   @brief Base class for size based aggregators
   @ingroup QualExprAggregatorSize
 */
class QualExprAggregatorSize : public QualExprAggregatorEvalBasic<long64_t>{
protected:
    /* Constructor */ QualExprAggregatorSize( size_t id, bool r = false ) :
        QualExprAggregatorEvalBasic<long64_t>( id ) {
    }

public:
    static void registerToAggregatorNS( QualExprAggregatorNamespace& aggregNs );                    //!< Record all aggregators in the group in the namespace.
};

/**
   @class QualExprAggregatorSizeSum
   @brief Direct aggregation of event sizes.
   @ingroup QualExprAggregatorSize
 */
class QualExprAggregatorSizeSum : public QualExprAggregatorSize {
public:
    /* Constructor */ QualExprAggregatorSizeSum( size_t id ) : QualExprAggregatorSize( id ) {
        reset();
    }
    /* Constructor */ QualExprAggregatorSizeSum( QualExprAggregatorNamespace& aggregNs ) : QualExprAggregatorSize( 0 ) {
        aggregNs.registerNewAggregator( '|', this );
    }
    /* Destructor */ virtual ~QualExprAggregatorSizeSum( void ) {
    }

public:
    virtual void display( const std::string& indent,
                          std::stringstream& s ) const;

    virtual void processEvent( const QualExprEvent& event );                                                                                                                 //!< Aggregate the given event.

    virtual long64_t evaluate( void ) const {
        return value();
    }                                                                                                                                                           //!< Aggregator evaluation.

    virtual const char* name( void ) const {
        return "|size";
    }                                                                                                                                                           //!< Aggregator fully qualified name.

    virtual const char* description( void ) const {
        return "Accumulated size";
    }                                                                                                                                                           //!< Aggregator description.

    virtual QualExprAggregator* build( size_t id ) const {
        return new QualExprAggregatorSizeSum( id );
    }                                                                                                                                                           //!< Auto-constructor.
};

/**
   @class QualExprAggregatorSizeAverage
   @brief Average aggregation of event sizes.
   @ingroup QualExprAggregatorSize
 */
class QualExprAggregatorSizeAverage : public QualExprAggregatorSizeSum {
public:
    /* Constructor */ QualExprAggregatorSizeAverage( size_t id ) : QualExprAggregatorSizeSum( id ) {
        reset();
    }
    /* Constructor */ QualExprAggregatorSizeAverage( QualExprAggregatorNamespace& aggregNs ) : QualExprAggregatorSizeSum( 0 ) {
        aggregNs.registerNewAggregator( '~', this );
    }
    /* Destructor */ virtual ~QualExprAggregatorSizeAverage( void ) {
    }

public:
    virtual void display( const std::string& indent,
                          std::stringstream& s ) const;

    virtual long64_t evaluate( void ) const {
        return average();
    }                                                                                                                                                                   //!< Aggregator evaluation.

    virtual const char* name( void ) const {
        return "~size";
    }                                                                                                                                                                   //!< Aggregator fully qualified name.

    virtual const char* description( void ) const {
        return "Average size";
    }                                                                                                                                                                   //!< Aggregator description.

    virtual QualExprAggregator* build( size_t id ) const {
        return new QualExprAggregatorSizeAverage( id );
    }                                                                                                                                                                   //!< Auto-constructor.
};

/**
   @class QualExprAggregatorSizeMax
   @brief Maximum of all event sizes.
   @ingroup QualExprAggregatorSize
 */
class QualExprAggregatorSizeMax : public QualExprAggregatorSize {
public:
    /* Constructor */ QualExprAggregatorSizeMax( size_t id ) : QualExprAggregatorSize( id ) {
        reset();
    }
    /* Constructor */ QualExprAggregatorSizeMax( QualExprAggregatorNamespace& aggregNs ) : QualExprAggregatorSize( 0 ) {
        aggregNs.registerNewAggregator( '+', this );
    }
    /* Destructor */ virtual ~QualExprAggregatorSizeMax( void ) {
    }

public:
    virtual void display( const std::string& indent,
                          std::stringstream& s ) const;

    virtual void processEvent( const QualExprEvent& event );                                                                                                                 //!< Aggregate the given event.

    virtual long64_t evaluate( void ) const {
        return value();
    }                                                                                                                                                           //!< Aggregator evaluation.

    virtual const char* name( void ) const {
        return "+size";
    }                                                                                                                                                           //!< Aggregator fully qualified name.

    virtual const char* description( void ) const {
        return "Max size";
    }                                                                                                                                                           //!< Aggregator description.

    virtual QualExprAggregator* build( size_t id ) const {
        return new QualExprAggregatorSizeMax( id );
    }                                                                                                                                                           //!< Auto-constructor.
};

/**
   @class QualExprAggregatorSizeMin
   @brief Minimum of all event sizes.
   @ingroup QualExprAggregatorSize
 */
class QualExprAggregatorSizeMin : public QualExprAggregatorSize {
public:
    /* Constructor */ QualExprAggregatorSizeMin( size_t id ) : QualExprAggregatorSize( id ) {
        reset();
    }

    /* Constructor */ QualExprAggregatorSizeMin( QualExprAggregatorNamespace& aggregNs ) : QualExprAggregatorSize( 0 ) {
        aggregNs.registerNewAggregator( '-', this );
    }

    /* Destructor */ virtual ~QualExprAggregatorSizeMin( void ) {
    }

public:
    virtual void display( const std::string& indent,
                          std::stringstream& s ) const;

    virtual void processEvent( const QualExprEvent& event );                                                                                                                         //!< Aggregate the given event.

    virtual long64_t evaluate( void ) const {
        return value();
    }                                                                                                                                                                   //!< Aggregator evaluation.

    virtual const char* name( void ) const {
        return "-size";
    }                                                                                                                                                                   //!< Aggregator fully qualified name.

    virtual const char* description( void ) const {
        return "Min size";
    }                                                                                                                                                                   //!< Aggregator description.

    virtual QualExprAggregator* build( size_t id ) const {
        return new QualExprAggregatorSizeMin( id );
    }                                                                                                                                                                   //!< Auto-constructor.

    virtual void reset( void ) {
        m_value = -1;
        m_count = 0;
    }                                                                                                                                                                   //!< Reset the aggregator state.
};
}

#endif
