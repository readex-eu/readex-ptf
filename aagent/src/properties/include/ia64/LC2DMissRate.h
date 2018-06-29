/**
   @file    ia64/LC2DMissRate.h
   @ingroup Itanium2Properites
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

    LC2DMissRateProp(); // should only be used for serialization

public:
    LC2DMissRateProp( double threshold = IA64_PROPS_THRESHOLD::LC2DMissRateProp ) :
        Property(), nr_mem_ref( 0 ), nr_lc2_cache_miss( 0 ), execution_time( 0 ), threshold( threshold ) {
    }

    LC2DMissRateProp( Context* c, double threshold = IA64_PROPS_THRESHOLD::LC2DMissRateProp ) :
        Property( c ), nr_mem_ref( 0 ), nr_lc2_cache_miss( 0 ), execution_time( 0 ), threshold( threshold ) {
    }

    ~LC2DMissRateProp() {
    }

    bool condition() const;

    double confidence() const;

    double severity() const;

    Gather_Required_Info_Type request_metrics();

    std::string name();

    PropertyID id();

    std::string info() {
        return "";
    }

    void evaluate();

    Property* clone();
};

#endif //LC2DMISSRATE_H_INCLUDED
