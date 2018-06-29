/**
   @file    ActiveCyclesObjective.h
   @ingroup TuningProperties
   @brief   Active Cycles Objective header
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


#ifndef ACTIVECYCLESOBJECTIVE_H_
#define ACTIVECYCLESOBJECTIVE_H_

#include "Property.h"
#include "PropertyID.h"
#include "global.h"

class ActiveCyclesObjective : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& threshold;
        ar& activeCycles;
        ar& evaluationResult;
    }

private:
    double threshold;
    INT64  activeCycles;
    double evaluationResult;

    ActiveCyclesObjective( void ); // should only be used for serialization

public:
    ActiveCyclesObjective( Context* ct,
                           Context* phaseCt,
                           double   threshold = 0.0 );

    virtual ~ActiveCyclesObjective( void );

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


#endif /*ACTIVECYCLESOBJECTIVE_H_*/
