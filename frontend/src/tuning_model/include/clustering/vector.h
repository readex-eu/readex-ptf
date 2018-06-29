/**
   @file    matrix.h
   @ingroup Frontend
   @brief   Vector header
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

#ifndef CLUSTERING_VECTOR_H_INCLUDED
#define CLUSTERING_VECTOR_H_INCLUDED

#include "common.h"

#include <vector>

namespace tmg {
namespace cluster {

static inline std::vector<double>
operator+(const std::vector<double> & v1, const std::vector<double> & v2)
{
    TUNING_MODEL_DEBUG_ASSERT(v1.size() == v2.size());

    std::vector<double> v;
    for (size_t n = 0; n < v1.size(); n++)
        v.push_back(v1[n] + v2[n]);

    return v;
}

static inline std::vector<double>
operator*(const std::vector<double> & v1, const std::vector<double> & v2)
{
    TUNING_MODEL_DEBUG_ASSERT(v1.size() == v2.size());

    std::vector<double> v;
    for (size_t n = 0; n < v1.size(); n++)
        v.push_back(v1[n] * v2[n]);

    return v;
}

static inline std::vector<double>
operator*(const std::vector<double> & v1, double f)
{
    return v1 * std::vector<double>(v1.size(), f);
}

static inline double
euclidean_distance(const std::vector<double> & v1, const std::vector<double> & v2)
{
    TUNING_MODEL_DEBUG_ASSERT(v1.size() == v2.size());

    double d = 0.0;
    for (size_t n = 0; n < v1.size(); n++)
        d += (v1[n] - v2[n]) * (v1[n] - v2[n]);

    return sqrt(d);
}

}
}

#endif
