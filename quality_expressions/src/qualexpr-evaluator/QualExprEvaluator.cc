/**
   @file    QualExprEvaluator.cc
   @ingroup QualityExpressionEvaluation
   @brief   Evaluation of quality expressions - implementation
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

#include "qualexpr-evaluator/QualExprEvaluator.h"
#include "qualexpr-evaluator/QualExprEvaluatorParserDriver.h"

namespace quality_expressions_core {
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */

void QualExprAggregatorNamespace::closeAllAggregators( void ) {
    for( size_t index = 0; index < m_aggregatorList.size(); index++ ) {
        const QualExprAggregator* aggreg = m_aggregatorList[ index ];
        delete aggreg;
    }
    m_aggregatorList.clear();
}

QualExprAggregator* QualExprAggregatorNamespace::buildNewAggregator( const char         op,
                                                                     const std::string& aggregatorName,
                                                                     size_t             id ) {
    for( size_t index = 0; index < m_aggregatorList.size(); index++ ) {
        const QualExprAggregator* aggreg = m_aggregatorList[ index ];
        if( aggreg->name() == aggregatorName ) {
            return aggreg->build( id );
        }
    }
    return NULL;
}

void QualExprAggregatorNamespace::display( const std::string& indent,
                                           std::stringstream& s ) const {
    if( m_aggregatorList.empty() ) {
        s << m_namespace.c_str() << ":: empty.";
    }
    else {
        s << m_namespace.c_str() << "::";
        for( size_t index = 0; index < m_aggregatorList.size(); index++ ) {
            const QualExprAggregator* aggreg = m_aggregatorList[ index ];
            s << indent << " " << aggreg->name() << ": " << aggreg->description();
        }
    }
}

/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */

void QualExprAggregatorImmediate::processEvent( const QualExprEvent& event ) {
    if( event.m_state == D_COUNTER ) {
        m_value = event.m_value;
    }
}

void QualExprAggregatorImmediate::display( const std::string& indent,
                                           std::stringstream& s ) const {
    s << " immediate=" << std::setprecision( 24 ) << evaluate();
}

/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */

void QualExprAggregatorTime::registerToAggregatorNS( QualExprAggregatorNamespace& aggregNs ) {
    new QualExprAggregatorTimeSum( aggregNs );
    new QualExprAggregatorTimeAverage( aggregNs );
    new QualExprAggregatorTimeMax( aggregNs );
    new QualExprAggregatorTimeMin( aggregNs );
}

void QualExprAggregatorTimeAverage::processEvent( const QualExprEvent& event ) {
    if( event.m_state == D_START ) {
        m_previousTimeStamp = ( event.m_timestamp * 1e9 );
        m_previousEid       = event.m_eid;
    }
    else if( event.m_state == D_STOP ) {
        if( m_previousEid == event.m_eid ) {
            m_count++;
            m_value += ( event.m_timestamp * 1e9 ) - m_previousTimeStamp;
        }
    }
}

void QualExprAggregatorTimeAverage::display( const std::string& indent,
                                             std::stringstream& s ) const {
    s << " time average[" << m_count << "]=" << std::setprecision( 24 ) << evaluate();
}

void QualExprAggregatorTimeMax::processEvent( const QualExprEvent& event ) {
    if( event.m_state == D_START ) {
        m_previousTimeStamp = ( event.m_timestamp * 1e9 );
        m_previousEid       = event.m_eid;
    }
    else if( event.m_state == D_STOP ) {
        if( m_previousEid == event.m_eid ) {
            m_count++;
            long64_t newValue = ( event.m_timestamp * 1e9 ) - m_previousTimeStamp;
            if( m_value < newValue ) {
                m_value = newValue;
            }
        }
    }
}

void QualExprAggregatorTimeMax::display( const std::string& indent,
                                         std::stringstream& s ) const {
    s << " time max[" << m_count << "]=" << std::setprecision( 24 ) << evaluate();
}

void QualExprAggregatorTimeMin::processEvent( const QualExprEvent& event ) {
    if( event.m_state == D_START ) {
        m_previousTimeStamp = ( event.m_timestamp * 1e9 );
        m_previousEid       = event.m_eid;
    }
    else if( event.m_state == D_STOP ) {
        if( m_previousEid == event.m_eid ) {
            m_count++;
            long64_t newValue = ( event.m_timestamp * 1e9 ) - m_previousTimeStamp;
            if( m_value < 0 || m_value > newValue ) {
                m_value = newValue;
            }
        }
    }
}

void QualExprAggregatorTimeMin::display( const std::string& indent,
                                         std::stringstream& s ) const {
    s << " time min[" << m_count << "]=" << std::setprecision( 24 ) << evaluate();
}

