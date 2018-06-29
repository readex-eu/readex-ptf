/**
   @file    ConfigurableProperty.cc
   @ingroup ConfigurableProperties
   @brief   Classes managing properties configured at runtime with a severity configuration
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

#include "PropertyConfiguration.h"
#include "psc_errmsg.h"

/* --------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------- */

void MetricConfiguration::display( const std::string& indent, std::stringstream& s ) const {
    s << '(';
    m_expression.display( indent, s );
    s << ':' << EventList[ m_mid ].EventName << ')';
}

/* --------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------- */

/* Constructor */ PropertyConfigurationManager::PropertyConfigurationManager( QualityExpressionDB& qualityExpressionsDB ) :
    m_qualityExpressionsDB( qualityExpressionsDB ), m_metricAllocationCursor( PSC_CONFIGURABLE_MEASURE ), m_metricsByQualExpr(), m_metricConfigList() {
}

/* Destructor */ PropertyConfigurationManager::~PropertyConfigurationManager( void ) {
    clear();
}

void PropertyConfigurationManager::clear( void ) {
    m_metricsByQualExpr.clear();
    m_propertyConfigurationList.clear();
    for( size_t index = 0; index < m_metricConfigList.size(); index++ ) {
        delete m_metricConfigList[ index ];
    }
    m_metricConfigList.clear();
    m_metricAllocationCursor = PSC_CONFIGURABLE_MEASURE;
}

/**
   @brief Register the property configuration.

   This methods register a property configuration in the database.
   The registration is made in the following way :
     1- we filter out already existing property configuration,
     2- we register in a local database all the quality expression
        entries (m_qualityExpressionsDB)
     3- from the list of unique quality expression ids provided by
        the local database we allocate a unique Metric in the range
        of the "PSC_CONFIGURABLE_MEASURE".
     4- we store back the list of metrics in the property configuration

     @param propConfig The property configuration to register.
     @return           The property configuration registered.
 */
const PropertyConfiguration& PropertyConfigurationManager::registerPropertyConfiguration( const PropertyConfiguration& propConfig ) throw( PropertyConfigurationManager::Exception )                              {
    PropertyConfigurationMapByName_T::value_type                propConfigPair( propConfig.name(), propConfig );
    std::pair<PropertyConfigurationMapByName_T::iterator, bool> newElem       = m_propertyConfigurationList.insert( propConfigPair );
    PropertyConfiguration&                                      newPropConfig = newElem.first->second;
    if( newElem.second ) { // New element
        std::vector<Metric>& newPropConfigMetricList = newPropConfig.metricList();

        std::vector<QualityExpressionID_T> p_idList;
        m_qualityExpressionsDB.registerQualityExpression( propConfig.severityExpression(), p_idList );

        for( size_t index = 0; index < p_idList.size(); index++ ) {
            Metric                                   nextMetric;
            QualityExpressionID_T                    qualExprId = p_idList[ index ];
            MetricMapByQualityExpression_T::iterator metricItem = m_metricsByQualExpr.find( qualExprId );
            if( metricItem == m_metricsByQualExpr.end() ) {
                // Allocation of a new metric if there are still metrics available.
                if( m_metricAllocationCursor == PSC_CONFIGURABLE_MEASURE_LAST ) {
                    throw( Exception( "PropertyConfigurationManager::registerQualityExpressionEntry: no more metric available for configurable properties" ) );
                }
                nextMetric                        = m_metricAllocationCursor;
                m_metricAllocationCursor          = ( Metric )( m_metricAllocationCursor + 1 );
                m_metricsByQualExpr[ qualExprId ] = nextMetric;

                // Create the metric config
                MetricConfiguration* metricConfig = new MetricConfiguration( nextMetric, m_qualityExpressionsDB.getQualityExpressionById( qualExprId ) );
                m_metricConfigList.push_back( metricConfig );
            }
            else {
                nextMetric = metricItem->second;
            }
            newPropConfigMetricList.push_back( nextMetric );
        }
    }
    else {
        if( propConfig.severityExpression() != newPropConfig.severityExpression() ) {
            throw( Exception( "PropertyConfigurationManager::registerPropertyConfiguration: inconsistent property configurations with the same name" ) );
        }
    }
    return newPropConfig;
}

const PropertyConfiguration& PropertyConfigurationManager::getPropertyConfigurationByName( const std::string& name ) throw( PropertyConfigurationManager::Exception )                              {
    PropertyConfigurationMapByName_T::iterator dbItem = m_propertyConfigurationList.find( name );
    if( dbItem == m_propertyConfigurationList.end() ) {
        throw( Exception( "PropertyConfigurationManager::getPropertyConfigurationByName: not found" ) );
    }
    return dbItem->second;
}

std::vector<MetricConfiguration*>& PropertyConfigurationManager::metricConfigurationList( void ) {
    return m_metricConfigList;
}

void PropertyConfigurationManager::display( const std::string& indent, std::stringstream& s ) const {
    std::string nindent = indent;
    nindent += "  ";
    s << "Quality expression DB :";
    s << indent;
    m_qualityExpressionsDB.display( nindent, s );
    s << indent << "Next metric :" << EventList[ m_metricAllocationCursor ].EventName;
    s << indent << "Property configuration entries:";
    for( PropertyConfigurationMapByName_T::const_iterator dbItem = m_propertyConfigurationList.begin(); dbItem != m_propertyConfigurationList.end(); dbItem++ ) {
        s << nindent;
        dbItem->second.display( nindent, s );
    }
    s << nindent << "Metric configuration list:";
    for( size_t index = 0; index < m_metricConfigList.size(); index++ ) {
        s << nindent;
        m_metricConfigList[ index ]->display( nindent, s );
    }
}
