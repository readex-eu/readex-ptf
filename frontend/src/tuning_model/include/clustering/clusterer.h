/**
   @file    identifiers.h
   @ingroup Frontend
   @brief   Clusterer header
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

#ifndef CLUSTERING_CLUSTERER_H_INCLUDED
#define CLUSTERING_CLUSTERER_H_INCLUDED

#include <memory>
#include <unordered_set>
#include <vector>

namespace tmg {

class configuration;
class rts;

namespace cluster {

typedef double(*vector_distance_t)(const std::vector<double>&, const std::vector<double>&);
typedef double(*cluster_distance_t)(size_t, size_t, size_t, double, double, double);

class clusterer {
public:
    virtual
    ~clusterer();

    /*
        FIXME: I would preferably have std::unordered_set<std::unordered_set<rts*>>
    */
    virtual std::vector<std::unordered_set<rts*>>
    cluster(const std::unordered_set<std::unique_ptr<rts>> & rtss) const = 0;
};

class equality_clusterer final : public clusterer {
public:
    virtual
    ~equality_clusterer();

    virtual std::vector<std::unordered_set<rts*>>
    cluster(const std::unordered_set<std::unique_ptr<rts>> & rtss) const override;
};

class hierarchical_clusterer final : public clusterer {
public:
    virtual
    ~hierarchical_clusterer();

    inline
    hierarchical_clusterer(
        const vector_distance_t vector_d,
        const cluster_distance_t cluster_d)
    : vector_d_(vector_d)
    , cluster_d_(cluster_d)
    {}

    virtual std::vector<std::unordered_set<rts*>>
    cluster(const std::unordered_set<std::unique_ptr<rts>> & rtss) const override;

private:
    vector_distance_t vector_d_;
    cluster_distance_t cluster_d_;
};

}
}

#endif
