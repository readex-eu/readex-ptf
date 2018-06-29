/**
   @file    OverheadProp.h
   @ingroup OverheadProperties
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

#ifndef OVERHEADPROP_H_
#define OVERHEADPROP_H_

#include <iostream>
#include "Property.h"
#include "PropertyID.h"

class OverheadProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& issues;
        ar& theoreticalIssues;
        ar& Overhead;
        ar& phaseCycles;
        ar& cycles;
        ar& context;
        ar& requiredRegions;
        ar& threshold;
    }

private:
    INT64       issues;
    INT64       theoreticalIssues;
    double      Overhead;
    INT64       phaseCycles;
    INT64       cycles;
    Context*    context;
    std::string requiredRegions;
    double      threshold;

    OverheadProp( void ); // should only be used for serialization

public:
    OverheadProp( Context* ct, Context* phaseCt, double overhead,
                  double threshold = OVERHEAD_PROPS_THRESHOLD::OverheadProp ) :
        Property( ct ), Overhead( overhead ), context( ct ), threshold( threshold ) {
    }

    ~OverheadProp( void ) {
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

#endif /*OVERHEADPROP_H_*/
