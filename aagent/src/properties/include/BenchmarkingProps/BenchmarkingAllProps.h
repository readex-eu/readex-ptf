/**
   @file    BenchmarkingAllProps.h
   @ingroup BenchmarkingProperties
   @brief   All metrics requested for benchmarking
   @author  Diana Gudu
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
 * @defgroup BenchmarkingProperties Benchmarking Properties
 * @ingroup Properties
 */

#include "Property.h"
#include "PropertyID.h"


class BenchmarkingAllProps : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& phaseCycles;
        ar& stallCycles;
        ar& importance;
        ar& phaseContext;
        ar& metrics;
        ar& metricNames;
        ar& threshold;
    }
private:
    INT64    phaseCycles;
    INT64    stallCycles;
    double   importance;
    Context* phaseContext;

    // all metrics
    INT64                      metrics[ 20 ];
    std::map<int, std::string> metricNames;
    double                     threshold;

    BenchmarkingAllProps( void ); // should only be used for serialization

public:
    BenchmarkingAllProps( Context*                          ct,
                          Context*                          phaseCt,
                          double                            threshold =
                              BENCHMARKING_PROPS_THRESHOLD::BenchmarkingAllProps );

    virtual ~BenchmarkingAllProps( void );

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

    Prop_List next( void );
};
