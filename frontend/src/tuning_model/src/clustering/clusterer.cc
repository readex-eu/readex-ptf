/**
   @file    clusterer.cc
   @ingroup Frontend
   @brief   Clusterer
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
#include "clustering/dendrogram.h"
#include "configuration.h"
#include "rts.h"

namespace tmg {
namespace cluster {

/* clusterer */

clusterer::~clusterer()
{}

/* equality clusterer */

equality_clusterer::~equality_clusterer()
{}

std::vector<std::unordered_set<tmg::rts*>>
equality_clusterer::cluster(const std::unordered_set<std::unique_ptr<rts>> & rtss) const
{
    std::unordered_map<configuration, std::unordered_set<rts*>> map;
    for (const auto & rts : rtss) {
        if (map.find(rts->configuration()) == map.end())
            map[rts->configuration()] = std::unordered_set<tmg::rts*>({rts.get()});
        else
            map[rts->configuration()].insert(rts.get());
    }

    std::vector<std::unordered_set<tmg::rts*>> clusters;
    for (const auto & pair : map)
        clusters.emplace_back(pair.second);

    return clusters;
}

/* hierarchical clusterer */

/*
    FIXME: integrate this function into the dendrogram
*/
static std::unordered_set<const node*>
leaves(const node & root)
{
    if (dynamic_cast<const leaf*>(&root))
        return std::unordered_set<const node*>({&root});

    TUNING_MODEL_DEBUG_ASSERT(root.left());
    auto clusters = leaves(*root.left());

    TUNING_MODEL_DEBUG_ASSERT(root.right());
    auto tmp = leaves(*root.right());
    clusters.insert(tmp.begin(), tmp.end());

    return clusters;
}

hierarchical_clusterer::~hierarchical_clusterer()
{}

std::vector<std::unordered_set<tmg::rts*>>
hierarchical_clusterer::cluster(const std::unordered_set<std::unique_ptr<rts>> & rtss) const
{
    auto dgram = generate_dendrogram(rtss, vector_d_, cluster_d_);
    auto nodes = cut(*dgram, vector_d_);

    size_t n = 0;
    std::vector<std::unordered_set<tmg::rts*>> clusters;
    for (const auto & node : nodes) {
        auto lvs = leaves(*node);
        std::unordered_set<tmg::rts*> cluster;
        for (const auto & l : lvs) {
            cluster.insert(dynamic_cast<const tmg::cluster::leaf*>(l)->rts());
        }
        clusters.emplace_back(cluster);
        n++;
    }

    return clusters;
}

}
}
