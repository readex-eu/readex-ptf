/**
   @file    HighInclOverheadProp.h
   @ingroup OverheadProperties
   @brief   Property identifies a region with high MRI overhead
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


#ifndef HIGHINCLOVERHEADPROP_H_
#define HIGHINCLOVERHEADPROP_H_

#include <iostream>
#include "Property.h"
#include "PropertyID.h"

class HighInclOverheadProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& ovhdCycles;
        ar& exclOvhdCycles;
        ar& ovhdPercent;
        ar& phaseCycles;
        ar& cycles;
        ar& context;
        ar& phaseContext;
        ar& threshold;
    }
private:
    INT64    ovhdCycles;
    INT64    exclOvhdCycles;
    double   ovhdPercent;
    INT64    phaseCycles;
    INT64    cycles;
    Context* context;
    Context* phaseContext;
    double   threshold;

    HighInclOverheadProp( void ); // should only be used for serialization

public:
    HighInclOverheadProp( Context* ct, Context* phaseCt, double threshold = OVERHEAD_PROPS_THRESHOLD::HighInclOverheadProp ) :
        Property( ct ), context( ct ), phaseContext( phaseCt ), threshold( threshold ),
        ovhdCycles( 0 ), exclOvhdCycles( 0 ), ovhdPercent( 0.0 ) {
    }

    ~HighInclOverheadProp( void ) {
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

    Context* get_phaseContext( void );

    Property* clone( void );
};

#endif /*HIGHINCLOVERHEADPROP_H_*/
