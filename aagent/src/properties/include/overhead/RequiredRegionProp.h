/**
   @file    RequiredRegionProp.h
   @ingroup OverheadProperties
   @brief   Itanium2 specific property
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#ifndef REQUIREDREGIONPROP_H_
#define REQUIREDREGIONPROP_H_

#include <iostream>
#include "Property.h"
#include "PropertyID.h"

class RequiredRegionProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& context;
        ar& phcontext;
        ar& requiredRegions;
        ar& threshold;
    }

private:
    Context*    context;
    Context*    phcontext;
    std::string requiredRegions;
    double      threshold;

    RequiredRegionProp( void ); // should only be used for serialization

public:
    RequiredRegionProp( Context* ct, Context* phaseCt, double threshold = OVERHEAD_PROPS_THRESHOLD::RequiredRegionProp ) :
        Property( ct ), context( ct ), phcontext( phaseCt ), threshold( threshold ) {
    }

    ~RequiredRegionProp( void ) {
    }

    bool condition( void ) const;

    double confidence( void ) const;

    double severity( void ) const;

    Gather_Required_Info_Type request_metrics( void );

    void evaluate( void );

    PropertyID id( void );

    std::string name( void );

    void print( void );

    std::string info( void );

    /**
     * @brief Additional XML property information
     */
    std::string toXMLExtra( void );

    void add_region( std::string rgn );

    Context* get_phaseContext( void );

    Property* clone( void );
};

#endif /*OVERHEADPROP_H_*/