void QualExprAggregatorTimeSum::processEvent( const QualExprEvent& event ) {
    if( event.m_state == D_START ) {
        m_previousTimeStamp = ( event.m_timestamp * 1e9 );
        m_previousEid       = event.m_eid;
    }
    else if( event.m_state == D_STOP ) {
        if( m_previousEid == event.m_eid ) {
            m_count++;
            long64_t newValue = ( event.m_timestamp * 1e9 ) - m_previousTimeStamp;
            m_value += newValue;
        }
    }
}

void QualExprAggregatorTimeSum::display( const std::string& indent,
                                         std::stringstream& s ) const {
    s << " time accumulated[" << m_count << "]=" << std::setprecision( 24 ) << evaluate();
}

/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */

void QualExprAggregatorSize::registerToAggregatorNS( QualExprAggregatorNamespace& aggregNs ) {
    new QualExprAggregatorSizeSum( aggregNs );
    new QualExprAggregatorSizeAverage( aggregNs );
    new QualExprAggregatorSizeMax( aggregNs );
    new QualExprAggregatorSizeMin( aggregNs );
}

void QualExprAggregatorSizeSum::processEvent( const QualExprEvent& event ) {
    if( event.m_state == D_START || event.m_state == D_COUNTER ) {
        m_count++;
        m_value += event.m_value;
    }
}

void QualExprAggregatorSizeSum::display( const std::string& indent,
                                         std::stringstream& s ) const {
    s << " size accumulated=" << std::setprecision( 24 ) << evaluate() << " in " << m_count << " calls";
}

void QualExprAggregatorSizeAverage::display( const std::string& indent,
                                             std::stringstream& s ) const {
    s << " size average=" << std::setprecision( 24 ) << evaluate() << " in " << m_count << " calls";
}

void QualExprAggregatorSizeMax::processEvent( const QualExprEvent& event ) {
    if( event.m_state == D_START ) {
        m_count++;
        long64_t newValue = event.m_value;
        if( m_value < newValue ) {
            m_value = newValue;
        }
    }
}

void QualExprAggregatorSizeMax::display( const std::string& indent,
                                         std::stringstream& s ) const {
    s << " size max=" << std::setprecision( 24 ) << evaluate() << " in " << m_count << " calls";
}

void QualExprAggregatorSizeMin::processEvent( const QualExprEvent& event ) {
    if( event.m_state == D_START ) {
        m_count++;
        long64_t newValue = event.m_value;
        if( m_value < 0 || m_value > newValue ) {
            m_value = newValue;
        }
    }
}

void QualExprAggregatorSizeMin::display( const std::string& indent,
                                         std::stringstream& s ) const {
    s << " size min=" << std::setprecision( 24 ) << evaluate() << " in " << m_count << " calls";
}

/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */

void QualExprAggregatorBandwidth::registerToAggregatorNS( QualExprAggregatorNamespace& aggregNs ) {
    new QualExprAggregatorBandwidthAverage( aggregNs );
    new QualExprAggregatorBandwidthMax( aggregNs );
    new QualExprAggregatorBandwidthMin( aggregNs );
}

void QualExprAggregatorBandwidthAverage::processEvent( const QualExprEvent& event ) {
    if( event.m_state == D_START ) {
        m_previousTimeStamp = event.m_timestamp;
        m_previousEid       = event.m_eid;
        m_currentSize       = event.m_value;
    }
    else if( event.m_state == D_STOP ) {
        if( m_previousEid == event.m_eid ) {
            m_count++;
            double time = event.m_timestamp - m_previousTimeStamp;
            m_value = ( m_currentSize / time );
        }
    }
}

void QualExprAggregatorBandwidthAverage::display( const std::string& indent,
                                                  std::stringstream& s ) const {
    s << indent << " bandwidth average=" << std::setprecision( 24 ) << evaluate() << " in " << m_count << " calls";
}

void QualExprAggregatorBandwidthMax::processEvent( const QualExprEvent& event ) {
    if( event.m_state == D_START ) {
        m_previousTimeStamp = event.m_timestamp;
        m_previousEid       = event.m_eid;
        m_currentSize       = event.m_value;
    }
    else if( event.m_state == D_STOP ) {
        if( m_previousEid == event.m_eid ) {
            m_count++;
            double   time     = event.m_timestamp - m_previousTimeStamp;
            long64_t newValue = m_currentSize / time;
            if( m_value < newValue ) {
                m_value = newValue;
            }
        }
    }
}

void QualExprAggregatorBandwidthMax::display( const std::string& indent,
                                              std::stringstream& s ) const {
    s << indent << " bandwidth max=" << std::setprecision( 24 ) << evaluate() << " in " << m_count << " calls";
}

