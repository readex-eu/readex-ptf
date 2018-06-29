/**
   @file    tuning_model.h
   @ingroup Frontend
   @brief   Tuning model header
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

#include <clustering/clusterer.h>
#include <configuration.h>

namespace tmg {
namespace cluster {

class clusterer;

}

std::string
generate_tuning_model(
    const Rts * root,
    const std::unordered_map<std::string, std::string> & input_ids,
    const tmg::cluster::clusterer & clusterer,
    const tmg::configuration_selector_t & selector);

}

#endif /* TUNING_MODEL_H_INCLUDED */
