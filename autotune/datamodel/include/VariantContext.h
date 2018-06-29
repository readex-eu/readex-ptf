/**
   @file    VariantContext.h
   @ingroup Autotune
   @brief   Variant Context
   @author  Isaias Compres
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

#ifndef VARIANT_CONTEXT_H_
#define VARIANT_CONTEXT_H_

#include <list>

#include "application.h"
#include "PropertyRequest.h"
#include "Variant.h"

typedef union ContextUnion_t {
    list<string>* entity_list; //can be wither regionID or rts callpath string
    list<string>* file_list;
} ContextUnion;

enum variant_context_type {
    PROGRAM = 0, REGION_LIST, RTS_LIST, FILE_LIST
};

typedef struct VariantContext_t {
    int          type;
    ContextUnion context_union;

    template <class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& type;
        if( type == variant_context_type( REGION_LIST ) ) {
            ar& context_union.entity_list;
        }
        else if( type == variant_context_type( RTS_LIST ) ) {
            ar& context_union.entity_list;
        }
        else if( type == variant_context_type( FILE_LIST ) ) {
            ar& context_union.file_list;
        }
    }

    bool operator==( const struct VariantContext_t& in ) const {
        if( type != in.type ) {
            return false;
        }

        if( type == variant_context_type( REGION_LIST ) ) {
            if( context_union.entity_list != in.context_union.entity_list ) {
                return false;
            }
        }
        if( type == variant_context_type( RTS_LIST ) ) {
            if( context_union.entity_list != in.context_union.entity_list ) {
                return false;
            }
        }

        if( type == variant_context_type( FILE_LIST ) ) {
            if( context_union.file_list != in.context_union.file_list ) {
                return false;
            }
        }

        return true;
    }

    bool operator!=( const struct VariantContext_t& in ) const {
        return !( *this == in );
    }
} VariantContext;

#endif
