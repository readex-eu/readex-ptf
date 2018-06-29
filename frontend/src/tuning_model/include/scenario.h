/**
   @file    scenario.h
   @ingroup Frontend
   @brief   Scenario header
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

#ifndef SCENARIO_H_INCLUDED
#define SCENARIO_H_INCLUDED

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "rts.h"

namespace tmg {

class scenario {
public:
    inline
    scenario(const std::unordered_set<rts*> & rtss, const configuration_selector_t & selector)
    {
        std::unordered_set<const tmg::configuration*> cs;
        for (const auto & rts : rtss) {
            auto r = rts->copy();
            cs.insert(&r->configuration());
            r->assign_scenario(this);
            rtss_.insert(std::move(r));
        }
        config_ = selector(cs);
    }

	inline std::unordered_set<rts*>
	rtss() const
	{
        std::unordered_set<rts*> rtss;
        for (const auto & rts : rtss_)
            rtss.insert(rts.get());

		return rtss;
	}

    inline size_t
    nrtss() const noexcept
    {
        return rtss_.size();
    }

    inline const tmg::configuration &
    configuration() const
    {
        return config_;
    }

private:
    tmg::configuration config_;
    std::unordered_set<std::unique_ptr<rts>> rtss_;
};

}

#endif /* SCENARIO_H_INCLUDED */
