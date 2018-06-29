/**
   @file    HotRegionExecTime.h
   @ingroup OpenMPProperties
   @brief   Hot OpenMP region header
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

#ifndef HOTREGIONEXECTIME_H_
#define HOTREGIONEXECTIME_H_

#include "Property.h"
#include "PropertyID.h"
#include "global.h"
#include <vector>

typedef enum {
    USER_REGION_TYPE,
    MAIN_REGION_TYPE,
    OMP_FOR_REGION_TYPE,
    PARALLEL_REGION_TYPE,
    DO_REGION_TYPE,
    CALL_REGION_TYPE,
    SUB_REGION_TYPE
} RegionTypeIdentity;

class HotRegionExecTimeProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& threshold;
        ar& phaseCycles;
        ar& phaseContext;
        ar& threads_ExecTime;
        ar& regionType;
        ar& ExecTime;
    }
private:
    double             threshold;
    INT64              phaseCycles;
    Context*           phaseContext;
    std::vector<INT64> threads_ExecTime;
    RegionTypeIdentity regionType;
    INT64              ExecTime;

    HotRegionExecTimeProp( void ); // should only be used for serialization

public:
    HotRegionExecTimeProp( Context* ct, Context* phaseCt, double threshold = OMP_PROPS_THRESHOLD::HotRegionExecTimeProp ) :
        Property( ct ), phaseContext( phaseCt ), phaseCycles( 0 ), threshold( threshold ) {
    }

    HotRegionExecTimeProp( Context* ct, Context* phaseCt, RegionTypeIdentity regionT, double threshold =
                               OMP_PROPS_THRESHOLD::HotRegionExecTimeProp ) :
        Property( ct ), phaseContext( phaseCt ), phaseCycles( 0 ), regionType( regionT ), threshold( threshold ) {
    }

    ~HotRegionExecTimeProp( void ) {
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

#endif /*HOTREGIONEXECTIME_H_*/
