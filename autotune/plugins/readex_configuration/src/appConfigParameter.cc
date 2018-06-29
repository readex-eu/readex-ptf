/**
   @file    appConfigParameter.cc
   @ingroup readex_configuration_Plugin
   @brief   Application Configuration Tuning Parameter
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
#include "AppConfigParameter.h"

using boost::property_tree::ptree;


// inclusion of archive types must precede this line
BOOST_CLASS_EXPORT_IMPLEMENT( AppConfigParameter )

// generic serialization function
template<class Archive>
void AppConfigParameter::serialize( Archive& ar, const unsigned int version ) {
    ar& boost::serialization::base_object<TuningParameter>( *this );
    ar& valueStrings;
    ar& filePath;
}

//
// instantiations for all supported archive types
//
template void
AppConfigParameter::serialize<boost::archive::polymorphic_iarchive>(
    boost::archive::polymorphic_iarchive& ar,
    const unsigned int                    version );

template void
AppConfigParameter::serialize<boost::archive::polymorphic_oarchive>(
    boost::archive::polymorphic_oarchive& ar,
    const unsigned int                    version );

template void
AppConfigParameter::serialize<boost::archive::binary_iarchive>(
    boost::archive::binary_iarchive& ar,
    const unsigned int               version );

template void
AppConfigParameter::serialize<boost::archive::binary_oarchive>(
    boost::archive::binary_oarchive& ar,
    const unsigned int               version );

template void
AppConfigParameter::serialize<boost::archive::text_iarchive>(
    boost::archive::text_iarchive& ar,
    const unsigned int             version );

template void
AppConfigParameter::serialize<boost::archive::text_oarchive>(
    boost::archive::text_oarchive& ar,
    const unsigned int             version );


void AppConfigParameter::addValueString( string str ) {
    valueStrings.push_back( str );
}

const std::string* AppConfigParameter::getValueString( int i ) const {
    i = i - 1;
    if( i >= valueStrings.size() ) {
        return NULL;
    }
    else {
        return &valueStrings[ i ];
    }
}

void AppConfigParameter::setfilePath( string str ) {
    filePath = str;
}

const std::string AppConfigParameter::getfilePath() const {
    return filePath;
}

void AppConfigParameter::setTemplateFilePath( string str ) {
    templateFilePath = str;
}

const std::string AppConfigParameter::getTemplateFilePath() const {
    return templateFilePath;
}

void AppConfigParameter::removeEmptyValues() {
    for( int i = 0; i < valueStrings.size(); i++ ) {
        if( boost::trim_copy( valueStrings[ i ] ).empty() ) {
            valueStrings.erase( valueStrings.begin() + i-- );
            setRange( getRangeFrom(), getRangeTo() - 1, getRangeStep() );
        }
    }
}



string AppConfigParameter::toString() {
    string ret_value;
    cout << "TuningParameter details:" << endl;
    cout << "ID:             " << this->getId() << endl;
    cout << "Plugin type:    " << this->getPluginType() << endl;
    cout << "Parameter type: " << this->getRuntimeActionType() << endl;
    cout << "Parameter name: " << this->getName() << endl;
    cout << "Template file:  " << this->getTemplateFilePath() << endl;
    cout << "Input file:     " << this->getfilePath() << endl;
    cout << "Range:          " << "(" << this->getRangeFrom() << "," << this->getRangeTo() << "," << this->getRangeStep() << ")" << endl;
    if( !filePath.empty() ) {
        cout << "File path:    " << "<" << filePath << ">" << endl;
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
