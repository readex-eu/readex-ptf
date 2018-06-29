/**
   @file    p6HotSpotMem.h
   @ingroup Power6Properties
   @brief   Power6 specific property
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

#include "Property.h"
#include "PropertyID.h"


class p6HotSpotMem : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& phaseCycles;
        ar& stallCycles;
        ar& cycles;
        ar& instr_cmpl;
        ar& CPI;
        ar& importance;
        ar& threshold;
        ar& phaseContext;
        ar& P6Freq;
        ar& withMemProp;
    }
private:
    INT64    phaseCycles;
    INT64    stallCycles;
    INT64    cycles;
    INT64    instr_cmpl;
    double   CPI;
    double   importance;
    double   threshold;
    Context* phaseContext;
    INT64    P6Freq;
    bool     withMemProp;

    p6HotSpotMem( void ); // should only be used for serialization

public:
    p6HotSpotMem( Context* ct,
                  Context* phaseCt );

    //p6HotSpotMem( Context *ct,
    //              Context *phaseCt,
    //              bool memProp);

    virtual ~p6HotSpotMem( void );

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

    Prop_List next( void );
};
