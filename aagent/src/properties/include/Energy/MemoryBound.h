/**
   @file    MemoryBound.h
   @ingroup EnergyProperties
   @brief   Return number of L3 cache misses per FLOP, indicating memory bound regions, as severity header
   @author  Robert Mijakovic
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

#ifndef MEMORYBOUND_H_
#define MEMORYBOUND_H_

#include "Property.h"
#include "PropertyID.h"


class MemoryBound : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& threshold;
        ar& phaseCycles;
        ar& context;
        ar& phaseContext;
        ar& L3CacheMissesPerFLOP;
        ar& L3CacheMisses;
        ar& FLOPS;
    }
private:
    double   threshold;
    INT64    phaseCycles;
    Context* context;
    Context* phaseContext;
    double   L3CacheMissesPerFLOP;
    double   L3CacheMisses;
    double   FLOPS;

    MemoryBound( void ); // should only be used for serialization

public:
    MemoryBound( Context* ct,
                 Context* phaseCt,
                 double   threshold = 0.0 );

    virtual ~MemoryBound( void );

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
#endif /* MEMORYBOUND_H_ */
