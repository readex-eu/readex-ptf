/**
   @file    L2MissesProp.h
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

#ifndef L2MISSPROP_H_
#define L2MISSPROP_H_

#include "Property.h"
#include "PropertyID.h"


class L2MissProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& phaseCycles;
        ar& stallCycles;
        ar& L1Misses;
        ar& L2Misses;
        ar& L3Misses;
        ar& L2StallCycles;
        ar& importance;
        ar& threshold;
        ar& phaseContext;
    }

private:
    INT64    phaseCycles;
    INT64    stallCycles;
    INT64    L1Misses;
    INT64    L2Misses;
    INT64    L3Misses;
    INT64    L2StallCycles;
    double   importance;
    double   threshold;
    Context* phaseContext;

    L2MissProp( void ); // should only be used for serialization

public:
    L2MissProp( Context* ct, Context* phaseCt, double threshold = IA64_PROPS_THRESHOLD::L2MissProp ) :
        Property( ct ), threshold( threshold ), phaseCycles( 0 ), stallCycles( 0 ), L1Misses( 0 ),
        L2Misses( 0 ), L3Misses( 0 ), L2StallCycles( 0 ), importance( 0.0 ), phaseContext( phaseCt ) {
    }

    ~L2MissProp( void ) {
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

#endif /*L2MISSPROP_H_*/
