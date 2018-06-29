/**
   @file    p6HighLevelMem.cc
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


#include "p6HighLevelMem.h"
#include "Property.h"
#include <iostream>



#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include <algorithm>
#include "psc_errmsg.h"
p6HighLevelMem::p6HighLevelMem( Context* ct, Context* phaseCt ) : Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    importance   = 0.0;
    threshold    = 1.0;
    cycles       = 0;
    phaseContext = phaseCt;

    for( int i = 0; i < NUM_CASE; i++ ) {
        mem_cases.set( i, 0 );
    }

    deviation0 = 20;
    deviation1 = 10;
    deviation2 = 5;
    deviation3 = 3;
}

p6HighLevelMem::~p6HighLevelMem() {
}

PropertyID p6HighLevelMem::id() {
    return p6HIGHLEVELMEM;
}

std::string p6HighLevelMem::subId() {
    std::stringstream sstr;
    for( int i = 0; i < NUM_CASE; i++ ) {
        if( mem_cases[ i ] ) {
            sstr << 1;
        }
        else {
            sstr << 0;
        }
    }
    return sstr.str();
}
void p6HighLevelMem::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6HighLevelMem::condition() const {
    // Set the thresholds very low when high debug enabled
    double debug_threshold = 10.0;
    if( psc_get_debug_level() == 6 ) {
        debug_threshold = 0.0;
    }

    psc_dbgmsg( 10, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );

    severities.sort();
    if( severities.back() > debug_threshold || INTERVAL( 30, dSRQ, 110 ) ) {
        return true;
    }
    else {
        return false;
    }
}

double p6HighLevelMem::confidence() const {
    return 1.0;
}

double p6HighLevelMem::severity() const {
    if( mem_cases[ ST_BOUND ] ) {
        severities.sort();
        return severities.back();
    }
    else {
        severities.sort();
        return severities.back();
    }
}

Context* p6HighLevelMem::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6HighLevelMem::request_metrics() {
    pdb->request( context, PSC_PM_LD_MISS_L1_CYC );             //L1 miss cycles
    pdb->request( context, PSC_PM_L2_PREF_LD );                 //L2pref
    pdb->request( context, PSC_PM_LD_REF_L1 );                  //L1ref
    pdb->request( context, PSC_PM_L1_PREF );                    //L1pref
    pdb->request( context, PSC_PM_DATA_FROM_L2MISS );           //L3references
    pdb->request( context, PSC_PM_DATA_FROM_L3 );               //L3hits
    pdb->request( context, PSC_PM_DATA_FROM_L3MISS );           //L3miss
    pdb->request( context, PSC_PM_LD_MISS_L1 );                 //L1miss
    pdb->request( context, PSC_PM_LSU_REJECT_STQ_FULL );        //SRQfull
    pdb->request( context, PSC_PM_ST_REF_L1 );                  //StoreRef
    pdb->request( context, PSC_PM_LSU_DERAT_MISS_CYC );         //DeratMissCyc
    pdb->request( context, PSC_PM_DATA_FROM_LMEM );             //Data from local memory
    pdb->request( context, PSC_PM_RUN_CYC );
    pdb->request( phaseContext, PSC_PM_RUN_CYC );
    return ALL_INFO_GATHERED;
}

std::string p6HighLevelMem::name() {
    std::stringstream stream;
    //stream << "Memory access pattern ("<<bandwidth<<" MB/s) : ";
    stream << "Memory access pattern : ";
    if( mem_cases[ ST_BOUND ] ) {
        stream << ' ' << "Store queue flooded, too many consequent stores; ";
    }
    if( mem_cases[ STREAMING ] ) {
        stream << ' ' << "Streaming from memory (" << bandwidth << " MB/s); ";
    }
    if( mem_cases[ NON_LOCAL ] ) {
        stream << ' ' << "Strided data access;";
    }
    if( mem_cases[ REUSE ] ) {
        stream << ' ' << "Insufficient cached data reuse with " << reuse_factor << "% ";
    }
    if( mem_cases[ NONE ] ) {
        stream << ' '
               << "Pattern not classified, please rerun analysis with --dont cluster and report your .psc file to periscope@lrr.in.tum.de";
    }
    return stream.str();
}

void p6HighLevelMem::evaluate() {
    l1MissCyc      = pdb->get( context, PSC_PM_LD_MISS_L1_CYC );      //L1 miss cycles
    l2Pref         = 128 * pdb->get( context, PSC_PM_L2_PREF_LD );    //L2pref
    l1Ref          = pdb->get( context, PSC_PM_LD_REF_L1 );           //L1ref
    l1RefBytes     = 8 * pdb->get( context, PSC_PM_LD_REF_L1 );
    l1Pref         = 128 * pdb->get( context, PSC_PM_L1_PREF );       //L1pref
    l3Ref          = pdb->get( context, PSC_PM_DATA_FROM_L2MISS );    //L3references
    l3Hits         = pdb->get( context, PSC_PM_DATA_FROM_L3 );        //L3hits
    l3Miss         = pdb->get( context, PSC_PM_DATA_FROM_L3MISS );    //L3miss
    l1Miss         = pdb->get( context, PSC_PM_LD_MISS_L1 );          //L1miss
    srqFull        = pdb->get( context, PSC_PM_LSU_REJECT_STQ_FULL ); //SRQfull
    storeRef       = pdb->get( context, PSC_PM_ST_REF_L1 );           //StoreRef
    deratMissCyc   = pdb->get( context, PSC_PM_LSU_DERAT_MISS_CYC );  //DERATMissCYC
    data_from_lmem = pdb->get( context, PSC_PM_DATA_FROM_LMEM );
    cycles         = pdb->get( context, PSC_PM_RUN_CYC );
    phaseCycles    = pdb->get( phaseContext, PSC_PM_RUN_CYC );

    data_from_outside1 = l3Miss - data_from_lmem;
    data_from_outside2 = l3Ref - data_from_lmem;
    stallCycles        = ( double )l1MissCyc / ( double )phaseCycles * 100.0;
    severities.push_back( stallCycles );
    dL1Pref       = ( double )l1Pref / ( double )l1RefBytes * 100.0;
    dL2Pref       = ( double )l2Pref / ( double )l1RefBytes * 100.0;
    dL3MissRate   = ( double )l3Miss / ( double )l3Ref * 100.0;
    dL2MissRate   = ( double )l3Ref / ( double )l1Miss * 100.0;
    dL1MissRate   = ( double )l1Miss / ( double )l1Ref * 100.0;
    dSRQ          = ( double )srqFull / ( double )storeRef * 100.0;
    dAvgL1Cyc     = ( double )l1MissCyc / ( double )l1Miss;
    dDERATMissCYC = ( double )deratMissCyc / ( double )phaseCycles * 100.0;
    severities.push_back( dDERATMissCYC );
    dSRQCyc = dSRQ * 0.65;
    //severities.push_back(dSRQCyc);
    impact       = ( double )cycles / ( double )phaseCycles * 100.0;
    mbytes       = ( l1RefBytes + 8 * storeRef ) / 1000000;
    bandwidth    = ( ( double )mbytes ) / ( double )cycles * NANOSEC_PER_SEC_DOUBLE;
    reuse_factor = 100 - dL1Pref;

    /*if (INTERVAL(0,dL1Pref,80) && INTERVAL(0,dL1MissRate,0.5))
       mem_cases[REUSE] = true;
       if (INTERVAL(80, dL1Pref, 120) && INTERVAL(80, dL2Pref, 120) && INTERVAL(90, dL3MissRate, 110))
       mem_cases[STREAMING] = true;
       if (INTERVAL(120, dL1Pref,999999) || INTERVAL(120, dL2Pref, 999999))
       mem_cases[NON_LOCAL] = true;
       if (INTERVAL(30, dSRQ, 110))
       mem_cases[ST_BOUND] = true;
     */
    if( INTERVAL( 0, dL1Pref, 80 ) && INTERVAL( 5000, bandwidth, 99999999 ) ) {
        mem_cases[ REUSE ] = true;
    }
    if( INTERVAL( 150, dAvgL1Cyc, 9999999 ) && INTERVAL( 0, bandwidth, 5000 ) ) {
        mem_cases[ STREAMING ] = true;
    }
    if( INTERVAL( 120, dL1Pref, 999999 ) || INTERVAL( 120, dL2Pref, 999999 ) ) {
        mem_cases[ NON_LOCAL ] = true;
    }
    if( INTERVAL( 30, dSRQ, 110 ) ) {
        mem_cases[ ST_BOUND ] = true;
    }

    mem_cases[ NONE ] = true;
    for( int i = 0; i < NUM_CASE - 1; i++ ) {
        if( mem_cases[ i ] ) {
            mem_cases[ NONE ] = false;
        }
    }

    /*if (!(dL1Pref > (100 - deviation1) && dL1Pref < (100 + deviation1) && dL2Pref > (100 - deviation1) && dL2Pref < (100 + deviation1)))
       mem_cases[PREF_FAIL] = true;
       if (dAvgL1Cyc > 100 && dL1MissRate < deviation3 && dL3MissRate > 20)
       mem_cases[STREAMING] = true;
       if (dL2Pref > (100 + deviation2))
       mem_cases[NON_LOCAL] = true;
       if (dL3MissRate > (100 - deviation2))
       mem_cases[NO_CACHE_REUSAGE] = true;
       if (dSRQ > deviation2)
       mem_cases[ST_BOUND] = true;
       if (dAvgL1Cyc < 60 && dAvgL1Cyc > 15)
       mem_cases[L2_FIT] = true;
       if (dL1MissRate > deviation3)
       mem_cases[L1_MISS_HIGH] = true;*/
}

