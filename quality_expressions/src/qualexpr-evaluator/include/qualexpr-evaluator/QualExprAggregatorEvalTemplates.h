/**
   @file    QualExprAggregatorEvalTemplates.h
   @ingroup QualityExpressionEvaluation
   @brief   Evaluation of quality expressions - aggregator evaluation template
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

#ifndef QUALEXP_AGGREGATOR_EVAL_TEMPLATES_H_
#define QUALEXP_AGGREGATOR_EVAL_TEMPLATES_H_

#include "qualexpr-evaluator/QualExprAggregator.h"

namespace quality_expressions_core {
/**
   @class QualExprAggregatorEval
   @brief Extend the basic aggregator type with a type number and an evaluation API.
   @param kind The numeric type used for the computation.
   @ingroup QualityExpressionEvaluation
 */
template <typename kind>
class QualExprAggregatorEval : public QualExprAggregator {
protected:
    /* Constructor */ QualExprAggregatorEval( size_t id ) : QualExprAggregator( id ) {
    }                                                                                           //!< Pure interface, no direct constructor.

public:
    /* Constructor */ virtual ~QualExprAggregatorEval( void ) {
    }

    virtual kind evaluate( void ) const = 0;                                                                         //!< Aggregator evaluation.
};

/**
   @class QualExprAggregatorEvalBasic
   @brief Propose a basic aggregator evaluation kind based on a value and on basic aggregation computations.
   @param kind The numeric type used for the computation.

   This basic aggregator can be used to accumulate a value and the numner of aggregation occurences, and
   thus, is able to perform basic statistics : average, sum, max, min...

   @ingroup QualityExpressionEvaluation
 */
template <typename kind>
class QualExprAggregatorEvalBasic : public QualExprAggregatorEval<kind>{
protected:
    /* Constructor */ QualExprAggregatorEvalBasic( size_t id ) : QualExprAggregatorEval<kind>( id ), m_count( 0 ), m_value( 0 ) {
    }

protected:   // -- Evaluation API
    virtual kind average( void ) const {
        return m_count ? m_value / m_count : 0;
    }                                                                                                   //!< Average is the value per aggregation occurrences.

    virtual kind value( void ) const {
        return m_value;
    }                                                                                                   //!< Current aggregation value - by value.

    virtual size_t count( void ) const {
        return m_count;
    }                                                                                                   //!< Aggregation occurrences - by value.

    virtual kind& value( void ) {
        return m_value;
    }                                                                                                   //!< Current aggregation value - by reference.

    virtual size_t& count( void ) {
        return m_count;
    }                                                                                                   //!< Aggregation occurrences - by reference.

    virtual void reset( void ) {
        m_value = 0;
        m_count = 0;
    }     //!< Reset the aggregator state.

public:   // -- Access API
    /** @brief Display debugging information about the object. */
    virtual void display( const std::string& indent, std::stringstream& s ) const {
        s << indent << "<" << std::setprecision( 24 ) << m_value << ">" << "[" << std::setprecision( 24 ) << m_count << "]";
    }

protected:
    size_t m_count;                                     //!< Number of events.
    kind   m_value;                                     //!< The current aggregation value.
};
}

#endif
