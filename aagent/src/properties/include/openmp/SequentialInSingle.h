/**
   @file    SequentialInSingle.h
   @ingroup OpenMPProperties
   @brief   OpenMP Sequential Computation in Single Region property
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

#ifndef SEQUENTIALINSINGLE_H_
#define SEQUENTIALINSINGLE_H_

#include "Property.h"
#include "PropertyID.h"
#include "global.h"
#include <vector>

class SequentialInSingleProp : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& threshold;
        ar& phaseCycles;
        ar& phaseContext;
        ar& waitCycles;
        ar& singleRegionCycles;
        ar& singleRegionCycles_temp;
    }
private:
    double             threshold;
    INT64              phaseCycles;
    Context*           phaseContext;
    std::vector<INT64> waitCycles;
    INT64              singleRegionCycles;
    INT64              singleRegionCycles_temp;

    SequentialInSingleProp( void ); // should only be used for serialization

public:
    SequentialInSingleProp( Context* ct, Context* phaseCt, double threshold = OMP_PROPS_THRESHOLD::SequentialInSingleProp ) :
        Property( ct ), phaseContext( phaseCt ), phaseCycles( 0 ), threshold( threshold ) {
    }

    ~SequentialInSingleProp( void ) {
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

#endif /*SEQUENTIALINSINGLE_H_*/
