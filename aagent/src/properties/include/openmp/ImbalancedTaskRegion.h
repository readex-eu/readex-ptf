/**
   @file    ImbalancedTaskRegion.h
   @ingroup OpenMPProperties
   @brief   Task region with imbalanced distribution
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

#ifndef IMBALANCEDTASKREGION_H_
#define IMBALANCEDTASKREGION_H_

#include "Property.h"
#include "PropertyID.h"
#include "global.h"
#include <vector>

typedef enum {
    Imbalanced_Task_Region, Imbalace_Due_To_Uneven_Distribution_of_Tasks, Numberof_Tasks_Smaller_than_Numberof_Threads
} LoadImbalanceDueTo;

class ImbalancedTaskRegionProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& threshold;
        ar& Max_ExecTime;
        ar& Max_ExecTasks;
        ar& Max_Tasks;
        ar& Total_CreatedTasks;
        ar& Average_Differential_Time;
        ar& Average_Differential_Tasks;
        ar& Mean_Task_Body;
        ar& propertyName;
        ar& phaseCycles;
        ar& phaseContext;
        ar& TaskRegion_eachthread;
        ar& TaskBody_eachthread;
        ar& ExecTime_eachthread;
        ar& nooftasks_eachthread;
        ar& nooftasks_created;
    }
private:
    double             threshold;
    double             Max_ExecTime;
    INT64              Max_ExecTasks;
    INT64              Max_Tasks;
    INT64              Total_CreatedTasks;
    double             Average_Differential_Time;
    double             Average_Differential_Tasks;
    double             Mean_Task_Body;
    LoadImbalanceDueTo propertyName;
    INT64              phaseCycles;
    Context*           phaseContext;
    std::vector<INT64> TaskRegion_eachthread;
    std::vector<INT64> TaskBody_eachthread;
    std::vector<INT64> ExecTime_eachthread;
    std::vector<INT64> nooftasks_eachthread;
    std::vector<INT64> nooftasks_created;

    ImbalancedTaskRegionProp( void ); // should only be used for serialization

public:
    ImbalancedTaskRegionProp( Context* ct, Context* phaseCt, double threshold =
                                  OMP_PROPS_THRESHOLD::ImbalancedTaskRegionProp ) :
        Property( ct ), phaseContext( phaseCt ), phaseCycles( 0 ), threshold( threshold ) {
    }

    ImbalancedTaskRegionProp( Context* ct, Context* phaseCt, LoadImbalanceDueTo propName, double threshold =
                                  OMP_PROPS_THRESHOLD::ImbalancedTaskRegionProp ) :
        Property( ct ), phaseContext( phaseCt ), phaseCycles( 0 ), propertyName( propName ), threshold( threshold ) {
    }

    ~ImbalancedTaskRegionProp( void ) {
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

#endif /*IMBALANCEDTASKREGION_H_*/
