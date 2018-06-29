#include "Sqlite3TuningDatabase.h"

#include "psc_errmsg.h"
#include <set>

#ifdef PSC_SQLITE3_ENABLED

/*
 * Helper classes for iteration
 */

class BenchmarkIterator : public Iterator<int>{
public:
    explicit BenchmarkIterator( sqlite3_stmt* statement_ );

    bool hasNext() const {
        return hasNext_;
    }
    int next();

private:
    boost::shared_ptr<sqlite3_stmt> statement;
    bool                            hasNext_;

    void step();
};

class CaseIterator : public Iterator<TuningCase>{
public:
    CaseIterator( sqlite3*      db_,
                  sqlite3_stmt* statement_ );

    bool hasNext() const {
        return hasNext_;
    }
    TuningCase next();

private:
    sqlite3*                        db;
    boost::shared_ptr<sqlite3_stmt> statement;
    bool                            hasNext_;

    void step();
};

class RemoveDuplicates : public Iterator<TuningCase>{
public:
    explicit RemoveDuplicates( Iterator<TuningCase>* source_ );

    bool hasNext() const {
        return hasValue;
    }
    TuningCase next();

private:
    boost::scoped_ptr< Iterator<TuningCase> > source;
    bool                                      hasValue;
    TuningCase                                value;
    std::set<TuningConfiguration>             configurations;

    void step();
};


/**
 * Opens database, or creates it and sets up its schema, if it does not exist yet.
 *
 * @param filename Location of the database file.
 */
Sqlite3TuningDatabase::Sqlite3TuningDatabase( std::string filename ) : db( NULL ) {
    if( sqlite3_open( "tuning.db", &db ) != SQLITE_OK ) {
        psc_errmsg( "cannot open database: %s\n", sqlite3_errmsg( db ) );
        throw sqlite3_error( "cannot open database" );
    }

    static const char schema[] =
        "CREATE TABLE IF NOT EXISTS benchmarks (\n"
        "    id INTEGER PRIMARY KEY,\n"
        "    text TEXT UNIQUE\n"
        ");\n"

        "CREATE TABLE IF NOT EXISTS counters (\n"
        "    id INTEGER NOT NULL,\n"
        "    name TEXT NOT NULL,\n"
        "    value INTEGER NOT NULL,\n"
        "    FOREIGN KEY(id) REFERENCES benchmarks(id)\n"
        ");\n"

        "CREATE TABLE IF NOT EXISTS measurements (\n"
        "    id INTEGER PRIMARY KEY,\n"
        "    bid INTEGER NOT NULL,\n"
        "    time REAL NOT NULL,\n"
        "    FOREIGN KEY(bid) REFERENCES benchmarks(id)\n"
        ");\n"

        "CREATE TABLE IF NOT EXISTS configurations (\n"
        "    mid INTEGER NOT NULL,\n"
        "    plugintype INTEGER NOT NULL,\n"
        "    name TEXT,\n"
        "    value INTEGER,\n"
        "    FOREIGN KEY(mid) REFERENCES measurements(id),\n"
        "    UNIQUE(mid, plugintype, name)\n"
        ");\n"
    ;

    char* errmsg;
    if( sqlite3_exec( db, schema, NULL, NULL, &errmsg ) != SQLITE_OK ) {
        psc_errmsg( "fail to create schema: %s\n", errmsg );
        sqlite3_free( errmsg );
        throw sqlite3_error( "fail to create schema" );
    }
}

/**
 * Closes database connection.
 */
Sqlite3TuningDatabase::~Sqlite3TuningDatabase() {
    sqlite3_close( db );
}


Iterator<int>* Sqlite3TuningDatabase::queryPrograms() {
    static const char zSql[] =
        "SELECT id "
        "FROM benchmarks;";

    sqlite3_stmt* statement = NULL;
    if( sqlite3_prepare_v2( db, zSql, -1, &statement, NULL ) != SQLITE_OK ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }
    return new BenchmarkIterator( statement );
}

