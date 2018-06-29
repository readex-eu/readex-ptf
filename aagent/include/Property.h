/**
   @file    Property.h
   @ingroup AnalysisAgent
   @brief   Performance property definition
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2015, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

/**
 * @defgroup Properties Properties
 * @ingroup AnalysisAgent
 */

#ifndef PROPERTY_H_
#define PROPERTY_H_

#include "Context.h"
#include "DataProvider.h"
#include "PerformanceDataBase.h"
#include "application.h"
#include "PropertyID.h"
#include "PropertyThresholdConfig.h"
#include "PropertyPurpose.h"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/list.hpp>

typedef std::list<Property*> Prop_List;




/**
 * Represents a performance property of the investigated application. Properties are built from metrics and are only
 * valid within a specific context.
 */
class Property {
    friend class boost::serialization::access;

public:
    Property() : context( NULL ), phaseContext( NULL ), purpose( PSC_PROPERTY_PURPOSE_ANALYSIS ) {
    }

    Property( Context* c ) : context( c ), phaseContext( NULL ), purpose( PSC_PROPERTY_PURPOSE_ANALYSIS ) {
    }

    virtual ~Property() {
        delete context;
        delete phaseContext;
    }

    /** Indicates whether the property has been detected in the application. */
    virtual bool condition() const = 0;

    /** Indicates the confidence we have in this specific property of the application [0, 1]. */
    virtual double confidence() const = 0;

    /** Indicates the severity of the property. This value is specific to the type of property. */
    virtual double severity() const = 0;

    /**
     * Internal property type identification
     */
    virtual PropertyID id() = 0;

    /**
     * Internal sub-property type identification
     */
    virtual std::string subId() {
        return "0";
    }

    /**
     * Textual description of the property
     */
    virtual std::string name() = 0;

    /**
     * Get the property's context
     */
    Context* get_context() const;

    /**
     * Set the property's context
     */
    void set_context( Context* context );

    /**
     * Gets the region for which the property is determined.
     */
    Region* get_region() const;

    /**
     * Gets the MPI rank of the process in which the property is determined.
     */
    int get_rank() const;

    /**
     * Gets the OMP thread number in which the property is determined.
     */
    int get_thread() const;

    void set_PhaseContext( Context* phase );

    void add_ScenarioId( int scId );

    std::list<int>* get_ScenarioIdList();

    void set_Purpose( PropertyPurpose purps );

    bool isRtsBased(){return context->isRtsBased();}

    PropertyPurpose get_Purpose();

    /**
     * Requests all metrics from the data provider that are required to evaluate this property.
     */
    virtual Gather_Required_Info_Type request_metrics() = 0;

    /**
     * Evaluates the properties. These assumes that the required metrics have been stored in the database.
     */
    virtual void evaluate() = 0;

    virtual Property* clone() = 0;

    virtual Prop_List next() {
        Prop_List returnList;
        return returnList;
    }

    virtual std::string info() = 0;



    /**
     * Serializes this property to XML.
     */
    std::string toXML();

    /**
     * Use this to serialize the property to XML while sending from aagent to the frontend.
     */
    std::string toXMLSerialize();

    /**
     * Property-specific extra XML.
     */
    virtual std::string toXMLExtra() {
        return "";
    }

protected:
    Context*        context;
    Context*        phaseContext;
    std::list<int>  scenarioIds;
    PropertyPurpose purpose;

private:
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& context;
        ar& phaseContext;
        ar& scenarioIds;
        ar& purpose;
    }
};

#endif /* PROPERTY_H_ */
