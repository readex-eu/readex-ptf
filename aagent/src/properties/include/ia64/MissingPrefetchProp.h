/**
   @file    MissingPrefetchProp.h
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

#ifndef MISSINGPREFETCHPROP_H_
#define MISSINGPREFETCHPROP_H_

#include "Property.h"
#include "PropertyID.h"


class MissingPrefetchProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& prefetch;
        ar& stalls;
        ar& phaseCycles;
        ar& phaseContext;
        ar& threshold;
    }

private:
    INT64    prefetch;
    INT64    stalls;
    INT64    phaseCycles;
    Context* phaseContext;
    double   threshold;

    MissingPrefetchProp( void ); // should only be used for serialization

public:
    MissingPrefetchProp( Context* ct, Context* phaseCt, double threshold = IA64_PROPS_THRESHOLD::MissingPrefetchProp ) :
        Property( ct ), threshold( threshold ), phaseCycles( 0 ), prefetch( 0 ), phaseContext( phaseCt ), stalls( 0 ) {
    }

    ~MissingPrefetchProp( void ) {
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

#endif /*MISSINGPREFETCHPROP_H_*/
