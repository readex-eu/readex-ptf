/**
   @file    Task_StartupAndShutdown_Overhead.h
   @ingroup OpenMPProperties
   @brief   OpenMP Task startup and Shutdown overhead property
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

#ifndef TASK_STARTUPANDSHUTDOWN_OVERHEAD_H_
#define TASK_STARTUPANDSHUTDOWN_OVERHEAD_H_

#include "Property.h"
#include "PropertyID.h"
#include "global.h"
#include <vector>

class Task_StartupAndShutdown_OverheadProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& threshold;
        ar& phaseCycles;
        ar& phaseContext;
        ar& ExecTime_TaskRegion;
        ar& ExecTime_TaskBody_forMaster;
        ar& Startup_Time;
        ar& TaskBody_eachthread;
        ar& TaskRegion_eachthread;
    }
private:
    double             threshold;
    INT64              phaseCycles;
    Context*           phaseContext;
    INT64              ExecTime_TaskRegion;
    INT64              ExecTime_TaskBody_forMaster;
    INT64              Startup_Time;
    std::vector<INT64> TaskBody_eachthread;
    std::vector<INT64> TaskRegion_eachthread;

    Task_StartupAndShutdown_OverheadProp( void ); // should only be used for serialization
public:
    Task_StartupAndShutdown_OverheadProp( Context* ct, Context* phaseCt, double threshold =
                                              OMP_PROPS_THRESHOLD::Task_StartupAndShutdown_OverheadProp ) :
        Property( ct ), phaseContext( phaseCt ), phaseCycles( 0 ), threshold( threshold ) {
    }

    ~Task_StartupAndShutdown_OverheadProp( void ) {
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
    INT64 exec_time( void );

    /**
     * @brief Additional XML property information
     */
    std::string toXMLExtra( void );

    Context* get_phaseContext( void );

    Property* clone( void );
};

#endif /*TASK_STARTUPANDSHUTDOWN_OVERHEAD_H_*/
