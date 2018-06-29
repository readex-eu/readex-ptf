#ifndef _PROGRAMSIGNATURE_H
#define _PROGRAMSIGNATURE_H

#include <map>
#include <string>

#include <boost/iterator/transform_iterator.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>

#include "global.h"
#include "MetaProperty.h"

class ProgramSignature {
    std::map<std::string, INT64> values;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& values;
    }

public:
    typedef boost::transform_iterator<const std::string& ( * )( const std::pair<std::string, INT64>& ),
                                      std::map<const std::string, INT64>::const_iterator> const_iterator;

    ProgramSignature() {
    }

    explicit ProgramSignature( const addInfoType& info );

    explicit ProgramSignature( const std::map<std::string, INT64>& values_ ) : values( values_ ) {
    }

    double operator[]( std::string key ) const;

    /**
     * Returns value for performance counter by name without normalization.
     */
    INT64 rawAt( std::string key ) const {
        return values.find( key )->second;
    }

    size_t size() const {
        return values.size();
    }

    const_iterator begin() const;

    const_iterator end() const;

    boost::property_tree::ptree toPtree() const;

    static ProgramSignature fromPtree( boost::property_tree::ptree const& );

    const std::map<std::string, INT64>& getValues() const {
        return values;
    }
};

#endif /* _PROGRAMSIGNATURE_H */
