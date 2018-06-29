/**
   @file    serialization.h
   @ingroup Frontend
   @brief   Tuning model serialization header
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

#ifndef SERIALIZATION_H_INCLUDED
#define SERILIAZATION_H_INCLUDED

#include <phase.h>
#include <scenario.h>

#include <memory>
#include <unordered_set>

namespace tmg {

std::string
serialize(
    const std::unordered_set<std::unique_ptr<scenario>> & scenarios,
    const tmg::cluster_phases & cphases);

std::unordered_set<std::unique_ptr<scenario>>
deserialize(std::istream & is, const configuration_selector_t & selector);

}

#endif /* SERIALIZATION_H_INCLUDED */
