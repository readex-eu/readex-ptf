/**
   @file    AdvanceSpeculativeLoadProp.h
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

/**
 * @defgroup Itanium2Properites Itanium2 Properties
 * @ingroup Properties
 */

#ifndef ADVANCESPECULATIVELOADPROP_H_
#define ADVANCESPECULATIVELOADPROP_H_

#include "Property.h"
#include "PropertyID.h"


class AdvanceSpeculativeLoadProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& storesALAT;
        ar& instrInBody;
        ar& totalInstructionsIssued;
        ar& totalStoresALAT;
        ar& phaseCycles;
        ar& phaseContext;
        ar& threshold;
    }

private:
    INT64    storesALAT;
    INT64    instrInBody;
    INT64    totalInstructionsIssued;
    INT64    totalStoresALAT;
    INT64    phaseCycles;
    Context* phaseContext;
    double   threshold;

    AdvanceSpeculativeLoadProp( void ); // should only be used for serialization

public:
    AdvanceSpeculativeLoadProp( Context* ct, Context* phaseCt, double threshold =
                                    IA64_PROPS_THRESHOLD::AdvanceSpeculativeLoadProp ) :
        Property( ct ), threshold( threshold ), storesALAT( 0 ), instrInBody( 0 ), totalInstructionsIssued( 0 ), totalStoresALAT( 0 ), phaseCycles(
            0 ), phaseContext( phaseCt ) {
    }

    ~AdvanceSpeculativeLoadProp( void ) {
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

#endif /*ADVANCESPECULATIVELOADPROP_H_*/
