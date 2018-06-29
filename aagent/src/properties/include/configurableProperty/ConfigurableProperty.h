/**
   @file    ConfigurableProperty.h
   @ingroup ConfigurableProperty
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

#ifndef CONFIGURABLE_PROPERTY_H_
#define CONFIGURABLE_PROPERTY_H_

#include "PropertyConfiguration.h"
#include "Property.h"
#include "PropertyID.h"

/**
 * @defgroup ConfigurableProperties Configurable Properties
 * @ingroup Properties
 */


/**
   @class ConfigurableProperty
   @brief Template for properties configured at runtime with a severity configuration
   @ingroup ConfigurableProperties

   A configurable property is a only a template property for measurements
   decided at configured at runtime typically by a tuning plugin.
   It is configured with the class @ref PropertyConfiguration.

   @sa PropertyConfiguration
 */
class ConfigurableProperty : public Property {
public:
    /**
     * @brief Constructor
     */
    ConfigurableProperty( Context*                     context,
                          const PropertyConfiguration& config,
                          double                       threshold = 0 );
    /**
     * @brief Destructor
     */
    ~ConfigurableProperty( void ) {
    }

    virtual bool condition( void ) {
        return severity() > m_threshold;
    }

    virtual double confidence( void ) {
        return 1.0;
    }

    virtual double severity( void ) {
        return m_value.size() ? m_value[ 0 ] : 0.0;
    }

    virtual Gather_Required_Info_Type request_metrics( void );

    virtual void evaluate( void );

    virtual PropertyID id( void ) {
        return CONFIGURABLEPROPERTY;
    }

    virtual std::string name( void ) {
        return m_config->name();
    }

    virtual void print( void );

    virtual std::string info( void );

    virtual std::string toXMLExtra( void );

    virtual Property* clone( void );

private:        // Data structures
    const PropertyConfiguration* m_config;
    std::vector<double>          m_value;
    double                       m_threshold;

private:        // Serialization API
    friend class boost::serialization::access;

    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& m_config;
        ar& m_value;
    }
};

#endif /*CONFIGURABLE_PROPERTY_H_*/
