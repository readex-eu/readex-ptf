/**
   @file    phase.h
   @ingroup Frontend
   @brief   Phase header
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

#ifndef PHASE_H_INCLUDED
#define PHASE_H_INCLUDED

#include "common.h"

#include <memory>
#include <set>
#include <unordered_map>

class Rts;

namespace tmg {

class phase_data final {
public:
    inline
    phase_data(
        const std::set<unsigned int> & cluster_phases,
        const std::unordered_map<std::string, std::pair<double, double>> & phase_ranges)
    : cluster_phases_(cluster_phases)
    , phase_ranges_(phase_ranges)
    {}

    inline const std::set<unsigned int> &
    cluster_phases() const noexcept
    {
        return cluster_phases_;
    }

    inline const std::unordered_map<std::string, std::pair<double, double>> &
    phase_ranges() const noexcept
    {
        return phase_ranges_;
    }

private:
    std::set<unsigned int> cluster_phases_;
    std::unordered_map<std::string, std::pair<double, double>> phase_ranges_;
};

typedef std::unordered_map<ssize_t, std::unique_ptr<phase_data>> cluster_phases;

cluster_phases
extract_cluster_phases(const Rts * root);

}

#endif
