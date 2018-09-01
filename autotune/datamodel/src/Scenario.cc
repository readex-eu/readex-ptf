/**
   @file    Scenario.cc
   @ingroup Autotune
   @brief   Scenario Description
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

#include "Scenario.h"
#include <psc_errmsg.h>

// TODO add sanity checks
unsigned int    Scenario::scenario_id = 0;
pthread_mutex_t Scenario::lock        = PTHREAD_MUTEX_INITIALIZER;

Scenario::Scenario() {
    rtsBased = false;
}

//We create it to remove ambuiguity between region and rts while creating creating Scenario object
Scenario::Scenario( list<TuningSpecification*>* ts_in ){

    // class-wide lock on the id counter
    pthread_mutex_lock( &lock );
    id = scenario_id++;
    pthread_mutex_unlock( &lock );
    rtsBased=false;

    tuned_entity ="";
    ts           = ts_in;
    pr           = NULL;

}

Scenario::Scenario( Region*                     region_in,
                    list<TuningSpecification*>* ts_in,
                    list<PropertyRequest*>*     pr_in ) {
    // class-wide lock on the id counter
    pthread_mutex_lock( &lock );
    id = scenario_id++;
    pthread_mutex_unlock( &lock );
    rtsBased=false;

    tuned_entity = region_in  ?  region_in->getRegionID() : "";
    ts           = ts_in;
    pr           = pr_in;
}

Scenario::Scenario( Rts*                        rts,
                    list<TuningSpecification*>* ts_in,
                    list<PropertyRequest*>*     pr_in ) {
    // class-wide lock on the id counter
    pthread_mutex_lock( &lock );
    id = scenario_id++;
    pthread_mutex_unlock( &lock );
    rtsBased=true;

    //tuned_entity="";
    tuned_entity = rts ?  rts->getCallPath() : "";
    ts           = ts_in;
    pr           = pr_in;
}


Scenario::~Scenario() {
    ts->remove_if( deleteTuningSpecification );
    pr->remove_if( deletePropertyRequest );
}

unsigned int Scenario::getID() {
    return id;
}

string Scenario::getDescription() {
    return description;
}

void Scenario::setDescription( string description ) {
    this->description = description;
}

Region* Scenario::getRegion() {
    if( rtsBased ) {
        psc_abort( "Scenario is not region based, but region is requested. Scenario::getRegion()\n" );
    }
    if( tuned_entity.empty() ) {
        return NULL;
    }

    return Application::instance().getRegionByID( tuned_entity );
}

Rts* Scenario::getRts()
{
    if( !rtsBased ) {
        psc_abort( "Scenario is not rts based, but rts is requested. Scenario::getRts()\n" );
    }

    return tuned_entity.empty() ? NULL : Application::instance().getCalltreeRoot()->getRtsByCallpath( tuned_entity );
}

list<TuningSpecification*>* Scenario::getTuningSpecifications() {
    return ts;
}

void Scenario::setRtsBased() {
    rtsBased = true;
}

void Scenario::setRegionBased() {
    rtsBased = false;
}


void Scenario::setTuningSpecifications( list<TuningSpecification*>* t ) {
    ts = t;
}

const list<PropertyRequest*>* Scenario::getPropertyRequests() {
    return pr;
}

void Scenario::setPropertyRequests( list<PropertyRequest*>* propRequestList ) {
    pr = propRequestList;
}

void Scenario::setTunedRegion( Region* r ) {
    string r_id;
    r_id = r ? r->getRegionID() : "";
    setTunedEntity(r_id);
    rtsBased=false;
}
void Scenario::setTunedRts( Rts* rts ) {
    string r_id;
    r_id = rts ? rts->getCallPath() : "";
    setTunedEntity(r_id);
    rtsBased=true;
}

void Scenario::setTunedEntity( const std::string& id ) {
    tuned_entity = id;
}

/**
 * Marks whether the scenario is rts_based or region_based
 * @return bool
 */
bool Scenario::isRtsBased() {

    return rtsBased;

}

