/**
   @file    QualExprSemanticAggregatorDB.h
   @ingroup QualityExpressionEvaluation
   @brief   Evaluation of quality expressions - Database of active aggregators
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

#ifndef QUALEXP_H_
#define QUALEXP_H_

namespace quality_expressions_core {
/**
   @class QualExprSemanticAggregatorDB
   @brief Database of all semantic/aggregator combinations.
   @ingroup QualityExpressionEvaluation
 */
class QualExprSemanticAggregatorDB {
public:
    /**
       @class Exception
       @brief Hold an exception with a description.
     */
    class Exception : public QualExprException {
public:
        /* Constructor */ Exception( const char* message ) throw( ) : QualExprException( message ) {
        }

        /* Constructor */ Exception( const std::string& message ) throw( ) : QualExprException( message ) {
        }

        /* Destructor */ ~Exception( void ) throw( ) {
        }
    };

public:
    /* Constructor */ QualExprSemanticAggregatorDB( QualExprSemanticNamespaceStem& semRootNs, QualExprAggregatorNamespace& aggregRootNs ) :
        m_aggregatorNextID( 0 ), m_semanticRootNamespace( semRootNs ), m_aggregatorRootNamespace( aggregRootNs ), m_semAggregatorQuickList( NULL ) {
    }

    /* Destructor */ ~QualExprSemanticAggregatorDB( void );

public:   // -- Access API
    void display( const std::string& indent,
                  std::stringstream& s ) const;                             //!< Display the full namespace description.

public:                                                                     // -- Semantic aggregation API
    QualExprSemanticAggregator& pushAggregator( const QualityExpression& qualExpr ) throw( Exception ); //!< Parse and build a semantic aggregator. @return the aggregator ID
    .
    QualExprSemanticAggregator&
    pushAggregator( const std::string& eventName,
                    const std::string aggregName ) throw( Exception );                                                                  //!< Build a semantic aggregator. @return the aggregator ID.

    void resetMeasures( void );                                                                                                              //!< Reset to the neutral value all aggregators.

    void clearMeasures( void );                                                                                                              //!< Remove all semantic aggregator.

    void consolidate( void ) throw( );                                                                                                       //!< If possible improve data structures to speed-up event evaluations.

    void evaluateEvent( const QualExprEvent& event ) throw( );                                                                               //!< Update semantic aggregators with event properties.

private:
    void cacheAggregators( void );                                                                                                           //!< Cache the current list of aggregators.

    void cleanAggregatorCache( void );                                                                                                       //!< Clear the aggregator cache.

private:
    size_t                                         m_aggregatorNextID;                          //!< Next aggregator ID, strictly growing, it is unique.
    QualExprSemanticNamespaceStem&                 m_semanticRootNamespace;                     //!< Reference to the root namespace of semantics.
    QualExprAggregatorNamespace&                   m_aggregatorRootNamespace;                   //!< Reference to the root namespace of aggregators.
    std::vector<class QualExprSemanticAggregator*> m_semAggregatorList;                         //!< List of all active semantic aggregators.
    QualExprSemanticAggregator**                   m_semAggregatorQuickList;                    //!< Temporary List of all active semantic aggregators.
};
}

#endif
