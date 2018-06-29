/**
   @file    StallCycles.h
   @ingroup Itanium2Properites
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

#ifndef STALLCYCLES_H_
#define STALLCYCLES_H_

#include "Property.h"
#include "PropertyID.h"

class StallCyclesProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& threshold;
        ar& phaseCycles;
        ar& stallCycles;
        ar& phaseContext;
        ar& m;
        ar& phaseInclOvhdCycles;
        ar& exclOvhdCycles;
        ar& ovhdPercent;
    }
private:
    double threshold;
    INT64 phaseCycles
    INT64    stallCycles;
    Context* phaseContext;
    Metric   m;
    INT64    phaseInclOvhdCycles;
    INT64    exclOvhdCycles;
    double   ovhdPercent;

    StallCyclesProp( void ); // should only be used for serialization

public:
    StallCyclesProp( Context* ct, Context* phaseCt, Metric m_p, double threshold = IA64_PROPS_THRESHOLD::StallCyclesProp ) :
        Property( ct ), phaseCycles( 0 ), stallCycles( 0 ), m( m_p ), phaseContext( phaseCt ), threshold( threshold ) {
    }

    ~StallCyclesProp( void ) {
    }

    bool condition( void ) const;

    double confidence( void ) const;

    double severity( void ) const;

    Gather_Required_Info_Type request_metrics( void );

    void evaluate( void );

    PropertyID id( void );

    std::string subId( void );

    std::string name( void );

    void print( void );

    std::string info( void ) {
        return "";
    }

    Metric get_metric( void );

    Context* get_phaseContext( void );

    Property* clone( void );
};

#endif /*STALLCYCLES_H_*/
