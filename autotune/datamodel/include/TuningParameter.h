/**
   @file    TuningParameter.h
   @ingroup Autotune
   @brief   Tuning Parameter header
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

#ifndef TUNINGPARAMETER_H_
#define TUNINGPARAMETER_H_

#include <list>
#include <vector>
#include <utility>
#include <string>
#include <boost/optional.hpp>
#include <boost/serialization/export.hpp>

#include "application.h"
#include "Restriction.h"

using namespace std;

class TuningValue;

// RM: Not used, should be removed
enum tPlugin {
    UNKOWN_PLUGIN, CFS, CFSCL, MPI, MPICAP, PCAP, Energy, Pipeline, Power,
    TUTCOMBINESCEN, TUTGENSRCH, TUTMULTIOBJ, TUTMULTIRANK, TUTMULTITUNE, TUTOBJFUNC,
    TUTREGIONS, TUTSCALABILITYBASIC, TUTSCENANALYSIS, TUTTPCROSS, TUTVECTORRANGE, Readex_Intraphase, Readex_Interphase, ACP };


class TuningParameter {
    friend class boost::serialization::access;
    template<class Archive>
    void serialize( Archive&           ar,
                    const unsigned int version );

    long                    ID;
    tPlugin                 pluginType;
    runtimeTuningActionType runtimeActionType;
    string                  name; //Selector1
    int                     from, to, step, default_val;
    list<string>            stringValues;
    Restriction*            restriction; //e.g. Region

public:
    TuningParameter();

    virtual ~TuningParameter();

    long getId() const;

    void setId( long id );

    runtimeTuningActionType getRuntimeActionType() const;

    void setRuntimeActionType( runtimeTuningActionType actionType );

    tPlugin getPluginType() const;

    void setPluginType( tPlugin pluginType );

    void setDomain( string d );

    Restriction* getRestriction() const;

    void setRestriction( Restriction* restriction );

    string getName() const;

    void setName( string name );

    int getRangeFrom() const;

    int getRangeTo() const;

    int getRangeStep() const;

    int getDefaultValue() const;

    void setDefaultValue( int default_value);

    string getDomain () const;

    void setRange( int from,
                   int to,
                   int step );

    void setRangeTo( int to, int step );

    void addStringValue( string value );

//  const std::string *getStringValue(int i) const;
    const list<string>* getStringValues() const;

    bool operator==( const TuningParameter& in ) const;

    bool operator!=( const TuningParameter& in ) const;

    string toString( int    indent,
                     string indentation_character );
    string toString();

    virtual boost::optional<TuningValue>getTuningValue( int value ) const;

    bool isATP();
};

BOOST_CLASS_EXPORT_KEY( TuningParameter )

#endif /* TUNINGPARAMETER_H_ */
