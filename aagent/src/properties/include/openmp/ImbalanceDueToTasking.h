/**
   @file    ImbalanceDueToTasking.h
   @ingroup OpenMPProperties
   @brief   OpenMP Imbalance in threads due to tasking property
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

#ifndef IMBALANCEDUETOTASKING_H_
#define IMBALANCEDUETOTASKING_H_

#include "Property.h"
#include "PropertyID.h"
#include "global.h"
#include <vector>

class ImbalanceDueToTaskingProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& threshold;
        ar& Max_ExecTime;
        ar& Average_Differential_Time;
        ar& phaseCycles;
        ar& phaseContext;
        ar& TaskRegion_eachthread;
        ar& TaskBody_eachthread;
        ar& ExecTime_eachthread;
    }

private:
    double             threshold;
    double             Max_ExecTime;
    double             Average_Differential_Time;
    INT64              phaseCycles;
    Context*           phaseContext;
    std::vector<INT64> TaskRegion_eachthread;
    std::vector<INT64> TaskBody_eachthread;
    std::vector<INT64> ExecTime_eachthread;

    ImbalanceDueToTaskingProp( void ); // should only be used for serialization

public:
    ImbalanceDueToTaskingProp( Context* ct, Context* phaseCt, double threshold =
                                   OMP_PROPS_THRESHOLD::ImbalanceDueToTaskingProp ) :
        Property( ct ), phaseContext( phaseCt ), phaseCycles( 0 ), threshold( threshold ) {
    }

    ~ImbalanceDueToTaskingProp( void ) {
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

    Context* get_phaseContext( void );

    Property* clone( void );
};

#endif /*IMBALANCEDUETOTASKING_H_*/
