/**
   @file    QualityExpressions.h
   @ingroup QualityExpressionDataModel
   @brief   Basic quality expression entry
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

#ifndef QUALITYEXPRESSIONENTRY_H_
#define QUALITYEXPRESSIONENTRY_H_

//** @brief Define the type for a unique identifier for quality expressions.
//** @ingroup QualityExpressionDataModel
typedef unsigned int QualityExpressionID_T;

/**
   @class QualityExpressionEntry
   @brief Define and identify uniquely a quality expression
   @ingroup QualityExpressionDataModel

   A quality expression entry is used to avoid the multiple evaluation
   of a quality expression with the binding with a unique ID.

   @sa QualityExpression
 */
class QualityExpressionEntry {
public:         // Class Definitions
    /* Constructor */ QualityExpressionEntry( QualityExpressionID_T id, const QualityExpression& expression ) : m_id( id ), m_expression( expression ) {
    }

    /* Destructor */ ~QualityExpressionEntry( void ) {
    }

public:         // Access API
    QualityExpressionID_T id( void ) const {
        return m_id;
    }

    QualityExpression& expression( void ) {
        return m_expression;
    }

    const QualityExpression& expression( void )  const {
        return m_expression;
    }

    void setExpression( const QualityExpression& expression ) {
        m_expression = expression;
    }

    void display( const std::string& indent,
                  std::stringstream& s ) const; //!< Display debugging information about the object.

private:                                   // Data structures
    QualityExpressionID_T m_id;            //!< Globally unique ID for the management and evaluation of the quality expression.
    QualityExpression     m_expression;    //!< Quality expression used for the evaluation.
};

#endif // QUALITYEXPRESSIONENTRY_H_
