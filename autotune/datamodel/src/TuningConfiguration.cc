#include "TuningConfiguration.h"

using boost::property_tree::ptree;

TuningConfiguration::TuningConfiguration( Variant const& variant ) {
    map<TuningParameter*, int> vs = variant.getValue();
    for( map<TuningParameter*, int>::const_iterator it = vs.begin(); it != vs.end(); ++it ) {
        TuningParameter* param = ( *it ).first;
        int              value = ( *it ).second;

        if( boost::optional<TuningValue> tv0 = param->getTuningValue( value ) ) {
            add( tv0.get() );
        }
    }
}

ptree TuningConfiguration::toPtree() const {
    ptree pt;
    for( const_iterator it = begin(); it != end(); ++it ) {
        pt.push_back( std::make_pair( "", ( *it ).toPtree() ) );
    }
    return pt;
}

TuningConfiguration TuningConfiguration::fromPtree( ptree const& pt ) {
    TuningConfiguration result;
    for( ptree::const_iterator it = pt.begin(); it != pt.end(); ++it ) {
        result.add( TuningValue::fromPtree( ( *it ).second ) );
    }
    return result;
}
