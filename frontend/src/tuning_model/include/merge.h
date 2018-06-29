/**
   @file    merge.h
   @ingroup Frontend
   @brief   Merge header
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

#ifndef MERGE_H_INCLUDED
#define MERGE_H_INCLUDED

#include "configuration.h"

#include <memory>
#include <unordered_set>
#include <vector>

namespace tmg {
namespace cluster {
    class clusterer;
}

class scenario;

typedef std::unordered_set<std::unique_ptr<scenario>> scenario_set;

scenario_set
merge(
    const std::vector<scenario_set> & scnrvector,
    const tmg::configuration_selector_t & selector,
    const tmg::cluster::clusterer & clusterer);

}

#endif
