/**
   @file    Context.cc
   @ingroup AnalysisAgent
   @brief   Execution context
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

#include "Context.h"
#include "psc_errmsg.h"

Context::Context ( Region* reg_p, int rank_p, int thread_p ): aa_rts_id( 0 ) {
    reg       = reg_p;
    rank      = rank_p;
    thread    = thread_p;
    rtsBased  = false;
    aa_rts_id = 0;
}

Context::Context( Rts* rts_node, int rank_p, int thread_p ) {
    rts       = rts_node;
    reg       = rts_node->getRegion();
    rank      = rank_p;
    thread    = thread_p;
    rtsBased  = true;
    aa_rts_id = rts_node->getRtsID();
}

Context::Context( int fileId_p, int rlf_p, int rank_p, int thread_p ): aa_rts_id( 0 ) {
    reg = appl->searchRegion( fileId_p, rlf_p );
    if( !reg ) {
        psc_errmsg( "Region not found in context creation (%d,%d)!\n", fileId_p, rlf_p );
    }
    rank     = rank_p;
    thread   = thread_p;
    rtsBased = false;
}

Context::~Context() {
}

std::string Context::getCallpath() {
    if ( rtsBased ) {
        return rts->getCallPath();
    } else {
        psc_errmsg( "RTS context expected.\n" );
        return "";
    }
}

Context* Context::copy() {
    if ( rtsBased ) {
        return new Context( rts, rank, thread );
    } else {
        return new Context( reg, rank, thread );
    }
}

int Context::getFileId() {
    Region* region = rtsBased ? rts->getRegion() : reg;

    return region->get_ident().file_id;
}

int Context::getRfl() {
    Region* region = rtsBased ? rts->getRegion() : reg;

    return region->get_ident().rfl;
}
int Context::getStartPosition() {
    Region* region = rtsBased ? rts->getRegion() : reg;

    return region->get_ident().start_position;
}

std::string Context::getFileName() {
    Region* region = rtsBased ? rts->getRegion() : reg;

    return region->get_ident().file_name;
}

Region* Context::getRegion() {
    return rtsBased ? rts->getRegion() : reg;
}

RegionType Context::getRegionType() {
    Region* region = rtsBased ? rts->getRegion() : reg;

    return region->get_ident().type;
}

int Context::getRank() {
    return rank;
}

int Context::getThread() {
    return thread;
}

int Context::getRtsID() {
    return aa_rts_id;
}

bool Context::isRtsBased() {
    return rtsBased;
}

std::string Context::getRegionId() {
    Region* region = rtsBased ? rts->getRegion() : reg;

    return region->getRegionID();
}

Rts* Context::getRts() {
    return rts;
}
