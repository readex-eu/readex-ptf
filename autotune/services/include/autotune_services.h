/**
   @file    autotune_services.h
   @ingroup Autotune
   @brief   Common functionality used by tuning plugins header
   @author  Isaias Compres, Toni Pimenta, Michael Gerndt, Robert Mijakovic
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

#ifndef _AUTOTUNE_SERVICES_
#define _AUTOTUNE_SERVICES_

#include "ISearchAlgorithm.h"
#include "IPlugin.h"
#include "DriverContext.h"

#include <vector>
#include <string>
using namespace std;

vector<TuningParameter*>extractTuningParameters( void );

void print_loaded_search( int    major,
                          int    minor,
                          string name,
                          string description );

void print_loaded_plugin( int    major,
                          int    minor,
                          string name,
                          string description );

void generate_context_argc_argv( DriverContext* context,
                                 list<string>   parameters,
                                 char*          exec_name );

list<Region*>extractMPICallRegions( void );

#endif
