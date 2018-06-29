/**
   @file    ImbalanceInParSections.h
   @ingroup OpenMPProperties
   @brief   OpenMP Load Imbalance in Parallel Sections property
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

   Refinements:
 *   i) Load imbalance due to not enough sections property
 *   ii) Load imbalance due to uneven sections property
 */

#ifndef ImbalanceInParSections_H_
#define ImbalanceInParSections_H_

#include "Property.h"
#include "PropertyID.h"
#include "global.h"
#include <vector>

class ImbalanceInParSectionsProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& threshold;
        ar& phaseCycles;
        ar& phaseContext;
        ar& waitCycles;
        ar& minWaitTime;
        ar& maxWaitTime;
        ar& numOfParallelSectionBodies;
    }
private:
    double             threshold;
    INT64              phaseCycles;
    Context*           phaseContext;
    std::vector<INT64> waitCycles;
    INT64              minWaitTime;
    INT64              maxWaitTime;
    INT64              numOfParallelSectionBodies;

    ImbalanceInParSectionsProp( void ); // should only be used for serialization

public:
    ImbalanceInParSectionsProp( Context* ct, Context* phaseCt, double threshold =
                                    OMP_PROPS_THRESHOLD::ImbalanceInParSectionsProp ) :
        Property( ct ), phaseContext( phaseCt ), phaseCycles( 0 ),
        numOfParallelSectionBodies( 0 ), threshold( threshold ) {
    }

    ~ImbalanceInParSectionsProp( void ) {
    }

    bool condition( void ) const;

    double confidence( void ) const;

    double severity( void ) const;

    Gather_Required_Info_Type request_metrics( void );

    void countNumSectionBodies( void );

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

#endif  /*ImbalanceInParSections_H_*/
