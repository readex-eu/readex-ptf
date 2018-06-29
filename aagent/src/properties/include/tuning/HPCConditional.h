/**
   @file    HPCConditional.h
   @ingroup TuningProperties
   @brief   Returns important metrics used as criterium on the plugin selection process in adaptive-sequence plugin header
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

#ifndef HPCCONDITIONAL_H_
#define HPCCONDITIONAL_H_

#include "Property.h"
#include "PropertyID.h"
#include "global.h"
#include <vector>

class HPCConditional : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& threshold;
        ar& phaseCycles;
        ar& ExecutionTime;
        ar& RegionExecutedCount;
        ar& TimePerInstance;
        ar& MPITime;
        ar& TotalInstructions;
        ar& EnergyCores;
        ar& EnergyDRAM;
    }
private:
    double threshold;
    INT64  phaseCycles;

    double ExecutionTime;
    INT64  RegionExecutedCount;
    double TimePerInstance;
    double MPITime;
    INT64  TotalInstructions;
    INT64  EnergyCores;
    INT64  EnergyDRAM;

    HPCConditional( void ); // should only be used for serialization

public:
    HPCConditional( Context* ct,
                    Context* phaseCt,
                    double   threshold = 0.0 );

    ~HPCConditional( void );

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

#endif /*HPCCONDITIONAL_H_*/
