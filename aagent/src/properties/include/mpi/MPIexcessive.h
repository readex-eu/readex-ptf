/**
   @file    MPIexcessive.h
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

#ifndef MPIEXCESSIVE_H_
#define MPIEXCESSIVE_H_

#include "Property.h"
#include "PropertyID.h"

class MPIexcessiveProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& threshold;
        ar& phaseTime;
        ar& totalCount;
        ar& siteTime;
        ar& startCtx;
        ar& late_metric;
        ar& msgSize;
    }

private:
    double   threshold;
    INT64    phaseTime;
    INT64    totalCount;
    INT64    siteTime;
    INT64    msgSize;
    Context* startCtx;
    Metric   late_metric;

    MPIexcessiveProp( void );  // should only be used for serialization

public:
    MPIexcessiveProp( Context* ct = 0, Context* startCtx_p = 0, Metric m = PSC_UNDEFINED_METRIC, double threshold =
                          MPI_PROPS_THRESHOLD::MPIexcessiveProp ) :
        Property( ct ), startCtx( startCtx_p ), late_metric( m ),
        threshold( threshold ), phaseTime( 0 ), totalCount( 0 ), siteTime( 0 ) {
    }

    ~MPIexcessiveProp( void ) {
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

//BOOST_CLASS_EXPORT_IMPLEMENT(MPIexcessiveProp);

#endif /*MPILATE_H_*/
