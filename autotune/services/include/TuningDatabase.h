#ifndef TUNINGDATABASE_H_
#define TUNINGDATABASE_H_

#include <vector>

#include "ProgramID.h"
#include "ProgramSignature.h"
#include "TuningConfiguration.h"


/**
 * Iterator base class.
 *
 * Example usage:
 *   boost::scoped_ptr< Iterator<T> > it(pointer to iterator instance);
 *   while (it->hasNext()) {
 *     T value = it->next();
 *     do something with value
 *   }
 *
 * For details, see Iterator pattern in Gamma et al: Design Patterns.
 */
template<typename T>
class Iterator {
public:
    virtual ~Iterator() {
    }

    virtual bool hasNext() const = 0;

    virtual T next() = 0;
};


/**
 * TuningCase = (TuningConfiguration, execution time)
 */
typedef std::pair<TuningConfiguration, double> TuningCase;


class TuningDatabase {
public:
    virtual ~TuningDatabase() {
    }

    /**
     * Query all programs from the database.
     *
     * @return iterator to integer IDs
     */
    virtual Iterator<int>* queryPrograms() = 0;

    /**
     * Query signature of program by integer ID.
     */
    virtual ProgramSignature querySignature( int id ) = 0;

    /**
     * Query stored tuning cases of program by integer ID.
     *
     * @return iterator to tuning cases in INCREASING ORDER by execution time
     */
    virtual Iterator<TuningCase>* queryCases( int id ) = 0;

    /**
     * Query the best stored configuration for given program by integer ID.
     */
    TuningConfiguration queryBestConfiguration( int id );

    /**
     * Query best configurations of program by integer ID.
     *
     * @return vector of configurations for which execution time
     *         is at most 'r' times the best execution time.
     */
    std::vector<TuningConfiguration>queryConfigurationsByRatio( int    id,
                                                                double r );


    /**
     * Save signature into database.
     *
     * @param id program identification
     * @param signature program signature to save
     */
    virtual void saveSignature( ProgramID const&        id,
                                ProgramSignature const& signature ) = 0;

    /**
     * Save tuning case into database.
     *
     * @param id program identification
     * @param tc tuning case to save
     */
    virtual void saveTuningCase( ProgramID const&  id,
                                 TuningCase const& tc ) = 0;

    /**
     * Commit changes into database.
     */
    virtual void commit() = 0;
};


/**
 * Global reference to tuning database.
 */
extern TuningDatabase* tdb;


#endif /* TUNINGDATABASE_H_ */
