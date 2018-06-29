/**
   @file    ExecTimeImportance.h
   @ingroup BenchmarkingProperties
   @brief   Return the execution time of the region in thread 0 as severity header
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

#ifndef ExecTimeImportance_H_
#define ExecTimeImportance_H_

#include "Property.h"
#include "PropertyID.h"
#include "global.h"
#include <vector>

class ExecTimeImportanceProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& threshold;
        ar& phaseCycles;
        ar& cycles;
        ar& nestedCycles;
        ar& energyConsumption;
        ar& totalInstr;
        ar& cpuEnergy;
    }

private:
    double threshold;
    INT64  phaseCycles;
    INT64  cycles;
    INT64  nestedCycles;
    INT64  energyConsumption;
    INT64  cpuEnergy;
    INT64  totalInstr;


public:
    ExecTimeImportanceProp( void ); // should only be used for serialization

    ExecTimeImportanceProp( Context* ct,
                            Context* phaseCt,
                            double   threshold = 0.0 );

    ~ExecTimeImportanceProp( void );

    bool condition( void ) const;

    double confidence( void ) const;

    double severity( void ) const;

    Gather_Required_Info_Type request_metrics( void );

    void evaluate( void );

    PropertyID id( void );

    std::string name( void );

    void print( void );

    std::string info( void );

    std::string toXMLExtra( void );

    Context* get_phaseContext( void );

    Property* clone( void );

    void RequestNestedCalls( Region* reg );

    void EvaluateNestedCalls( Region* reg );
};

//BOOST_CLASS_EXPORT_KEY(ExecTimeImportanceProp);

#endif /*ExecTimeImportance_H_*/
