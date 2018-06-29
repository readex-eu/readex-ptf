/**
   @file    SignificantRegion.h
   @ingroup READEX
   @brief   Data Struction declaration
   @author  Anamika Chowdhury
   @verbatim
   Revision:       $Revision$
   Revision date:  $Date$
   Committed by:   $Author$

   This file is part of READEX.

   Copyright (c) 2016, Technische Universitaet Muenchen, Germany
   See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#ifndef READEX_SIGNIFICANTREGION_H
#define READEX_SIGNIFICANTREGION_H

#include <string>
#include <iostream>
#include <vector>
#include <stdint.h>

struct DynamismMetric
{
    std::string metricName;
    double      min;
    double      max;
    double      execTime;
    double      dev_perc_reg;
    double      dev_perc_phase;
    double      dyn_perc_phase;
    bool        has_variation;
    DynamismMetric( std::string mname,
                    double      min,
                    double      max,
                    double      exec_t,
                    double      dev_reg,
                    double      dev_phase,
                    double      variation_phase ) : metricName( mname ),
                                                    min( min ),
                                                    max( max ),
                                                    execTime( exec_t ),
                                                    dev_perc_reg( dev_reg ),
                                                    dev_perc_phase( dev_phase ),
                                                    dyn_perc_phase( variation_phase ),
                                                    has_variation( false ){}
};
typedef DynamismMetric DynamismMetric;

struct SignificantRegion
{
    std::string                   name;
    double                        granularity;
    //std::string                   source_location;
    //long                          line_no;
    std::vector< DynamismMetric > dynamism_metrics;
};
typedef SignificantRegion SignificantRegion;

struct PhaseRegion
{
    std::string name;
    double      granularity;
    double      min;
    double      max;
    double      mean;
    double      absTime;
    double      dev_perc;
    double      var_perc;
    bool        has_dynamism;
};
typedef PhaseRegion PhaseRegion;

#endif //READEX_SIGNIFICANTREGION_H
