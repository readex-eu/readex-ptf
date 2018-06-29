/**
   @file    ShortSoftwarePipelinedLoopProp.h
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

#ifndef SHORTSOFTWAREPIPELINEDLOOPPROP_H_
#define SHORTSOFTWAREPIPELINEDLOOPPROP_H_

#include "Property.h"
#include "PropertyID.h"


class ShortSoftwarePipelinedLoopProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& count;
        ar& cycles;
        ar& iterations;
        ar& issues;
        ar& instances;
        ar& phaseCycles;
        ar& phaseContext;
        ar& threshold;
    }

private:
    INT64    count;
    INT64    cycles;
    INT64    iterations;
    INT64    issues;
    INT64    instances;
    INT64    phaseCycles;
    Context* phaseContext;
    double   threshold;

    ShortSoftwarePipelinedLoopProp( void ); // should only be used for serialization

public:
    ShortSoftwarePipelinedLoopProp( Context* ct, Context* phaseCt, double threshold =
                                        IA64_PROPS_THRESHOLD::ShortSoftwarePipelinedLoopProp ) :
        Property( ct ), threshold( threshold ), count( 0 ), cycles( 0 ),
        iterations( 0 ), phaseCycles( 0 ), issues( 0 ), phaseContext( phaseCt ) {
    }

    ~ShortSoftwarePipelinedLoopProp( void ) {
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

#endif /*SHORTSOFTWAREPIPELINEDLOOPPROP_H_*/
