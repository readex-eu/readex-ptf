/**
   @file    TLBMissRate.h
   @ingroup GenericProperties
   @brief   Generic TLB miss rate property
   @author  Yury Oleynik
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


#ifndef TLBMISSRATE_H_
#define TLBMISSRATE_H_

#include "Property.h"
#include "PropertyID.h"

class TLBMissRate : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& availableProp;
        ar& available;
        ar& TLBAccess;
        ar& TLBMisses;
        ar& importance;
        ar& threshold;
        ar& phaseContext;
    }
private:
    bool availableProp;
    bool available;

    INT64 TLBAccess;
    INT64 TLBMisses;

    double   importance;
    double   threshold;
    Context* phaseContext;

    TLBMissRate( void ); // should only be used for serialization

public:
    TLBMissRate( Context* ct, Context* phaseCt, double threshold = CPPROPS_PROPS_THRESHOLD::TLBMissRate ) :
        Property( ct ), TLBAccess( 0 ), TLBMisses( 0 ), importance( 0.0 ),
        phaseContext( phaseCt ), availableProp( true ), threshold( threshold ) {
    }

    ~TLBMissRate( void );

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

#endif /*TLBMISSPROP_H_*/
