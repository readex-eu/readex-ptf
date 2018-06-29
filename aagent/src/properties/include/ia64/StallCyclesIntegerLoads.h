/**
   @file    StallCyclesIntegerLoads.h
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

#ifndef STALLCYCLESINTEGERLOADS_H_
#define STALLCYCLESINTEGERLOADS_H_

#include "Property.h"
#include "PropertyID.h"

class StallCyclesIntegerLoadsProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& threshold;
        ar& phaseCycles;
        ar& stallCyclesGRALL;
        ar& stallCyclesGRGR;
        ar& phaseContext;
    }

private:
    double threshold;
    INT64 phaseCycles
    INT64    stallCyclesGRALL;
    INT64    stallCyclesGRGR;
    Context* phaseContext;

public:
    StallCyclesIntegerLoadsProp( Context* ct, Context* phaseCt, double threshold =
                                     IA64_PROPS_THRESHOLD::StallCyclesIntegerLoadsProp ) :
        Property( ct ), phaseCycles( 0 ), stallCyclesGRALL( 0 ),
        stallCyclesGRGR( 0 ), phaseContext( phaseCt ), threshold( threshold ) {
    }

    ~StallCyclesIntegerLoadsProp( void ) {
    }

    bool condition( void ) const;

    double confidence( void ) const;

    double severity( void ) const;

    Gather_Required_Info_Type request_metrics( void );

    void evaluate( void );

    std::string name( void );

    PropertyID id( void );

    void print( void );

    std::string info( void ) {
        return "";
    }

    Context* get_phaseContext( void );

    Property* clone( void );
};

#endif /*STALLCYCLESINTEGERLOADS_H_*/