void Scenario::setSingleTunedRegionWithPropertyRank( Region*      region,
                                                     int          propertyId,
                                                     unsigned int rank ) {
    list<PropertyRequest*>* propertyRequestList = new list<PropertyRequest*>();
    list<int>*              propertyIds         = new list<int>;
    list<unsigned int>*     ranks               = new list<unsigned int>;

    rtsBased = false;
    propertyIds->push_back( propertyId );
    ranks->push_back( rank );
    propertyRequestList->push_back( new PropertyRequest( propertyIds, ranks ) );
    this->setPropertyRequests( propertyRequestList );
    this->setTunedRegion( region );
}

void Scenario::setSingleTunedRtsWithPropertyRank( Rts*         rts,
                                                  int          propertyId,
                                                  unsigned int rank ) {
    list<PropertyRequest*>* propertyRequestList = new list<PropertyRequest*>();
    list<int>*              propertyIds         = new list<int>;
    list<unsigned int>*     ranks               = new list<unsigned int>;

    rtsBased = true;
    propertyIds->push_back( propertyId );
    ranks->push_back( rank );
    propertyRequestList->push_back( new PropertyRequest( propertyIds, ranks ) );
    this->setPropertyRequests( propertyRequestList );
    this->setTunedRts( rts );
}

void Scenario::setSingleTunedRegionWithPropertyALLRanks( Region* region,
                                                         int     propertyId ) {
    list<PropertyRequest*>* propertyRequestList = new list<PropertyRequest*>();
    list<int>*              propertyIds         = new list<int>;

    rtsBased = false;
    propertyIds->push_back( propertyId );
    propertyRequestList->push_back( new PropertyRequest( propertyIds ) );
    this->setPropertyRequests( propertyRequestList );
    this->setTunedRegion( region );
}