Property* p6HighLevelMem::clone() {
    p6HighLevelMem* prop = new p6HighLevelMem( context, phaseContext );
    return prop;
}

std::string p6HighLevelMem::info() {
    std::stringstream stream;

    return stream.str();
}
;

std::string p6HighLevelMem::toXMLExtra() {
    std::stringstream stream;
    stream << "\t\t<L1MissCYC>" << l1MissCyc << "</L1MissCYC>" << std::endl;
    stream << "\t\t<l1Ref>" << l1Ref << "</l1Ref>" << std::endl;
    stream << "\t\t<l1Miss>" << l1Miss << "</l1Miss>" << std::endl;
    stream << "\t\t<l3Ref>" << l3Ref << "</l3Ref>" << std::endl;
    stream << "\t\t<l3Miss>" << l3Miss << "</l3Miss>" << std::endl;
    stream << "\t\t<data_from_outside1>" << data_from_outside1 << "</data_from_outside1>" << std::endl;
    stream << "\t\t<data_from_outside2>" << data_from_outside2 << "</data_from_outside2>" << std::endl;
    stream << "\t\t<bandwidth>" << bandwidth << "</bandwidth>" << std::endl;
    stream << "\t\t<mbytes>" << mbytes << "</mbytes>" << std::endl;
    stream << "\t\t<dL1Pref>" << dL1Pref << "</dL1Pref>" << std::endl;
    stream << "\t\t<dL2Pref>" << dL2Pref << "</dL2Pref>" << std::endl;
    stream << "\t\t<dL1MissRate>" << dL1MissRate << "</dL1MissRate>" << std::endl;
    stream << "\t\t<dL2MissRate>" << dL2MissRate << "</dL2MissRate>" << std::endl;
    stream << "\t\t<dL3MissRate>" << dL3MissRate << "</dL3MissRate>" << std::endl;
    stream << "\t\t<dAvgL1Cyc>" << dAvgL1Cyc << "</dAvgL1Cyc>" << std::endl;
    stream << "\t\t<dCacheCyc>" << stallCycles << "</dCacheCyc>" << std::endl;
    stream << "\t\t<dDERATMissCYC>" << dDERATMissCYC << "</dDERATMissCYC>" << std::endl;
    stream << "\t\t<dSRQ>" << dSRQ << "</dSRQ>" << std::endl;
    stream << "\t\t<dSRQCyc>" << dSRQCyc << "</dSRQCyc>" << std::endl;
    stream << "\t\t<impact>" << impact << "</impact>" << std::endl;
    stream << "\t\t<cycles>" << cycles << "</cycles>" << std::endl;

    return stream.str();
}
