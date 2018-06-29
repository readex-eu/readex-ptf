#ifndef SQLITE3TUNINGDATABASE_H_
#define SQLITE3TUNINGDATABASE_H_

#include "config.h"
#include "TuningDatabase.h"

#ifdef PSC_SQLITE3_ENABLED

#include <sqlite3.h>

class sqlite3_error : public std::runtime_error {
public:
    explicit sqlite3_error( const string& arg ) : runtime_error( arg ) {
    }
};


class Sqlite3TuningDatabase : public TuningDatabase {
public:
    explicit Sqlite3TuningDatabase( std::string filename );

    ~Sqlite3TuningDatabase();

    Iterator<int>* queryPrograms();

    ProgramSignature querySignature( int id );

    Iterator<TuningCase>* queryCases( int id );

    void saveSignature( ProgramID const&        id,
                        ProgramSignature const& signature );

    void saveTuningCase( ProgramID const&  id,
                         TuningCase const& tc );

    void commit();

private:
    sqlite3* db; ///< SQLite3 database connection

    void disableAutocommit();

    int queryIdByText( ProgramID const& text );

    boost::optional<int>selectBenchmark( ProgramID const& text );

    int insertBenchmark( ProgramID const& text );

    void insertCountersEntry( int         id,
                              std::string feature,
                              INT64       value );

    int insertMeasurementEntry( int    bid,
                                double execTime );

    void insertTuningValueEntry( int                mid,
                                 TuningValue const& tv );
};

#endif /* PSC_SQLITE3_ENABLED */

#endif /* SQLITE3TUNINGDATABASE_H_ */
