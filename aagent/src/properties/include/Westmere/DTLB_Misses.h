/**
   @file    DTLB_Misses.h
   @ingroup WestmereProperties
   @brief   Westmere specific property
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

#ifndef DTLB_MISSES_H_
#define DTLB_MISSES_H_

#include "Property.h"
#include "PropertyID.h"


class DTLB_Misses : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& phaseContext;
        ar& DTLB_Misses_Any;
        ar& DTLB_Load_Misses_Any;
        ar& ITLB_Misses_Any;
        ar& DTLB_Misses_Walk_Completed;
        ar& Thread_P;
        ar& phaseCycles;
        ar& cycles;
        ar& evaluationResult;
        ar& threshold;
    }
private:
    Context* phaseContext;

    INT64 DTLB_Misses_Any;
    INT64 DTLB_Load_Misses_Any;
    INT64 ITLB_Misses_Any;
    INT64 DTLB_Misses_Walk_Completed;
    INT64 Thread_P;
    INT64 phaseCycles;
    INT64 cycles;

    double evaluationResult;
    double threshold;

    DTLB_Misses( void ); // should only be used for serialization

public:
    DTLB_Misses( Context* ct, Context* phaseCt, double threshold = WESTMERE_PROPS_THRESHOLD::DTLB_Misses ) :
        Property( ct ), phaseCycles( 0 ), evaluationResult( 0.0 ),
        phaseContext( phaseCt ), threshold( threshold ) {
    }

    ~DTLB_Misses( void ) {
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
#endif
