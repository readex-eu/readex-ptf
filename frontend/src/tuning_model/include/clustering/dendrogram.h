/**
   @file    dendrogram.h
   @ingroup Frontend
   @brief   Dendrogram header
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

#ifndef CLUSTERING_DENDROGRAM_H_INCLUDED
#define CLUSTERING_DENDROGRAM_H_INCLUDED

#include <common.h>

#include <cmath>
#include <memory>
#include <unordered_set>
#include <vector>

namespace tmg {

class configuration;
class rts;

namespace cluster {

typedef double(*vector_distance_t)(const std::vector<double>&, const std::vector<double>&);

typedef double(*cluster_distance_t)(
    size_t ni, size_t nj, size_t nk,
    double dik, double djk, double dij
);

class node {
public:
    virtual
    ~node();

    virtual size_t
    nelements() const noexcept = 0;

    virtual double
    distance() const noexcept = 0;

    virtual const node *
    left() const noexcept = 0;

    virtual const node *
    right() const noexcept = 0;

    std::string
    to_str() const;
};

class branch final : public node {
public:
    virtual
    ~branch();

    inline
    branch(double distance, std::unique_ptr<node> l, std::unique_ptr<node> r)
    : distance_(distance)
    , nelements_(l->nelements() + r->nelements())
    , left_(std::move(l))
    , right_(std::move(r))
    {
        TUNING_MODEL_DEBUG_ASSERT(distance > 0.0);
    }

    branch(const branch & other) = delete;

    branch(branch && other) = default;

    branch &
    operator=(const branch & other) = delete;

    branch &
    operator=(branch && other) = default;

    virtual const node *
    left() const noexcept override;

    virtual const node *
    right() const noexcept override;

    virtual double
    distance() const noexcept override;

    virtual size_t
    nelements() const noexcept override;

private:
    double distance_;
    size_t nelements_;
    std::unique_ptr<node> left_;
    std::unique_ptr<node> right_;
};

class leaf final : public node {
public:
    virtual
    ~leaf();

    inline
    leaf(tmg::rts * rts)
    : rts_(rts)
    {}

    leaf(const leaf & other) = delete;

    leaf(leaf && other) = default;

    leaf &
    operator=(const leaf & other) = delete;

    leaf &
    operator=(leaf && other) = default;

    virtual const node *
    left() const noexcept override;

    virtual const node *
    right() const noexcept override;

    virtual size_t
    nelements() const noexcept override;

    virtual double
    distance() const noexcept override;

    inline tmg::rts *
    rts() const noexcept
    {
        return rts_;
    }

private:
    tmg::rts * rts_;
};

std::unique_ptr<node>
generate_dendrogram(
    const std::unordered_set<std::unique_ptr<rts>> & rtss,
    const vector_distance_t d,
    const cluster_distance_t);

/*
    within-cluster variation
*/
double
wcv(const std::unordered_set<const node*> & clusters, const vector_distance_t d);

/*
    between-cluster variation
*/
double
bcv(const std::unordered_set<const node*> & clusters, const vector_distance_t d);

std::unordered_set<const node*>
cut(const node & root, double value);

std::unordered_set<const node*>
cut(const node & root, size_t nclusters);

std::unordered_set<const node*>
cut(const node & root, size_t n, const vector_distance_t d);

static inline std::unordered_set<const node*>
cut(const node & root, const vector_distance_t d)
{
    return cut(root, root.nelements(), d);
}

}
}

#endif
