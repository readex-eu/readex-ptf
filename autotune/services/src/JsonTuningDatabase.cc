#include "JsonTuningDatabase.h"

#include <algorithm>
#include <ctime>
#include <fstream>
#include <sstream>
#include <boost/property_tree/json_parser.hpp>

using std::make_pair;
using std::vector;
using boost::property_tree::ptree;


struct Counter : Iterator<int>{
    int i, size;
    Counter( int size_ ) : i( 0 ), size( size_ ) {
    }
    bool hasNext() const {
        return i < size;
    }
    int next() {
        return i++;
    }
};

template<typename T>
struct VectorIterator : Iterator<T>{
    std::vector<T> const& vec;
    size_t                i;

    explicit VectorIterator( std::vector<T> const& vec_ ) : vec( vec_ ), i( 0 ) {
    }

    bool hasNext() const {
        return i < vec.size();
    }
    T next() {
        return vec[ i++ ];
    }
};


bool lessTuningCase( TuningCase const& c1,
                     TuningCase const& c2 ) {
    return c1.second < c2.second;
}


Iterator<int>* JsonTuningDatabase::queryPrograms() {
    return new Counter( signatures.size() );
}

ProgramSignature JsonTuningDatabase::querySignature( int id ) {
    return signatures[ id ];
}

Iterator<TuningCase>* JsonTuningDatabase::queryCases( int id ) {
    return new VectorIterator<TuningCase>( casez[ id ] );
}

void JsonTuningDatabase::saveSignature( ProgramID const&        id,
                                        ProgramSignature const& signature ) {
    ptree signatureRecord;
    signatureRecord.put( "id", id.toString() );
    signatureRecord.add_child( "signature", signature.toPtree() );
    signatureRecords.push_back( make_pair( "", signatureRecord ) );
}

void JsonTuningDatabase::saveTuningCase( ProgramID const&  id,
                                         TuningCase const& tc ) {
    TuningConfiguration const& config   = tc.first;
    double                     execTime = tc.second;

    ptree tuningCaseRecord;
    tuningCaseRecord.put( "id", id.toString() );
    tuningCaseRecord.add_child( "configuration", config.toPtree() );
    tuningCaseRecord.put( "exectime", execTime );
    tuningCaseRecords.push_back( make_pair( "", tuningCaseRecord ) );
}

JsonTuningDatabase::JsonTuningDatabase( std::string srcPrefix,
                                        std::string dstPrefix_ ) : dstPrefix( dstPrefix_ ) {
    std::ifstream in( ( srcPrefix + ".json" ).c_str() );
    if( !in.is_open() ) {
        return;
    }

    ptree obj;
    boost::property_tree::read_json( in, obj );
    in.close();

    for( ptree::const_iterator it = obj.begin(); it != obj.end(); ++it ) {
        ptree const& benchmark = ( *it ).second;

        signatures.push_back( ProgramSignature::fromPtree( benchmark.get_child( "signature" ) ) );

        ptree const&       configurationsPtree = benchmark.get_child( "tuningcases" );
        vector<TuningCase> cases;
        for( ptree::const_iterator it = configurationsPtree.begin(); it != configurationsPtree.end(); ++it ) {
            ptree const& casePtree = ( *it ).second;
            cases.push_back( TuningCase( TuningConfiguration::fromPtree( casePtree.get_child( "configuration" ) ), casePtree.get<double>( "exectime" ) ) );
        }
        std::sort( cases.begin(), cases.end(), &lessTuningCase );
        casez.push_back( cases );
    }
}

void JsonTuningDatabase::commit() {
    ptree obj;
    obj.add_child( "signatures", signatureRecords );
    obj.add_child( "tuningcases", tuningCaseRecords );
    std::ostringstream ss;
    ss << dstPrefix << '.' << std::time( 0 ) << ".json";
    boost::property_tree::write_json( ss.str().c_str(), obj );
}
