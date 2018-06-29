#ifndef TUNINGVALUE_H_
#define TUNINGVALUE_H_

#include <string>
#include <boost/property_tree/ptree.hpp>
#include "TuningParameter.h"

struct TuningValueExtension {
    virtual ~TuningValueExtension() {
    }
    virtual int  compare( TuningValueExtension const& ) const  = 0;
    virtual void toPtree( boost::property_tree::ptree& ) const = 0;
};

class TuningValue {
public:
    // tuning parameter identification
    tPlugin     pluginType;
    std::string name;

    // extensions
    TuningValueExtension* extension;

    // tuning value
    int value;

    static const int NULL_VALUE;

    TuningValue( tPlugin type_, std::string name_, TuningValueExtension* ext_, int value_ )
        : pluginType( type_ ), name( name_ ), extension( ext_ ), value( value_ ) {
    }

    int compare( const TuningValue& tv ) const;

    bool operator==( const TuningValue& tv ) const {
        return compare( tv ) == 0;
    }

    bool operator!=( const TuningValue& tv ) const {
        return compare( tv ) != 0;
    }

    bool operator<( const TuningValue& tv ) const {
        return compare( tv ) < 0;
    }

    bool operator>( const TuningValue& tv ) const {
        return compare( tv ) > 0;
    }

    bool operator<=( const TuningValue& tv ) const {
        return compare( tv ) <= 0;
    }

    bool operator>=( const TuningValue& tv ) const {
        return compare( tv ) >= 0;
    }

    boost::property_tree::ptree toPtree() const;

    static TuningValue fromPtree( boost::property_tree::ptree const& );
};

typedef TuningValueExtension*(*TuningValueParser)( boost::property_tree::ptree const& );

void registerTuningValueParser( std::string, TuningValueParser );

struct TuningValueParserRegistrator {
    TuningValueParserRegistrator( const char* typeName, TuningValueParser parser ) {
        registerTuningValueParser( typeName, parser );
    }
};

#endif /* TUNINGVALUE_H_ */
