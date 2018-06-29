/**
   @file    QualExprSemanticLocal.h
   @ingroup QualExprSemanticLocal
   @brief   Local Semantic definitions - headers
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

#ifndef QUALEXP_SEMANTICLOCAL_H_
#define QUALEXP_SEMANTICLOCAL_H_

#include "quality-expressions/QualityExpressionsProfilerLocal.h"
#include "quality-expressions/QualityExpressionsProfilerSystem.h"
#include "quality-expressions/QualExprSemanticNamespace.h"

namespace quality_expressions_ns {
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */

/**
 * @defgroup QualExprSemanticLocal Local profiling semantic namespace
 * Group of semantics interfacing event of the local profiler.
 * @ingroup QualityExpressionNamespace
 */

/**
   @class QualExprSemanticLocal
   @brief Semantics interfacing event of the local profiler.
   @ingroup QualExprSemanticLocal
 */
class QualExprSemanticLocal : public QualExprSemantic {
protected:
    /* Constructor */ QualExprSemanticLocal( void ) : QualExprSemantic() {
    }
public:
    /* Destructor */ virtual ~QualExprSemanticLocal( void ) {
    }

    virtual const char* name( void ) const {
        return "";
    }
};

/**
   @class QualExprSemanticNamespaceLocal
   @brief Local of semantics namespace.
   @ingroup QualExprSemanticLocal
 */
class QualExprSemanticNamespaceLocal : public QualExprSemanticNamespaceLeaf {
public:
    /* Constructor */ QualExprSemanticNamespaceLocal( void ) : QualExprSemanticNamespaceLeaf( "local" ) {
        registerEventsToAggregatorNS();
    }

    /* Destructor */ virtual ~QualExprSemanticNamespaceLocal( void ) {
    }
private:
    void registerEventsToAggregatorNS( void );
};

#define QUALEXPRSEMANTIC_LOCAL_DEF( DefName, matchSemanticExpression )                                                                   \
    class QualExprSemanticLocal ## DefName : public QualExprSemanticLocal {                                                               \
public:                                                                                                                               \
        /* Constructor */ QualExprSemanticLocal ## DefName( void ) {}                                             \
        /* Constructor */ QualExprSemanticLocal ## DefName( QualExprSemanticNamespaceLeaf & aggregNs ) { aggregNs.registerNewSemantic( this ); }   \
public:                                                                                                                               \
        virtual bool matchSemantic( unsigned int sem ) const { return matchSemanticExpression; }                             \
        virtual const char* name( void ) const { return "local::" # DefName; }                                  \
        virtual QualExprSemantic* build( void ) const { return new QualExprSemanticLocal ## DefName(); }             \
    };
#include "quality-expressions/QualityExpressionsProfilerLocal.h"
#undef QUALEXPRSEMANTIC_LOCAL_DEF
}

#endif