ProgramSignature Sqlite3TuningDatabase::querySignature( int id ) {
    static const char zSql[] =
        "SELECT name, CAST(AVG(value) AS INTEGER) "
        "FROM counters "
        "WHERE id = ? "
        "GROUP BY name;";

    sqlite3_stmt* pStmt = NULL;
    int           rc    = sqlite3_prepare_v2( db, zSql, -1, &pStmt, NULL );

    boost::shared_ptr<sqlite3_stmt> statement( pStmt, &sqlite3_finalize );
    if( rc != SQLITE_OK ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }

    if( sqlite3_bind_int( statement.get(), 1, id ) != SQLITE_OK ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }

    std::map<std::string, INT64> values;
    for(;; ) {
        int rc = sqlite3_step( statement.get() );
        if( rc == SQLITE_DONE ) {
            break;
        }
        if( rc != SQLITE_ROW ) {
            throw sqlite3_error( sqlite3_errmsg( db ) );
        }

        std::string name  = reinterpret_cast<const char*>( sqlite3_column_text( statement.get(), 0 ) );
        INT64       value = sqlite3_column_int64( statement.get(), 1 );

        values[ name ] = value;
    }
    return ProgramSignature( values );
}

Iterator<TuningCase>* Sqlite3TuningDatabase::queryCases( int id ) {
    static const char zSql[] =
        "SELECT id, time "
        "FROM measurements "
        "WHERE bid = ? "
        "ORDER BY time;";

    sqlite3_stmt* statement = NULL;
    if( sqlite3_prepare_v2( db, zSql, -1, &statement, NULL ) != SQLITE_OK ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }

    if( sqlite3_bind_int( statement, 1, id ) != SQLITE_OK ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }

    return new RemoveDuplicates( new CaseIterator( db, statement ) );
}


void Sqlite3TuningDatabase::saveSignature( ProgramID const&        id,
                                           ProgramSignature const& signature ) {
    disableAutocommit();

    int bid = queryIdByText( id );
    for( ProgramSignature::const_iterator it = signature.begin(); it != signature.end(); ++it ) {
        std::string key   = *it;
        INT64       value = signature.rawAt( key );
        insertCountersEntry( bid, key, value );
    }
}

void Sqlite3TuningDatabase::saveTuningCase( ProgramID const&  id,
                                            TuningCase const& tc ) {
    disableAutocommit();

    TuningConfiguration const& configuration = tc.first;
    double                     execTime      = tc.second;

    int bid = queryIdByText( id );
    int mid = insertMeasurementEntry( bid, execTime );

    for( TuningConfiguration::const_iterator it = configuration.begin(); it != configuration.end(); ++it ) {
        insertTuningValueEntry( mid, *it );
    }
}

void Sqlite3TuningDatabase::commit() {
    char* errmsg;
    if( !sqlite3_get_autocommit( db ) ) {
        if( sqlite3_exec( db, "COMMIT;", NULL, NULL, &errmsg ) != SQLITE_OK ) {
            throw sqlite3_error( errmsg );
        }
    }
}


/**
 * Starts a transaction, unless one has already been started.
 *
 * This is used by saveSignature() and saveTuningCase() to make sure
 * that all gathered training data is saved atomically, when calling commit().
 */
void Sqlite3TuningDatabase::disableAutocommit() {
    char* errmsg;
    if( sqlite3_get_autocommit( db ) ) {
        if( sqlite3_exec( db, "BEGIN;", NULL, NULL, &errmsg ) != SQLITE_OK ) {
            throw sqlite3_error( errmsg );
        }
    }
}

/**
 * Turns a ProgramID into the internal integer id of the database
 * using the table 'benchmarks'.
 *
 * ProgramID is added to 'benchmarks' if it has not been added yet.
 */
int Sqlite3TuningDatabase::queryIdByText( ProgramID const& text ) {
    boost::optional<int> id0 = selectBenchmark( text );
    if( !id0 ) {
        insertBenchmark( text );
        id0 = selectBenchmark( text );
    }
    return id0.get();
}

/**
 * Tries to look up ProgramID in 'benchmarks', returns integer id if found.
 *
 * @return integer id, or boost::none if ProgramID is not found.
 */
boost::optional<int> Sqlite3TuningDatabase::selectBenchmark( ProgramID const& id ) {
    static const char zSql[] =
        "SELECT id "
        "FROM benchmarks "
        "WHERE text = ?;";

    sqlite3_stmt* pStmt = NULL;
    int           rc    = sqlite3_prepare_v2( db, zSql, -1, &pStmt, NULL );

    boost::shared_ptr<sqlite3_stmt> statement( pStmt, &sqlite3_finalize );
    if( rc != SQLITE_OK ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }

    std::string text = id.toString();
    if( sqlite3_bind_text( statement.get(), 1, text.c_str(), text.length(), SQLITE_TRANSIENT ) != SQLITE_OK ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }

    switch( sqlite3_step( statement.get() ) ) {
    case SQLITE_ROW:
        return sqlite3_column_int( statement.get(), 0 );
    case SQLITE_DONE:
        return boost::none;
    default:
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }
}

