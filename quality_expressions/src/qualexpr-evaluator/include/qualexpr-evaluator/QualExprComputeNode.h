/**
   @file    QualExprComputeNode.h
   @ingroup QualityExpressionEvaluation
   @brief   Evaluation of quality expressions - Dynamic compute nodes
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

#ifndef QUALEXP_COMPUTE_NODE_H_
#define QUALEXP_COMPUTE_NODE_H_

namespace quality_expressions_core {
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */

/**
   @class QualExprComputeNode
   @brief Pure interface for on the fly quality expression compute nodes
   @ingroup QualityExpressionEvaluation
 */
class QualExprComputeNode {
public:
    /* Destructor */ virtual ~QualExprComputeNode( void ) {
    }

public:   // -- Access API
    virtual void display( const std::string& indent, std::stringstream& s ) const {
        s << "<abstract node>";
    }

protected:
    /* Constructor */ QualExprComputeNode( void ) {
    }
};

/**
   @class QualExprComputeNode
   @brief Interface for a compute node with an abstract evaluation
   @ingroup QualityExpressionEvaluation
 */
template<typename kind>
class QualExprComputeNodeOf : public QualExprComputeNode {
public:
    /* Destructor */ virtual ~QualExprComputeNodeOf( void ) {
    }

public:   // -- Access API
    virtual void display( const std::string& indent, std::stringstream& s ) const {
        s << "<abstract node>";
    }

    virtual kind eval( void ) = 0;

protected:
    /* Constructor */ QualExprComputeNodeOf( void ) {
    }
};

/**
   @class QualExprComputeNodeOfImmediate
   @brief Compute node with an abstract and absolute immediate value
   @ingroup QualityExpressionEvaluation
 */
template<typename kind>
class QualExprComputeNodeImmediate : public QualExprComputeNodeOf<kind>{
public:
    /* Constructor */ QualExprComputeNodeImmediate( kind value ) : m_value( value ) {
    }

    /* Destructor */ virtual ~QualExprComputeNodeImmediate( void ) {
    }

public:   // -- Access API
    virtual void display( const std::string& indent, std::stringstream& s ) const {
        s << std::setprecision( 24 ) << m_value;
    }

    virtual kind eval( void ) {
        return m_value;
    }
private:
    kind m_value;
};

/**
   @class QualExprComputeNodeOfAggreg
   @brief Compute node evaluation via a semantic aggregator with and abstract evaluation
   @ingroup QualityExpressionEvaluation
 */
template<typename kind>
class QualExprComputeNodeAggreg : public QualExprComputeNodeOf<kind>{
public:
    /* Constructor */ QualExprComputeNodeAggreg( QualExprSemanticAggregator& aggreg ) : m_semanticAggregator( aggreg ) {
    }

    /* Destructor */ virtual ~QualExprComputeNodeAggreg( void ) {
    }

public:   // -- Access API
    virtual void display( const std::string& indent, std::stringstream& s ) const {
        m_semanticAggregator.display( indent, s );
    }

    virtual kind eval( void ) {
        return m_semanticAggregator.evaluate<kind>();
    }

private:
    QualExprSemanticAggregator& m_semanticAggregator;                           //!< Semantic aggregators.
};

/**
   @class QualExprComputeNodeOfBinOp
   @brief
   @ingroup QualityExpressionEvaluation
 */
template <typename kind, class arithmetic>
class QualExprComputeNodeBinOp : public QualExprComputeNodeOf<kind>{
public:
    /* Constructor */ QualExprComputeNodeBinOp( QualExprComputeNodeOf<kind>& lhs, QualExprComputeNodeOf<kind>& rhs ) : m_lhs( lhs ), m_rhs( rhs ) {
    }

    /* Destructor */ virtual ~QualExprComputeNodeBinOp( void ) {
        delete &m_lhs;
        delete &m_rhs;
    }
public:   // -- Access API
    virtual void display( const std::string& indent, std::stringstream& s ) const {
        m_lhs.display( indent, s );
        arithmetic::display( indent, s );
        m_rhs.display( indent, s );
    }

    virtual kind eval( void ) {
        return arithmetic::compute( m_lhs.eval(), m_rhs.eval() );
    }

private:
    QualExprComputeNodeOf<kind>& m_lhs;
    QualExprComputeNodeOf<kind>& m_rhs;
};
}

#endif
