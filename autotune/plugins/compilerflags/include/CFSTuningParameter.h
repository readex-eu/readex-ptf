/**
   @file    CFSTuningParameter.h
   @ingroup CompilerFlagsPlugin
   @brief   Compiler Flag Selection Tuning Parameter
   @author  Michael Gerndt
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

#ifndef CFSTuningParameter_H_
#define CFSTuningParameter_H_
#include <string>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/export.hpp>
#include "TuningParameter.h"
#include "TuningValue.h"


class CFSTuningParameter : public TuningParameter {
    vector<std::string> valueStrings;
    std::string         flagString;
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive&           ar,
                    const unsigned int version );

public:
    CFSTuningParameter() {
    }

    CFSTuningParameter( CFSTuningParameter& tp ) {
        valueStrings = tp.valueStrings;
        flagString   = tp.flagString;
    }

    void addValueString( std::string str );

    const std::string* getValueString( int i ) const;

    void setFlagString( std::string str );

    const std::string* getFlagString() const;

    std::string getFlagWithValue( int i ) const;

    boost::optional<TuningValue>getTuningValue( int value ) const;

    std::string toString();

    void removeEmptyValues();
};

BOOST_CLASS_EXPORT_KEY( CFSTuningParameter )


#endif
