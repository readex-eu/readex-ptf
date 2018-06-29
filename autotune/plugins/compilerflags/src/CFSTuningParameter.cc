/**
   @file    CFSTuningParameter.cc
   @ingroup CompilerFlagsPlugin
   @brief   Compiler Flag Selection Tuning Parameter
   @author  Michael Gerndt
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

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include "CFSTuningParameter.h"

using boost::property_tree::ptree;


// inclusion of archive types must precede this line
BOOST_CLASS_EXPORT_IMPLEMENT( CFSTuningParameter )

// generic serialization function
template<class Archive>
void CFSTuningParameter::serialize( Archive& ar, const unsigned int version ) {
    ar& boost::serialization::base_object<TuningParameter>( *this );
    ar& valueStrings;
    ar& flagString;
}

//
// instantiations for all supported archive types
//
template void
CFSTuningParameter::serialize<boost::archive::polymorphic_iarchive>(
    boost::archive::polymorphic_iarchive& ar,
    const unsigned int                    version );

template void
CFSTuningParameter::serialize<boost::archive::polymorphic_oarchive>(
    boost::archive::polymorphic_oarchive& ar,
    const unsigned int                    version );

template void
CFSTuningParameter::serialize<boost::archive::binary_iarchive>(
    boost::archive::binary_iarchive& ar,
    const unsigned int               version );

template void
CFSTuningParameter::serialize<boost::archive::binary_oarchive>(
    boost::archive::binary_oarchive& ar,
    const unsigned int               version );

template void
CFSTuningParameter::serialize<boost::archive::text_iarchive>(
    boost::archive::text_iarchive& ar,
    const unsigned int             version );

template void
CFSTuningParameter::serialize<boost::archive::text_oarchive>(
    boost::archive::text_oarchive& ar,
    const unsigned int             version );


void CFSTuningParameter::addValueString( string str ) {
    valueStrings.push_back( str );
}

const std::string* CFSTuningParameter::getValueString( int i ) const {
    i = i - 1;
    if( i >= valueStrings.size() ) {
        return NULL;
    }
    else {
        return &valueStrings[ i ];
    }
}

void CFSTuningParameter::setFlagString( string str ) {
    flagString = str;
}

void CFSTuningParameter::removeEmptyValues() {
    for( int i = 0; i < valueStrings.size(); i++ ) {
        if( boost::trim_copy( valueStrings[ i ] ).empty() ) {
            valueStrings.erase( valueStrings.begin() + i-- );
            setRange( getRangeFrom(), getRangeTo() - 1, getRangeStep() );
        }
    }
}


const std::string* CFSTuningParameter::getFlagString() const {
    return &flagString;
}

string CFSTuningParameter::getFlagWithValue( int i ) const {
    string str;
    if( getValueString( i ) == 0 ) {
        str = static_cast<ostringstream&>( ostringstream() << i ).str();
    }
    else {
        str = *getValueString( i );
    }
    return *getFlagString() + str;
}

boost::optional<TuningValue> CFSTuningParameter::getTuningValue( int value ) const {
    string flag;
    int    intVal;

    if( getValueString( value ) ) {
        flag   = *getFlagString() + *getValueString( value );
        intVal = TuningValue::NULL_VALUE;
    }
    else {
        flag   = *getFlagString();
        intVal = value;
    }
    boost::trim( flag );

    if( flag.empty() ) {
        return boost::none;
    }

    return TuningValue( CFS, flag, 0, intVal );
}

string CFSTuningParameter::toString() {
    string ret_value;
    cout << "TuningParameter details:" << endl;
    cout << "ID:             " << this->getId() << endl;
    cout << "Plugin type:    " << this->getPluginType() << endl;
    cout << "Parameter type: " << this->getRuntimeActionType() << endl;
    cout << "Action name:    " << this->getName() << endl;
    cout << "Range:          " << "(" << this->getRangeFrom() << "," << this->getRangeTo() << "," << this->getRangeStep() << ")" << endl;
    if( !flagString.empty() ) {
        cout << "Flag String:    " << "<" << flagString << ">" << endl;
    }
    if( valueStrings.size() > 0 ) {
        cout << "Values:         ";
    }
    for( int i = 0; i < valueStrings.size(); i++ ) {
        cout << valueStrings[ i ];
        if( i < valueStrings.size() - 1 ) {
            cout << ", ";
        }
    }
    cout << endl << endl;
    return ret_value;
}
