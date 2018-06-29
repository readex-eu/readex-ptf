/**
   @file    QualityExpressions.cc
   @ingroup TuningProperties
   @brief   Classes managing quality expressions
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
#include <iostream>
#include <sstream>

#include "quality-expressions/QualityExpressions.h"

/* --------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------- */

const std::string& QualityExpression::mangle( void ) const {
    // TODO: Implement a real mangling
    return ( const std::string& )*this;
}

bool QualityExpression::setMangledExpression( const std::string& mangled ) {
    // TODO: Implement a real unmangling
    assign( mangled );
    return true;
}

void QualityExpression::display( const std::string& indent, std::stringstream& s ) const {
    s << this->c_str();
}

void QualityExpressionEntry::display( const std::string& indent, std::stringstream& s ) const {
    s << '(';
    m_expression.display( indent, s );
    s << ',' << m_id << ')';
}

/* --------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------- */
