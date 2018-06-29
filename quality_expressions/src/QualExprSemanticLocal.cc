/**
   @file    QualExprSemanticLocal.cc
   @ingroup QualExprSemanticLocal
   @brief   Local Semantic definitions - implementation
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

#include "quality-expressions/QualExprSemanticLocal.h"

namespace quality_expressions_ns {
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */

void QualExprSemanticNamespaceLocal::registerEventsToAggregatorNS( void ) {
#define QUALEXPRSEMANTIC_LOCAL_DEF( DefName, matchSemanticExpression ) new QualExprSemanticLocal ## DefName( *this );
#include "quality-expressions/QualityExpressionsProfilerLocal.h"
#undef QUALEXPRSEMANTIC_LOCAL_DEF
}
}
