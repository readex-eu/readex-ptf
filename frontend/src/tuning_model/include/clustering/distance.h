/**
   @file    distance.h
   @ingroup Frontend
   @brief   Distance header
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

#ifndef CLUSTERING_DISTANCE_H_INCLUDED
#define CLUSTERING_DISTANCE_H_INCLUDED

#include <cmath>

namespace tmg {
namespace cluster {

class node;

typedef double(*cluster_distance_t)(
    size_t ni, size_t nj, size_t nk,
    double dik, double djk, double dij
);

static inline double
f00(size_t, size_t, size_t)
{
    return 0.0;
}

static inline double
f05(size_t, size_t, size_t)
{
    return 0.5;
}

static inline double
fm05(size_t, size_t, size_t)
{
    return -0.5;
}

static inline double
avgi(size_t ni, size_t nj, size_t nk)
{
    return ni / (double)(ni + nj);
}

static inline double
avgj(size_t ni, size_t nj, size_t nk)
{
    return avgi(nj, ni, nk);
}

static inline double
centroid_beta(size_t ni, size_t nj, size_t nk)
{
    size_t nij = ni+nj;
    return -1.0 * ((ni * nj) / (double)(nij*nij));
}

static inline double
ward_alphai(size_t ni, size_t nj, size_t nk)
{
    size_t nik = ni + nk;
    return nik / (double)(nik + nj);
}

static inline double
ward_alphaj(size_t ni, size_t nj, size_t nk)
{
    return ward_alphai(nj, ni, nk);
}

static inline double
ward_beta(size_t ni, size_t nj, size_t nk)
{
    return -1.0 * (nk / (double)(ni + nj + nk));
}

template<
    double (*alphai)(size_t, size_t, size_t),
    double (*alphaj)(size_t, size_t, size_t),
    double (*beta)(size_t, size_t, size_t),
    double (*gamma)(size_t, size_t, size_t)
> static inline double
distance(
    size_t ni, size_t nj, size_t nk,
    double dik, double djk, double dij)
{
    /* Lance-Williams Algorithm */
    return alphai(ni, nj, nk) * dik
         + alphaj(nj, nj, nk) * djk
         + beta(ni, nj, nk) * dij
         + gamma(ni, nj, nk) * std::fabs(dik - djk);
}

static const cluster_distance_t & single_linkage = &distance<f05, f05, f00, fm05>;
static const cluster_distance_t & complete_linkage = &distance<f05, f05, f00, f05>;
static const cluster_distance_t & group_average = &distance<avgi, avgj, f00, f00>;
static const cluster_distance_t & weighted_group_average = &distance<f05, f05, f00, f00>;
static const cluster_distance_t & centroid = &distance<avgi, avgj, centroid_beta, f00>;
static const cluster_distance_t & ward = &distance<ward_alphai, ward_alphaj, ward_beta, f00>;

}
}

#endif
