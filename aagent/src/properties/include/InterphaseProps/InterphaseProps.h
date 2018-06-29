/**
   @file    InterphaseProps.h
   @ingroup InterphaseProperties
   @brief   Properties for phase identifiers header
   @author  Madhura Kumaraswamy
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2017, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#ifndef INTERPHASEPROPS_H_
#define INTERPHASEPROPS_H_

#include "Property.h"
#include "PropertyID.h"


class InterphaseProps : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& phaseCycles;
        ar& context;
        ar& phaseContext;
        ar& phase_iter;
        ar& L3Misses;
        ar& totalInstr;
        ar& branchInstr;
        ar& energyConsumption;
        ar& cpuEnergy;
    }

private:
    static const std::string name_constant;

    INT64          phaseCycles;
    Context*       context;
    Context*       phaseContext;
    unsigned int   phase_iter;
    INT64          L3Accesses;
    INT64          L3Misses;
    INT64          totalInstr;
    INT64          branchInstr;

    INT64          instances;
    double         exectime;
    INT64          cpuEnergy;
    INT64          energyConsumption;

    InterphaseProps( void ); // should only be used for serialization

public:
    InterphaseProps( Context* ct,
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
#endif /* INTERPHASEPROPS_H_ */
