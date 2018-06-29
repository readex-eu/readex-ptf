/**
   @file    TooFineGranularTasks.h
   @ingroup OpenMPProperties
   @brief   OpenMP Fine Granularity Tasks property
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

#ifndef TOO_FINE_GRANULAR_TASKS_H_
#define TOO_FINE_GRANULAR_TASKS_H_

#include "Property.h"
#include "PropertyID.h"
#include "global.h"
#include <vector>

class TooFineGranularTasksProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& threshold;
        ar& phaseCycles;
        ar& phaseContext;
        ar& ExecTime_TaskRegion;
        ar& ExecTime_TaskBody;
        ar& Mean_TaskBody;
        ar& Max_TaskRegion;
        ar& Tasks_Created;
        ar& fine_granular_tasks;
        ar& TaskRegion_eachthread;
        ar& TaskBody_eachthread;
        ar& Tasks_Created_eachthread;
    }
private:
    double             threshold;
    INT64              phaseCycles;
    Context*           phaseContext;
    INT64              ExecTime_TaskRegion;
    INT64              ExecTime_TaskBody;
    INT64              Mean_TaskBody;
    INT64              Max_TaskRegion;
    double             Tasks_Created;
    INT64              fine_granular_tasks;
    std::vector<INT64> TaskRegion_eachthread;
    std::vector<INT64> TaskBody_eachthread;
    std::vector<INT64> Tasks_Created_eachthread;

    TooFineGranularTasksProp( void ); // should only be used for serialization

public:
    TooFineGranularTasksProp( Context* ct, Context* phaseCt, double threshold =
                                  OMP_PROPS_THRESHOLD::TooFineGranularTasksProp ) :
        Property( ct ), phaseContext( phaseCt ), phaseCycles( 0 ), threshold( threshold ) {
    }
    ~TooFineGranularTasksProp( void ) {
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

#endif /*TOO_FINE_GRANULAR_TASKS_H_*/
