/**
   @file    matrix.h
   @ingroup Frontend
   @brief   Matrix header
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

#ifndef CLUSTERING_MATRIX_H_INCLUDED
#define CLUSTERING_MATRIX_H_INCLUDED

#include "common.h"

#include <limits>
#include <unordered_map>

namespace tmg {
namespace cluster {

class matrix;
class node;

/*
    FIXME: Remove minimum method and add iterators.
*/
class matrix final {
public:
    inline void
    set(cluster::node * c1, cluster::node * c2, double d) noexcept
    {
        auto pair = get_row_clm(c1, c2);
        cluster::node * row = pair.first;
        cluster::node * clm = pair.second;
        data_[row][clm] = d;
    }

    inline double
    get(cluster::node * c1, cluster::node * c2) const
    {
        TUNING_MODEL_DEBUG_ASSERT(has(c1, c2));

        auto pair = get_row_clm(c1, c2);
        cluster::node * row = pair.first;
        cluster::node * clm = pair.second;
        return data_.at(row).at(clm);
    }

    inline bool
    has(cluster::node * c1, cluster::node * c2) const
    {
        auto pair = get_row_clm(c1, c2);
        cluster::node * row = pair.first;
        cluster::node * clm = pair.second;

        auto it = data_.find(row);
        if (it == data_.end())
            return false;
        if (it->second.find(clm) == it->second.end())
            return false;

        return true;
    }

    inline void
    remove(cluster::node * c)
    {
        data_.erase(c);
        for (auto & pair : data_)
            pair.second.erase(c);
    }

    inline std::pair<cluster::node*, cluster::node*>
    minimum(double * d) const noexcept
    {
        cluster::node * c1 = nullptr;
        cluster::node * c2 = nullptr;
        double min = std::numeric_limits<double>::max();
        for (const auto & p1 : data_) {
            for (const auto & p2 : p1.second) {
                if (p1.first == p2.first)
                    continue;

                if (p2.second < min) {
                    c1 = p1.first;
                    c2 = p2.first;
                    min = p2.second;
                }
            }
        }

        *d = min;
        return std::make_pair(c1, c2);
    }

private:
    inline std::pair<cluster::node*, cluster::node*>
    get_row_clm(cluster::node * c1, cluster::node * c2) const noexcept
    {
        cluster::node * row = c1;
        cluster::node * clm = c2;
        if (c2 < c1) {
            row = c2;
            clm = c1;
        }

        return std::make_pair(row, clm);
    }

    std::unordered_map<
        cluster::node*,
        std::unordered_map<cluster::node*, double>
    > data_;
};

}
}

#endif
