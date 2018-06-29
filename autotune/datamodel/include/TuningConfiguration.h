#ifndef TUNINGCONFIGURATION_H_
#define TUNINGCONFIGURATION_H_

#include <set>

#include "TuningValue.h"
#include "Variant.h"

class TuningConfiguration {
    std::set<TuningValue> values;

public:
    typedef std::set<TuningValue>::const_iterator const_iterator;

    TuningConfiguration() {
    }

    TuningConfiguration( Variant const& variant );

    void add( TuningValue const& tv ) {
        values.insert( tv );
    }

    const_iterator begin() const {
        return values.begin();
    }

    const_iterator end() const {
        return values.end();
    }

    bool has( TuningValue const& tv ) const {
        return values.find( tv ) != values.end();
    }

    bool operator==( const TuningConfiguration& other ) const {
        return values == other.values;
    }

    bool operator!=( const TuningConfiguration& other ) const {
        return values != other.values;
    }

    bool operator<( const TuningConfiguration& other ) const {
        return values <  other.values;
    }

    boost::property_tree::ptree toPtree() const;

    static TuningConfiguration fromPtree( boost::property_tree::ptree const& );
};

#endif /* TUNINGCONFIGURATION_H_ */
