/**
   @file    LongLatencyInstructionException.h
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

#ifndef LONGLATENCYINSTRUCTIONEXCEPTION_H_
#define LONGLATENCYINSTRUCTIONEXCEPTION_H_

#include "Property.h"
#include "PropertyID.h"

class LongLatencyInstructionException : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& uopsDecodedMS;
        ar& uopsRetiredAny;
        ar& phaseCycles;
        ar& cycles;
        ar& evaluationResult;
        ar& phaseContext;
        ar& threshold;
    }
private:
    INT64    uopsDecodedMS;
    INT64    uopsRetiredAny;
    INT64    phaseCycles;
    INT64    cycles;
    double   evaluationResult;
    Context* phaseContext;
    double   threshold;

    LongLatencyInstructionException( void ); // should only be used for serialization

public:
    LongLatencyInstructionException( Context* ct, Context* phaseCt, double threshold =
                                         WESTMERE_PROPS_THRESHOLD::LongLatencyInstructionException ) :
        Property( ct ), phaseCycles( 0 ), evaluationResult( 0.0 ), phaseContext( phaseCt ), threshold( threshold ) {
    }
    ~LongLatencyInstructionException( void ) {
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
