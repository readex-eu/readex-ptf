/**
   @file    PerformanceCounters.h
   @ingroup PerformanceCounters
   @brief   Performance counters as program signature
   @author  Miklos Homolya
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

/**
 * @defgroup PerformanceCounters Performance Counter Properties
 * @ingroup Properties
 */

#ifndef PERFORMANCECOUNTERS_H_
#define PERFORMANCECOUNTERS_H_

#include "Property.h"
#include "PropertyID.h"


class PerformanceCounters : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& threshold;
        ar& execTime;
        ar& instructionCount;
        ar& features;
    }

private:
    double             threshold;
    double             execTime;
    INT64              instructionCount;
    std::vector<INT64> features;

    // should only be used for serialization
    PerformanceCounters( void ) : threshold( 0.0 ), execTime( 0.0 ), instructionCount( 0 ) {
    }

public:
    PerformanceCounters( Context* ctx,
                         Context* phaseCtx,
                         double   th = 0.0 );
    Property* clone( void );

    bool condition( void ) const;

    double confidence( void ) const;

    double severity( void ) const;

    PropertyID id( void );

    std::string name( void );

    Gather_Required_Info_Type request_metrics( void );

    void evaluate( void );

    std::string info( void );

    std::string toXMLExtra( void );
};

#endif /* PERFORMANCECOUNTERS_H_ */
