/**
   @file    merge.cc
   @ingroup Frontend
   @brief   Tuning model merging
   @author  Nico Reissmann
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2016, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include "clustering/clusterer.h"
#include "merge.h"
#include "rts.h"
#include "scenario.h"

namespace tmg {

scenario_set
merge(
    const std::vector<scenario_set> & scnrvector,
    const configuration_selector_t & selector,
    const tmg::cluster::clusterer & clusterer)
{

    std::unordered_set<tmg::rtsid> rtsids;
    std::unordered_set<std::unique_ptr<tmg::rts>> rtss;
    for (const auto & scnrset : scnrvector) {
        for (const auto & scnr : scnrset) {
            auto scnrrtss = scnr->rtss();
            for (const auto & rts : scnrrtss) {
                if (rtsids.find(rts->rtsid()) == rtsids.end()) {
                    rtss.insert(rts->copy());
                    rtsids.insert(rts->rtsid());
                }
            }
        }
    }

    auto clusters = clusterer.cluster(rtss);

    std::unordered_set<std::unique_ptr<tmg::scenario>> scenarios;
    for (const auto & cluster : clusters)
        scenarios.insert(std::move(std::make_unique<tmg::scenario>(cluster, selector)));

    return scenarios;
}

}
