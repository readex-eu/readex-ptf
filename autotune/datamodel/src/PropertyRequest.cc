#include "PropertyRequest.h"
#include <psc_errmsg.h>




PropertyRequest::PropertyRequest() {
    ranks.type   = ranks_type( ALL );
    property_ids = new list<int>;
    entities     = new list<string>();
    rtsBased	 = false;
}

PropertyRequest::PropertyRequest( list<int>* property_ids_in ) {
    ranks.type   = ranks_type( ALL );
    property_ids = property_ids_in;
    entities     = new list<string>();
    rtsBased	 = false;
}

PropertyRequest::PropertyRequest( list<int>*          property_ids_in,
                                  list<unsigned int>* rank_list ) {
    ranks.type                  = ranks_type( RANK_LIST );
    property_ids                = property_ids_in;
    ranks.ranks_union.rank_list = rank_list;
    entities                    = new list<string>();
    rtsBased	                = false;
}

PropertyRequest::PropertyRequest( list<int>*   property_ids_in,
                                  list<Range>* range_list ) {
    ranks.type                   = ranks_type( RANGE_LIST );
    property_ids                 = property_ids_in;
    ranks.ranks_union.range_list = range_list;
    entities                     = new list<string>();
    rtsBased	                 = false;
}

PropertyRequest::~PropertyRequest() {
    if( ranks.type == RANGE_LIST ) {
        delete ranks.ranks_union.range_list;
    }
    if( ranks.type == RANK_LIST ) {
        delete ranks.ranks_union.rank_list;
    }
    delete property_ids;
    delete entities;
}

list<int>* PropertyRequest::getPropertyIDs() {
    return property_ids;
}



list<string>* PropertyRequest::getEntities() {
    return entities;
}


void PropertyRequest::addPropertyID( int property_id ) {
    if( property_id < 0 ) {
        psc_abort( "Negative property ID specified in PropertyRequest::addPropertyID()\n" );
    }

    property_ids->push_back( property_id );
}



void PropertyRequest::addRegion( Region* reg ) {
    if( !reg ) {
        psc_abort( "NULL region specified in PropertyRequest::addRegion()\n" );
    }
    rtsBased	= false;

    addEntity( reg->getRegionID() );
}

void PropertyRequest::addRts( Rts * rts ) {
    if( !rts ) {
        psc_abort( "NULL RTS specified in PropertyRequest::addRts()\n" );
    }
    rtsBased	= true;

    addEntity( rts->getCallPath() );
}

void PropertyRequest::addEntity( const std::string& id ) {

    entities->push_back( id );
}

void PropertyRequest::addSingleProcess( int p ) {
    if( p < 0 ) {
        perror( "Negative process rank specified in PropertyRequest::addPropertyID()\n" );
        throw 0;
    }
    if( ranks.type != RANK_LIST ) {
        if( ranks.type == RANGE_LIST ) {
            delete ranks.ranks_union.range_list;
        }
        ranks.type                  = ranks_type( RANK_LIST );
        ranks.ranks_union.rank_list = new list<unsigned int>;
        ranks.ranks_union.rank_list->push_back( p );
    }
    else {
        ranks.ranks_union.rank_list->push_back( p );
    }
}

void PropertyRequest::addProcessRange( int from,
                                       int to ) {
    Range range;
    range.start = from;
    range.end   = to;

    if( from > to ) {
        perror( "Invalid range in PropertyRequest::addProcessRange()\n" );
        throw 0;
    }
    if( ranks.type != RANGE_LIST ) {
        if( ranks.type == RANK_LIST ) {
            delete ranks.ranks_union.rank_list;
        }
        ranks.type                   = ranks_type( RANGE_LIST );
        ranks.ranks_union.range_list = new list<Range>;
        ranks.ranks_union.range_list->push_back( range );
    }
    else {
        ranks.ranks_union.range_list->push_back( range );
    }
}


void PropertyRequest::addAllProcesses() {
    if( ranks.type != ALL ) {
        if( ranks.type == RANK_LIST ) {
            delete ranks.ranks_union.rank_list;
        }
        if( ranks.type == RANGE_LIST ) {
            delete ranks.ranks_union.range_list;
        }
        ranks.type = ALL;
    }
}


int PropertyRequest::getTypeOfRanks() {
    return ranks.type;
}

const Ranks PropertyRequest::getRanks() {
    return ranks;
}

void PropertyRequest::setSingleRank( int rank ) {
    // remove any previous process restriction
    if( ranks.type == ranks_type( RANK_LIST ) ) {
        delete ranks.ranks_union.rank_list;
    }
    else if( ranks.type == ranks_type( RANGE_LIST ) ) {
        delete ranks.ranks_union.range_list;
    }

    ranks.type = ranks_type( RANK_LIST );
    list<unsigned int>* single_rank_list = new list<unsigned int>();
    single_rank_list->push_back( rank );
    ranks.ranks_union.rank_list = single_rank_list;
}


bool processInRanks( Ranks ranks,
                     int   p ) {
    if( ranks.type == ranks_type( ALL ) ) {
        return true;
    }
    else if( ranks.type == ranks_type( RANK_LIST ) ) {
        std::list<unsigned int>::iterator rank;
        std::list<unsigned int>*          rank_list = ranks.ranks_union.rank_list;
        for( rank = rank_list->begin(); rank != rank_list->end(); rank++ ) {
            if( *rank == p ) {
                return true;
            }
        }
    }
    else if( ranks.type == ranks_type( RANGE_LIST ) ) {
        std::list<Range>::iterator range;
        std::list<Range>*          range_list = ranks.ranks_union.range_list;
        for( range = range_list->begin(); range != range_list->end(); range++ ) {
            if( ( *range ).start <= p && p <= ( *range ).end ) {
                return true;
            }
        }
    }

    return false;
}