void QualExprAggregatorBandwidthMin::processEvent( const QualExprEvent& event ) {
    if( event.m_state == D_START ) {
        m_previousTimeStamp = event.m_timestamp;
        m_previousEid       = event.m_eid;
        m_currentSize       = event.m_value;
    }
    else if( event.m_state == D_STOP ) {
        if( m_previousEid == event.m_eid ) {
            m_count++;
            double   time     = event.m_timestamp - m_previousTimeStamp;
            long64_t newValue = m_currentSize / time;
            if( m_value < 0 || m_value > newValue ) {
                m_value = newValue;
            }
        }
    }
}

void QualExprAggregatorBandwidthMin::display( const std::string& indent,
                                              std::stringstream& s ) const {
    s << indent << " bandwidth min=" << std::setprecision( 24 ) << evaluate() << " in " << m_count << " calls";
}

/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */

/* Destructor */ QualExprSemanticAggregatorDB::~QualExprSemanticAggregatorDB( void ) {
    clearMeasures();
}

void QualExprSemanticAggregatorDB::cacheAggregators( void ) {
    if( !m_semAggregatorQuickList ) {
        size_t size = m_semAggregatorList.size();
        m_semAggregatorQuickList = ( QualExprSemanticAggregator** )malloc( ( size + 1 ) * sizeof( QualExprSemanticAggregator* ) );
        for( size_t index = 0; index < size; index++ ) {
            m_semAggregatorQuickList[ index ] = m_semAggregatorList[ index ];
        }
        m_semAggregatorQuickList[ size ] = NULL;
    }
}

void QualExprSemanticAggregatorDB::cleanAggregatorCache( void ) {
    if( m_semAggregatorQuickList ) {
        free( m_semAggregatorQuickList );
        m_semAggregatorQuickList = NULL;
    }
}

void QualExprSemanticAggregatorDB::resetMeasures( void ) {
    for( size_t index = 0; index < m_semAggregatorList.size(); index++ ) {
        m_semAggregatorList[ index ]->reset();
    }
}

void QualExprSemanticAggregatorDB::clearMeasures( void ) {
    cleanAggregatorCache();
    for( size_t index = 0; index < m_semAggregatorList.size(); index++ ) {
        delete m_semAggregatorList[ index ];
    }
    m_semAggregatorList.clear();
}

QualExprSemanticAggregator& QualExprSemanticAggregatorDB::pushAggregator( const QualityExpression& measure ) throw( QualExprSemanticAggregatorDB::Exception )                             {
    size_t                      kindLoc   = measure.find_last_of( ":" );
    QualExprSemanticAggregator* newAggreg = NULL;

    if( kindLoc && kindLoc != std::string::npos ) {
        const std::string eventName( measure.substr( 0, kindLoc ) );
        const std::string aggregName( measure.substr( kindLoc + 1 ) );
        newAggreg = &pushAggregator( eventName, aggregName );
    }
    else {
        throw( Exception( "Invalid aggregator" ) );
    }

    return *newAggreg;
}

QualExprSemanticAggregator& QualExprSemanticAggregatorDB::pushAggregator( const std::string& eventName,
                                                                          const std::string  aggregName ) throw( QualExprSemanticAggregatorDB::Exception ) {
    bool                        error     = false;
    size_t                      id        = m_semAggregatorList.size();
    QualExprSemanticAggregator* newAggreg = NULL;

    QualExprAggregator* aggreg = m_aggregatorRootNamespace.buildNewAggregator( '.', aggregName, id );
    if( aggreg ) {
        QualExprSemantic* semDesc = m_semanticRootNamespace.buildNewSemantic( eventName );
        if( semDesc ) {
            for( size_t index = 0; index < m_semAggregatorList.size(); index++ ) {
                QualExprSemanticAggregator* semAggreg = m_semAggregatorList[ index ];
                if( semAggreg->aggregName() == aggreg->name() && semAggreg->semanticName() == semDesc->name() ) {
                    newAggreg = semAggreg;
                    break;
                }
            }

            if( !newAggreg ) {
                newAggreg = new QualExprSemanticAggregator( *semDesc, *aggreg );
                cleanAggregatorCache();
                m_semAggregatorList.push_back( newAggreg );
            }
        }
        else {
            error = true;
            delete aggreg;
        }
    }
    else {
        error = true;
    }

    if( error ) {
        throw( Exception( "Invalid aggregator descriptor" ) );
    }
    return *newAggreg;
}

/** @brief If possible improve data structures to speed-up event evaluations.
 */
void QualExprSemanticAggregatorDB::consolidate( void ) throw( )      {
    cacheAggregators();
}

