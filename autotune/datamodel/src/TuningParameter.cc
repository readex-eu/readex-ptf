/**
   @file    TuningParameter.cc
   @ingroup Autotune
   @brief   Tuning Parameter
   @author  Houssam Haitof
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

#include "TuningParameter.h"
#include "TuningValue.h"
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <iostream>
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

using namespace std;


TuningParameter::TuningParameter() {
    this->restriction = NULL;
}

TuningParameter::~TuningParameter() {
}

// inclusion of archive types must precede this line
BOOST_CLASS_EXPORT_IMPLEMENT( TuningParameter )

// generic serialization function
template<class Archive>
void TuningParameter::serialize( Archive& ar, const unsigned int version ) {
    ar& ID;
    ar& pluginType;
    ar& runtimeActionType;
    ar& name;
    ar& from;
    ar& to;
    ar& step;
    if( runtimeActionType == runtimeTuningActionType( TUNING_ACTION_VARIABLE_STRING ) ) {
        ar& stringValues;
    }
    ar& restriction;
}

//
// instantiations for all supported archive types
//
template void
TuningParameter::serialize<boost::archive::polymorphic_iarchive>(
    boost::archive::polymorphic_iarchive& ar,
    const unsigned int                    version );

template void
TuningParameter::serialize<boost::archive::polymorphic_oarchive>(
    boost::archive::polymorphic_oarchive& ar,
    const unsigned int                    version );

template void
TuningParameter::serialize<boost::archive::binary_iarchive>(
    boost::archive::binary_iarchive& ar,
    const unsigned int               version );

template void
TuningParameter::serialize<boost::archive::binary_oarchive>(
    boost::archive::binary_oarchive& ar,
    const unsigned int               version );

template void
TuningParameter::serialize<boost::archive::text_iarchive>(
    boost::archive::text_iarchive& ar,
    const unsigned int             version );

template void
TuningParameter::serialize<boost::archive::text_oarchive>(
    boost::archive::text_oarchive& ar,
    const unsigned int             version );

long TuningParameter::getId() const {
    return this->ID;
}

void TuningParameter::setId( long id ) {
    this->ID = id;
}

void TuningParameter::setRuntimeActionType( runtimeTuningActionType actionType ) {
    this->runtimeActionType = actionType;
}

runtimeTuningActionType TuningParameter::getRuntimeActionType() const {
    return runtimeActionType;
}

void TuningParameter::setPluginType( tPlugin pluginType ) {
    this->pluginType = pluginType;
}

tPlugin TuningParameter::getPluginType() const {
    return pluginType;
}

int TuningParameter::getRangeFrom() const {
    return this->from;
}

int TuningParameter::getRangeTo() const {
    return this->to;
}

int TuningParameter::getRangeStep() const {
    return this->step;
}

int TuningParameter::getDefaultValue() const {
    return this->default_val;
}

void TuningParameter::setRange( int start,
                                int stop,
                                int incr ) {
    this->from = start;
    this->to   = stop;
    this->step = incr;
}

void TuningParameter::setRangeTo( int stop, int incr ) {
    this->to   = stop;
    this->step = incr;
}

void TuningParameter::setDefaultValue( int default_value ) {
    this->default_val = default_value;
}

void TuningParameter::addStringValue( string value ) {
    this->stringValues.push_back( value );
}

//const std::string *TuningParameter::getStringValue(int i) const {
//  i = i - 1;
//  if (i >= stringValues.size()) {
//    return NULL ;
//  } else {
//    return &stringValues[i];
//  }
//}

const list<string>* TuningParameter::getStringValues() const {
    return &stringValues;
}


Restriction* TuningParameter::getRestriction() const {
    return restriction;
}

void TuningParameter::setRestriction( Restriction* restriction ) {
    this->restriction = restriction;
}


string TuningParameter::getName() const {
    return this->name;
}

void TuningParameter::setName( string name ) {
    this->name = name;
}

bool TuningParameter::operator==( const TuningParameter& in ) const {
    if( ID != in.ID ||
        pluginType != in.pluginType ||
        runtimeActionType != in.runtimeActionType ||
        name.compare( in.name ) != 0 ||
        from != in.from ||
        to != in.to ||
        step != in.step ||
        *restriction != *in.restriction ) {
        return false;
    }

//  if (runtimeActionType == runtimeTuningActionType(TUNING_ACTION_VARIABLE_STRING)) {
//    vector<string>::iterator stringValue1;
//    vector<string>::iterator stringValue2;
//    for (stringValue1 = stringValues.begin(), stringValue2 = in.stringValues.begin();
//         stringValue1 != stringValues.end() && stringValue2 != in.stringValues.end();
//         stringValue1++, stringValue2++) {
//      if (stringValue1->compare(*stringValue2)) {
//        return false;
//      }
//    }
//  }
    return true;
}

bool TuningParameter::operator!=( const TuningParameter& in ) const {
    return !( *this == in );
}

string TuningParameter::toString( int    indent,
                                  string indentation_character ) {
    string base_indentation;
    for( int i = 0; i < indent; i++ ) {
        base_indentation.append( indentation_character );
    }
    stringstream temp;

    temp << base_indentation << "TuningParameter: " << getId() << endl;
    temp << base_indentation << indentation_character << "Parameter type: " << getRuntimeActionType() << endl;
    temp << base_indentation << indentation_character << "Action name:    " << getName() << endl;
    temp << base_indentation << indentation_character << "Range:          " << "(" << from << "," << to << "," << step << ")" << endl;
    if( runtimeActionType == runtimeTuningActionType( TUNING_ACTION_VARIABLE_STRING ) ) {
        list<string>::iterator stringValue;
        temp << base_indentation << indentation_character << "String Parameters: ";
        for( stringValue = stringValues.begin(); stringValue != stringValues.end(); stringValue++ ) {
            temp << *stringValue << " ";
        }
        temp << endl;
    }

    //  TODO need to check the implications of this removal -IC
//  if (vectorOfParameters != NULL) {
//
//      vector<int> elements = vectorOfParameters->getElements();
//        temp << base_indentation << indentation_character << "Vector Range Restriction:          ";
//
//
//        for ( int i = 0; i < elements.size(); i++ ) {
//                temp << elements[i] <<", ";
//        }
//
//        temp << endl;
//  }
//

    return temp.str();
}

string TuningParameter::toString() {
    return toString( 0, "\t" );
}

boost::optional<TuningValue> TuningParameter::getTuningValue( int value ) const {
    return TuningValue( pluginType, name, 0, value );
}
