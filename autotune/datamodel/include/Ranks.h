#ifndef RANKS__H_
#define RANKS__H_

#include <stdio.h>
#include <list>
#include <boost/serialization/list.hpp>
using namespace std;

typedef struct Range_t {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& start;
        ar& end;
    }
    unsigned int start;
    unsigned int end;
} Range;

typedef union RanksUnion_t {
    list<unsigned int>* rank_list;
    list<Range>*        range_list;
} RanksUnion;

enum ranks_type {
    ALL = 0, RANK_LIST, RANGE_LIST
};

typedef struct Ranks_t {
    int        type;
    RanksUnion ranks_union;

    template <class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& type;
        if( type == ranks_type( RANK_LIST ) ) {
            ar& ranks_union.rank_list;
        }
        else if( type == ranks_type( RANGE_LIST ) ) {
            ar& ranks_union.range_list;
        }
    }

    bool operator==( const struct Ranks_t& in ) const {
        if( type != in.type ) {
            return false;
        }
        if( type == ranks_type( RANK_LIST ) ) {
            if( ranks_union.rank_list != in.ranks_union.rank_list ) {
                return false;
            }
        }
        if( type == ranks_type( RANGE_LIST ) ) {
            if( ranks_union.range_list != in.ranks_union.range_list ) {
                return false;
            }
        }

        return true;
    }

    bool operator!=( const struct Ranks_t& in ) const {
        return !( *this == in );
    }
} Ranks;

bool processInRanks( Ranks ranks,
                     int   p );

#endif
