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

#include "ConfigurableProperty.h"
#include "psc_errmsg.h"

/* --------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------- */

/* Constructor */ ConfigurableProperty::ConfigurableProperty( Context* context, const PropertyConfiguration& config, double threshold ) :
    Property( context ), m_value(), m_threshold( threshold ), m_config( NULL ) {
//
// Records the property configuration in the global database provided by the DataProvider class
//   - REMOVED DURING SCORE-P INTEGRATION
//
//
//  PropertyConfigurationManager * prop_config_manager = dp->getPropertyConfigurationManager();
//  m_config = &prop_config_manager->registerPropertyConfiguration(config);
//
//  if ( active_dbgLevel( PSC_SELECTIVE_DEBUG_LEVEL( QualityExpressions ) ) )
//  {
//    std::stringstream s; s << "Registered Property Configuration:["; m_config->display("][", s); s<<']'<< std::endl;
//    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( QualityExpressions ), s.str().c_str() );
//  }
}

//bool ConfigurableProperty::condition(void)
//{
//    if ( severity() >= 0 ) {
//        return true;
//    }
//    else {
//        return false;
//    }
//}


void ConfigurableProperty::print( void ) {
    std::stringstream configStr;
    m_config->display( "", configStr );
    std::cout << "Property: Configurable" <<
    "  configuration: "  << configStr.str() <<
    "  Process " << context->getRank() <<
    "  Thread " <<  context->getThread() << std::endl;
}

void ConfigurableProperty::evaluate( void ) {
    const std::vector<Metric>& metricList = m_config->metricList();
    if( !metricList.size() ) {
        psc_errmsg( "ConfigurableProperty:: metric not allocated\n" );
        return;
    }

    for( size_t index = 0; index < metricList.size(); index++ ) {
        Metric metric   = metricList[ index ];
        INT64  rawValue = pdb->get( context, metric );
        assert( sizeof( double ) == sizeof( INT64 ) );
        double realValue = rawValue;
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( QualityExpressions ), "Got Configurable Property Metric [%d]: %g (%" PRIu64 ")\n", metric, realValue, rawValue );
        m_value.push_back( ( double )realValue );
    }
}

Gather_Required_Info_Type ConfigurableProperty::request_metrics( void ) {
    const std::vector<Metric>& metricList = m_config->metricList();
    if( !metricList.size() ) {
        psc_abort( "ConfigurableProperty:: metrics not allocated\n" );
    }

    for( size_t index = 0; index < metricList.size(); index++ ) {
        Metric metric = metricList[ index ];
        pdb->request( context, metric );
    }

    return ALL_INFO_GATHERED;
}

std::string ConfigurableProperty::info( void ) {
    std::stringstream          stream;
    const std::vector<Metric>& metricList = m_config->metricList();

    if( m_value.size() == metricList.size() ) {
        for( size_t index = 0; index < metricList.size(); index++ ) {
            Metric metric    = metricList[ index ];
            char   valueName = 'A' + metric - PSC_CONFIGURABLE_MEASURE;
            double realValue = m_value[ index ];
            stream << "\t    CONFIGURABLE_MEASURE_" << valueName << "= "   << realValue << "  ";
        }
    }
    else {
        stream << "\t    Metric not evaluated ";
    }

    return stream.str();
}

std::string ConfigurableProperty::toXMLExtra( void ) {
    const std::vector<Metric>& metricList = m_config->metricList();
    std::stringstream          stream;
    stream << "\t\t  <expression>" << m_config->severityExpression() << "</expression>"  << std::endl;
    if( m_value.size() == metricList.size() ) {
        for( size_t index = 0; index < metricList.size(); index++ ) {
            char   valueName = 'A' + index;
            Metric metric    = metricList[ index ];
            double realValue = m_value[ index ];
            stream << "\t\t    <value" << valueName << ">"        << realValue     << "</value" << valueName << ">"       << std::endl;
        }
    }
    return stream.str();
}

Property* ConfigurableProperty::clone( void ) {
    return new ConfigurableProperty( context, *m_config, m_threshold );
}
