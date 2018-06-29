/**
   @file    EnergyConsumption.h
   @ingroup EnergyProperties
   @brief   Energy consumption property header
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

#ifndef ENERGYCONSUMPTION_H_
#define ENERGYCONSUMPTION_H_

#include "Property.h"
#include "PropertyID.h"


class EnergyConsumption : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& phaseCycles;
        ar& context;
        ar& phaseContext;
        ar& energyConsumption;
    }

private:
    static const std::string name_constant;

    INT64    phaseCycles;
    INT64    instances;
    double   time;
    INT64    cpuEnergy;
    INT64    totalInstr;
    double   energyConsumption;


    EnergyConsumption( void ); // should only be used for serialization

public:
    EnergyConsumption( Context* ct,
                       Context* phaseCt );

    bool condition( void ) const;

    double confidence( void ) const;

    double severity( void ) const;

    Gather_Required_Info_Type request_metrics( void );

    void evaluate( void );

    PropertyID id( void );

    std::string name( void );

    void print( void );

    std::string info( void );

    Context* get_phaseContext( void );

    Property* clone( void );

    std::string toXMLExtra();
};
#endif /* ENERGYCONSUMPTION_H_ */
