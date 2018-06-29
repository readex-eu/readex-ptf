/**
   @file	MPIexcessive.h
   @ingroup AnalysisAgent
   @brief   MPI specific property
   @verbatim
        Revision:       $Revision$
        Revision date:  $Date$
        Committed by:   $Author$

        This file is part of the Periscope performance measurement tool.
        See http://www.lrr.in.tum.de/periscope for details.

        Copyright (c) 2005-2011, Technische Universitaet Muenchen, Germany
        See the COPYING file in the base directory of the package for details.
   @endverbatim
 */
#ifndef PERFORMANCEDYNAMICS_H_
#define PERFORMANCEDYNAMICS_H_



#include "Property.h"
#include "PropertyID.h"


#include <TDA_Vector.hpp>
#include <TDA_QSequence.hpp>
#include <vector>

using namespace std;

class PerformanceDynamics : public Property {
private:
    double     threshold;
    Property*  staticProperty;
    TDA_Stuff& tda_data;

    double severity_val;

    std::vector<TDA_QEdge*> changes;

public:
    PerformanceDynamics( Property*  staticProperty,
                         TDA_Stuff& tda_data,
                         double     threshold = 10 );

    ~PerformanceDynamics( void ) {
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

#endif /*PERFORMANCEDYNAMICS_H_*/
