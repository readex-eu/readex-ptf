/**
   @file    EmptyTaskinTaskRegion.h
   @ingroup OpenMPProperties
   @brief   OpenMP 'Empty task in tasks' Property header
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

#ifndef EMPTYTASKINTASKREGION_H_
#define EMPTYTASKINTASKREGION_H_

#include "Property.h"
#include "PropertyID.h"
#include "global.h"
#include <vector>

class EmptyTasksinTaskRegionProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& numberofCreatedTasks;
        ar& numberofExecutedTasks;
        ar& threshold;
        ar& phaseCycles;
        ar& phaseContext;
        ar& ExecTime_Region;
        ar& ExecTime_Body;
        ar& ExecTime_singleTask;
        ar& empty_count;
        ar& TaskRegion_thread;
        ar& TaskBody_thread;
        ar& empty_task_for_all_threads;
    }

private:
    INT64              numberofCreatedTasks;
    INT64              numberofExecutedTasks;
    double             threshold;
    INT64              phaseCycles;
    Context*           phaseContext;
    INT64              ExecTime_Region;
    INT64              ExecTime_Body;
    INT64              ExecTime_singleTask;
    INT64              empty_count;
    std::vector<INT64> TaskRegion_thread;
    std::vector<INT64> TaskBody_thread;
    bool               empty_task_for_all_threads;

    EmptyTasksinTaskRegionProp( void ); // should only be used for serialization

public:
    EmptyTasksinTaskRegionProp( Context* ct, Context* phaseCt, double threshold =
                                    OMP_PROPS_THRESHOLD::EmptyTasksinTaskRegionProp ) :
        Property( ct ), phaseContext( phaseCt ), phaseCycles( 0 ), threshold( threshold ) {
    }

    ~EmptyTasksinTaskRegionProp( void ) {
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

#endif /*EMPTYTASKINTASKREGION_H_*/
