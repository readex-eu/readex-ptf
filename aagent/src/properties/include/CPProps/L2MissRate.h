/**
   @file    L2MissRate.h
   @ingroup GenericProperties
   @brief   Generic L2 cache miss rate property header
   @author  Yury Oleynik
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

#ifndef L2MISSRATE_H_
#define L2MISSRATE_H_

#include "Property.h"
#include "PropertyID.h"

class L2MissRate : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& availableProp;
        ar& available;
        ar& L2Access;
        ar& L2Misses;
        ar& importance;
        ar& threshold;
        ar& phaseContext;
    }

private:
    bool availableProp;
    bool available;

    INT64 L2Access;
    INT64 L2Misses;

    double   importance;
    double   threshold;
    Context* phaseContext;

    L2MissRate( void ); // should only be used for serialization

public:
    L2MissRate( Context* ct, Context* phaseCt, double threshold = CPPROPS_PROPS_THRESHOLD::L2MissRate ) :
        Property( ct ), L2Access( 0 ), L2Misses( 0 ), importance( 0.0 ),
        phaseContext( phaseCt ), availableProp( true ), threshold( threshold ) {
    }

    ~L2MissRate( void );

    bool condition( void ) const;

    double confidence( void ) const;

    double severity( void ) const;

    Gather_Required_Info_Type request_metrics( void );

    void evaluate( void );

    PropertyID id( void );

    std::string name( void );

    void print( void );

    std::string info( void );

    /**
     * @brief Additional XML property information
     */
    std::string toXMLExtra( void );

    Context* get_phaseContext( void );

    Property* clone( void );
};

#endif /*L2MISSPROP_H_*/