// TO DO
bool mpiProfilingRequired( list<PropertyRequest*>* requests ) {
    std::list<PropertyRequest*>::iterator req;

    for( req = requests->begin(); req != requests->end(); req++ ) {
        std::list<int>*          propIds = ( *req )->getPropertyIDs();
        std::list<int>::iterator propId;
        for( propId = propIds->begin(); propId != propIds->end(); propId++ ) {
            if( ( *propId ) == MPIPROP ) {
                return true;
            }
            if( ( *propId ) == MPITIME ) {
                return true;
            }
            if( ( *propId ) == OVERLOADEDMASTER ) {
                return true;
            }

            std::list<string>*          entities = ( *req )->getEntities();
            std::list<string>::iterator reg;
            for( reg = entities->begin(); reg != entities->end(); reg++ ) { //change for rts
                if( Application::instance().getRegionByID( *reg )->get_ident().type == MPI_CALL ) {
                    return true;
                }
            }
        }
    }
    return false;
}

void PropertyRequest::print() {
    cout << "Property IDs:" << endl << "   ";
    std::list<int>::iterator propId;
    for( propId = property_ids->begin(); propId != property_ids->end(); propId++ ) {
        if( propId == property_ids->begin() ) {
            cout << *propId;
        }
        else {
            cout << ", " << *propId;
        }
    }
    cout << endl;

    if( isRtsBased() )
        cout << "RTSs:" << endl << "   ";

    else
        cout << "Regions:" << endl << "   ";

    std::list<std::string>::iterator reg;
    for( reg = entities->begin(); reg != entities->end(); reg++ ) {
        if( reg == entities->begin() ) {
            cout << "(" << ( *reg ) << ")";
        }
        else {
            cout << ", (" << ( *reg ) << ")";
        }
    }
    cout << endl;

    if( ranks.type == ranks_type( RANK_LIST ) ) {
        cout << "Processes:" << endl << "   ";
        std::list<unsigned int>::iterator rank;
        std::list<unsigned int>*          rank_list = ranks.ranks_union.rank_list;
        for( rank = rank_list->begin(); rank != rank_list->end(); rank++ ) {
            if( rank == rank_list->begin() ) {
                cout << *rank;
            }
            else {
                cout << ", " << *rank;
            }
        }
        cout << endl;
    }
    else if( ranks.type == ranks_type( RANGE_LIST ) ) {
        cout << "Processes:" << endl << "   ";
        std::list<Range>::iterator range;
        std::list<Range>*          range_list = ranks.ranks_union.range_list;
        for( range = range_list->begin(); range != range_list->end(); range++ ) {
            if( range == range_list->begin() ) {
                cout << "[" << ( *range ).start << "," << ( *range ).end << "]";
            }
            else {
                cout << ", [" << ( *range ).start << "," << ( *range ).end << "]";
            }
        }
        cout << endl;
    }
    else if( ranks.type == ranks_type( ALL ) ) {
        cout << "Processes:" << endl << "   ALL" << endl;
    }
}

//TO Do add region information
string PropertyRequest::toString( int    indent,
                                  string indentation_character ) {
    string              base_indentation;
    list<int>::iterator property_id_iterator;
    for( int i = 0; i < indent; i++ ) {
        base_indentation.append( indentation_character );
    }
    stringstream temp;

    if( isRtsBased() )
        temp << base_indentation << indentation_character << "RTS Properties:" << endl;
    else
        temp << base_indentation << indentation_character << "Region Properties:" << endl;
    temp << base_indentation << indentation_character << "Total Property IDs: " << property_ids->size() << endl;
    temp << base_indentation << indentation_character << "Property IDs:       ";
    for( property_id_iterator = property_ids->begin(); property_id_iterator != property_ids->end(); property_id_iterator++ ) {
        temp << ( *property_id_iterator );
    }

    temp << endl << base_indentation << indentation_character << "Ranks type:         ";
    if( ranks.type == ranks_type( ALL ) ) {
        temp << "ALL" << endl;
    }
    else if( ranks.type == ranks_type( RANK_LIST ) ) {
        list<unsigned int>::iterator iter;
        temp << "RANK_LIST" << endl << base_indentation << indentation_character << "Ranks:              ";
        for( iter = ranks.ranks_union.rank_list->begin(); iter != ranks.ranks_union.rank_list->end(); iter++ ) {
            temp << ( *iter ) << " ";
        }
        temp << endl;
    }
    else if( ranks.type == ranks_type( RANGE_LIST ) ) {
        list<Range>::iterator iter;
        temp << "RANGE_LIST" << endl << base_indentation << indentation_character << "Ranges:            ";
        for( iter = ranks.ranks_union.range_list->begin(); iter != ranks.ranks_union.range_list->end(); iter++ ) {
            temp << "( " << ( *iter ).start << " , " << ( *iter ).end << " ); ";
        }
        temp << endl;
    }

    return temp.str();
}
/**
 * Marks whether the propertyrequesty is for rts_based or region_based
 * @return bool
 */
bool PropertyRequest::isRtsBased() {

    return (rtsBased);

}
