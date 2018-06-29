/**
   @file    PerformanceCounters.cc
   @ingroup PerformanceCounters
   @brief   Performance counters as program signature
   @author  Miklos Homolya
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


#include "PerformanceCounters.h"
#include <iostream>

#include "global.h"
#include "Metric.h"
#include "Context.h"
#include <sstream>

using std::string;
using std::stringstream;
using std::endl;

//Description: Performance counters as program signature

namespace {
Metric counterId[] = {
    PSC_PAPI_L1_DCM,
    PSC_PAPI_L1_ICM,
    PSC_PAPI_L2_DCM,
    PSC_PAPI_L3_DCM,
    PSC_PAPI_L1_TCM,
    PSC_PAPI_L2_TCM,
    PSC_PAPI_L3_TCM,
    PSC_PAPI_TLB_DM,
    PSC_PAPI_TLB_IM,
    PSC_PAPI_BR_CN,
    PSC_PAPI_BR_MSP,
    PSC_PAPI_TOT_INS,
    PSC_PAPI_FP_INS,
    PSC_PAPI_LD_INS,
    PSC_PAPI_SR_INS,
    PSC_PAPI_BR_INS,
    PSC_PAPI_TOT_CYC,
    PSC_PAPI_L2_DCH,
    PSC_PAPI_L2_DCA,
    PSC_PAPI_L3_DCA,
    PSC_PAPI_L2_ICH,
    PSC_PAPI_L2_ICR,
    PSC_PAPI_L3_TCA,
    PSC_PAPI_FP_OPS
};

const char* counterName[] = {
    "PAPI_L1_DCM",
    "PAPI_L1_ICM",
    "PAPI_L2_DCM",
    "PAPI_L3_DCM",
    "PAPI_L1_TCM",
    "PAPI_L2_TCM",
    "PAPI_L3_TCM",
    "PAPI_TLB_DM",
    "PAPI_TLB_IM",
    "PAPI_TLB_TL",
    "PAPI_L3_DCH",
    "PAPI_BR_CN",
    "PAPI_BR_MSP",
    "PAPI_TOT_IIS",
    "PAPI_TOT_INS",
    "PAPI_INT_INS",
    "PAPI_FP_INS",
    "PAPI_LD_INS",
    "PAPI_SR_INS",
    "PAPI_BR_INS",
    "PAPI_RES_STL",
    "PAPI_TOT_CYC",
    "PAPI_LST_INS",
    "PAPI_L1_DCH",
    "PAPI_L2_DCH",
    "PAPI_L1_DCA",
    "PAPI_L2_DCA",
    "PAPI_L3_DCA",
    "PAPI_L2_ICH",
    "PAPI_L2_ICR",
    "PAPI_L2_TCH",
    "PAPI_L3_TCA",
    "PAPI_FP_OPS"
};
}  /* unnamed namespace */

/**
 * @brief Constructor
 * @ingroup PerformanceCounters
 *
 * @param ctx Context
 * @param phaseCtx Phase Context (ignored)
 * @param th Threshold for condition (ignored)
 **/
PerformanceCounters::PerformanceCounters( Context* ctx, Context* phaseCtx, double th )
    : Property( ctx ), threshold( th ), execTime( 0.0 ), instructionCount( 0 )
    , features( sizeof( counterId ) / sizeof( counterId[ 0 ] ) ) {
    phaseContext = phaseCtx;
}

/**
 * @brief Returns a clone of the property.
 * @ingroup PerformanceCounters
 *
 * @return Clone of a property
 */
Property* PerformanceCounters::clone() {
    PerformanceCounters* clone_ = new PerformanceCounters( context, phaseContext, threshold );
    clone_->execTime         = execTime;
    clone_->instructionCount = instructionCount;
    clone_->features         = features;
    return clone_;
}

/**
 * @brief Returns the performance property going to be reported.
 * @ingroup PerformanceCounters
 *
 * @return true if performance property is going to be reported
 */