void Scenario::setSingleTunedRtsWithPropertyALLRanks( Rts*        rts,
                                                      int     propertyId ) {

    list<PropertyRequest*>* propertyRequestList = new list<PropertyRequest*>();
    list<int>*              propertyIds         = new list<int>;

    rtsBased = true;
    propertyIds->push_back( propertyId );
    propertyRequestList->push_back( new PropertyRequest( propertyIds ) );
    if(getPropertyRequests() == nullptr) {
        this->setPropertyRequests( propertyRequestList );
    }
    else {
        pr->push_back(propertyRequestList->back());
    }
    this->setTunedRts( rts );
}
//TO DO variant context has to be changed for RTS
void Scenario::print( void ) {
    typedef map<TuningParameter*, int>::iterator tp_iter;
    typedef list<TuningSpecification*>::iterator ts_iter;
    typedef list<PropertyRequest*>::iterator     propreq_iter;
    typedef list<int>::iterator                  propid_iter;
    //RM: Extend printouts, e.g. region, ranks etc.
    printf( "--------------------------------------------------------------------------------\n" );
    printf( "Scenario ID: %d\n", id );
    fflush( stdout );
    if( rtsBased ){
        printf( " Rts information:\n" );
        fflush( stdout );
    }
    else
    {
        printf( " Region information:\n" );
        fflush( stdout );
    }
    if( !tuned_entity.empty() ) {
        printf( " - Name: %s\n", tuned_entity.c_str() );
        fflush( stdout );
    }
    else {
        printf( " No tuned region/rts specified.\n" );
        fflush( stdout );
    }

    if( ts ) {
        int ts_count = 0;
        printf( " TuningSpecifications: %lu\n", ts->size() );
        for( ts_iter tsi = ts->begin(); tsi != ts->end(); tsi++, ts_count++ ) {
            printf( " - TuningSpecification: %d\n", ts_count );
            fflush( stdout );
            map<TuningParameter*, int> values = ( *tsi )->getVariant()->getValue();
            printf( " -- Variant (%lu TuningParameters):\n", values.size() );
            fflush( stdout );
            int par_count = 0;
            for( tp_iter i = values.begin(); i != values.end(); i++, par_count++ ) {
                printf( " --- TuningParameter %d:\n", par_count );
                fflush( stdout );
                printf( " ---- Name: %s\n", i->first->getName().c_str() );
                fflush( stdout );
                printf( " ---- Value: %d\n", i->second );
                fflush( stdout );
            }
            printf( " -- VariantContext type: " );
            fflush( stdout );
            if( ( *tsi )->getTypeOfVariantContext() == variant_context_type( PROGRAM ) ) {
                printf( "PROGRAM\n" );
                fflush( stdout );
            }
            else if( ( *tsi )->getTypeOfVariantContext() == variant_context_type( RTS_LIST ) ) {
                VariantContext context = ( *tsi )->getVariantContext();
                list<string>*  entities = context.context_union.entity_list;
                printf( "RTS_LIST of size %lu\n", entities->size() );
                fflush( stdout );
                std::list<string>::iterator entity_iter;
                for( entity_iter = entities->begin(); entity_iter != entities->end(); entity_iter++ ) {
                    printf( " --- RTS (%s)\n", entity_iter->c_str() );
                    fflush( stdout );
                }
            }
            else if( ( *tsi )->getTypeOfVariantContext() == variant_context_type( REGION_LIST ) ) {
                VariantContext context = ( *tsi )->getVariantContext();
                list<string>*  entities = context.context_union.entity_list;
                printf( "REGION_LIST of size %lu\n", entities->size() );
                fflush( stdout );
                std::list<string>::iterator entity_iter;
                for( entity_iter = entities->begin(); entity_iter != entities->end(); entity_iter++ ) {
                    printf( " --- Region (%s)\n", entity_iter->c_str() );
                    fflush( stdout );

                }
            }
            else if( ( *tsi )->getTypeOfVariantContext() == variant_context_type( FILE_LIST ) ) {
                printf( "FILE_LIST\n" );
                fflush( stdout );
            }
            else {
                perror( "Invalid VariantContext type\n" );
                throw 0;
            }
            printf( " -- Ranks type: " );
            fflush( stdout );
            if( ( *tsi )->getTypeOfRanks() == ranks_type( ALL ) ) {
                printf( "ALL\n" );
                fflush( stdout );
            }
            else if( ( *tsi )->getTypeOfRanks() == ranks_type( RANK_LIST ) ) {
                printf( "RANK_LIST\n" );
                fflush( stdout );
                list<unsigned int>*               processes = ( *tsi )->getRanks().ranks_union.rank_list;
                std::list<unsigned int>::iterator process_iter;
                for( process_iter = processes->begin(); process_iter != processes->end();
                     process_iter++ ) {
                    printf( " --- Rank %d \n", *process_iter );
                }
            }
            else if( ( *tsi )->getTypeOfRanks() == ranks_type( RANGE_LIST ) ) {
                printf( "RANGE_LIST\n" );
                fflush( stdout );
            }
            else {
                perror( "Invalid Ranks type\n" );
                throw 0;
            }
        }
    }
    else {
        printf( "No tuning specification\n" );
    }
    if( pr ) {
        printf( " PropertyRequests: %lu\n", pr->size() );
        fflush( stdout );
        int propreq_count = 0;
        for( propreq_iter pri = pr->begin(); pri != pr->end(); pri++, propreq_count++ ) {
            printf( " - PropertyRequest: %d\n", propreq_count );
            fflush( stdout );
            int        propid_count = 0;
            list<int>* property_ids = ( *pri )->getPropertyIDs();
            printf( " -- Properties: %lu\n", ( *pri )->getPropertyIDs()->size() );
            fflush( stdout );
            printf( " -- IDs: " );
            fflush( stdout );
            for( propid_iter pii = property_ids->begin(); pii != property_ids->end(); pii++, propid_count++ ) {
                printf( "%d ", ( *pii ) );
                fflush( stdout );
            }
            printf( "\n" );
            fflush( stdout );
            printf( " -- Ranks type: " );
            fflush( stdout );
            if( ( *pri )->getTypeOfRanks() == ranks_type( ALL ) ) {
                printf( "ALL\n" );
                fflush( stdout );
            }
            else if( ( *pri )->getTypeOfRanks() == ranks_type( RANK_LIST ) ) {
                printf( "RANK_LIST\n" );
                fflush( stdout );
                list<unsigned int>*               processes = ( *pri )->getRanks().ranks_union.rank_list;
                std::list<unsigned int>::iterator process_iter;
                for( process_iter = processes->begin(); process_iter != processes->end();
                     process_iter++ ) {
                    printf( " --- Rank %d \n", *process_iter );
                }
            }
            else if( ( *pri )->getTypeOfRanks() == ranks_type( RANGE_LIST ) ) {
                printf( "RANGE_LIST\n" );
                fflush( stdout );
            }
            else {
                perror( "Invalid Ranks type\n" );
                fflush( stdout );
                throw 0;
            }
        }
    }
    printf( "--------------------------------------------------------------------------------\n" );
    fflush( stdout );
}

