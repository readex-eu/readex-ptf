/**
   @file    SuitedForEnergyConfiguration.h
   @ingroup EnergyProperties
   @brief   Return the execution time of the region in thread 0 as severity header
   @author  Robert Mijakovic
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

#ifndef SUITEDFORENERGYCONFIGURATION_H_
#define SUITEDFORENERGYCONFIGURATION_H_

#include "Property.h"
#include "PropertyID.h"
#include "global.h"
#include <vector>

class SuitedForEnergyConfiguration : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& threshold;
        ar& phaseCycles;
        ar& l3CacheMisses;
        ar& l2CacheMisses;
        ar& instr;
        ar& cycles;
        ar& nanojoules;
        ar& nanojoules_dram;
        ar& TimePerInstance;
        ar& ExecutionTime;
        ar& RegionExecutedCount;
    }
private:
    double threshold;
    INT64  phaseCycles;
    INT64  l3CacheMisses;
    INT64  l2CacheMisses;
    INT64  instr;
    INT64  cycles;
    INT64  nanojoules;
    INT64  nanojoules_dram;

    double TimePerInstance;
    double ExecutionTime;
    INT64 RegionExecutedCount;

    SuitedForEnergyConfiguration( void ); // should only be used for serialization

public:
    SuitedForEnergyConfiguration( Context* ct,
                                  Context* phaseCt,
                                  double   threshold = 0.0 );

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
};

#endif /*SUITEDFORENERGYCONFIGURATION_H_*/
