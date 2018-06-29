#include "TuningDatabase.h"
#include "JsonTuningDatabase.h"
#include "Sqlite3TuningDatabase.h"
#include "config.h"

using std::vector;

#ifdef PSC_SQLITE3_ENABLED
static Sqlite3TuningDatabase sqlite3_tdb( "tuning.db" );
TuningDatabase*              tdb = &sqlite3_tdb;
#else /* PSC_SQLITE3_ENABLED */
static JsonTuningDatabase json_tdb( "database", "measurements" );
TuningDatabase*           tdb = &json_tdb;
#endif /* PSC_SQLITE3_ENABLED */

TuningConfiguration TuningDatabase::queryBestConfiguration( int id ) {
    boost::scoped_ptr< Iterator<TuningCase> > it( queryCases( id ) );
    if( !it->hasNext() ) {
        fprintf( stderr, "TuningDatabase::queryBestConfiguration: no configurations found for %d\n", id );
        throw std::runtime_error( "TuningDatabase::queryBestConfiguration: no configurations found" );
    }
    return it->next().first;
}

vector<TuningConfiguration> TuningDatabase::queryConfigurationsByRatio( int id, double r ) {
    vector<TuningConfiguration> result;
    double                      minExecTime;

    boost::scoped_ptr< Iterator<TuningCase> > it( queryCases( id ) );
    if( it->hasNext() ) {
        TuningCase const& case_ = it->next();

        TuningConfiguration const& config   = case_.first;
        double                     execTime = case_.second;

        result.push_back( config );
        minExecTime = execTime;
    }
    else {
        return vector<TuningConfiguration>();
    }

    while( it->hasNext() ) {
        TuningCase const& case_ = it->next();

        TuningConfiguration const& config   = case_.first;
        double                     execTime = case_.second;

        if( execTime > r * minExecTime ) {
            break;
        }
        result.push_back( config );
    }
    return result;
}
