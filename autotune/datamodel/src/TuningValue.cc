#include "TuningValue.h"

#include <climits>
#include <map>

using std::map;
using std::string;
using boost::property_tree::ptree;

const int TuningValue::NULL_VALUE = INT_MIN;

int TuningValue::compare( const TuningValue& tv ) const {
    // Compare pluginType and ID first
    if( pluginType != tv.pluginType ) {
        return ( pluginType < tv.pluginType ) ? -1 : 1;
    }
    if( int retval = name.compare( tv.name ) ) {
        return retval;
    }

    // Compare extensions
    if( extension != tv.extension ) {
        if( extension == 0 ) {
            return -1;
        }
        if( tv.extension == 0 ) {
            return 1;
        }

        if( typeid( *extension ) != typeid( *tv.extension ) ) {
            return typeid( *this ).before( typeid( tv ) );
        }

        if( int retval = extension->compare( *tv.extension ) ) {
            return retval;
        }
    }

    // Compare tuning value
    if( value != tv.value ) {
        return ( value < tv.value ) ? -1 : 1;
    }

    // If we reach this point, the two values are equal.
    return 0;
}

ptree TuningValue::toPtree() const {
    ptree pt;
    pt.put( "type", "generic" );
    pt.put( "plugintype", pluginType );
    pt.put( "name", name );

    if( extension ) {
        extension->toPtree( pt );
    }

    if( value != NULL_VALUE ) {
        pt.put( "value", value );
    }
    else {
        pt.put( "value", "null" );
    }
    return pt;
}

static map<string, TuningValueParser>& typeToParser() {
    static map<string, TuningValueParser> result;
    return result;
}

TuningValue TuningValue::fromPtree( ptree const& pt ) {
    TuningValue result( UNKOWN_PLUGIN, "", 0, NULL_VALUE );
    result.pluginType = static_cast<tPlugin>( pt.get<int>( "plugintype" ) );
    result.name       = pt.get<string>( "name" );

    string type = pt.get<string>( "type" );
    if( type != "generic" ) {
        result.extension = typeToParser()[ type ]( pt );
    }

    result.value = pt.get( "value", TuningValue::NULL_VALUE );
    return result;
}

void registerTuningValueParser( std::string       typeName,
                                TuningValueParser parser ) {
    typeToParser()[ typeName ] = parser;
}
