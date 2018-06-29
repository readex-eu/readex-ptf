/**
   @file    conversion.h
   @ingroup Frontend
   @brief   Conversion header
   @author  Nico Reissmann
   @verbatim
        Revision:       $Revision$
        Revision date:  $Date$
        Committed by:   $Author$

        This file is part of the Periscope performance measurement tool.
        See http://www.lrr.in.tum.de/periscope for details.

        Copyright (c) 2005-2016, Technische Universitaet Muenchen, Germany
        See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#ifndef TUNING_MODEL_H_INCLUDED
#define TUNING_MODEL_H_INCLUDED

#include <memory>
#include <unordered_map>
#include <unordered_set>

class Rts;

namespace tmg {

class rts;

std::unordered_set<std::unique_ptr<rts>>
convert_rtss(
    const std::unordered_set<const Rts*> & rtss,
    const std::unordered_map<std::string, std::string> & input_ids);

}

#endif
