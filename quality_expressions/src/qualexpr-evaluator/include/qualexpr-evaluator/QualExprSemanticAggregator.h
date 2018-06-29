/**
   @file    QualExprSemanticAggregator.h
   @ingroup QualityExpressionEvaluation
   @brief   Evaluation of quality expressions - Dual aggregator/semantic association
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

#ifndef QUALEXP_SEMANTIC_AGGREGATOR_H_
#define QUALEXP_SEMANTIC_AGGREGATOR_H_

namespace quality_expressions_core {
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */

/**
   @class QualExprSemanticAggregator
   @brief Combines a semantic and an aggregator.
   @ingroup QualityExpressionEvaluation
 */
class QualExprSemanticAggregator {
public:
    /* Constructor */ QualExprSemanticAggregator( const QualExprSemantic& sem, QualExprAggregator& aggregator ) : m_sem( sem ), m_aggregator( aggregator ) {
    }

    /* Destructor */ ~QualExprSemanticAggregator( void ) {
        delete &m_sem;
        delete &m_aggregator;
    }

public:   // -- Access API
    const char* aggregName( void ) const {
        return m_aggregator.name();
    }

    const char* semanticName( void ) const {
        return m_sem.name();
    }

    std::string name( void ) const {
        std::string r = m_sem.name();
        r += ':';
        r += m_aggregator.name();
        return r;
    }

    void display( const std::string& indent, std::stringstream& s ) const {
        s << m_sem.name() << ':';
        m_aggregator.display( indent, s );
    }

    size_t getId( void ) const {
        return m_aggregator.getId();
    }

    void reset( void ) {
        return m_aggregator.reset();
    }     //!< Reset to the neutral value all aggregators.

public:   // -- Semantic aggregation API
    bool matchSemantic( unsigned int sem ) {
        return m_sem.matchSemantic( sem );
    }                                                                                                                                   //!< Return if the semantic match the given semantic ID.

    void processEvent( const QualExprEvent& event ) {
        m_aggregator.processEvent( event );
    }                                                                                                                                   //!< Aggregate the given event.

    /** @brief Aggregator evaluation method.
        @remarks kind is the numeric type used for the computation.
        @param neutralValue

        TODO: This aggregator evaluation takes a neutral value in reaplacement of a correct error management.
        This must be changed, defensive code is never the right answer.
     */
    template <typename kind>
    kind evaluate( kind neutralValue = 0 ) const {
        QualExprAggregatorEval<kind>* aggregator = dynamic_cast<QualExprAggregatorEval<kind>*>( &m_aggregator );
        return aggregator ? aggregator->evaluate() : neutralValue;
    }

private:
    const QualExprSemantic& m_sem;                                                      //!< The semantic descriptor.
    QualExprAggregator&     m_aggregator;                                               //!< The event aggregator.
};
}

#endif
