/**
   @file    PowerUsed.h
   @ingroup EnergyProperties
   @brief   Power Used property header
   @author  Carmen Navarrete
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

#ifndef POWERUSED_H_
#define POWERUSED_H_

#include "Property.h"
#include "PropertyID.h"


class PowerUsed : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& threshold;
        ar& phaseCycles;
        ar& context;
        ar& phaseContext;
        ar& powerUsed;
    }
private:
    double   threshold;
    INT64    phaseCycles;
    Context* context;
    Context* phaseContext;
    double   powerUsed;

    PowerUsed( void ); // should only be used for serialization

public:
    PowerUsed( Context* ct,
               Context* phaseCt,
               double   threshold = 0.0 );

    virtual ~PowerUsed( void );

    bool condition( void ) const;

    double confidence( void ) const;

    double severity( void ) const;

    Gather_Required_Info_Type request_metrics( void );

    void evaluate( void );

    PropertyID id( void );

    std::string name( void );

    void print( void );

    std::string info( void );

    Context* get_phaseContext( void );

    Property* clone( void );
};
#endif /* POWERUSED_H_ */
