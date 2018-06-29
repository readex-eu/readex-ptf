/**
   @file    readex_configuration.h
   @ingroup Communication
   @brief   Readex Configuration header
   @author Anamika Chowdhury
   @date 26.10.2016
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2016 Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#ifndef READEX_CONFIGURATION_H
#define READEX_CONFIGURATION_H

//#include "../../readex/cube_tools/datamodel/include/SignificantRegion.h"

/*struct Readex_Configuration
{
    SignificantRegion sigRegions;
    Readex_Metrics metrics;
    Readex_tuning_parameter tuning_parameter;

};*/

struct Readex_Metrics{
    double node_energy;
    double cpu0_energy;
    double cpu1_energy;
    Readex_Metrics() : node_energy( 0.0 ), cpu0_energy( 0.0 ), cpu1_energy( 0.0 )
    {
    }
    Readex_Metrics( double node_energy, double cpu0_energy, double cpu1_energy ): node_energy( node_energy ), cpu0_energy( cpu0_energy ), cpu1_energy( cpu1_energy ){}
};
typedef Readex_Metrics Readex_Metrics;

struct Freq_parameter{
    double min_frequency;
    double max_frequency;
    double step_frequency;
};
typedef Freq_parameter Freq_parameter;

struct OpenMP_parameter{
    double lower_num;
    double incremental_step;
    double power_step;
};
typedef OpenMP_parameter OpenMP_parameter;

struct Readex_tuning_parameter{
    Freq_parameter freq_parameter;
    OpenMP_parameter openMP_parameter;
};
typedef Readex_tuning_parameter Readex_tuning_parameter;



#endif /*READEX_CONFIGURATION_H*/
