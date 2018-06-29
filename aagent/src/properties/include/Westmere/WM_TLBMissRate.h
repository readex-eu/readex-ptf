/**
   @file    WM_TLBMissRate.h
   @ingroup WestmereProperties
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

#ifndef WM_TLBMissRate_H_
#define WM_TLBMissRate_H_

#include "Property.h"
#include "PropertyID.h"

class WM_TLBMissRate : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& availableProp;
        ar& available;
        ar& TLBAccess;
        ar& TLBMisses;
        ar& phaseCycles;
        ar& cycles;
        ar& importance;
        ar& threshold;
        ar& phaseContext;
    }
private:
    bool availableProp;
    bool available;

    INT64 TLBAccess;
    INT64 TLBMisses;
    INT64 phaseCycles;
    INT64 cycles;

    double   importance;
    double   threshold;
    Context* phaseContext;

    WM_TLBMissRate( void ); // should only be used for serialization

public:
    WM_TLBMissRate( Context* ct,
                    Context* phaseCt,
                    double   threshold = WESTMERE_PROPS_THRESHOLD::WM_TLBMissRate );

    virtual ~WM_TLBMissRate( void );

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
