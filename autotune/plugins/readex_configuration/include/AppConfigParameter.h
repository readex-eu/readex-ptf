/**
   @file    AppConfigParameter.h
   @ingroup Readex
   @brief   Application Configuration Tuning Parameter
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

#ifndef AppConfigParameter_H_
#define AppConfigParameter_H_
#include <string>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/export.hpp>
#include "TuningParameter.h"
#include "TuningValue.h"


class AppConfigParameter : public TuningParameter {
    vector<std::string> valueStrings;
    std::string         filePath;
    std::string         templateFilePath;
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive&           ar,
                    const unsigned int version );

public:
    AppConfigParameter() {
    }

    AppConfigParameter( AppConfigParameter& tp ) {
        valueStrings     = tp.valueStrings;
        filePath         = tp.filePath;
        templateFilePath = tp.templateFilePath;
    }

    void addValueString( std::string str );
    const std::string* getValueString( int i ) const;

    void setfilePath( std::string str );
    const std::string getfilePath() const;

    void setTemplateFilePath( std::string str );
    const std::string getTemplateFilePath() const;

    std::string toString();

    void removeEmptyValues();
};

BOOST_CLASS_EXPORT_KEY( AppConfigParameter )


#endif
