/**
   @file    conversion.cc
   @ingroup Frontend
   @brief   Conversion
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

#include "conversion.h"
#include "scenario.h"

#include "aagent/include/rts.h"

#include <unordered_map>

namespace tmg {

static tmg::region_id
create_region_id(const Rts * rts)
{
    auto r = rts->getRegion();
    return std::move(region_id(r->getFileName(), r->getFirstLine(), r->get_name()));
}

static tmg::identifiers
create_identifiers(const std::vector<Parameter_t*> & parameters)
{
    tmg::identifiers ids;
    for (const auto & p : parameters) {
        if (p->param_type == NODE_PARAMETER_UNSIGNEDINT) {
            tmg::identifier<uint64_t> id(p->param_name, convert<uint64_t>(p->param_value));
            ids.add_uint(id);
        } else if (p->param_type == NODE_PARAMETER_INTEGER) {
            tmg::identifier<int64_t> id(p->param_name, convert<int64_t>(p->param_value));
            ids.add_int(id);
        } else if (p->param_type == NODE_PARAMETER_STRING) {
            tmg::identifier<std::string> id(p->param_name, p->param_value);
            ids.add_string(id);
        } else
            assert(0 && "No case for this parameter type!");
    }

    return std::move(ids);
}


static callpath
create_callpath(const Rts * rts)
{
    std::function<void (const Rts *, callpath &)> create = [&](
        const Rts * rts,
        callpath & cp
    ){
        if (rts->getParent())
            create(rts->getParent(), cp);

        callpath_element cpe(create_region_id(rts), create_identifiers(rts->getParameter()));
        cp.add_element(std::move(cpe));
    };


    tmg::callpath cp;
    create(rts, cp);
    return std::move(cp);
}

static std::unordered_map<std::string, std::shared_ptr<tmg::tuning_parameter>>
create_tuning_parameters(const Rts * rts)
{
    std::unordered_map<std::string, std::shared_ptr<tmg::tuning_parameter>> tps;
    for (const auto & pr : rts->getTuningResults()) {
        auto & cmap = pr.second.at(RTS_BEST)->tuning_parameter_value_mapping;
        for (const auto & tr : cmap) {
            if (tps.find(tr.first->getName()) == tps.end()) {
                tps[tr.first->getName()] = std::make_shared<tmg::tuning_parameter>(
                    tr.first->getName(),
                    tr.first->getRangeFrom(), tr.first->getRangeStep(), tr.first->getRangeTo()
                );
            } else {
                throw tmg::exception("Invalid tuning parameter name");
            }
        }
    }

    return tps;
}

static configuration
create_configuration(
    const Rts * rts,
    const std::unordered_map<std::string, std::shared_ptr<tmg::tuning_parameter>> & tps)
{
    configuration c;
    for (const auto & pr : rts->getTuningResults()) {
        auto & cmap = pr.second.at(RTS_BEST)->tuning_parameter_value_mapping;
        for (const auto & tr : cmap) {
            auto it = tps.find(tr.first->getName());
            if (it != tps.end()) {
                c.add_tuning_value({it->second, tr.second});
            } else {
                throw tmg::exception("Unknown tuning parameter.");
            }
        }
    }

    if (tps.size() != c.nvalues())
        throw tmg::exception("Insufficient number of tuning values in rts configuration.");

    return std::move(c);
}

static double
exectime(const Rts * rts)
{
    auto & pr = rts->getTuningResults();
    if (pr.size() != 1)
        return 0.0;

    auto & info = pr.begin()->second.at(RTS_BEST)->extraInfo;
    if (info.find("ExecTime") == info.end())
        return 0.0;

    return info["ExecTime"];
}

static std::shared_ptr<tmg::input_id>
create_input_identifier(const std::unordered_map<std::string, std::string> & input_ids)
{
    std::unique_ptr<tmg::identifiers> ids(new tmg::identifiers());
    for (const auto & pair : input_ids)
        ids->add_string({pair.first, pair.second});

    return std::make_shared<tmg::input_id>(std::move(ids));
}

std::unordered_set<std::unique_ptr<rts>>
convert_rtss(
    const std::unordered_set<const Rts*> & old_rtss,
    const std::unordered_map<std::string, std::string> & input_ids)
{
    auto iid = create_input_identifier(input_ids);

    /* convert regions */
    std::unordered_map<region_id, std::shared_ptr<tmg::region>> regions;
    for (const auto rts : old_rtss) {
        auto r = rts->getRegion();
        region_id rid(r->getFileName(), r->getFirstLine(), r->get_name());
        if (regions.find(rid) == regions.end())
            regions[rid] = std::make_shared<region>(rid);
    }

    /* create tuning parameters */
    auto tps = create_tuning_parameters(*old_rtss.begin());

    /* convert rtss */
    std::unordered_set<std::unique_ptr<rts>> new_rtss;
    for (const auto rts : old_rtss) {
        auto r = rts->getRegion();
        region_id rid(r->getFileName(), r->getFirstLine(), r->get_name());
        TUNING_MODEL_DEBUG_ASSERT(regions.find(rid) != regions.end());
        auto region = (*regions.find(rid)).second;
        new_rtss.insert(std::make_unique<tmg::rts>(
            create_callpath(rts),
            region,
            iid,
            create_configuration(rts, tps),
            exectime(rts)));
    }

    return std::move(new_rtss);
}

}
