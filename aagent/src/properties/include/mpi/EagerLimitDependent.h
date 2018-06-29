/**
   @file    EagerLimitDependent.h
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

/**
 * @defgroup MPIProperties MPI Properties
 * @ingroup Properties
 */

#ifndef NEAR_EAGER_THRESHOLD_TRAFFIC_H_
#define NEAR_EAGER_THRESHOLD_TRAFFIC_H_

#include "Property.h"
#include "PropertyID.h"

class EagerLimitDependent : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& mpi_msg_p2p_tot;
        ar& mpi_msg_p2p_thr;
        ar& mpi_msg_freq_2k;
        ar& mpi_msg_freq_4k;
        ar& mpi_msg_freq_8k;
        ar& mpi_msg_freq_16k;
        ar& mpi_msg_freq_32k;
        ar& mpi_msg_freq_64k;
        ar& threshold;
        ar& metric;
    }

private:
    INT64    mpi_msg_p2p_tot;
    INT64    mpi_msg_p2p_thr;
    INT64    mpi_msg_freq_2k;
    INT64    mpi_msg_freq_4k;
    INT64    mpi_msg_freq_8k;
    INT64    mpi_msg_freq_16k;
    INT64    mpi_msg_freq_32k;
    INT64    mpi_msg_freq_64k;
    double   threshold;
    Context* startCtx;
    Metric   metric;

public:
    EagerLimitDependent( Context* ct = 0, Context* startCtx_p = 0, Metric m = PSC_UNDEFINED_METRIC, double threshold =
                             MPI_PROPS_THRESHOLD::MPIexcessiveProp ) :
        Property( ct ), startCtx( startCtx_p ), metric( m ), threshold( threshold ) {
    }

    ~EagerLimitDependent( void ) {
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

    std::string toXMLExtra( void );

    Property* clone( void );
};

#endif
