/**
   @file    dendrogram.cc
   @ingroup Frontend
   @brief   Dendrogram
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

#include "clustering/dendrogram.h"
#include "clustering/matrix.h"
#include "clustering/vector.h"
#include "rts.h"

#include <algorithm>

namespace tmg {
namespace cluster {

/* dendrogram nodes */

node::~node()
{}

std::string
node::to_str() const
{
    std::function<std::string(const node*, size_t)> f = [&](const node * n, size_t depth)
    {
        std::string s(depth, '-');
        if (auto l = dynamic_cast<const leaf*>(n)) {
            s += l->rts()->debug_string() + "\n";
        } else {
            s += strfmt(dynamic_cast<const branch*>(n)->distance(), "\n");
        }

        if (n->left() != nullptr)
            s += f(n->left(), depth+1);
        if (n->right() != nullptr)
            s += f(n->right(), depth+1);
        return s;
    };

    return f(this, 0);
}

branch::~branch()
{}

size_t
branch::nelements() const noexcept
{
    return nelements_;
}

const node *
branch::left() const noexcept
{
    return left_.get();
}

const node *
branch::right() const noexcept
{
    return right_.get();
}

double
branch::distance() const noexcept
{
    return distance_;
}

leaf::~leaf()
{}

size_t
leaf::nelements() const noexcept
{
    return 1;
}

const node *
leaf::left() const noexcept
{
    return nullptr;
}

const node *
leaf::right() const noexcept
{
    return nullptr;
}

double
leaf::distance() const noexcept
{
    return 0.0;
}

/* dendrogram generation */

static inline std::unique_ptr<matrix>
generate_distance_matrix(
    const std::unordered_set<node*> & clusters,
    const vector_distance_t d)
{
    auto D = std::make_unique<matrix>();
    for (const auto & c1 : clusters) {
        for (const auto & c2 : clusters) {
            TUNING_MODEL_DEBUG_ASSERT(dynamic_cast<const leaf*>(c1));
            TUNING_MODEL_DEBUG_ASSERT(dynamic_cast<const leaf*>(c2));
            auto l1 = static_cast<const leaf*>(c1);
            auto l2 = static_cast<const leaf*>(c2);

            auto d12 = d(l1->rts()->configuration().to_vector(),
                l2->rts()->configuration().to_vector());
            D->set(c1, c2, d12);
        }
    }

    return std::move(D);
}

static inline std::pair<node *, node *>
find_closest_clusters(
    const std::unordered_set<node*> & clusters,
    const std::unique_ptr<matrix> & D,
    double * d)
{
    node * ci;
    node * cj;
    double min = std::numeric_limits<double>::max();
    for (const auto & c1 : clusters) {
        for (const auto & c2 : clusters) {
            auto nd = D->get(c1, c2);
            if (nd < min) {
                ci = c1;
                cj = c2;
                min = nd;
            }
        }
    }

    *d = min;
    return std::make_pair(ci, cj);
}

std::unique_ptr<node>
generate_dendrogram(
    const std::unordered_set<std::unique_ptr<rts>> & rtss,
    const vector_distance_t vector_d,
    const cluster_distance_t clstd)
{
    std::unordered_set<node*> clusters;
    for (const auto & rts : rtss)
        clusters.insert(new leaf(rts.get()));

    auto D = generate_distance_matrix(clusters, vector_d);

    while (clusters.size() != 1) {
        /* find closest clusters */
        double dij;
        auto pair = D->minimum(&dij);
        auto ci = pair.first;
        auto cj = pair.second;

        /* create new cluster */
        auto uci = std::unique_ptr<node>(ci);
        auto ucj = std::unique_ptr<node>(cj);
        auto cij = new branch(dij, std::move(uci), std::move(ucj));
        TUNING_MODEL_DEBUG_ASSERT(ci == cij->left());
        TUNING_MODEL_DEBUG_ASSERT(cj == cij->right());
        TUNING_MODEL_DEBUG_ASSERT(uci.get() == nullptr);
        TUNING_MODEL_DEBUG_ASSERT(ucj.get() == nullptr);

        /* remove old clusters from cluster set */
        clusters.erase(ci);
        clusters.erase(cj);

        /* add new cluster to distance matrix and cluster set */
        for (const auto & ck : clusters) {
            auto dkij = clstd(ci->nelements(), cj->nelements(), ck->nelements(),
                D->get(ci, ck), D->get(cj, ck), dij);
            D->set(ck, cij, dkij);
        }
        clusters.insert(cij);

        /* remove old clusters from cluster set and distance matrix */
        D->remove(ci);
        D->remove(cj);
    }

    TUNING_MODEL_DEBUG_ASSERT(clusters.size() == 1);
    return std::unique_ptr<node>(*clusters.begin());
}

