#ifndef TUNING_SPECIFICATION_H_
#define TUNING_SPECIFICATION_H_

#include <list>

#include "application.h"
#include "PropertyRequest.h"
#include "Variant.h"
#include "Ranks.h"
#include "VariantContext.h"

class TuningSpecification {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& variant;
        ar& context;
        ar& ranks;
    }
private:
    Variant*       variant;
    VariantContext context;
    Ranks          ranks;
    bool           is_rts;

    TuningSpecification();     // should only be used for serialization

public:
    // no restrictions
    TuningSpecification( Variant* variant );

    // restricted processes
    TuningSpecification( Variant*            variant,
                         list<unsigned int>* processes );              // individual process ranks

    TuningSpecification( Variant*     variant,
                         list<Range>* processes );                     // process rank ranges

    // restricted locations
    TuningSpecification( Variant*       variant,
                         list<Region*>* regions );                     // instrumented regions

    TuningSpecification( Variant*      variant,
                         list<string>* regions );                      // instrumented region ids

    // restricted processes and locations
    TuningSpecification( Variant*            variant,
                         list<unsigned int>* processes,
                         list<Region*>*      location );

    // restricted locations
    TuningSpecification( Variant*       variant,
                         list<Rts*>*    rts_s );                     // instrumented regions


    virtual ~TuningSpecification();

    void setSingleRank( int rank );

    void setALLRanks();

    const Variant* getVariant();

    int getTypeOfVariantContext();

    VariantContext getVariantContext();

    int getTypeOfRanks();

    const Ranks getRanks();

    bool isRTS(){
        return is_rts;
    };

    bool operator==( const TuningSpecification& in ) const;

    bool operator!=( const TuningSpecification& in ) const;

    string toString( int    indent,
                     string indentation_character );
};
#endif
