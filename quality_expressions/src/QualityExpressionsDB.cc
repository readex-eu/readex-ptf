/**
   @file    QualityExpressionsDB.cc
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

/**
   @brief Public DB constructor
   Dabase cleaned and starting ID set to 0.
 */
/* Constructor */ QualityExpressionDB::QualityExpressionDB( void ) :
    m_idIterator( 0 ), m_qualityExpressionsSet(), m_qualityExpressionsByID() {
}

/**
   @brief Public destructor.
   Dabase cleared before destruction.
 */
/* Destructor */ QualityExpressionDB::~QualityExpressionDB( void ) {
    clear();
}

void QualityExpressionDB::clear( void ) {
    m_qualityExpressionsSet.clear();
    for( QualityExpressionSet_T::iterator dbItem = m_qualityExpressionsSet.begin(); dbItem != m_qualityExpressionsSet.end(); dbItem++ ) {
        delete dbItem->second;
    }
    m_qualityExpressionsByID.clear();
}

/**
   @brief Register the quality expression and return the associated list of ids.

   This methods register a quality expression in the database.
   A generic quality expression can have multiple independent sub-expressions separated by ';',
   each sub-expressions is identify as a quality expressions entry.
   The registration record each quality expressions entry with a unique ID.
   If an equivalent the expressions entry already exist, the expression is not duplicated.
 */
void QualityExpressionDB::registerQualityExpression( const QualityExpression&            expression,
                                                     std::vector<QualityExpressionID_T>& p_idList ) {
    const char        delimiter[] = ";";
    size_t            cur         = 0;
    size_t            nextCur     = expression.find_first_of( delimiter );
    QualityExpression subExpression;

    do {
        if( nextCur != std::string::npos ) {
            subExpression = expression.substr( cur, nextCur - cur );
        }
        else {
            subExpression = expression.substr( cur );
        }

        registerQualityExpressionEntry( subExpression, p_idList );

        cur = nextCur;
        if( nextCur != std::string::npos ) {
            cur     = nextCur + 1;
            nextCur = expression.find_first_of( delimiter, cur + 1 );
        }
    }
    while( cur != std::string::npos );
}

void QualityExpressionDB::registerQualityExpressionEntry( const QualityExpression&            expression,
                                                          std::vector<QualityExpressionID_T>& p_idList ) {
    QualityExpressionID_T            result = 0;
    QualityExpressionSet_T::iterator dbItem = m_qualityExpressionsSet.find( expression );
    if( dbItem != m_qualityExpressionsSet.end() ) {
        result = dbItem->second->id();
    }
    else {
        result = ++m_idIterator;
        QualityExpressionEntry* entry = new QualityExpressionEntry( m_idIterator, expression );
        m_qualityExpressionsSet[ expression.mangle() ] = entry;
        m_qualityExpressionsByID[ result ]             = entry;
    }
    p_idList.push_back( result );
}

const QualityExpression& QualityExpressionDB::getQualityExpressionById( QualityExpressionID_T id ) throw( QualityExpressionDB::Exception )                          {
    QualityExpressionMapById_T::iterator dbItem = m_qualityExpressionsByID.find( id );
    if( dbItem == m_qualityExpressionsByID.end() ) {
        throw( Exception( "QualityExpressionDB::getQualityExpressionById: not found" ) );
    }
    return dbItem->second->expression();
}

void QualityExpressionDB::display( const std::string& indent,
                                   std::stringstream& s ) const {
    std::string nindent = indent;
    nindent += "  ";
    s << "idIterator=" << m_idIterator;
    s << indent << "Quality expression entries:";
    for( QualityExpressionSet_T::const_iterator dbItem = m_qualityExpressionsSet.begin(); dbItem != m_qualityExpressionsSet.end(); dbItem++ ) {
        s << indent;
        dbItem->second->display( nindent, s );
    }
}

/* --------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------- */