bool PerformanceCounters::condition() const {
    return instructionCount > 0 && severity() > threshold;
}

/**
 * @brief Returns the degree of confidence about the existence of a performance property.
 * @ingroup PerformanceCounters
 *
 * @return the degree of confidence about the existence of a performance property.
 */
double PerformanceCounters::confidence() const {
    return 1.0;
}

/**
 * @brief Returns a severity figure that specifies the importance of the property.
 * @ingroup PerformanceCounters
 *
 * Returns a severity figure that specifies the importance of the property.
 * The higher this figure the more important or severe a performance property is.
 * The severity can be used to concentrate first on the most severe performance
 * property during the performance tuning process.
 *
 * @return Severity of a property
 */
double PerformanceCounters::severity() const {
    return execTime;
}

/**
 * @brief Returns a unique ID for the property
 * @ingroup PerformanceCounters
 *
 * @return Property ID
 */
PropertyID PerformanceCounters::id() {
    return PERFORMANCECOUNTERS;
}

/**
 * @brief Returns the name of the property.
 * @ingroup PerformanceCounters
 *
 * @return Name of the property
 */
string PerformanceCounters::name() {
    return "Performance Counters";
}

/**
 * @brief Requests the information required to evaluate the property.
 * @ingroup PerformanceCounters
 *
 * Requests the information required to evaluate the property from the
 * performance database.
 *
 * @return information about the gathered info type
 */
Gather_Required_Info_Type PerformanceCounters::request_metrics() {
    pdb->request( context, PSC_EXECUTION_TIME );
    pdb->request( context, PSC_PAPI_TOT_INS );

    int nCounters = sizeof( counterId ) / sizeof( counterId[ 0 ] );
    for( int i = 0; i < nCounters; i++ ) {
        pdb->request( context, counterId[ i ] );
    }

    return ALL_INFO_GATHERED;
}

/**
 * @brief Evaluates a property.
 * @ingroup PerformanceCounters
 *
 * Takes the information from the performance database the information about the
 * execution for a property. Evaluates the property from this information.
 */
void PerformanceCounters::evaluate() {
    INT64 cycles = pdb->get( context, PSC_EXECUTION_TIME );
    if( cycles > ( INT64 )0 ) {
        execTime = cycles / NANOSEC_PER_SEC_DOUBLE;
    }
    else {
        execTime = 0.0;
    }

    instructionCount = pdb->get( context, PSC_PAPI_TOT_INS );

    int nCounters = sizeof( counterId ) / sizeof( counterId[ 0 ] );
    for( int i = 0; i < nCounters; i++ ) {
        features[ i ] = pdb->get( context, counterId[ i ] );
    }
}

/**
 * @brief Returns information about the property as a stream.
 * @ingroup PerformanceCounters
 *
 * Returns the information about the property as a stream.
 *
 * @return information about the property
 */
string PerformanceCounters::info() {
    stringstream stream;

    stream << '\t' << "Execution Time: " << execTime << endl;
    int nCounters = sizeof( counterId ) / sizeof( counterId[ 0 ] );
    for( int i = 0; i < nCounters; i++ ) {
        if( features[ i ] != -1 ) {
            stream << "\t" << counterName[ i ] << ": " << features[ i ] << endl;
        }
    }

    return stream.str();
}

/**
 * @brief Returns extra information about the property as a XML stream.
 * @ingroup PerformanceCounters
 *
 * Returns extra information about the property as a XML stream. This information
 * is used to transfer non standard information about the property.
 *
 * @return Extra XML information about the property
 */
string PerformanceCounters::toXMLExtra() {
    stringstream stream;

    int nCounters = sizeof( counterId ) / sizeof( counterId[ 0 ] );
    for( int i = 0; i < nCounters; i++ ) {
        if( features[ i ] != -1 ) {
            stream << "\t\t";
            stream << "<" << counterName[ i ] << ">";
            stream << features[ i ];
            stream << "</" << counterName[ i ] << ">";
            stream << endl;
        }
    }

    return stream.str();
}
