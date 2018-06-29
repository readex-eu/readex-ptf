/**
   @file    ImbalanceOMPBarrier.h
   @ingroup OpenMPProperties
   @brief   OpenMP Load Imbalance in Explicit Barrier property
   @author  Shajulin Benedict
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

#ifndef IMBALANCEOMPBARRIERREGION_H_
#define IMBALANCEOMPBARRIERREGION_H_

#include "Property.h"
#include "PropertyID.h"
#include "global.h"
#include <vector>

class ImbalanceOMPBarrierProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& threshold;
        ar& phaseCycles;
        ar& phaseContext;
        ar& waitCycles;
        ar& minWaitTime;
        ar& maxWaitTime;
        ar& avg_threads;
    }
private:
    double             threshold;
    INT64              phaseCycles;
    Context*           phaseContext;
    std::vector<INT64> waitCycles;
    INT64              minWaitTime;
    INT64              maxWaitTime;
    INT64              avg_threads;

    ImbalanceOMPBarrierProp( void ); // should only be used for serialization

public:
    ImbalanceOMPBarrierProp( Context* ct, Context* phaseCt,
                             double threshold = OMP_PROPS_THRESHOLD::ImbalanceOMPBarrierProp ) :
        Property( ct ), phaseContext( phaseCt ), phaseCycles( 0 ), threshold( threshold ) {
    }

    ~ImbalanceOMPBarrierProp( void ) {
    }

    bool condition( void ) const;

    double confidence( void ) const;

    double severity( void ) const;

    Gather_Required_Info_Type request_metrics( void );

    void evaluate( void );

    PropertyID id( void );

    std::string name( void );

    void print( void );

    std::string info( void ) {
        return "";
    }

    /**
     * @brief Additional XML property information
     */
    std::string toXMLExtra( void );

    Context* get_phaseContext( void );

    Property* clone( void );
};

#endif  /*IMBALANCEOMPBARRIERREGION_H_*/
