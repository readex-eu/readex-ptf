/**
   @file    phase.cc
   @ingroup Frontend
   @brief   Phase
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

#include "phase.h"

#include "aagent/include/rts.h"

#include <iostream>

namespace tmg {

static std::unique_ptr<phase_data>
create_phase_data(const Rts * rts)
{
    auto piranges = rts->getTuningResults().begin()->second.at(STATIC_BEST)->phase_identifier_range;
    auto cphases = rts->getTuningResults().begin()->second.at(STATIC_BEST)->phases_in_cluster;

    return std::unique_ptr<phase_data>(new phase_data(cphases, piranges));
}

cluster_phases
extract_cluster_phases(const Rts * root)
{
    TUNING_MODEL_DEBUG_ASSERT(root->getParent() == nullptr);

    cluster_phases pd;
    for (const auto & child : root->getChildren()) {
        if (child->getParameter().size() == 1) {
            auto parameter = child->getParameter().front();
            if (parameter->param_type != NODE_PARAMETER_INTEGER)
                std::cerr << "TM ERROR: Node parameter is not of type integer.\n";
            size_t id = std::stoi(parameter->param_value);
            pd[id] = std::move(create_phase_data(child));
        }
    }

    return pd;
}

}
