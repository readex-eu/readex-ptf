/**
   @file    L2Misses.h
   @ingroup BenchmarkingProperties
   @brief   L2 Cache Miss Ratio Property header
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

#ifndef L2Misses_H_
#define L2Misses_H_

#include "Property.h"
#include "PropertyID.h"

class L2Misses : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& PAPI_L2_TM;
        ar& PAPI_L2_TH;
        ar& phaseCycles;
        ar& cycles;
        ar& evaluationResult;
        ar& phaseContext;
        ar& threshold;
    }
private:
    INT64    PAPI_L2_TM;
    INT64    PAPI_L2_TH;
    INT64    phaseCycles;
    INT64    cycles;
    double   evaluationResult;
    Context* phaseContext;
    double   threshold;

    L2Misses( void ); // should only be used for serialization

public:
    L2Misses( Context* ct, Context* phaseCt, double threshold = BENCHMARKING_PROPS_THRESHOLD::L2Misses ) :
        Property( ct ), phaseCycles( 0 ), evaluationResult( 0.0 ), phaseContext( phaseCt ), threshold( threshold ) {
    }

    ~L2Misses( void ) {
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
