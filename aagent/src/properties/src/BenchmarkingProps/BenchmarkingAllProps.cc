/**
   @file    BenchmarkingAllProps.cc
   @ingroup BenchmarkingProperties
   @brief   All metrics requested for benchmarking
   @author  Diana Gudu
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */
#include "BenchmarkingAllProps.h"

#include "Property.h"
#include <iostream>
#include "global.h"
#include "Metric.h"
#include "Context.h"
#include "PropertyID.h"
#include <sstream>
#include "psc_errmsg.h"

BenchmarkingAllProps::BenchmarkingAllProps( Context* ct, Context* phaseCt, double threshold )
    : Property( ct ), phaseCycles( 0 ), stallCycles( 0 ), phaseContext( phaseCt ), threshold( threshold ), importance( 0.0 ) {
    metricNames.insert( std::pair<int, std::string>( 0, "condBranchIns" ) );
    metricNames.insert( std::pair<int, std::string>( 1, "branchIns" ) );
    metricNames.insert( std::pair<int, std::string>( 2, "brMispredicted" ) );

    metricNames.insert( std::pair<int, std::string>( 3, "fpIns" ) );
    metricNames.insert( std::pair<int, std::string>( 4, "intIns" ) );
    metricNames.insert( std::pair<int, std::string>( 5, "L1DCA" ) );
    metricNames.insert( std::pair<int, std::string>( 6, "L1DCH" ) );
    metricNames.insert( std::pair<int, std::string>( 7, "L1DCM" ) );

    metricNames.insert( std::pair<int, std::string>( 8, "L2DCA" ) );
    metricNames.insert( std::pair<int, std::string>( 9, "L2DCH" ) );
    metricNames.insert( std::pair<int, std::string>( 10, "L2DCM" ) );

    metricNames.insert( std::pair<int, std::string>( 11, "L3DCA" ) );
    metricNames.insert( std::pair<int, std::string>( 12, "L3DCH" ) );
    metricNames.insert( std::pair<int, std::string>( 13, "L3DCM" ) );

    metricNames.insert( std::pair<int, std::string>( 14, "memInst" ) );
    metricNames.insert( std::pair<int, std::string>( 15, "cycStalledAnyRes" ) );
    metricNames.insert( std::pair<int, std::string>( 16, "totTLBMisses" ) );
    metricNames.insert( std::pair<int, std::string>( 17, "totCyc" ) );
    metricNames.insert( std::pair<int, std::string>( 18, "totInsIssued" ) );
    metricNames.insert( std::pair<int, std::string>( 19, "totInsCompleted" ) );

    for( int i = 0; i < 20; i++ ) {
        metrics[ i ] = 0;
    }
}

BenchmarkingAllProps::~BenchmarkingAllProps() {
}

PropertyID BenchmarkingAllProps::id() {
    return BENCHALLPROPS;
}

void BenchmarkingAllProps::print() {
    std::cout << "Property:" << name() <<
    "  Process " << context->getRank() <<
    "  Thread " <<  context->getThread() << std::endl << "                  " <<
    context->getRegion()->str_print() <<
    std::endl;
}

bool BenchmarkingAllProps::condition() const {
    INT64 sum = 0;
    for( int i = 0; i < 20; i++ ) {
        sum += metrics[ i ];
    }
    return sum != threshold;
}

double BenchmarkingAllProps::confidence() const {
    return 1.0;
}

double BenchmarkingAllProps::severity() const {
    return 100.0;
}

Context* BenchmarkingAllProps::get_phaseContext() {
    return phaseContext;
}


