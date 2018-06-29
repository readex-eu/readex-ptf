/**
   @file    p6HighLevelMem.h
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
#include <bitset>

#define NUM_CASE   5
#define STREAMING  0
#define REUSE      1
#define NON_LOCAL  2
#define ST_BOUND   3
#define NONE       4

#define INTERVAL( var1, var, var2 ) ( var > var1 && var <= var2 )
#define INTERVAL_80_to_120( var ) ( var > 80 && var <= 120 )
#define INTERVAL_greater_120( var ) ( var > 120 )

class p6HighLevelMem : public Property {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& boost::serialization::base_object<Property>( *this );
        ar& phaseCycles;
        ar& stallCycles;
        ar& cycles;
        ar& importance;
        ar& threshold;
        ar& phaseContext;
        ar& l1MissCyc;
        ar& l2Pref;
        ar& l1Ref;
        ar& l1RefBytes;
        ar& l1Pref;
        ar& l3Ref;
        ar& l3Hits;
        ar& l3Miss;
        ar& l1Miss;
        ar& srqFull;
        ar& storeRef;
        ar& deratMissCyc;
        ar& data_from_lmem;
        ar& data_from_outside1;
        ar& data_from_outside2;
        ar& dSRQCyc;
        ar& dDERATMissCYC;
        ar& dL1Pref;
        ar& dL2Pref;
        ar& dL3MissRate;
        ar& dL2MissRate;
        ar& dL1MissRate;
        ar& dSRQ;
        ar& dAvgL1Cyc;
        ar& impact;
        ar& mbytes;
        ar& bandwidth;
        ar& reuse_factor;

        ar& deviation0;
        ar& deviation1;
        ar& deviation2;
        ar& deviation3;
        ar& mem_cases;
        ar& severities;
    }
private:
    INT64    phaseCycles;
    double   stallCycles;
    INT64    cycles;
    double   importance;
    double   threshold;
    Context* phaseContext;

    INT64 l1MissCyc;
    INT64 l2Pref;
    INT64 l1Ref;
    INT64 l1RefBytes;
    INT64 l1Pref;
    INT64 l3Ref;
    INT64 l3Hits;
    INT64 l3Miss;
    INT64 l1Miss;
    INT64 srqFull;
    INT64 storeRef;
    INT64 deratMissCyc;
    INT64 data_from_lmem;
    INT64 data_from_outside1;
    INT64 data_from_outside2;

    double dSRQCyc;
    double dDERATMissCYC;
    double dL1Pref;
    double dL2Pref;
    double dL3MissRate;
    double dL2MissRate;
    double dL1MissRate;
    double dSRQ;
    double dAvgL1Cyc;
    double impact;
    double mbytes;
    double bandwidth;
    double reuse_factor;

    int                   deviation0;
    int                   deviation1;
    int                   deviation2;
    int                   deviation3;
    std::bitset<NUM_CASE> mem_cases;
    std::list<double>     severities;

    p6HighLevelMem( void ); // should only be used for serialization

public:
    p6HighLevelMem( Context* ct,
                    Context* phaseCt );

    virtual ~p6HighLevelMem( void );

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

    std::string subId( void );

    Context* get_phaseContext( void );

    Property* clone( void );
};
