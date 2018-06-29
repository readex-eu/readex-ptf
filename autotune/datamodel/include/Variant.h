/**
   @file	Variant.h
   @ingroup Autotune
   @brief   Variant
   @author	Houssam Haitof
   @verbatim
        Revision:       $Revision$
        Revision date:  $Date$
        Committed by:   $Author$

        This file is part of the Periscope performance measurement tool.
        See http://www.lrr.in.tum.de/periscope for details.

        Copyright (c) 2005-2012, Technische Universitaet Muenchen, Germany
        See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#ifndef VARIANT_H_
#define VARIANT_H_

#include <map>

#include "TuningParameter.h"
#include <boost/serialization/map.hpp>

using namespace std;

class Variant {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive& ar, const unsigned int version ) {
        ar& value;
    }
private:
    map<TuningParameter*, int> value;
public:
    Variant();

    Variant( map<TuningParameter*, int> value );

    map<TuningParameter*, int>getValue() const;

    void setValue( map<TuningParameter*, int> value );

    bool operator==( const Variant& in ) const;

    bool operator!=( const Variant& in ) const;

    string toString( int    indent,
                     string indentation_character );
};

#endif /* VARIANT_H_ */
