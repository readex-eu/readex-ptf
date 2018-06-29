/**
   @file    EnergyInefficient.h
   @ingroup EnergyProperties
   @brief   Return energy spent per floating point instruction, indicating energy inefficient regions, as severity header.
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


#ifndef ENERGYINEFFICIENT_H_
#define ENERGYINEFFICIENT_H_

#include "Property.h"
#include "PropertyID.h"


class EnergyInefficient : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& threshold;
        ar& phaseContext;
        ar& FlopsPerJoule;
        ar& ConsumedEnergy;
        ar& FLOPS;
        ar& evaluationResult;
    }

private:
    double   threshold;
    Context* phaseContext;
    INT64    FlopsPerJoule;
    INT64    ConsumedEnergy;
    INT64    FLOPS;

    double evaluationResult;

    EnergyInefficient( void ); // should only be used for serialization

public:
    EnergyInefficient( Context* ct,
                       Context* phaseCt,
                       double   threshold = 0.0 );

    virtual ~EnergyInefficient( void );

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
#endif /* ENERGYINEFFICIENT_H_ */
