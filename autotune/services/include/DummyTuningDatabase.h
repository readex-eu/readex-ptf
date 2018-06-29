#ifndef DUMMYTUNINGDATABASE_H_
#define DUMMYTUNINGDATABASE_H_

#include "TuningDatabase.h"

class DummyTuningDatabase : public TuningDatabase {
public:
    Iterator<int>* queryPrograms();

    ProgramSignature querySignature( int id );

    Iterator<TuningCase>* queryCases( int id );

    void saveSignature( ProgramID const&        id,
                        ProgramSignature const& signature );

    void saveTuningCase( ProgramID const&  id,
                         TuningCase const& tc );

    void commit();
};

#endif /* DUMMYTUNINGDATABASE_H_ */
