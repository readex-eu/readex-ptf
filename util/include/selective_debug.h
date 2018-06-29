/**
   @file    selective_debug.h
   @brief   Simple debugging/logging functionality header
   @author  Michael Gerndt
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

#ifndef SELECTIVEDEBUG_H_INCLUDED
#define SELECTIVEDEBUG_H_INCLUDED

#include <string>
#include "psc_errmsg.h"
#include "timing.h"

// NOTE: all the selective debug levels were moved to psc_errmsg.h

void handle_dbgLevelList( const std::string& dbgList );

void print_dbgLevelsDefs();

#endif /* SELECTIVEDEBUG_H_INCLUDED */
