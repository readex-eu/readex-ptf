/**
   @file    p6HotSpotMem.cc
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

#include "p6HotSpotMem.h"
#include "p6DPUHeldFXMulDivTot.h"
#include "p6DPUHeldGPR.h"
#include "p6L1MissCYC.h"
#include "p6DPUHeldFPQ.h"
#include "p6DPUHeldFPUCR.h"
#include "p6DPUHeldSMT.h"
#include "p6DERATMissCYC.h"
#include "p6LSULHSReject.h"
#include "p6DPUStall.h"
#include "p6DPUStallOther.h"
#include "p6DPUStallICache.h"
#include "p6SRQFull.h"
#include "p6HighLevelMem.h"

#include "Property.h"
#include <iostream>
#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

p6HotSpotMem::p6HotSpotMem( Context* ct, Context* phaseCt ) :
    Property( ct ) {
    phaseCycles  = 0;
    stallCycles  = 0;
    cycles       = 0;
    importance   = 0.0;
    threshold    = 1.0;
    phaseContext = phaseCt;
    P6Freq       = NANOSEC_PER_SEC;
    withMemProp  = true;
    CPI          = 0.0;
}

p6HotSpotMem::~p6HotSpotMem() {
}

PropertyID p6HotSpotMem::id() {
    return P6HOTSPOT;
}

void p6HotSpotMem::print() {
    std::cout << "Property:" << name() << "  Process " << context->getRank() << "  Thread " << context->getThread()
              << std::endl << "                  " << context->getRegion()->str_print() << std::endl;
}

bool p6HotSpotMem::condition() const {
    psc_dbgmsg( 10, "Rank %d: %s :%s: condition=%f, severity=%f\n", phaseContext->getRank(), name().c_str(),
                get_region()->str_print().c_str(), importance, severity() );
    return ( importance > 10 ) && ( CPI >= 1.1 );
}

double p6HotSpotMem::confidence() const {
    return 1.0;
}

double p6HotSpotMem::severity() const {
    return ( double )stallCycles / ( double )phaseCycles * 100.0;
}

Context* p6HotSpotMem::get_phaseContext() {
    return phaseContext;
}

Gather_Required_Info_Type p6HotSpotMem::request_metrics() {
    pdb->request( context, PSC_PM_RUN_CYC );
    pdb->request( phaseContext, PSC_PM_RUN_CYC );
    pdb->request( context, PSC_PM_RUN_INST_CMPL );
    return ALL_INFO_GATHERED;
}

std::string p6HotSpotMem::name() {
    if( CPI == 0.0 ) {
        return "Hot spot of the application candidate property";
    }
    if( CPI > 0.0 && CPI < 0.5 ) {
        return "Hot spot of the application - suspiciously extremely high performance";
    }
    if( CPI >= 0.5 && CPI < 0.75 ) {
        return "Hot spot of the application - extremely efficient";
    }
    if( CPI >= 0.75 && CPI < 1.1 ) {
        return "Hot spot of the application - very good performance";
    }
    if( CPI >= 1.1 && CPI < 1.5 ) {
        return "Hot spot of the application - acceptable performance";
    }
    if( CPI >= 1.5 && CPI < 2.25 ) {
        return "Hot spot of the application - tuning potential on POWER6";
    }
    if( CPI >= 2.25 ) {
        return "Hot spot of the application - poor performance";
    }
    return "Hot spot of the application negative CPI???";
}

void p6HotSpotMem::evaluate() {
    phaseCycles = pdb->get( phaseContext, PSC_PM_RUN_CYC );
    stallCycles = pdb->get( context, PSC_PM_RUN_CYC );
    instr_cmpl  = pdb->get( context, PSC_PM_RUN_INST_CMPL );
    importance  = ( double )( stallCycles ) / ( double )( phaseCycles ) * 100.0;
    CPI         = ( double )( stallCycles ) / ( double )instr_cmpl;
}

Property* p6HotSpotMem::clone() {
    p6HotSpotMem* prop = new p6HotSpotMem( context, phaseContext );
    return prop;
}

Prop_List p6HotSpotMem::next() {
    Prop_List returnList;

    //returnList.push_back(new p6DPUHeldFXMulDivTot(context, phaseContext));
    //returnList.push_back(new p6DPUHeldGPR(context, phaseContext));
    returnList.push_back( new p6DPUHeldFPQ( context, phaseContext ) );
    returnList.push_back( new p6DPUHeldFPUCR( context, phaseContext ) );
    //returnList.push_back(new p6DPUHeldSMT(context, phaseContext));
    //returnList.push_back(new p6LSULHSReject(context, phaseContext));
    //returnList.push_back(new p6DPUStallICache(context, phaseContext));
    returnList.push_back( new p6DPUStallOther( context, phaseContext ) );
    returnList.push_back( new p6DPUStall( context, phaseContext ) );

    if( withMemProp ) {
        returnList.push_back( new p6HighLevelMem( context, phaseContext ) );
    }
    else {
        returnList.push_back( new p6SRQFull( context, phaseContext ) );
        returnList.push_back( new p6DERATMissCYC( context, phaseContext ) );
        returnList.push_back( new p6L1MissCYC( context, phaseContext ) );
    }
    return returnList;
}

std::string p6HotSpotMem::info() {
    std::stringstream stream;

    stream << '\t' << " Cycles: " << stallCycles;

    return stream.str();
}
;

std::string p6HotSpotMem::toXMLExtra() {
    std::stringstream stream;

    stream << "\t\t<Cycles>" << stallCycles << "</Cycles>" << std::endl;
    stream << "\t\t<Time>" << ( double )stallCycles / ( double )P6Freq << "</Time>" << std::endl;
    stream << "\t\t<phaseCycles>" << phaseCycles << "</phaseCycles>" << std::endl;
    stream << "\t\t<CPI>" << CPI << "</CPI>" << std::endl;

    return stream.str();
}