/**
 * Insert ProgramID into table 'benchmarks'.
 *
 * @return rowid
 */
int Sqlite3TuningDatabase::insertBenchmark( ProgramID const& id ) {
    static const char zSql[] =
        "INSERT INTO benchmarks(text) "
        "VALUES (?);";

    sqlite3_stmt* pStmt = NULL;
    int           rc    = sqlite3_prepare_v2( db, zSql, -1, &pStmt, NULL );

    boost::shared_ptr<sqlite3_stmt> statement( pStmt, &sqlite3_finalize );
    if( rc != SQLITE_OK ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }

    std::string text = id.toString();
    if( sqlite3_bind_text( statement.get(), 1, text.c_str(), text.length(), SQLITE_TRANSIENT ) != SQLITE_OK ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }
    if( sqlite3_step( statement.get() ) != SQLITE_DONE ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }
    return sqlite3_last_insert_rowid( db );
}


/**
 * Insert a (benchmark id, feature name, value) entry into the 'counters' table.
 */
void Sqlite3TuningDatabase::insertCountersEntry( int id, std::string feature, INT64 value ) {
    static const char zSql[] =
        "INSERT INTO counters "
        "VALUES (?, ?, ?);";

    sqlite3_stmt* pStmt = NULL;
    int           rc    = sqlite3_prepare_v2( db, zSql, -1, &pStmt, NULL );

    boost::shared_ptr<sqlite3_stmt> statement( pStmt, &sqlite3_finalize );
    if( rc != SQLITE_OK ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }

    if( sqlite3_bind_int( statement.get(), 1, id ) != SQLITE_OK ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }
    if( sqlite3_bind_text( statement.get(), 2, feature.c_str(), feature.length(), SQLITE_TRANSIENT ) != SQLITE_OK ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }
    if( sqlite3_bind_int64( statement.get(), 3, value ) != SQLITE_OK ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }
    if( sqlite3_step( statement.get() ) != SQLITE_DONE ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }
}

/**
 * Insert a (benchmark id, measurement id, execution time) entry into the 'measurements' table.
 *
 * @param bid benchmark id
 * @param execTime execution time
 * @return measurement id
 */
int Sqlite3TuningDatabase::insertMeasurementEntry( int    bid,
                                                   double execTime ) {
    static const char zSql[] =
        "INSERT INTO measurements(bid, time) "
        "VALUES (?, ?);";

    sqlite3_stmt* pStmt = NULL;
    int           rc    = sqlite3_prepare_v2( db, zSql, -1, &pStmt, NULL );

    boost::shared_ptr<sqlite3_stmt> statement( pStmt, &sqlite3_finalize );
    if( rc != SQLITE_OK ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }

    if( sqlite3_bind_int( statement.get(), 1, bid ) != SQLITE_OK ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }
    if( sqlite3_bind_double( statement.get(), 2, execTime ) != SQLITE_OK ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }
    if( sqlite3_step( statement.get() ) != SQLITE_DONE ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }
    return sqlite3_last_insert_rowid( db );
}

/**
 * Insert a TuningValue corresponding to a measurement into the 'configurations' table.
 *
 * @param mid measurement id
 * @param tv TuningValue of the TuningConfiguration used to measure execution time
 */
void Sqlite3TuningDatabase::insertTuningValueEntry( int                mid,
                                                    TuningValue const& tv ) {
    static const char zSql[] =
        "INSERT INTO configurations "
        "VALUES (?, ?, ?, ?);";

    sqlite3_stmt* pStmt = NULL;
    int           rc    = sqlite3_prepare_v2( db, zSql, -1, &pStmt, NULL );

    boost::shared_ptr<sqlite3_stmt> statement( pStmt, &sqlite3_finalize );
    if( rc != SQLITE_OK ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }

    if( sqlite3_bind_int( statement.get(), 1, mid ) != SQLITE_OK ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }
    if( sqlite3_bind_int( statement.get(), 2, tv.pluginType ) != SQLITE_OK ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }
    if( sqlite3_bind_text( statement.get(), 3, tv.name.c_str(), tv.name.length(), SQLITE_TRANSIENT ) != SQLITE_OK ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }
    if( tv.extension ) {
        psc_errmsg( "Sqlite3TuningDatabase cannot handle TuningValue extensions" );
    }
    if( tv.value == TuningValue::NULL_VALUE ) {
        if( sqlite3_bind_null( statement.get(), 4 ) != SQLITE_OK ) {
            throw sqlite3_error( sqlite3_errmsg( db ) );
        }
    }
    else {
        if( sqlite3_bind_int( statement.get(), 4, tv.value ) != SQLITE_OK ) {
            throw sqlite3_error( sqlite3_errmsg( db ) );
        }
    }
    if( sqlite3_step( statement.get() ) != SQLITE_DONE ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }
}