void QualExprSemanticAggregatorDB::evaluateEvent( const QualExprEvent& event ) throw( )      {
    cacheAggregators();
    unsigned int sem = event.m_semanticId;
    for( size_t index = 0; m_semAggregatorQuickList[ index ]; index++ ) {
        QualExprSemanticAggregator* semAggreg = m_semAggregatorQuickList[ index ];
        if( semAggreg->matchSemantic( sem ) ) {
            semAggreg->processEvent( event );
        }
    }
}

void QualExprSemanticAggregatorDB::display( const std::string& indent,
                                            std::stringstream& s ) const {
    s << "Registered:";
    for( size_t index = 0; index < m_semAggregatorList.size(); index++ ) {
        QualExprSemanticAggregator* semAggreg = m_semAggregatorList[ index ];
        s << indent << " - " << semAggreg->name() << ": ";
        semAggreg->display( "", s );
    }
}


/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */

/** @brief Quality expression evaluator constructor
    Build all the data structures able to provide an event semantic or an aggregator.
 */
/* Constructor */ QualExprEvaluator::QualExprEvaluator( void ) :
    m_semanticRootNamespace( "" ), m_aggregatorRootNamespace( "" ),
    m_semanticAggregatorDB( m_semanticRootNamespace, m_aggregatorRootNamespace ),
    m_computeNodeDB(), m_parser( *new QualExprEvaluatorParsingDriver( m_semanticAggregatorDB ) ) {
    QualExprAggregatorImmediate::registerToAggregatorNS( m_aggregatorRootNamespace );
    QualExprAggregatorTime::registerToAggregatorNS( m_aggregatorRootNamespace );
    QualExprAggregatorSize::registerToAggregatorNS( m_aggregatorRootNamespace );
    QualExprAggregatorBandwidth::registerToAggregatorNS( m_aggregatorRootNamespace );
}

/* Destructor */ QualExprEvaluator::~QualExprEvaluator( void ) {
    clearMeasures();
    delete &m_parser;
}

void QualExprEvaluator::clearMeasures( void ) {
    m_semanticAggregatorDB.clearMeasures();
    for( ComputeNodeDB_t::iterator ite = m_computeNodeDB.begin(); ite != m_computeNodeDB.end(); ite++ ) {
        delete ite->second;
    }
    m_computeNodeDB.clear();
}

/** @brief Parse a quality expresion and register the corresponding aggregators.
    The quality expression is parsed to extract the list of entries and register
    it with the entry ID.
    Throw an exception for a parsing error.
    @return the number of entries parsed and registered.
 */
size_t QualExprEvaluator::pushMeasure( const QualityExpressionEntry& qualExprEntry ) throw( QualExprEvaluator::Exception )        {
    const QualityExpression& expression = qualExprEntry.expression();
    QualityExpressionID_T    entryID    = qualExprEntry.id();
    size_t                   count      = 0;

    if( m_computeNodeDB.find( entryID ) != m_computeNodeDB.end() ) {
        return 0;
    }

    try {
        m_parser.parse( expression, expression );
        QualExprComputeNode* newAggregNode = m_parser.result();

        if( newAggregNode ) {
            count++;
        }

        m_computeNodeDB[ entryID ] = newAggregNode;
    }
    catch( QualExprEvaluatorParsingDriver::Exception e ) {
        std::stringstream s;
        s << "syntax error: " << e.what();
        throw( Exception( s.str() ) );
    }

    return count;
}

/** @brief If possible improve data structures to speed-up event evaluations.
 */
void QualExprEvaluator::consolidate( void ) throw( )      {
    m_semanticAggregatorDB.consolidate();
}

/** @brief Evaluate an event with all all registered semantic aggregators
 */
void QualExprEvaluator::evaluateEvent( const QualExprEvent& event ) throw( )      {
    m_semanticAggregatorDB.evaluateEvent( event );
}

/** @brief Display internal datastructures.
 */
void QualExprEvaluator::display( const std::string& indent,
                                 std::stringstream& s ) const {
    s << "Semantic aggregators:";
    std::string nindent = indent;
    nindent += "  ";
    s << nindent;
    m_semanticAggregatorDB.display( nindent, s );
}

/** @brief Retreive the value of a semantic aggregator by its ID.
    Throw an exception if no aggregator is found with the given ID.
 */
long64_t QualExprEvaluator::getLongCounter( QualityExpressionID_T id ) throw( QualExprEvaluator::Exception )          {
    ComputeNodeDB_t::iterator ite = m_computeNodeDB.find( id );
    if( ite == m_computeNodeDB.end() ) {
        throw( Exception( "Quality expression not found" ) );
    }

    QualExprComputeNodeOf<long64_t>* node = dynamic_cast<QualExprComputeNodeOf<long64_t>*>( ite->second );
    if( !node ) {
        throw( Exception( "Quality expression not found value of wrong type" ) );
    }

    long64_t value = node->eval();
    return value;
}

/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------------------------- */
}