Gather_Required_Info_Type BenchmarkingAllProps::request_metrics() {
    pdb->request( context, PSC_PAPI_BR_CN );  //not avail on pwr6
    pdb->request( context, PSC_PAPI_BR_INS );
    pdb->request( context, PSC_PAPI_BR_MSP );

    pdb->request( context, PSC_PAPI_FP_OPS );
    pdb->request( context, PSC_PAPI_INT_INS );

    pdb->request( context, PSC_PAPI_L1_DCA );
    pdb->request( context, PSC_PAPI_L1_DCH );  //not avail on pwr6
    pdb->request( context, PSC_PAPI_L1_DCM );

    pdb->request( context, PSC_PAPI_L2_DCA );  //not avail on pwr6
    pdb->request( context, PSC_PAPI_L2_DCH );  //not avail on pwr6
    pdb->request( context, PSC_PAPI_L2_DCM );

    pdb->request( context, PSC_PAPI_L3_DCA );  //not avail on pwr6
    pdb->request( context, PSC_PAPI_L3_DCH );  //not avail on pwr6
    pdb->request( context, PSC_PAPI_L3_DCM );

    pdb->request( context, PSC_PAPI_LST_INS );
    pdb->request( context, PSC_PAPI_RES_STL ); //not avail on pwr6

    pdb->request( context, PSC_PAPI_TLB_TL );  //not avail on pwr6
    pdb->request( context, PSC_PAPI_TOT_CYC );
    pdb->request( context, PSC_PAPI_TOT_IIS );
    pdb->request( context, PSC_PAPI_TOT_INS );

    return ALL_INFO_GATHERED;
}

std::string BenchmarkingAllProps::name() {
    return "Benchmarking all props";
}

void BenchmarkingAllProps::evaluate() {
    metrics[ 0 ] = pdb->get( context, PSC_PAPI_BR_CN );
    metrics[ 1 ] = pdb->get( context, PSC_PAPI_BR_INS );
    metrics[ 2 ] = pdb->get( context, PSC_PAPI_BR_MSP );

    metrics[ 3 ] = pdb->get( context, PSC_PAPI_FP_OPS );
    metrics[ 4 ] = pdb->get( context, PSC_PAPI_INT_INS );

    metrics[ 5 ] = pdb->get( context, PSC_PAPI_L1_DCA );
    metrics[ 6 ] = pdb->get( context, PSC_PAPI_L1_DCH );
    metrics[ 7 ] = pdb->get( context, PSC_PAPI_L1_DCM );

    metrics[ 8 ]  = pdb->get( context, PSC_PAPI_L2_DCA );
    metrics[ 9 ]  = pdb->get( context, PSC_PAPI_L2_DCH );
    metrics[ 10 ] = pdb->get( context, PSC_PAPI_L2_DCM );

    metrics[ 11 ] = pdb->get( context, PSC_PAPI_L3_DCA );
    metrics[ 12 ] = pdb->get( context, PSC_PAPI_L3_DCH );
    metrics[ 13 ] = pdb->get( context, PSC_PAPI_L3_DCM );

    metrics[ 14 ] = pdb->get( context, PSC_PAPI_LST_INS );
    metrics[ 15 ] = pdb->get( context, PSC_PAPI_RES_STL );

    metrics[ 16 ] = pdb->get( context, PSC_PAPI_TLB_TL );
    metrics[ 17 ] = pdb->get( context, PSC_PAPI_TOT_CYC );
    metrics[ 18 ] = pdb->get( context, PSC_PAPI_TOT_IIS );
    metrics[ 19 ] = pdb->get( context, PSC_PAPI_TOT_INS );
}

Property* BenchmarkingAllProps::clone() {
    BenchmarkingAllProps* prop = new BenchmarkingAllProps( context, phaseContext );
    return prop;
}


Prop_List BenchmarkingAllProps::next() {
    Prop_List returnList;
    return returnList;
}

std::string BenchmarkingAllProps::info() {
    std::stringstream stream;

    stream << '\t' << " Cycles: " << metrics[ 17 ];

    return stream.str();
};

std::string BenchmarkingAllProps::toXMLExtra() {
    std::stringstream stream;

    for( int i = 0; i < 20; i++ ) {
        //printf("metric %s: %d \n", metricNames[i].c_str(), metrics[i]);
        if( metrics[ i ] == -1 ) {
            stream << "\t\t<" << metricNames[ i ].c_str() << ">" << "NOT_AVAILABLE" << "</" << metricNames[ i ].c_str() << ">" << std::endl;
        }
        else {
            stream << "\t\t<" << metricNames[ i ].c_str() << ">" << metrics[ i ] << "</" << metricNames[ i ].c_str() << ">" << std::endl;
        }
    }
    return stream.str();
}