/*
 * Implementation of iterator classes
 */

BenchmarkIterator::BenchmarkIterator( sqlite3_stmt* statement_ )
    : statement( statement_, &sqlite3_finalize ), hasNext_( false ) {
    step();
}

int BenchmarkIterator::next() {
    int result = sqlite3_column_int( statement.get(), 0 );
    return step(), result;
}

void BenchmarkIterator::step() {
    switch( sqlite3_step( statement.get() ) ) {
    case SQLITE_ROW:
        hasNext_ = true;
        break;
    case SQLITE_DONE:
        hasNext_ = false;
        break;
    default:
        throw sqlite3_error( "fail to retrieve next row" );
    }
}


CaseIterator::CaseIterator( sqlite3*      db_,
                            sqlite3_stmt* statement_ )
    : db( db_ ), statement( statement_, &sqlite3_finalize ), hasNext_( false ) {
    step();
}

void CaseIterator::step() {
    switch( sqlite3_step( statement.get() ) ) {
    case SQLITE_ROW:
        hasNext_ = true;
        break;
    case SQLITE_DONE:
        hasNext_ = false;
        break;
    default:
        throw sqlite3_error( "fail to retrieve next row" );
    }
}

TuningCase CaseIterator::next() {
    int    mid      = sqlite3_column_int( statement.get(), 0 );
    double execTime = sqlite3_column_double( statement.get(), 1 );

    static const char zSql[] =
        "SELECT plugintype, name, value "
        "FROM configurations "
        "WHERE mid = ?;";

    sqlite3_stmt* pStmt = NULL;
    int           rc    = sqlite3_prepare_v2( db, zSql, -1, &pStmt, NULL );

    boost::shared_ptr<sqlite3_stmt> configStmt( pStmt, &sqlite3_finalize );
    if( rc != SQLITE_OK ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }

    if( sqlite3_bind_int( configStmt.get(), 1, mid ) != SQLITE_OK ) {
        throw sqlite3_error( sqlite3_errmsg( db ) );
    }

    TuningConfiguration configuration;
    for(;; ) {
        int rc = sqlite3_step( configStmt.get() );
        if( rc == SQLITE_DONE ) {
            break;
        }
        if( rc != SQLITE_ROW ) {
            throw sqlite3_error( sqlite3_errmsg( db ) );
        }

        tPlugin pluginType = static_cast<tPlugin>( sqlite3_column_int( configStmt.get(), 0 ) );

        std::string name = reinterpret_cast<const char*>( sqlite3_column_text( configStmt.get(), 1 ) );

        int value;
        if( sqlite3_column_type( configStmt.get(), 2 ) == SQLITE_NULL ) {
            value = TuningValue::NULL_VALUE;
        }
        else {
            value = sqlite3_column_int( configStmt.get(), 2 );
        }

        configuration.add( TuningValue( pluginType, name, 0, value ) );
    }

    return step(), TuningCase( configuration, execTime );
}


RemoveDuplicates::RemoveDuplicates( Iterator<TuningCase>* source_ ) : source( source_ ) {
    hasValue = source->hasNext();
    if( hasValue ) {
        value = source->next();
    }
}

TuningCase RemoveDuplicates::next() {
    TuningCase result = value;
    return step(), result;
}

void RemoveDuplicates::step() {
    hasValue = false;
    configurations.insert( value.first );

    while( source->hasNext() ) {
        TuningCase nextValue = source->next();
        if( configurations.find( nextValue.first ) == configurations.end() ) {
            hasValue = true;
            value    = nextValue;
            break;
        }
    }
}

#endif /* PSC_SQLITE3_ENABLED */
