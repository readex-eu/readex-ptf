/**
   @file    ConvIntFloatProp.h
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

#ifndef CONVINTFLOATPROP_H_
#define CONVINTFLOATPROP_H_

#include "Property.h"
#include "PropertyID.h"

class ConvIntFloatProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& convInstr;
        ar& instrInBody;
        ar& totalInstructionsIssued;
        ar& phaseCycles;
        ar& phaseContext;
        ar& threshold;
    }

private:
    INT64    convInstr;
    INT64    instrInBody;
    INT64    totalInstructionsIssued;
    INT64    phaseCycles;
    Context* phaseContext;
    double   threshold;

public:
    ConvIntFloatProp( Context* ct, Context* phaseCt, double threshold = IA64_PROPS_THRESHOLD::ConvIntFloatProp ) :
        Property( ct ), convInstr( 0 ), instrInBody( 0 ), totalInstructionsIssued( 0 ),
        phaseCycles( 0 ), phaseContext( phaseCt ), threshold( threshold ) {
    }

    ~ConvIntFloatProp( void ) {
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

#endif /*CONVINTFLOATPROP_H_*/
