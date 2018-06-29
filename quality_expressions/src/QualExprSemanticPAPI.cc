/**
   @file    QualExprSemanticPAPI.cc
   @ingroup QualExprSemanticPAPI
   @brief   PAPI Semantic definitions - implementation
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

#include <stdio.h>
#include "quality-expressions/QualExprSemanticPAPI.h"

namespace quality_expressions_ns {
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */

void QualExprSemanticNamespacePAPI::registerEventsToAggregatorNS( void ) {
#define QUALEXPRSEMANTIC_PAPI_DEF( DefName, KIND ) new QualExprSemanticPAPI ## DefName( *this );
#include "quality-expressions/QualityExpressionsProfilerPAPI.h"
#undef QUALEXPRSEMANTIC_PAPI_DEF
}

void QualExprSemanticNamespacePAPI::checkAndRegisterNewSemantic( QualExprSemanticPAPI* semantic ) {
    if( qualExpr_papi_isAvailable( semantic->semantic() ) ) {
        registerNewSemantic( semantic );
    }
    else {
        delete semantic;
    }
}

QualExprSemantic* QualExprSemanticNamespacePAPI::buildNewSemantic( const std::string& semName ) const           //!< Build a new semantic entry from the the list of constructors available.
{
    QualExprSemantic*                 semantic     = QualExprSemanticNamespaceLeaf::buildNewSemantic( semName );
    const QualExprSemanticPAPI* const papiSemantic = dynamic_cast<const QualExprSemanticPAPI* const>( semantic );
    if( papiSemantic ) {
        qualExpr_papi_addSemantic( papiSemantic->semantic() );
    }
    return semantic;
}

void QualExprSemanticNamespacePAPI::closeAllSemantics( void ) {
    qualExpr_papi_reset();
    QualExprSemanticNamespaceLeaf::closeAllSemantics();
}
}
