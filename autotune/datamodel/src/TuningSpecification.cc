#include "TuningSpecification.h"
#include "psc_errmsg.h"
#include <boost/foreach.hpp>
#include <stdio.h>




TuningSpecification::TuningSpecification() {
}
// no restrictions
TuningSpecification::TuningSpecification( Variant* variant_in ) {
    variant      = variant_in;
    ranks.type   = ranks_type( ALL );
    context.type = variant_context_type( PROGRAM );
}

// restricted processes
TuningSpecification::TuningSpecification( Variant*            variant_in,
                                          list<unsigned int>* processes ) {
    variant                     = variant_in;
    ranks.type                  = ranks_type( RANK_LIST );
    ranks.ranks_union.rank_list = processes;
    context.type                = variant_context_type( PROGRAM );
}
TuningSpecification::TuningSpecification( Variant*     variant_in,
                                          list<Range>* processes ) {
    variant                      = variant_in;
    ranks.type                   = ranks_type( RANGE_LIST );
    ranks.ranks_union.range_list = processes;
    context.type                 = variant_context_type( PROGRAM );
}

// restricted locations
TuningSpecification::TuningSpecification( Variant*       variant_in,
                                          list<Region*>* regions ) {
    variant      = variant_in;
    ranks.type   = ranks_type( ALL );
    context.type = variant_context_type( REGION_LIST );
    is_rts       = false;

    list<string>* idList = new list<string>();
    BOOST_FOREACH( Region * r, *regions ) {
        if( !r ) {
            psc_errmsg( "INVALID PARAMETER to TuningSpecification::TuningSpecification(): One of the provided regions was a nullptr.\n" );
            abort();
        }

        idList->push_back( r->getRegionID() );
    }
    delete regions;
    context.context_union.entity_list = idList;
}

//rts based Tuning Specification

// restricted locations
TuningSpecification::TuningSpecification( Variant*       variant_in,
                                          list<Rts*>*    rts_s ) {
    variant      = variant_in;
    ranks.type   = ranks_type( ALL );
    context.type = variant_context_type( RTS_LIST );
    is_rts       = true;

    list<string>* callPathList = new list<string>();
    BOOST_FOREACH( Rts * rts, *rts_s ) {
        if( !rts ) {
            psc_errmsg( "INVALID PARAMETER to TuningSpecification::TuningSpecification(): One of the provided RTS was a nullptr.\n" );
            abort();
        }
        callPathList->push_back( rts->getCallPath() );
    }
    delete rts_s;
    context.context_union.entity_list = callPathList;
}

TuningSpecification::TuningSpecification( Variant*      variant_in,
                                          list<string>* regions ) {
    variant                           = variant_in;
    ranks.type                        = ranks_type( ALL );
    if( withRtsSupport() )
    {
        context.type = variant_context_type( RTS_LIST );
        is_rts     = true;
    }
    else
    {
        context.type = variant_context_type( REGION_LIST );
    }

    context.context_union.entity_list = regions;
}

TuningSpecification::~TuningSpecification() {
    delete variant;
}

void TuningSpecification::setSingleRank( int rank ) {
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

void TuningSpecification::setALLRanks() {
    // remove any previous process restriction
    if( ranks.type == ranks_type( RANK_LIST ) ) {
        delete ranks.ranks_union.rank_list;
    }
    else if( ranks.type == ranks_type( RANGE_LIST ) ) {
        delete ranks.ranks_union.range_list;
    }

    ranks.type = ranks_type( ALL );
}


const Variant* TuningSpecification::getVariant() {
    return variant;
}

int TuningSpecification::getTypeOfVariantContext() {
    return context.type;
}

VariantContext TuningSpecification::getVariantContext() {
    return context;
}

int TuningSpecification::getTypeOfRanks() {
    return ranks.type;
}

const Ranks TuningSpecification::getRanks() {
    return ranks;
}

bool TuningSpecification::operator==( const TuningSpecification& in ) const {
    if(
        *variant != *in.variant
        || context != in.context
        || ranks != in.ranks
        ) {
        // cout << "false in ts check\n";
        return false;
    }
    // cout << "true in ts check\n";
    return true;
}
bool TuningSpecification::operator!=( const TuningSpecification& in ) const {
    return !( *this == in );
}

//TODO
string TuningSpecification::toString( int    indent,
                                      string indentation_character ) {
    string base_indentation;
    for( int i = 0; i < indent; i++ ) {
        base_indentation.append( indentation_character );
    }
    stringstream temp;

    temp << variant->toString( indent + 1, indentation_character );

    temp << endl << base_indentation << indentation_character << "Variant context type:         ";
    if( context.type == variant_context_type( PROGRAM ) ) {
        temp << "PROGRAM" << endl;
    }
    else if( context.type == variant_context_type( RTS_LIST ) ) {
        list<string>::const_iterator entity_it;
        temp << "RTS_LIST" << endl;
        for( entity_it = context.context_union.entity_list->begin();
             entity_it != context.context_union.entity_list->end();
             entity_it++ ) {
            temp << Application::instance().getCalltreeRoot()->getRtsByCallpath( *entity_it )->getCallPath(); // TO Do
        }
    }
    else if( context.type == variant_context_type( REGION_LIST ) ) {
        list<string>::const_iterator entity_it;
        temp << "REGION_LIST" << endl;
        for( entity_it = context.context_union.entity_list->begin();
             entity_it != context.context_union.entity_list->end();
             entity_it++ ) {
            temp << Application::instance().getRegionByID( *entity_it )->toString( indent + 2, indentation_character );
        }
    }
    else if( context.type == variant_context_type( FILE_LIST ) ) {
        list<string>::const_iterator file_it;
        temp << "FILE_LIST" << endl << base_indentation << indentation_character << "File list:            " << endl;
        for( file_it = context.context_union.file_list->begin();
             file_it != context.context_union.file_list->end();
             file_it++ ) {
            temp << *file_it << " ";
        }
        temp << endl;
    }
    else {
        temp << base_indentation << "Variant Context undefined!" << endl;
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
