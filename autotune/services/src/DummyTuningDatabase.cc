#include "DummyTuningDatabase.h"

#include <boost/property_tree/json_parser.hpp>


template<typename T>
struct Nothing : Iterator<T>{
    bool hasNext() const {
        return false;
    }
    T next() {
        throw 0;
    }
};


Iterator<int>* DummyTuningDatabase::queryPrograms() {
    return new Nothing<int>();
}

ProgramSignature DummyTuningDatabase::querySignature( int id ) {
    throw 0;
}

Iterator<TuningCase>* DummyTuningDatabase::queryCases( int id ) {
    return new Nothing<TuningCase>;
}


void DummyTuningDatabase::saveSignature( ProgramID const&        id,
                                         ProgramSignature const& signature ) {
    std::cerr << "===>  DummyTuningDatabase::saveSignature" << std::endl;
    std::cerr << "id: \"" << id.toString() << "\"" << std::endl;
    std::cerr << "signature: ";
    boost::property_tree::write_json( cerr, signature.toPtree() );
    std::cerr << std::endl;
}

void DummyTuningDatabase::saveTuningCase( ProgramID const&  id,
                                          TuningCase const& tc ) {
    TuningConfiguration const& config   = tc.first;
    double                     execTime = tc.second;

    std::cerr << "===>  DummyTuningDatabase::saveTuningCase" << std::endl;
    std::cerr << "id: \"" << id.toString() << "\"" << std::endl;
    std::cerr << "configuration: ";
    boost::property_tree::write_json( cerr, config.toPtree() );
    std::cerr << std::endl;
    std::cerr << "execTime: " << execTime << std::endl;
}

void DummyTuningDatabase::commit() {
    // nothing to do
}
