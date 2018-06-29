/**
   @file    QualExprAggregatorImmediate.h
   @ingroup QualityExpressionEvaluation
   @brief   Evaluation of quality expressions - aggregator immediate evaluation
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

#ifndef QUALEXP_AGGREGATOR_IMMEDIATE_H_
#define QUALEXP_AGGREGATOR_IMMEDIATE_H_

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
 * @defgroup QualExprAggregatorImmediate Immediate value aggregator
 * Pseudo aggregator operating on the immediate value of an event or a counter.
 * @ingroup QualityExpressionEvaluation
 */

/**
   @class QualExprAggregatorImmediate
   @brief Immediate value management

   An immediate event aggregator will only process counters (state is D_COUNTER).
   @sa event_state_t

   @ingroup QualExprAggregatorImmediate
 */
class QualExprAggregatorImmediate : public QualExprAggregatorEval<long64_t>{
public:
    /* Constructor */ QualExprAggregatorImmediate( size_t id ) : QualExprAggregatorEval<long64_t>( id ), m_value( 0 ) {
    }

    /* Constructor */ QualExprAggregatorImmediate( QualExprAggregatorNamespace& aggregNs ) : QualExprAggregatorEval<long64_t>( 0 ), m_value( 0 ) {
        aggregNs.registerNewAggregator( '!', this );
    }

    /* Destructor */ virtual ~QualExprAggregatorImmediate( void ) {
    }

public:
    static void registerToAggregatorNS( QualExprAggregatorNamespace& aggregNs ) {
        new QualExprAggregatorImmediate( aggregNs );
    }                                                                                                                                                                           //!< Record the aggregator in the namespace.

public:
    virtual void display( const std::string& indent,
                          std::stringstream& s ) const;

    virtual void processEvent( const QualExprEvent& event );                                                                                                                                 //!< Aggregate the given event.

    virtual long64_t evaluate( void ) const {
        return m_value;
    }                                                                                                                                                                           //!< Aggregator evaluation.

    virtual const char* name( void ) const {
        return "!";
    }                                                                                                                                                                           //!< Aggregator fully qualified name.

    virtual const char* description( void ) const {
        return "Immediate";
    }                                                                                                                                                                           //!< Aggregator description.

    virtual QualExprAggregator* build( size_t id ) const {
        return new QualExprAggregatorImmediate( id );
    }                                                                                                                                                                           //!< Auto-constructor.

    virtual void reset( void ) {
        m_value = 0;
    }                                                                                                                                                                           //!< Reset the aggregator state.

protected:
    long64_t m_value;                           //!< Immediate value of the counter or event.
};
}

#endif