bool Scenario::operator==( const Scenario& in ) const {
    if( this == &in ) {
        return true;
    }

    if( id != in.id ||
        tuned_entity != in.tuned_entity ||
        ts->size() != in.ts->size() ||
        pr->size() != in.pr->size() ) {
        return false;
    }

    typedef list<TuningSpecification*>::iterator ts_iter;
    typedef map<TuningParameter*, int>::iterator tp_iter;
    int     ts_count = 0;
    ts_iter tsi      = ts->begin();
    ts_iter tsi_in   = in.ts->begin();
    for(; tsi != ts->end() && tsi_in != ts->end(); tsi++, tsi_in++, ts_count++ ) {
        map<TuningParameter*, int> values    = ( *tsi )->getVariant()->getValue();
        map<TuningParameter*, int> values_in = ( *tsi_in )->getVariant()->getValue();

        if( values.size() != values_in.size() ) {
            return false;
        }

        int     par_count = 0;
        tp_iter par       = values.begin();
        tp_iter par_in    = values_in.begin();
        for(; par != values.end() && par_in != values_in.end();
            par++, par_in++, par_count++ ) {
            if( *par->first != *par_in->first || par->second != par_in->second ) {
                return false;
            }
        }

        if( ( *tsi )->getVariantContext() != ( *tsi_in )->getVariantContext() ) {
            return false;
        }

        if( ( *tsi )->getRanks() != ( *tsi_in )->getRanks() ) {
            return false;
        }
    }

    typedef list<PropertyRequest*>::iterator propreq_iter;
    int          propreq_count = 0;
    propreq_iter pri           = pr->begin();
    propreq_iter pri_in        = in.pr->begin();
    for(; pri != pr->end() && pri_in != in.pr->end();
        pri++, pri_in++, propreq_count++ ) {
        int        propid_count    = 0;
        list<int>* property_ids    = ( *pri )->getPropertyIDs();
        list<int>* property_ids_in = ( *pri_in )->getPropertyIDs();

        if( *property_ids != *property_ids_in ) {
            return false;
        }

        if( ( *pri )->getTypeOfRanks() != ( *pri_in )->getTypeOfRanks() ) {
            return false;
        }

        Ranks ranks    = ( *pri )->getRanks();
        Ranks ranks_in = ( *pri_in )->getRanks();
        if( ranks != ranks_in ) {
            return false;
        }
    }

    return true;
}

bool Scenario::operator!=( const Scenario& in ) const {
    return !( *this == in );
}

string Scenario::toString( int    indent,
                           string indentation_character ) {
    string                               base_indentation;
    list<TuningSpecification*>::iterator ts_iter;
    list<PropertyRequest*>::iterator     pr_iter;
    for( int i = 0; i < indent; i++ ) {
        base_indentation.append( indentation_character );
    }
    stringstream temp;

    temp << base_indentation << "Scenario: " << id << endl;
    if( tuned_entity.empty() ) {
        temp << base_indentation << indentation_character << "Region: NULL" << endl;
    }
    else {
        temp << tuned_entity << endl;
    }
    if( isRtsBased() )
        temp << "Signature: RTS based" << endl;
    else
        temp << "Signature: Region based" << endl;
    if( ts == NULL ) {
        temp << base_indentation << indentation_character << "Tuning Specifications: NULL" << endl;
    }
    else {
        temp << base_indentation << indentation_character << "Tuning Specifications: " << ts->size() << endl;
        for( ts_iter = ts->begin(); ts_iter != ts->end(); ts_iter++ ) {
            temp << ( *ts_iter )->toString( indent + 1, indentation_character ) << endl;
        }
    }
    if( pr == NULL ) {
        temp << base_indentation << indentation_character << "Property Requests: NULL" << endl;
    }
    else {
        temp << base_indentation << indentation_character << "Property Requests: " << pr->size() << endl;
        for( pr_iter = pr->begin(); pr_iter != pr->end(); pr_iter++ ) {
            temp << ( *pr_iter )->toString( indent + 1, indentation_character ) << endl;
        }
    }

    map<string, double>::iterator res_iter;

    temp << base_indentation << indentation_character << "Results: " << endl;
    for( res_iter = results.begin(); res_iter != results.end(); res_iter++ ) {
        temp << base_indentation << indentation_character << indentation_character << res_iter->first << ": " << res_iter->second << endl;
    }

    return temp.str().c_str();
}

