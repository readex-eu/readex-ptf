/**
   @file    BadCodeProp.h
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

#ifndef BADCODEPROP_H_
#define BADCODEPROP_H_

#include "Property.h"
#include "PropertyID.h"

class BadCodeProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& issues;
        ar& theoreticalIssues;
        ar& r1;
        ar& phaseCycles;
        ar& cycles;
        ar& phaseContext;
        ar& threshold;
    }
private:
    INT64    issues;
    INT64    theoreticalIssues;
    double   r1;
    INT64    phaseCycles;
    INT64    cycles;
    Context* phaseContext;
    double   threshold;

    BadCodeProp( void ); // should only be used for serialization

public:
    BadCodeProp( Context* ct, Context* phaseCt, double threshold = IA64_PROPS_THRESHOLD::BadCodeProp ) :
        Property( ct ), issues( 0 ), theoreticalIssues( 0 ), phaseContext( phaseCt ),
        r1( 0.0 ), cycles( 0 ), phaseCycles( 0 ), threshold( threshold ) {
    }

    ~BadCodeProp( void ) {
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

#endif /*BADCODEPROP_H_*/
