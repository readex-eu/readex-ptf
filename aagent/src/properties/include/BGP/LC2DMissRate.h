/**
   @file    BGP/LC2DMissRate.h
   @ingroup BGPProperties
   @brief   Itanium2 specific property
   @author  Edmond Kereku
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

/**
 * @defgroup BGPProperties BGP Properties
 * @ingroup Properties
 */

#ifndef LC2DMISSRATE_H_INCLUDED
#define LC2DMISSRATE_H_INCLUDED

#include "global.h"
#include "Property.h"
#include "PropertyID.h"


//If the miss rate is more than xx % of memory references
class LC2DMissRateProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& threshold;
        ar& nr_mem_ref;
        ar& execution_time;
        ar& nr_lc2_cache_miss;
    }
private:
    double threshold;
    INT64  nr_mem_ref;
    INT64  execution_time;
    INT64  nr_lc2_cache_miss;

    LC2DMissRateProp( void ); // should only be used for serialization

public:
    LC2DMissRateProp( double threshold = BGP_PROPS_THRESHOLD::LC2DMissRateProp ) :
        Property(), nr_mem_ref( 0 ), nr_lc2_cache_miss( 0 ), execution_time( 0 ), threshold( threshold ) {
    }

    LC2DMissRateProp( Context* c, double threshold = BGP_PROPS_THRESHOLD::LC2DMissRateProp ) :
        Property( c ), nr_mem_ref( 0 ), nr_lc2_cache_miss( 0 ), execution_time( 0 ), threshold( threshold ) {
    }

    ~LC2DMissRateProp( void ) {
    }

    bool condition( void ) const;

    double confidence( void ) const;

    double severity( void ) const;

    Gather_Required_Info_Type request_metrics( void );

    std::string name( void );

    PropertyID id( void );

    std::string info( void ) {
        return "";
    }

    Prop_List next( void );

    void evaluate( void );

    Property* clone( void );

    std::string toXMLExtra( void );
};


#endif //LC2DMISSRATE_H_INCLUDED
