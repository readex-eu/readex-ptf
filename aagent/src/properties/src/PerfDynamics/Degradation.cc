/**
   @file    Degradation.cc
   @ingroup DynamicProperties
   @brief   Dynamic performance analysis property
   @author  Yury Oleynik
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2015, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include <iostream>
#include <sstream>
#include <vector>

#include "Degradation.hpp"
#include "global.h"
#include "PropertyID.h"
#include "psc_errmsg.h"

#include <algorithm>

#include <TDA_Vector.hpp>
#include <TDA_ScaleSpaceAnalyzer.hpp>
#include <TDA_IOUtils.hpp>
#include <TDA_SSFEngine.hpp>


using namespace std;

Degradation::Degradation( Property*  staticProperty,
                          TDA_Stuff& tda_data,
                          TDA_QEdge* edge,
                          double     threshold ) :
    Property( staticProperty->get_context() ),
    staticProperty( staticProperty ),
    threshold( threshold ),
    edge( edge ),
    tda_data( tda_data ) {
    cout << "Candidate property " << name() << " created" << endl;
    severity_val = 0.0;

    if( edge == NULL ) {
        throw std::invalid_argument( "Edge can not be NULL" );
    }
}

PropertyID Degradation::id() {
    return DEGRADATION;
}

std::string Degradation::subId() {
    std::stringstream sstr;
    sstr << staticProperty->id();

    return sstr.str();
}

void Degradation::print() {
    stringstream str;

    str << "Property:" << name() << "  Process " << context->getRank()
        << "  Thread " << context->getThread()
        << context->getRegion()->str_print();

    string help = str.str();
    psc_dbgmsg( 4, "%s\n", help.c_str() );
}

bool Degradation::condition() const {
    bool result = false;

    if( severity_val > threshold ) {
        result = true;
    }

    return result;
}

double Degradation::confidence() const {
    return 1.0;
}

double Degradation::severity() const {
    return severity_val;
}

Gather_Required_Info_Type Degradation::request_metrics() {
    return ALL_INFO_GATHERED;
}

std::string Degradation::name() {
    std::stringstream stream;
    if( most_stable_sequence.isEmpty() ) {
        stream << "Degradation in the severity of the property: " << staticProperty->name();
    }
    else {
        stream << "Degradation through iterations " << most_stable_sequence.getLeftX()
               << ":" << most_stable_sequence.getRightX()
               << " in the severity of the property: " << staticProperty->name();
    }

    return stream.str();
}

void Degradation::evaluate() {
    std::cout << std::endl << "Evaluating " << name() << std::endl;
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

    most_stable_sequence = TDA_QSequence();
    severity_val         = 0.0;

    double accumulated_dy_lower_threshold = 0.1;

    TDA_QBlockPattern left, right;
    left.addDesiredType( D_TYPE );
    right.addDesiredType( A_TYPE );
    TDA_QSequencePattern seq_qualitative_pattern;
    seq_qualitative_pattern.addQBlockPattern( left );
    seq_qualitative_pattern.addQBlockPattern( right );
    seq_qualitative_pattern.setAccDyLowerThr( accumulated_dy_lower_threshold );


    std::vector<TDA_QSequence> neighborhoods = edge->getAdjustedBlockSequences();
    for( int j = 0; j < neighborhoods.size(); j++ ) {
        if( seq_qualitative_pattern.matchSequence( neighborhoods[ j ] ) ) {
            if( most_stable_sequence.isEmpty() ) {
                most_stable_sequence = neighborhoods[ j ];
            }

            if( neighborhoods[ j ].getStability() > most_stable_sequence.getStability() ) {
                most_stable_sequence = neighborhoods[ j ];
            }
        }
    }

    if( most_stable_sequence.isEmpty() ) {
        return;
    }

    std::cout << "Found neighborhood:" << most_stable_sequence.toString();
    int left_x          = most_stable_sequence.getLeftX();
    int scale_to_filter = most_stable_sequence.getLowScale();

    std::cout << " Parameters for calculating the Severity are: x="
              << left_x << " scale=" << scale_to_filter << std::endl;

    TDA_SSFEngine ssfe( tda_data.vector );
    ssfe.goToScale( scale_to_filter );

    double degradation_impact_value = ssfe.getCumulativeIncrease( left_x );
    double dyn_sev_fraction         = degradation_impact_value / tda_data.vector.getArea();
    severity_val = dyn_sev_fraction * staticProperty->severity();
    std::cout << "Dynamics fraction=" << dyn_sev_fraction << ", static severity="
              << staticProperty->severity() << ", calculated severity="
              << severity_val << std::endl;

    std::vector<TDA_QEdge*> edges_to_plot;
    edges_to_plot.push_back( most_stable_sequence[ 0 ]->getLeftEdge() );

/*	TDA_Plot p2;
        p2.addQEdges(edges_to_plot);
        ssfe.plotCurrentScale(p2);
        wait_for_enter();
 */
}


std::string Degradation::info() {
    std::stringstream stream;

    stream << "empty";

    return stream.str();
}

Prop_List Degradation::next() {
    Prop_List new_candidates;
    if( !condition() ) {
        return new_candidates;         //empty
    }
    return new_candidates;
}

Property* Degradation::clone() {
    Degradation* prop = new Degradation( staticProperty, tda_data, edge );
    return prop;
}

std::string Degradation::toXMLExtra() {
    std::stringstream          stream;
    std::list<INT64>::iterator sample;
    stream << "\t\t<Severity>" << severity() << "</Severity>" << std::endl;

    return stream.str();
}
