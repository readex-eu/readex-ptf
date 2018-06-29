/**
   @file    PropertyConfiguration.h
   @ingroup AnalysisAgent
   @brief   Header of Configurable Properties
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

#ifndef PROPERTYCONFIGURATION_H_
#define PROPERTYCONFIGURATION_H_
#include <map>

#include "quality-expressions/QualityExpressions.h"
#include "global.h"

/**
   @class MetricConfiguration
   @brief Associate a metric ID with a quality expression
   @ingroup AnalysisAgent

   A metric configuration entry is used in the configure
   a new metric inside the monitor with a quality expression.

   @sa QualityExpression
   @sa Metric
 */
class MetricConfiguration {
public:         // Class Definitions
    /**
     *  Constructor
     */
    MetricConfiguration( Metric mid, const QualityExpression& expression ) : m_mid( mid ), m_expression( expression ) {
    }

    /**
     *  Destructor
     */
    ~MetricConfiguration( void ) {
    }

public:         // Access API
    Metric mid( void ) const {
        return m_mid;
    }

    QualityExpression& expression( void ) {
        return m_expression;
    }

    const QualityExpression& expression( void )  const {
        return m_expression;
    }

    void setExpression( const QualityExpression& expression ) {
        m_expression = expression;
    }

    /**
     * Display debugging information about the object.
     */
    void display( const std::string& indent,
                  std::stringstream& s ) const;

private:                                   // Data structures
    Metric            m_mid;               //!< Unique ID in the range PSC_CONFIGURABLE_MEASURE.
    QualityExpression m_expression;        //!< Quality expression used for the evaluation.
};

/**
   @class PropertyConfiguration
   @brief Hold a property configuration for a @ref ConfigurableProperty
   @ingroup AnalysisAgent

   The configuration associates a severity expression with a name.
   A property configuration is typically created during the definition
   of a new experiment based on the quality expressions expressions needed
   by the plugin.

   @sa ConfigurableProperty
   @sa QualityExpression
 */
class PropertyConfiguration {
public:         // Class Definitions
    /**
     *  Constructor
     */
    PropertyConfiguration( const std::string&       name,
                           const QualityExpression& severityExpression ) : m_name( name ), m_severityExpression( severityExpression ) {
    }

    /**
     *  Destructor
     */
    ~PropertyConfiguration( void ) {
    }


public:         // Access API
    /**
     * Property Name
     */
    const std::string& name( void ) const {
        return m_name;
    }

    /**
     * Severity expression
     */
    QualityExpression& severityExpression( void ) {
        return m_severityExpression;
    }

    /**
     * Const severity expression
     */
    const QualityExpression& severityExpression( void )  const {
        return m_severityExpression;
    }

    /**
     * List of induced metrics
     */
    std::vector<Metric>& metricList( void ) {
        return m_metricList;
    }

    /**
     * Const list of induced metrics
     */
    const std::vector<Metric>& metricList( void ) const {
        return m_metricList;
    }

    /**
     * Set the severity expression
     */
    void setSeverityExpression( const QualityExpression& expression ) {
        m_severityExpression = expression;
    }

    /**
     * Display debugging information about the object.
     */
    void display( const std::string& indent,
                  std::stringstream& s ) const;

private:                                      // Data structures
    std::string         m_name;               //!< Name of the configuration. Key for the identification of a Configurable Property.
    QualityExpression   m_severityExpression; //!< Severity expression built using the quality expression syntax @ref QualityExpression.
    std::vector<Metric> m_metricList;         //!< All metrics generated from the severity expression. By convention, the severity is associated to the first one in the list.

private:                                      // Serialization API
    friend class boost::serialization::access;
    /**
     *  Constructor
     */
    PropertyConfiguration() : m_name( "" ), m_severityExpression( "" ) {
    }

    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& m_name;
        ar& m_severityExpression;
        ar& m_metricList;
    }
};

/**
 * Standard debug function
 */
std::ostream& operator<<( std::ostream&                s,
                          const PropertyConfiguration& propConf );

inline void PropertyConfiguration::display( const std::string& indent,
                                            std::stringstream& s ) const {
    s << "name=" << m_name;
    s << indent << "severity expression=" << m_severityExpression;
    s << indent << "Metric list:";
    for( size_t index = 0; index < m_metricList.size(); index++ ) {
        if( index ) {
            s << ',';
        }
        s << m_metricList[ index ];
    }
}

inline std::ostream& operator<<( std::ostream& o, const PropertyConfiguration& propConf ) {
    std::stringstream s;
    propConf.display( ";", s );
    o << '(';
    return o << s.str() << ')';
}

/**
   @class PropertyConfigurationManager
   @brief Manage all property configurations
   @ingroup AnalysisAgent

   The property configuration database is in charge of the storage and binding
   of the property configurations withs metric configurations.

   @sa QualityExpressionDB
   @sa PropertyConfiguration
 */
class PropertyConfigurationManager {
private:        // Data structure Declarations
    typedef std::map<std::string, PropertyConfiguration> PropertyConfigurationMapByName_T;
    typedef std::map<QualityExpressionID_T, Metric>      MetricMapByQualityExpression_T;

/**
   @class Exception
   @brief Hold an exception with a description.
 */
    class Exception : public std::exception, public std::string {
public:
        /**
         *  Constructor
         */
        Exception( const char* message ) throw( ) : std::string( message ) {
        }

        /**
         *  Destructor
         */
        ~Exception( void ) throw( ) {
        }
        virtual const char* what() const throw( )                     {
            return c_str();
        }
    };

public:         // Class Definitions
    /**
     *  Constructor
     */
    PropertyConfigurationManager( QualityExpressionDB& qualityExpressionsDB );

    /**
     *  Destructor
     */
    ~PropertyConfigurationManager( void );

    /**
     * Register the property configuration.
     */
    const PropertyConfiguration& registerPropertyConfiguration( const PropertyConfiguration& propConfig ) throw( Exception );

    /**
     * Fetch the quality expression associated to the ID.
     */
    const PropertyConfiguration& getPropertyConfigurationByName( const std::string& name ) throw( Exception );

    /**
     * Return the list of metric configurations generated.
     */
    std::vector<MetricConfiguration*>& metricConfigurationList( void );

    /**
     * Reset the database.
     */
    void clear( void );

    /**
     * Display debugging information about the object.
     */
    void display( const std::string& indent,
                  std::stringstream& s ) const;

private:                                                           // Data structures

    QualityExpressionDB&              m_qualityExpressionsDB;      //!< Store and manage all quality expression measures. Must be provided.
    Metric                            m_metricAllocationCursor;    //!< Start at PSC_CONFIGURABLE_MEASURE and record the next metric available.
    PropertyConfigurationMapByName_T  m_propertyConfigurationList; //!< List of property configurations.
    MetricMapByQualityExpression_T    m_metricsByQualExpr;         //!< Index metrics with associated to quality expressions.
    std::vector<MetricConfiguration*> m_metricConfigList;          //!< List of all metric configurations.
};

#endif /* CONFIGURABLEPROPERTY_H_ */