std::unordered_set<const node*>
cut(const node & root, double value)
{
    std::unordered_set<const node*> nodes;
    if (dynamic_cast<const leaf*>(&root) || root.distance() <= value) {
        nodes.insert(&root);
        return nodes;
    }

    auto s = cut(*root.left(), value);
    nodes.insert(s.begin(), s.end());
    s = cut(*root.right(), value);
    nodes.insert(s.begin(), s.end());
    return nodes;
}

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

std::unordered_set<const node*>
cut(const node & root, size_t nclusters)
{
    if (nclusters == 0)
        return std::unordered_set<const node*>();

    if (nclusters >= root.nelements())
        return leaves(root);

    struct node_compare {
        bool
        operator()(const node * n1, const node * n2)
        {
            return n1->distance() < n2->distance();
        }
    };

    std::unordered_set<const node*> clusters;
    std::set<const node*, node_compare> to_visit({&root});
    while (to_visit.size() + clusters.size() != nclusters) {
        auto max = *to_visit.rbegin();
        to_visit.erase(max);

        if (dynamic_cast<const leaf*>(max->left()))
            clusters.insert(max->left());
        else
            to_visit.insert(max->left());

        if (dynamic_cast<const leaf*>(max->right()))
            clusters.insert(max->right());
        else
            to_visit.insert(max->right());
    }

    clusters.insert(to_visit.begin(), to_visit.end());
    return clusters;
}

static std::vector<double>
average(const node & cluster)
{
    auto elements = leaves(cluster);
    auto v = dynamic_cast<const leaf*>(*elements.begin())->rts()->configuration().to_vector();
    for (auto it = std::next(elements.begin()); it != elements.end(); it++)
        v = v + dynamic_cast<const leaf*>(*it)->rts()->configuration().to_vector();

    return v * (1.0 / elements.size());
}

static std::vector<double>
average(const std::unordered_set<const node*> & clusters)
{
    std::unordered_set<const node*> all_elements;
    for (const auto & c : clusters) {
        auto elements = leaves(*c);
        all_elements.insert(elements.begin(), elements.end());
    }

    auto v = dynamic_cast<const leaf*>(*all_elements.begin())->rts()->configuration().to_vector();
    for (auto it = std::next(all_elements.begin()); it != all_elements.end(); it++)
        v = v + dynamic_cast<const leaf*>(*it)->rts()->configuration().to_vector();

    return v * (1.0 / all_elements.size());
}

double
wcv(const std::unordered_set<const node*> & clusters, const vector_distance_t d)
{
    double sum = 0.0;
    for (const auto & c : clusters) {
        auto avg = average(*c);
        auto elements = leaves(*c);
        for (const auto & e : elements) {
            auto v = dynamic_cast<const leaf*>(e)->rts()->configuration().to_vector();
            sum += d(v, avg) * d(v, avg);
        }
    }

    return sum;
}

double
bcv(const std::unordered_set<const node*> & clusters, const vector_distance_t d)
{
    auto overall_avg = average(clusters);

    double sum = 0.0;
    for (const auto & c : clusters) {
        auto avg = average(*c);
        sum += c->nelements() * (d(avg, overall_avg) * d(avg, overall_avg));
    }

    return sum;
}

std::unordered_set<const node*>
cut(const node & root, size_t n, const vector_distance_t d)
{
    if (n == 0)
        return std::unordered_set<const node*>();

    if (n == 1)
        return std::unordered_set<const node*>({&root});

    /*
        Calinski and Harabasz index
    */
    size_t nclusters = 0;
    double max = std::numeric_limits<double>::min();
    for (size_t K = 2; K <= n; K++) {
        auto clusters = cut(root, K);
        double ch = (bcv(clusters, d) / (K-1)) / (wcv(clusters, d) / (n-K));
        if (ch > max) {
            max = ch;
            nclusters = K;
        }
    }

    return cut(root, nclusters);
}

}
}
