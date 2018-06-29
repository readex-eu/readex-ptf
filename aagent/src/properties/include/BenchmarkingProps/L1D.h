/**
   @file    L1D.h
   @ingroup BenchmarkingProperties
   @brief   L1 Data Cache Misses Ratio Property header
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

#ifndef L1D_H_
#define L1D_H_

#include "Property.h"
#include "PropertyID.h"

class L1D : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& PAPI_L1;
        ar& PAPI_L1_H;
        ar& phaseCycles;
        ar& cycles;
        ar& evaluationResult;
        ar& phaseContext;
        ar& threshold;
    }
private:
    INT64    PAPI_L1;
    INT64    PAPI_L1_H;
    INT64    phaseCycles;
    INT64    cycles;
    double   evaluationResult;
    Context* phaseContext;
    double   threshold;

    L1D( void ); // should only be used for serialization

public:
    L1D( Context* ct, Context* phaseCt, double threshold = BENCHMARKING_PROPS_THRESHOLD::L1D ) :
        Property( ct ), phaseCycles( 0 ), evaluationResult( 0.0 ), phaseContext( phaseCt ), threshold( threshold ) {
    }

    ~L1D( void ) {
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
