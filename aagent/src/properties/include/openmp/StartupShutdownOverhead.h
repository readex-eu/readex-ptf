/**
   @file    StartupShutdownOverhead.h
   @ingroup OpenMPProperties
   @brief   OpenMP Parallel Region Start up and Shutdown overhead property
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

#ifndef STARTUPSHUTDOWNOVERHEAD_H_
#define STARTUPSHUTDOWNOVERHEAD_H_

#include "Property.h"
#include "PropertyID.h"
#include "global.h"
#include <vector>

class StartupShutdownOverheadProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& slaveCount;
        ar& threshold;
        ar& phaseCycles;
        ar& phaseContext;
        ar& masterWaitCycles;
        ar& slaveWaitCycles;
        ar& waitCycles2;
    }
private:
    int                slaveCount;
    double             threshold;
    INT64              phaseCycles;
    Context*           phaseContext;
    INT64              masterWaitCycles;
    INT64              slaveWaitCycles;
    std::vector<INT64> waitCycles2;

    StartupShutdownOverheadProp( void ); // should only be used for serialization

public:
    StartupShutdownOverheadProp( Context* ct, Context* phaseCt, double threshold =
                                     OMP_PROPS_THRESHOLD::StartupShutdownOverheadProp ) :
        Property( ct ), phaseContext( phaseCt ), phaseCycles( 0 ), threshold( threshold ) {
    }

    ~StartupShutdownOverheadProp( void ) {
    }

    bool condition( void ) const;

    double confidence( void ) const;

    double severity( void ) const;

    Gather_Required_Info_Type request_metrics( void );

    void evaluate( void );

    PropertyID id( void );

    std::string name( void );

    void print( void );

    std::string info( void );

    INT64 exec_time( void );

    /**
     * @brief Additional XML property information
     */
    std::string toXMLExtra( void );

    Context* get_phaseContext( void );

    Property* clone( void );
};

#endif /*STARTUPSHUTDOWNOVERHEAD_H_*/
