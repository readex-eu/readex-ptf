#include "ProgramSignature.h"

#include <sstream>


using std::map;
using std::pair;
using std::string;
using std::istringstream;
using boost::property_tree::ptree;


namespace {
inline const string& selectKey( const pair<string, INT64>& assoc ) {
    return assoc.first;
}
} /* unnamed namespace */


ProgramSignature::ProgramSignature( const addInfoType& info ) {
    for( addInfoType::const_iterator it = info.begin(); it != info.end(); it++ ) {
        string key = ( *it ).first;
        INT64  value;

        istringstream ss( ( *it ).second );
        ss >> value;

        values.insert( pair<string, INT64>( key, value ) );
    }
}

/**
 * Returns value for performance counter by name with normalization.
 */
double ProgramSignature::operator[]( string key ) const {
    return static_cast<double>( values.find( key )->second ) / values.find( "PAPI_TOT_INS" )->second;
}

ProgramSignature::const_iterator ProgramSignature::begin() const {
    return boost::make_transform_iterator( values.begin(), &selectKey );
}

ProgramSignature::const_iterator ProgramSignature::end() const {
    return boost::make_transform_iterator( values.end(), &selectKey );
}

ptree ProgramSignature::toPtree() const {
    ptree result;
    for( map<string, INT64>::const_iterator it = values.begin(); it != values.end(); ++it ) {
        string key   = ( *it ).first;
        INT64  value = ( *it ).second;
        result.put( key, value );
    }
    return result;
}

ProgramSignature ProgramSignature::fromPtree( ptree const& pt ) {
    ProgramSignature result;
    for( ptree::const_iterator it = pt.begin(); it != pt.end(); ++it ) {
        string counterName = ( *it ).first;
        INT64  value       = ( *it ).second.get_value<INT64>();

        result.values[ counterName ] = value;
    }
    return result;
}
