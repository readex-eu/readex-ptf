/**
   @file    QualExprSemantic.cc
   @ingroup QualityExpressionNamespace
   @brief   Evaluation of quality expressions - semantic implementation
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
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "quality-expressions/QualExprSemantic.h"
#include "quality-expressions/QualExprSemanticNamespace.h"

namespace quality_expressions_ns {
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */

void QualExprSemanticNamespace::display( const std::string& indent, std::stringstream& s ) const {
    s << m_namespace.c_str() << ":: empty.";
}

/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */

void QualExprSemanticNamespaceStem::registerNewNamespace( const QualExprSemanticNamespace& ns ) {
    m_subNamespace.push_back( &ns );
}

void QualExprSemanticNamespaceStem::closeAllSemantics( void ) {
    for( size_t index = 0; index < m_subNamespace.size(); index++ ) {
        const QualExprSemanticNamespace* node = m_subNamespace[ index ];
        delete node;
    }
    m_subNamespace.clear();
}

QualExprSemantic* QualExprSemanticNamespaceStem::buildNewSemantic( const std::string& semName ) const {
    for( size_t index = 0; index < m_subNamespace.size(); index++ ) {
        const QualExprSemanticNamespace* node = m_subNamespace[ index ];
        if( semName.find( node->groupNamespace() ) == 0 ) {
            return node->buildNewSemantic( semName );
        }
    }
    return NULL;
}

void QualExprSemanticNamespaceStem::display( const std::string& indent,
                                             std::stringstream& s ) const {
    if( m_subNamespace.empty() ) {
        s << m_namespace.c_str() << ":: empty.";
    }
    else {
        std::string nindent = indent;
        nindent += " * ";
        s << m_namespace.c_str() << "::";
        for( size_t index = 0; index < m_subNamespace.size(); index++ ) {
            const QualExprSemanticNamespace* node = m_subNamespace[ index ];
            s << nindent;
            node->display( nindent, s );
        }
    }
}

/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */

void QualExprSemanticNamespaceLeaf::registerNewSemantic( const QualExprSemantic* semDesc ) {
    m_semanticList.push_back( semDesc );
}

void QualExprSemanticNamespaceLeaf::closeAllSemantics( void ) {
    for( size_t index = 0; index < m_semanticList.size(); index++ ) {
        const QualExprSemantic* semDesc = m_semanticList[ index ];
        delete semDesc;
    }
    m_semanticList.clear();
}

QualExprSemantic* QualExprSemanticNamespaceLeaf::buildNewSemantic( const std::string& semName ) const {
    for( size_t index = 0; index < m_semanticList.size(); index++ ) {
        const QualExprSemantic* semDesc = m_semanticList[ index ];
        if( semDesc->name() == semName ) {
            return semDesc->build();
        }
    }
    return NULL;
}

void QualExprSemanticNamespaceLeaf::display( const std::string& indent,
                                             std::stringstream& s ) const {
    if( m_semanticList.empty() ) {
        s << "::" << m_namespace.c_str() << ":: empty.";
    }
    else {
        s << "::" << m_namespace.c_str() << "::";
        for( size_t index = 0; index < m_semanticList.size(); index++ ) {
            const QualExprSemantic* semDesc = m_semanticList[ index ];
            s << indent << " " << semDesc->name();
        }
    }
}
}