string Scenario::toString() {
    return toString( 0, "\t" );
}


void Scenario::addResult( string s,
                          double v ) {
    results[ s ] = v;
}

map<string, double> Scenario::getResults() {
    return results;
}

bool Scenario::getResult( string  s,
                          double* v ) {
    map<string, double>::iterator res_iter = results.find( s );

    if( res_iter != results.end() ) {
        *v = res_iter->second;
        return true;
    }
    else {
        return false;
    }

    return false;
}

bool Scenario::getRankAffectedBySceanrioTS( int rank ) {
    std::list<TuningSpecification*>::const_iterator tuningSpec;
    std::list<unsigned int>*                        rankList;
    std::list<unsigned int>::const_iterator         rank_it;
    std::list<Range>*                               rangeList;
    std::list<Range>::const_iterator                range;

    bool scenario_applies_to_the_rank = false;
    if( ts ) {
        //Iterates through scenario's tuning specifications and stores scenarios in per-rank list
        for( tuningSpec = ts->begin(); tuningSpec != ts->end(); tuningSpec++ ) {
            const Ranks ranks = ( *tuningSpec )->getRanks();
            if( ranks.type == ranks_type( ALL ) ) {
                scenario_applies_to_the_rank = true;
            }
            else if( ranks.type == ranks_type( RANK_LIST ) ) {
                rankList = ranks.ranks_union.rank_list;
                for( rank_it = rankList->begin(); rank_it != rankList->end(); rank_it++ ) {
                    if( rank == *rank_it ) {
                        scenario_applies_to_the_rank = true;
                    }
                }
            }
            else if( ranks.type == ranks_type( RANGE_LIST ) ) {
                rangeList = ranks.ranks_union.range_list;
                for( range = rangeList->begin(); range != rangeList->end(); range++ ) {
                    if( rank <= range->end && rank >= range->start ) {
                        scenario_applies_to_the_rank = true;
                    }
                }
            }
            if( scenario_applies_to_the_rank ) {
                break;
            }
        }
    }
    else {
        /*
         * Why do we apply scenario to a process when the tuning specification
         * list of this scenario is absent?
         */
        //process->addScenarioPerTuningSpecification(*scenario);
        scenario_applies_to_the_rank = true;
    }
    return scenario_applies_to_the_rank;
}

bool Scenario::getRankAffectedBySceanrioPR( int rank ) {
    std::list<PropertyRequest*>::const_iterator propReq_it;
    std::list<unsigned int>*                    rankList;
    std::list<unsigned int>::const_iterator     rank_it;
    std::list<Range>*                           rangeList;
    std::list<Range>::const_iterator            range;

    bool scenario_applies_to_the_rank = false;
    if( pr ) {
        //Iterates through scenario's tuning specifications and stores scenarios in per-rank list
        for( propReq_it = pr->begin(); propReq_it != pr->end(); propReq_it++ ) {
            const Ranks ranks = ( *propReq_it )->getRanks();
            if( ranks.type == ranks_type( ALL ) ) {
                scenario_applies_to_the_rank = true;
            }
            else if( ranks.type == ranks_type( RANK_LIST ) ) {
                rankList = ranks.ranks_union.rank_list;
                for( rank_it = rankList->begin(); rank_it != rankList->end(); rank_it++ ) {
                    if( rank == *rank_it ) {
                        scenario_applies_to_the_rank = true;
                    }
                }
            }
            else if( ranks.type == ranks_type( RANGE_LIST ) ) {
                rangeList = ranks.ranks_union.range_list;
                for( range = rangeList->begin(); range != rangeList->end(); range++ ) {
                    if( rank <= range->end && rank >= range->start ) {
                        scenario_applies_to_the_rank = true;
                    }
                }
            }
            if( scenario_applies_to_the_rank ) {
                break;
            }
        }
    }
    else {
        /*
         * Why do we apply scenario to a process when the tuning specification
         * list of this scenario is absent?
         */
        //process->addScenarioPerTuningSpecification(*scenario);
        scenario_applies_to_the_rank = true;
    }
    return scenario_applies_to_the_rank;
}
