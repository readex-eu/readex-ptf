/**
   @file    QualExprSemantic.h
   @ingroup QualityExpressionNamespace
   @brief   Evaluation of quality expressions - Semantic model
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

#ifndef QUALEXP_SEMANTIC_H_
#define QUALEXP_SEMANTIC_H_

namespace quality_expressions_ns {
/**
 * @defgroup QualityExpressionNamespace Quality expressions namespaces
 * @ingroup QualityExpression
 */

/**
   @class QualExprSemantic
   @brief Interface for the definition of a particular semantics.
   @ingroup QualityExpressionNamespace
 */
class QualExprSemantic {
protected:
    /* Constructor */ QualExprSemantic( void ) {
    }                                                                                   //!< Pure interface, no direct constructor.
public:
    /* Destructor */ virtual ~QualExprSemantic( void ) {
    }

    virtual bool matchSemantic( unsigned int sem ) const = 0;                                        //!< Return if the semantic match the given semantic ID.

    virtual const char* name( void ) const = 0;                                                             //!< The semantic name - shall be globally unique.

    virtual QualExprSemantic* build( void ) const = 0;
};
}

#endif
