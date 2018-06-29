/**
   @file    MPIexcessive.h
   @ingroup DynamicProperties
   @brief   Dynamic performance analysis property header
   @author  Yury Oleynik
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2015, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */


/**
 * @defgroup DynamicProperties Dynamic Performance Analysis Properties
 * @ingroup Properties
 */


#ifndef DEGRADATION_H
#define DEGRADATION_H

#include "Property.h"
#include "PropertyID.h"


#include <TDA_Vector.hpp>
#include <TDA_QSequence.hpp>
#include <vector>

using namespace std;

class Degradation : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& threshold;
        ar& staticProperty;
        ar& tda_data;
        ar& edge;
        ar& severity_val;
        ar& most_stable_sequence;
    }

private:
    double        threshold;
    Property*     staticProperty;
    TDA_Stuff&    tda_data;
    TDA_QEdge*    edge;
    double        severity_val;
    TDA_QSequence most_stable_sequence;


public:
    Degradation( void ); // should only be used for serialization

    Degradation( Property*  staticProperty,
                 TDA_Stuff& tda_data,
                 TDA_QEdge* edge,
                 double     threshold = 10 );

    ~Degradation( void ) {
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

    Prop_List next( void );

    Property* clone( void );
};

#endif /*DEGRADATION_H*/
