/**
   @file    QualExprSemanticPAPI.h
   @ingroup QualExprSemanticPAPI
   @brief   PAPI Semantic definitions - headers
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

#ifndef QUALEXP_SEMANTICPAPI_H_
#define QUALEXP_SEMANTICPAPI_H_

#include "quality-expressions/QualityExpressionsProfilerPAPI.h"
#include "quality-expressions/QualityExpressionsProfilerSystem.h"
#include "quality-expressions/QualExprSemanticNamespace.h"

namespace quality_expressions_ns {
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */

/**
 * @defgroup QualExprSemanticPAPI PAPI profiling semantic namespace
 * Group of semantics interfacing event of the PAPI profiler.
 * @ingroup QualityExpressionNamespace
 */

/**
   @class QualExprSemanticPAPI
   @brief Semantics interfacing events of the PAPI profiler.
   @ingroup QualExprSemanticPAPI
 */
class QualExprSemanticPAPI : public QualExprSemantic {
protected:
    /* Constructor */ QualExprSemanticPAPI( void ) : QualExprSemantic() {
    }
public:
    /* Destructor */ virtual ~QualExprSemanticPAPI( void ) {
    }

    virtual bool matchSemantic( unsigned int sem ) const = 0;

    virtual const char* name( void ) const = 0;

    virtual enum qualexpr_event_papi_t semantic( void ) const = 0;
};

/**
   @class QualExprSemanticNamespacePAPI
   @brief Semantics namespace of all PAPI events.
   @ingroup QualExprSemanticPAPI
 */
class QualExprSemanticNamespacePAPI : public QualExprSemanticNamespaceLeaf {
public:
    /* Constructor */ QualExprSemanticNamespacePAPI( void ) : QualExprSemanticNamespaceLeaf( "papi" ) {
        registerEventsToAggregatorNS();
    }
    /* Destructor */ virtual ~QualExprSemanticNamespacePAPI( void ) {
    }

    virtual QualExprSemantic* buildNewSemantic( const std::string& semName ) const;                                       //!< Build a new semantic entry from the the list of constructors available.

    virtual void closeAllSemantics( void );                                                                  //!< Remove all semantic constructors.

    void checkAndRegisterNewSemantic( QualExprSemanticPAPI* semantic );                              //!< Check availability before adding the semantic. Destroy the object if not.

private:
    void registerEventsToAggregatorNS( void );
};

#define QUALEXPRSEMANTIC_PAPI_DEF( DefName, KIND )                                                                                                      \
    class QualExprSemanticPAPI ## DefName : public QualExprSemanticPAPI {                                                                               \
public:                                                                                                                                                 \
        /* Constructor */ QualExprSemanticPAPI ## DefName( void ) {}                                                     \
        /* Constructor */ QualExprSemanticPAPI ## DefName( QualExprSemanticNamespacePAPI & aggregNs ) { aggregNs.checkAndRegisterNewSemantic( this ); } \
public:                                                                                                                                                 \
        virtual bool matchSemantic( unsigned int sem ) const { return sem == QE_PROFILER_PAPI_ ## DefName; }                                            \
        virtual const char* name( void ) const { return "papi::" # DefName; }                                                                            \
        virtual QualExprSemantic* build( void ) const { return new QualExprSemanticPAPI ## DefName(); }                                                 \
        virtual enum qualexpr_event_papi_t semantic( void ) const { return QE_PROFILER_PAPI_ ## DefName; }                                              \
    };
#include "quality-expressions/QualityExpressionsProfilerPAPI.h"
#undef QUALEXPRSEMANTIC_PAPI_DEF
}

#endif
