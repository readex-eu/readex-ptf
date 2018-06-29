/**
   @file	PerformanceDynamics.cc
   @ingroup AnalysisAgent
   @brief   Dynamic performance analysis property
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
#include <iostream>
#include <sstream>
#include <vector>

#include "PerformanceDynamics.hpp"
#include "global.h"
#include "PropertyID.h"
#include "psc_errmsg.h"

#include "Degradation.hpp"

#include <algorithm>

#include <TDA_Vector.hpp>
#include <TDA_ScaleSpaceAnalyzer.hpp>
#include <TDA_IOUtils.hpp>
#include <TDA_SSFEngine.hpp>
#include <TDA_QSequence.hpp>


using namespace std;



PerformanceDynamics::PerformanceDynamics( Property* staticProperty, TDA_Stuff& tda_data, double threshold ) :
    Property( staticProperty->get_context() ), staticProperty( staticProperty ), threshold( threshold ), tda_data( tda_data ) {
    cout << "Candidate property " << name() << " created" << endl;
    severity_val = 0.0;
}

PropertyID PerformanceDynamics::id() {
    return PERFORMANCEDYNAMICS;
}

std::string PerformanceDynamics::subId() {
    std::stringstream sstr;
    sstr << staticProperty->id();

    return sstr.str();
}

void PerformanceDynamics::print() {
    stringstream str;

    str << "Property:" << name() << "  Process " << context->getRank()
        << "  Thread " << context->getThread()
        << context->getRegion()->str_print();

    string help = str.str();
    psc_dbgmsg( 4, "%s\n", help.c_str() );
}

bool PerformanceDynamics::condition() const {
    bool result = false;

    if( !changes.empty() ) {
        result = true;
    }

    return result;
}

double PerformanceDynamics::confidence() const {
    return 1.0;
}

double PerformanceDynamics::severity() const {
    return 0.0;
}

Gather_Required_Info_Type PerformanceDynamics::request_metrics() {
    return ALL_INFO_GATHERED;
}

std::string PerformanceDynamics::name() {
    std::stringstream stream;
    stream << "Significant dynamics in the severity of the property: \"" << staticProperty->name() << "\"";

    return stream.str();
}

void PerformanceDynamics::evaluate() {
    std::cout << "Evaluating " << name() << std::endl;
    stringstream id;
    id << "timeseries.";
    id << staticProperty->get_region()->get_ident().file_id << ".";
    id << staticProperty->get_region()->get_ident().rfl << ".";
    id << staticProperty->get_region()->get_name() << ".";
    id << staticProperty->get_rank() << ".";
    id << staticProperty->get_thread() << ".";
    id << staticProperty->id();
    id << staticProperty->subId();
    std::cout << "In: " << id.str() << std::endl;

    if( psc_get_debug_level() >= 6 ) {
        TDA_Plot                 p3;
        std::list<TDA_ZCContour> zcc;
        tda_data.ssa.getCZC( zcc );
        p3.addZCContours( zcc );

        std::vector<TDA_QInterval*> qi;
        qi = tda_data.ssa.getQIntervals();
        p3.addQIntervals( qi );
        p3.showLabels();
        wait_for_enter();
    }

    std::vector<TDA_QEdge*> edges = tda_data.qd.getAllEdges();

    TDA_QEdgePattern  edge_pattern;
    TDA_QBlockPattern left_block_pattern, right_block_pattern;
    left_block_pattern.addDesiredType( D_TYPE );
    left_block_pattern.addDesiredType( B_TYPE );
    right_block_pattern.addDesiredType( A_TYPE );
    right_block_pattern.addDesiredType( C_TYPE );
    edge_pattern.setLeftBlockPattern( left_block_pattern );
    edge_pattern.setRightBlockPattern( right_block_pattern );
    edge_pattern.setAccStabilityLowThr( 50 );
    edge_pattern.setAbsAccDyLowerThr( 0.05 );

    std::cout << "Searching for a pattern: " << edge_pattern.toString() << std::endl;
    for( int i = 0; i < edges.size(); i++ ) {
        if( edge_pattern.matchQEdge( edges[ i ] ) ) {
            std::cout << "Change found: " << edges[ i ]->toString() << std::endl;
            changes.push_back( edges[ i ] );
        }
    }

    if( psc_get_debug_level() >= 6 ) {
        TDA_Plot          p;
        std::stringstream title;
        title << "Found changes";
        p.addLine( tda_data.vector, title.str() );
        p.fixXRange();
        p.fixYRange();
        p.addQEdges( changes );
        wait_for_enter();
    }
}


std::string PerformanceDynamics::info() {
    std::stringstream stream;

    stream << "empty";

    return stream.str();
}

Prop_List PerformanceDynamics::next() {
    Prop_List new_candidates;
    if( !condition() ) {
        return new_candidates;         //empty
    }
    for( int i = 0; i < changes.size(); i++ ) {
        new_candidates.push_back( new Degradation( staticProperty, tda_data, changes[ i ], threshold ) );
    }

    return new_candidates;
}

Property* PerformanceDynamics::clone() {
    PerformanceDynamics* prop = new PerformanceDynamics( staticProperty, tda_data );
    return prop;
}

std::string PerformanceDynamics::toXMLExtra() {
    std::stringstream          stream;
    std::list<INT64>::iterator sample;
    stream << "\t\t<Severity>" << severity() << "</Severity>" << std::endl;

    return stream.str();
}
