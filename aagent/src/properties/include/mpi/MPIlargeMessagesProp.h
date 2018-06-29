/**
   @file    MPIlargeMessagesProp.h
   @ingroup MPIProperties
   @brief   MPI specific property
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

#ifndef MPILARGEMESSAGES_H_
#define MPILARGEMESSAGES_H_

#include "Property.h"
#include "PropertyID.h"

class MPIlargeMessagesProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& startCtx;
        ar& threshold;
        ar& agregateSize;
        ar& callCount;
        ar& siteTime;
        ar& phaseTime;
        ar& msgSize;
    }
private:
    Context* startCtx;
    double   threshold;
    INT64    agregateSize;
    INT64    callCount;
    INT64    siteTime;
    INT64    phaseTime;
    INT64    msgSize;

    MPIlargeMessagesProp( void ); // should only be used for serialization

public:
    MPIlargeMessagesProp( Context* ct = 0, Context* startCtx_p = 0, double threshold =
                              MPI_PROPS_THRESHOLD::MPIlargeMessagesProp ) :
        Property( ct ), startCtx( startCtx_p ), threshold( threshold ),
        phaseTime( 0 ), callCount( 0 ), siteTime( 0 ) {
    }

    ~MPIlargeMessagesProp( void ) {
    }

    bool condition( void ) const;

    double confidence( void ) const;

    double severity( void ) const;

    Gather_Required_Info_Type request_metrics( void );

    void evaluate( void );

    PropertyID id( void );

    std::string subId( void );

    std::string name( void );

    void print( void );

    std::string info( void );

    /**
     * @brief Additional XML property information
     */
    std::string toXMLExtra( void );

    Property* clone( void );
};

#endif
