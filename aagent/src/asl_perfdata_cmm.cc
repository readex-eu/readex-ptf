/**
   @file  asl_perfdata_cmm.cc
   @ingroup AnalysisAgent
   @brief   Code regions' abstraction
   @author  Edmond Kereku
   @verbatim
   Revision:       $Revision$
   Revision date:  $Date$
   Committed by:   $Author$

   This file is part of the Periscope performance measurement tool.
   See http://www.lrr.in.tum.de/periscope for details.

   Copyright (c) 2005-2011, Technische Universitaet Muenchen, Germany
   See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include "global.h"
#include "Context.h"
#include "Region.h"
#include "application.h"
#include "PerformanceDataBase.h"
#include "Property.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include "psc_errmsg.h"


//
//Implementation of class Plugin
//

Plugin::Plugin() {
    PluginId         = PLUGIN_UNKNOWN;
    TuningActionName = "";
    NumberOfVariants = 0;
}

Plugin::Plugin( PluginIdType pluginId, std::string tuningActionName, runtimeTuningActionType tuningActionType, int numberOfVariants ) {
    PluginId         = pluginId;
    TuningActionName = tuningActionName;
    TuningActionType = tuningActionType;
    NumberOfVariants = numberOfVariants;
    Rmin             = 0;
    Rmax             = 0;
    Rstep            = 0;
}

Plugin::~Plugin() {
}

void Plugin::setPluginId( const PluginIdType pluginId = PLUGIN_UNKNOWN ) {
    PluginId = pluginId;
}

PluginIdType Plugin::getPluginId() const {
    return PluginId;
}

void Plugin::setTuningActionName( const std::string& tuningActionName = "" ) {
    TuningActionName = tuningActionName;
}

std::string Plugin::getTuningActionName() const {
    return TuningActionName;
}

runtimeTuningActionType Plugin::getTuningActionType() const {
    return TuningActionType;
}

void Plugin::setNumberOfVariants( const int numberOfVariants = 0 ) {
    NumberOfVariants = numberOfVariants;
}

int Plugin::getNumberOfVariants() const {
    return NumberOfVariants;
}

void Plugin::setRmin( const int rmin = 0 ) {
    Rmin = rmin;
}

int Plugin::getRmin() const {
    return Rmin;
}

void Plugin::setRmax( const int rmax = 0 ) {
    Rmax = rmax;
}

int Plugin::getRmax() const {
    return Rmax;
}

void Plugin::setRstep( const int rstep ) {
    Rstep = rstep;
}

int Plugin::getRstep() const {
    return Rstep;
}

//
//Implementation of class CodeVariant
//

CodeVariant::CodeVariant() {
    Name  = "";
    Group = "";
}

CodeVariant::CodeVariant( std::string name, std::string group ) {
    Name  = name;
    Group = group;
}

CodeVariant::~CodeVariant() {
}

//
//Implementation of class CodeVariant
//

Target::Target() {
    TargetId = TARGET_UNKNOWN;
}

Target::Target( TargetType targetId ) {
    TargetId = targetId;
}
