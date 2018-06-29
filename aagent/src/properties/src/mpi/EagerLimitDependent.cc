/**
   @file    EagerLimitDependent.cc
   @ingroup MPIProperties
   @brief   MPI specific property
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

#include "EagerLimitDependent.h"
#include "global.h"
#include <iostream>
#include <sstream>
#include "psc_errmsg.h"

using namespace std;

PropertyID EagerLimitDependent::id() {
    return EAGERLIMITDEPENDENT;
}

std::string EagerLimitDependent::subId() {
    std::stringstream sstr;
    sstr << PSC_MPI_MSG_P2P_THR;
    return sstr.str();
}

void EagerLimitDependent::print() {
    stringstream str;
    str << "Property:" << name() << "  Process " << context->getRank()
        << "  Thread " << context->getThread() << context->getRegion()->str_print();
    psc_dbgmsg( 4, "%s\n", str.str().c_str() );
}

bool EagerLimitDependent::condition() const {
    return severity() > threshold;
}

double EagerLimitDependent::confidence() const {
    return 1.0;
}

double EagerLimitDependent::severity() const {
    double severity;
    if( mpi_msg_p2p_tot > 0 ) {
        severity = ( double )mpi_msg_p2p_thr / mpi_msg_p2p_tot;
        psc_dbgmsg( 9, "mpi_msg_p2p_thr=%" PRId64 "; mpi_msg_p2p_tot=%" PRId64 "; severity=%f\n", mpi_msg_p2p_thr, mpi_msg_p2p_tot, severity );
        return severity;
    }
    else {
        return 0.0;
    }
}

Gather_Required_Info_Type EagerLimitDependent::request_metrics() {
    pdb->request( context, PSC_MPI );
    pdb->request( startCtx, PSC_MPI_MSG_P2P_TOT );
    pdb->request( startCtx, PSC_MPI_MSG_P2P_THR );
    pdb->request( startCtx, PSC_MPI_MSG_FREQ_2K );
    pdb->request( startCtx, PSC_MPI_MSG_FREQ_4K );
    pdb->request( startCtx, PSC_MPI_MSG_FREQ_8K );
    pdb->request( startCtx, PSC_MPI_MSG_FREQ_16K );
    pdb->request( startCtx, PSC_MPI_MSG_FREQ_32K );
    pdb->request( startCtx, PSC_MPI_MSG_FREQ_64K );
    return ALL_INFO_GATHERED;
}

std::string EagerLimitDependent::name() {
    return "Near Eager Threshold Traffic";
}

void EagerLimitDependent::evaluate() {
    mpi_msg_p2p_tot  = pdb->get( context, PSC_MPI_MSG_P2P_TOT );
    mpi_msg_p2p_thr  = pdb->get( context, PSC_MPI_MSG_P2P_THR );
    mpi_msg_freq_2k  = pdb->get( context, PSC_MPI_MSG_FREQ_2K );
    mpi_msg_freq_4k  = pdb->get( context, PSC_MPI_MSG_FREQ_4K );
    mpi_msg_freq_8k  = pdb->get( context, PSC_MPI_MSG_FREQ_8K );
    mpi_msg_freq_16k = pdb->get( context, PSC_MPI_MSG_FREQ_16K );
    mpi_msg_freq_32k = pdb->get( context, PSC_MPI_MSG_FREQ_32K );
    mpi_msg_freq_64k = pdb->get( context, PSC_MPI_MSG_FREQ_64K );
}

std::string EagerLimitDependent::info() {
    std::stringstream stream;
    stream << '\t' << ">>Total point to point bytes transferred: " << mpi_msg_p2p_tot
           << '\t' << " Near threshold bytes transferred: " << mpi_msg_p2p_thr
           << '\t' << " 2K messages: " << mpi_msg_freq_2k
           << '\t' << " 4K messages: " << mpi_msg_freq_4k
           << '\t' << " 8K messages: " << mpi_msg_freq_8k
           << '\t' << " 16K messages: " << mpi_msg_freq_16k
           << '\t' << " 32K messages: " << mpi_msg_freq_32k
           << '\t' << " 64K messages: " << mpi_msg_freq_64k;
    return stream.str();
}

Property* EagerLimitDependent::clone() {
    EagerLimitDependent* prop = new EagerLimitDependent( context, startCtx );
    return prop;
}

std::string EagerLimitDependent::toXMLExtra() {
    std::stringstream stream;
    stream << "\t\t<mpi_msg_p2p_tot>" << mpi_msg_p2p_tot << "</mpi_msg_p2p_tot>" << std::endl;
    stream << "\t\t<mpi_msg_p2p_thr>" << mpi_msg_p2p_thr << "</mpi_msg_p2p_thr>" << std::endl;
    stream << "\t\t<mpi_msg_freq_2k>" << mpi_msg_freq_2k << "</mpi_msg_freq_2k>" << std::endl;
    stream << "\t\t<mpi_msg_freq_4k>" << mpi_msg_freq_4k << "</mpi_msg_freq_4k>" << std::endl;
    stream << "\t\t<mpi_msg_freq_8k>" << mpi_msg_freq_8k << "</mpi_msg_freq_8k>" << std::endl;
    stream << "\t\t<mpi_msg_freq_16k>" << mpi_msg_freq_16k << "</mpi_msg_freq_16k>" << std::endl;
    stream << "\t\t<mpi_msg_freq_32k>" << mpi_msg_freq_32k << "</mpi_msg_freq_32k>" << std::endl;
    stream << "\t\t<mpi_msg_freq_64k>" << mpi_msg_freq_64k << "</mpi_msg_freq_64k>" << std::endl;
    return stream.str();
}
