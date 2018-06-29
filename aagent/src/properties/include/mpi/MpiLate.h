/**
   @file    MpiLate.h
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

#ifndef MPILATE_H_
#define MPILATE_H_

#include "Property.h"
#include "PropertyID.h"
class MPI_PROPS_THRESHOLD;

class MpiLateProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& startCtx;
        ar& threshold;
        ar& lateTime;
        ar& totalTime;
        ar& totalCount;
        ar& remoteCount;
        ar& siteTime;
        ar& m;
    }

private:
    Context* startCtx;
    double   threshold;
    INT64    lateTime;
    INT64    totalTime;
    INT64    totalCount;
    INT64    remoteCount;
    INT64    siteTime;
    Metric   m;

    MpiLateProp( void ); // should only be used for serialization

public:
    MpiLateProp( Context* ct = 0, Context* startCtx_p = 0, Metric m_p = PSC_MPI_LATE_SEND, double threshold =
                     MPI_PROPS_THRESHOLD::MpiLateProp ) :
        Property( ct ), startCtx( startCtx_p ), m( m_p ), threshold( threshold ),
        totalTime( 0 ), totalCount( 0 ), remoteCount( 0 ), lateTime( 0 ), siteTime( 0 ) {
    }

    ~MpiLateProp( void ) {
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

    Metric get_metric( void );

    void set_metric( Metric m );

    Property* clone( void );
};

#endif /*MPILATE_H_*/
