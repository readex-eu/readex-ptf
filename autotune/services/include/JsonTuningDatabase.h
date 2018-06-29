#ifndef JSONTUNINGDATABASE_H_
#define JSONTUNINGDATABASE_H_

#include "TuningDatabase.h"

#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>

class JsonTuningDatabase : public TuningDatabase {
    std::string dstPrefix;

    std::vector<ProgramSignature>          signatures;
    std::vector< std::vector<TuningCase> > casez;

    boost::property_tree::ptree signatureRecords, tuningCaseRecords;

public:
    JsonTuningDatabase( std::string srcPrefix,
                        std::string dstPrefix_ );

    Iterator<int>* queryPrograms();

    ProgramSignature querySignature( int id );

    Iterator<TuningCase>* queryCases( int id );

    void saveSignature( ProgramID const&        id,
                        ProgramSignature const& signature );

    void saveTuningCase( ProgramID const&  id,
                         TuningCase const& tc );

    void commit();
};

#endif /* JSONTUNINGDATABASE_H_ */
