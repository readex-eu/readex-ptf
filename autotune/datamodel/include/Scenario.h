/**
   @file    Scenario.h
   @ingroup Autotune
   @brief   Scenario Description header
   @author  Houssam Haitof
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#ifndef SCENARIO_H_
#define SCENARIO_H_

#include <vector>
#include "global.h"
#include "Variant.h"
#include "rts.h"
#include "PropertyRequest.h"
#include "TuningSpecification.h"
#include "Ranks.h"
#include "VariantContext.h"


#include <boost/serialization/list.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

// Create an Autotune group
/// @defgroup Autotune Autotune

using namespace std;

class Scenario {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& id;
        ar& description;
        ar& tuned_entity;
        ar& ts;
        ar& pr;
        ar& results;
        ar& rtsBased;
    }
private:
    static pthread_mutex_t lock;        // used to protect the ID counter when multi-threading
    static unsigned int    scenario_id; // used to compute a new ID for created instances

    unsigned int id;
    string       description;
    string       tuned_entity;

    list<TuningSpecification*>* ts;      // <contexts, ranks, variant>
    list<PropertyRequest*>*     pr;      // <property IDs, ranks>
    map<string, double>         results; // objective values
    bool                        rtsBased;// true if scenario for rts

    // destructor helpers
    static bool deleteTuningSpecification( TuningSpecification* ts ) {
        delete ts;
        return true;
    }

    static bool deletePropertyRequest( PropertyRequest* pr ) {
        delete pr;
        return true;
    }

    void setTunedEntity( const std::string& id ); //callpath string from string


public:
    Scenario();

    Scenario( list<TuningSpecification*>* ts );

    Scenario( Region*                     region,
              list<TuningSpecification*>* ts,
              list<PropertyRequest*>*     pr );

    Scenario( Rts*                        rts,
              list<TuningSpecification*>* ts,
              list<PropertyRequest*>*     pr );

    virtual ~Scenario();

    unsigned int getID();            // unique ID given in constructor

    void setDescription( string description );

    string getDescription();

    static void resetScenarioIds() {
        scenario_id = 0;
    }

    Region* getRegion();

    Rts* getRts();

    list<TuningSpecification*>* getTuningSpecifications();

    void setTuningSpecifications( list<TuningSpecification*>* t );

    const list<PropertyRequest*>* getPropertyRequests();

    void setPropertyRequests( list<PropertyRequest*>* propRequestList );

    void setTunedRegion( Region* r );

    void setTunedRts( Rts* rts );

    void setRtsBased();

    void setRegionBased();



    bool isRtsBased();

    void setSingleTunedRegionWithPropertyRank( Region*      region,
                                               int          propertyId,
                                               unsigned int rank );

    void setSingleTunedRtsWithPropertyRank( Rts*            rts,
                                            int             propertyId,
                                            unsigned int    rank );

    void setSingleTunedRegionWithPropertyALLRanks( Region*  region,
                                                   int      propertyId );

    void setSingleTunedRtsWithPropertyALLRanks( Rts*        rts,
                                                int         propertyId );

    void addResult( string s,
                    double v );

    map<string, double>getResults();

    bool getResult( string  s,
                    double* v );

    bool getRankAffectedBySceanrioTS( int rank );

    bool getRankAffectedBySceanrioPR( int rank );

    bool operator==( const Scenario& in ) const;

    bool operator!=( const Scenario& in ) const;

    void print( void );

    string toString( int    indent,
                     string indentation_character );

    string toString();
};

#endif /* SCENARIODESCRIPTION_H_ */
