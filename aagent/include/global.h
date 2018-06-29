/**
   @file    global.h
   @ingroup AnalysisAgent
   @brief   Generic analysis agent configuration header
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

#ifndef GLOBAL_H_
#define GLOBAL_H_

#define BASEPORT 20123

// The PTF version.
#define _VERSION 2.0

// The number of nanoseconds per second.
#define NANOSEC_PER_SEC 1000000000

// The number of nanoseconds per second, as a double value.
#define NANOSEC_PER_SEC_DOUBLE 1000000000.0

class Context;
class DataProvider;
class Property;
class Region;
class PerformanceDataBase;
class Application;

typedef long long INT64;
typedef double FLOAT64;
typedef long INT32;

enum Gather_Required_Info_Type {
    SERIAL_INFO_GATHERED,
    PARALLEL_INFO_GATHERED,
    TASK_INFO_GATHERED,
    NOT_ALL_INFO_GATHERED,
    ALL_INFO_GATHERED
};

#include "PerformanceDataBase.h"
#include "DataProvider.h"
#include "application.h"
#include <boost/property_tree/ptree.hpp>



extern PerformanceDataBase* pdb;
extern DataProvider*        dp;
extern Application*         appl;
extern boost::property_tree::ptree configTree;
extern bool rts_support;
// extern Prop_List foundProperties; in strategy.h



extern bool TEST;
extern void print_property_set( std::list <Property*> property_set,
                                char*                 str,
                                bool                  with_severity = true );

extern bool withRtsSupport();

void generate_tuning_model();

#endif /* GLOBAL_H_ */
