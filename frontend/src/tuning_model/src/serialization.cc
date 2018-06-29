/**
   @file    serialization.cc
   @ingroup Frontend
   @brief   Tuning model seriliazation
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

#include "scenario.h"
#include "serialization.h"

#include <cereal/archives/json.hpp>

#include <sstream>

namespace cereal {

typedef struct tuning_parameter {
    inline
    tuning_parameter()
    : end(0)
    , step(0)
    , start(0)
    {}

    int end;
    int step;
    int start;
    std::string name;
} tuning_parameter;

typedef struct tuning_value {
    inline
    tuning_value()
    : value(0)
    {}

    int value;
    tuning_parameter parameter;
} tuning_value;

typedef std::unordered_set<std::shared_ptr<tuning_value>> configuration;

typedef struct rts {
    inline
    rts()
    : iid(0)
    , region(0)
    , exectime(0.0)
    , scenario(0)
    {}

    uint64_t iid;
    uint64_t region;
    double exectime;
    uint64_t scenario;
    tmg::callpath callpath;
} rts;

typedef std::unordered_set<std::shared_ptr<rts>> rtss;

/* clusters */

template <class Archive> void
save(
    Archive & archive,
    const std::pair<std::string, std::pair<double, double>> & range)
{
    archive(make_nvp("feature", range.first));
    archive(make_nvp("start", range.second.first));
    archive(make_nvp("end", range.second.second));
}

template <class Archive> void
save(
    Archive & archive,
    const std::unordered_map<std::string, std::pair<double, double>> & ranges)
{
    archive(make_size_tag(static_cast<size_type>(ranges.size())));
    for (const auto & pair : ranges)
        archive(pair);
}

template <class Archive> void
save(
    Archive & archive,
    const std::set<unsigned int> & phases)
{
    archive(make_size_tag(static_cast<size_type>(phases.size())));
    for (const auto & phase : phases)
        archive(phase);
}

template <class Archive> void
save(
    Archive & archive,
    const std::pair<ssize_t, const tmg::phase_data*> & pdata)
{
    archive(make_nvp("clusterid", pdata.first));
    archive(make_nvp("cluster_phases", pdata.second->cluster_phases()));
    archive(make_nvp("phase_ranges", pdata.second->phase_ranges()));
}

template <class Archive> void
save(Archive & archive, const tmg::cluster_phases & cphases)
{

    archive(make_size_tag(static_cast<size_type>(cphases.size())));
    for (const auto & pair : cphases)
        archive(std::make_pair(pair.first, pair.second.get()));
}

/* identifiers */

template <class Archive, typename T> void
save(Archive & archive, const tmg::identifier<T> & id)
{
    std::string type;
    if (typeid(T) == typeid(uint64_t))
        type = "uint";
    else if (typeid(T) == typeid(int64_t))
        type = "int";
    else if (typeid(T) == typeid(std::string))
        type = "string";
    else
        assert(0 && "No case for this identifier type!");

    archive(make_nvp("type", type));
    archive(make_nvp("name", id.name()));
    archive(make_nvp("value", tmg::strfmt(id.value())));
}

template <class Archive> void
save(Archive & archive, const tmg::identifiers & ids)
{
    archive(make_size_tag(static_cast<size_type>(ids.nidentifiers())));
    for (auto it = ids.begin_int(); it != ids.end_int(); it++) {
        archive(*it);
    }
    for (auto it = ids.begin_uint(); it != ids.end_uint(); it++) {
        archive(*it);
    }
    for (auto it = ids.begin_string(); it != ids.end_string(); it++) {
        archive(*it);
    }
}

template <class Archive> void
load(Archive & archive, std::tuple<std::string, std::string, std::string> & tpl)
{
    archive(make_nvp("type", std::get<0>(tpl)));
    archive(make_nvp("name", std::get<1>(tpl)));
    archive(make_nvp("value", std::get<2>(tpl)));
}

template <class Archive> void
load(Archive & archive, tmg::identifiers & ids)
{
    cereal::size_type size;
    archive(make_size_tag(size));
    for (size_type n = 0; n < size; n++) {
        std::tuple<std::string, std::string, std::string> id;
        archive(id);

        std::string type = std::get<0>(id);
        std::string name = std::get<1>(id);
        std::string value = std::get<2>(id);
        if (type == "int")
            ids.add_int(tmg::identifier<int64_t>(name, std::stoll(value)));
        else if (type == "uint")
            ids.add_uint(tmg::identifier<uint64_t>(name, std::stoull(value)));
        else if (type == "string")
            ids.add_string(tmg::identifier<std::string>(name, value));
        else
            throw tmg::exception("Unsupported identifier type.");
    }
}

/* input identifiers */

template <class Archive> void
save(Archive & archive, const tmg::input_id & iid)
{
    archive(make_nvp("id", (uint64_t)&iid));
    archive(make_nvp("identifiers", iid.ids()));
}

template <class Archive> void
save(Archive & archive, const std::unordered_set<const tmg::input_id*> & iids)
{
    archive(make_size_tag(static_cast<size_type>(iids.size())));
    for (const auto & ids : iids)
        archive(*ids);
}

template <class Archive> void
load(Archive & archive, std::pair<uint64_t, std::shared_ptr<tmg::input_id>> & pair)
{
    uint64_t id;
    std::unique_ptr<tmg::identifiers> ids(new tmg::identifiers());

    archive(make_nvp("id", id));
    archive(make_nvp("identifiers", *ids));
    pair = std::make_pair(id, std::make_shared<tmg::input_id>(std::move(ids)));
}

template <class Archive> void
load(Archive & archive, std::unordered_map<uint64_t, std::shared_ptr<tmg::input_id>> & map)
{
    cereal::size_type size;
    archive(make_size_tag(size));

    map.clear();
    for (size_type n = 0; n < size; n++) {
        std::pair<uint64_t, std::shared_ptr<tmg::input_id>> pair;
        archive(pair);
        map.emplace(pair);
    }
}

/* region serialization */

template <class Archive> void
save(Archive & archive, const tmg::region_id & rid)
{
    archive(make_nvp("file", rid.file()));
    archive(make_nvp("line", rid.line()));
    archive(make_nvp("name", rid.name()));
}

template <class Archive> void
save(Archive & archive, const tmg::region & region)
{
    archive(make_nvp("id", (uint64_t)&region));
    archive(make_nvp("file", region.file()));
    archive(make_nvp("line", region.line()));
    archive(make_nvp("name", region.name()));
}

template <class Archive> void
save(Archive & archive, const std::unordered_set<const tmg::region *> & regions)
{
    archive(make_size_tag(static_cast<size_type>(regions.size())));
    for (const auto & region : regions) {
        archive(*region);
    }
}

template <class Archive> void
load(Archive & archive, std::shared_ptr<tmg::region_id> & rid)
{
    size_t line;
    std::string file, name;
    archive(make_nvp("file", file));
    archive(make_nvp("line", line));
    archive(make_nvp("name", name));
    rid = std::make_shared<tmg::region_id>(file, line, name);
}

template <class Archive> void
load(Archive & archive, std::tuple<uint64_t, std::string, size_t, std::string> & tpl)
{
    archive(make_nvp("id", std::get<0>(tpl)));
    archive(make_nvp("file", std::get<1>(tpl)));
    archive(make_nvp("line", std::get<2>(tpl)));
    archive(make_nvp("name", std::get<3>(tpl)));
}

template <class Archive> void
load(Archive & archive, std::unordered_map<uint64_t, std::shared_ptr<tmg::region>> & map)
{
    cereal::size_type size;
    archive(make_size_tag(size));

    map.clear();
    for (size_type n = 0; n < size; n++) {
        std::tuple<uint64_t, std::string, size_t, std::string> tpl;
        archive(tpl);
        tmg::region_id rid(std::get<1>(tpl), std::get<2>(tpl), std::get<3>(tpl));
        map.emplace(std::make_pair(std::get<0>(tpl), std::make_shared<tmg::region>(rid)));
    }
}

/* callpath serialization */

template <class Archive> void
save(Archive & archive, const tmg::callpath_element & cpe)
{
    archive(make_nvp("region", cpe.region_id()));
    archive(make_nvp("identifiers", cpe.identifiers()));
}

template <class Archive> void
save(Archive & archive, const tmg::callpath & cp)
{
    archive(make_size_tag(static_cast<size_type>(cp.nelements())));
    for (const auto & cpe : cp) {
        archive(cpe);
    }
}

template <class Archive> void
load(Archive & archive, std::shared_ptr<tmg::callpath_element> & cpe)
{
    tmg::identifiers ids;
    std::shared_ptr<tmg::region_id> rid;
    archive(make_nvp("region", rid));
    archive(make_nvp("identifiers", ids));
    cpe = std::make_shared<tmg::callpath_element>(*rid, ids);
}

template <class Archive> void
load(Archive & archive, tmg::callpath & cp)
{
    cereal::size_type size;
    archive(make_size_tag(size));

    for (size_type n = 0; n < size; n++) {
        std::shared_ptr<tmg::callpath_element> cpe;
        archive(cpe);
        cp.add_element(*cpe);
    }
}

/* rts serialization */

template <class Archive> void
save(Archive & archive, const tmg::rts & rts)
{
    archive(make_nvp("region", (uint64_t)rts.region()));
    archive(make_nvp("scenario", (uint64_t)rts.scenario()));
    archive(make_nvp("exectime", rts.exectime()));
    archive(make_nvp("iid", (uint64_t)rts.iid()));
    archive(make_nvp("callpath", rts.callpath()));
}

template <class Archive> void
save(Archive & archive, const std::unordered_set<const tmg::rts*> & rtss)
{
    archive(make_size_tag(static_cast<size_type>(rtss.size())));
    for (const auto & rts : rtss) {
        archive(*rts);
    }
}

template <class Archive> void
load(Archive & archive, cereal::rts & rts)
{
    archive(make_nvp("region", rts.region));
    archive(make_nvp("scenario", rts.scenario));
    archive(make_nvp("exectime", rts.exectime));
    archive(make_nvp("iid", rts.iid));
    archive(make_nvp("callpath", rts.callpath));
}

template <class Archive> void
load(Archive & archive, cereal::rtss & rtss)
{
    cereal::size_type size;
    archive(make_size_tag(size));

    rtss.clear();
    for (size_type n = 0; n < size; n++) {
        auto r = std::make_shared<rts>();
        archive(*r);
        rtss.insert(r);
    }
}

/* configuration serialization */

template <class Archive> void
save(Archive & archive, const tmg::tuning_value & tv)
{
    archive(make_nvp("id", tv.parameter()->name()));
    archive(make_nvp("start", tv.parameter()->start()));
    archive(make_nvp("step", tv.parameter()->step()));
    archive(make_nvp("end", tv.parameter()->end()));
    archive(make_nvp("value", tv.value()));
}

template <class Archive> void
save(Archive & archive, const tmg::configuration & configuration)
{
    archive(make_size_tag(static_cast<size_type>(configuration.nvalues())));
    for (const auto & tp : configuration) {
        archive(tp);
    }
}

template <class Archive> void
load(Archive & archive, cereal::tuning_value & tv)
{
    archive(make_nvp("id", tv.parameter.name));
    archive(make_nvp("start", tv.parameter.start));
    archive(make_nvp("step", tv.parameter.step));
    archive(make_nvp("end", tv.parameter.end));
    archive(make_nvp("value", tv.value));
}

template <class Archive> void
load(Archive & archive, cereal::configuration & c)
{
    cereal::size_type size;
    archive(make_size_tag(size));

    c.clear();
    for (size_type n = 0; n < size; n++) {
        auto tv = std::make_shared<cereal::tuning_value>();
        archive(*tv);
        c.insert(tv);
    }
}

/* scenario serialization */

template <class Archive> void
save(Archive & archive, const tmg::scenario & scenario)
{
    archive(make_nvp("id", (uint64_t)&scenario));
    archive(make_nvp("configuration", scenario.configuration()));
}

template <class Archive> void
save(Archive & archive, const std::unordered_set<std::unique_ptr<tmg::scenario>> & scenarios)
{
    archive(make_size_tag(static_cast<size_type>(scenarios.size())));
    for (const auto & scenario : scenarios) {
        archive(*scenario);
    }
}

template <class Archive> void
load(Archive & archive, std::pair<uint64_t, cereal::configuration> & pair)
{
    archive(make_nvp("id", pair.first));
    archive(make_nvp("configuration", pair.second));
}

template <class Archive> void
load(Archive & archive, std::unordered_map<uint64_t, cereal::configuration> & map)
{
    cereal::size_type size;
    archive(make_size_tag(size));

    map.clear();
    for (size_type n = 0; n < size; n++) {
        std::pair<uint64_t, cereal::configuration> pair;
        archive(pair);
        map.emplace(pair);
    }
}

}

namespace tmg {

std::string
serialize(
    const std::unordered_set<std::unique_ptr<scenario>> & scenarios,
    const cluster_phases & cphases)
{
    std::unordered_set<const tmg::rts*> rtss;
    std::unordered_set<const tmg::input_id*> iids;
    std::unordered_set<const tmg::region*> regions;
    for (const auto & scnr : scenarios) {
        for (const auto & rts : scnr->rtss()) {
            rtss.insert(rts);
            regions.insert(rts->region());
            iids.insert(rts->iid());
        }
    }

    std::ostringstream sstream;
    /*
     * A cereal archive only fully flushes the data when it is destroyed.
    */
    {
        cereal::JSONOutputArchive archive(sstream);
        archive(cereal::make_nvp("clusters", cphases));
        archive(cereal::make_nvp("iids", iids));
        archive(cereal::make_nvp("regions", regions));
        archive(cereal::make_nvp("rtss", rtss));
        archive(cereal::make_nvp("scenarios", scenarios));
    }

    return sstream.str();
}

/* deserliazation */

typedef std::unordered_map<std::string, std::shared_ptr<tmg::tuning_parameter>> tuning_parameters;

static inline tuning_parameters
create_tuning_parameters(const std::unordered_map<uint64_t, cereal::configuration> & scenarios)
{
    tuning_parameters tps;
    for (const auto & pair : scenarios) {
        for (const auto & tv : pair.second) {
            auto start = tv->parameter.start;
            auto step = tv->parameter.step;
            auto end = tv->parameter.end;
            auto name = tv->parameter.name;

            if (tps.find(name) != tps.end()) {
                const auto & tp = tps.find(name)->second;
                if (tp->start() != start || tp->step() != step || tp->end() != end)
                    tmg::exception("Invalid tuning parameter.");
            }

            tps[name] = std::make_shared<tmg::tuning_parameter>(name, start, step, end);
        }
    }

    return tps;
}

static inline tmg::tuning_value
convert_tuning_value(const cereal::tuning_value & tv, const tuning_parameters & tps)
{
    auto tp = tps.find(tv.parameter.name);
    TUNING_MODEL_DEBUG_ASSERT(tp != tps.end());
    return tmg::tuning_value(tp->second, tv.value);
}

static inline tmg::configuration
convert_configuration(const cereal::configuration & srcconfig, const tuning_parameters & tps)
{
    tmg::configuration dstconfig;
    for (const auto & tv : srcconfig)
        dstconfig.add_tuning_value(convert_tuning_value(*tv, tps));

    return dstconfig;
}

std::unordered_set<std::unique_ptr<scenario>>
deserialize(std::istream & is, const configuration_selector_t & selector)
{
    cereal::rtss rtss;
    std::unordered_map<uint64_t, std::shared_ptr<input_id>> iids;
    std::unordered_map<uint64_t, cereal::configuration> scenarios;
    std::unordered_map<uint64_t, std::shared_ptr<region>> regions;

    cereal::JSONInputArchive archive(is);
    archive(cereal::make_nvp("iids", iids));
    archive(cereal::make_nvp("regions", regions));
    archive(cereal::make_nvp("rtss", rtss));
    archive(cereal::make_nvp("scenarios", scenarios));

    /* validate tuning model */

    auto tps = create_tuning_parameters(scenarios);

    for (const auto & rts : rtss) {
        if (iids.find(rts->iid) == iids.end())
            throw tmg::exception(strfmt("No input identifier found for id ", rts->iid));
        if (scenarios.find(rts->scenario) == scenarios.end())
            throw tmg::exception(strfmt("No scenario found for id ", rts->scenario));
        if (regions.find(rts->region) == regions.end())
            throw tmg::exception(strfmt("No region found for id ", rts->region));
    }

    /* convert to tuning model */

    std::unordered_map<uint64_t, std::unordered_set<tmg::rts*>> rtsmap;
    for (const auto & srcrts : rtss) {
        auto scnrid = srcrts->scenario;
        auto exectime = srcrts->exectime;
        auto dstrts = new tmg::rts(srcrts->callpath, regions[srcrts->region], iids[srcrts->iid],
            convert_configuration(scenarios[scnrid], tps), exectime);

        if (rtsmap.find(scnrid) == rtsmap.end())
            rtsmap[scnrid] = {dstrts};
        else
            rtsmap[scnrid].insert(dstrts);
    }

    std::unordered_set<std::unique_ptr<scenario>> scnrs;
    for (const auto & pair : rtsmap)
        scnrs.insert(std::move(std::make_unique<tmg::scenario>(pair.second, selector)));

    /* cleanup */

    for (const auto & pair : rtsmap) {
        for (const auto & rts : pair.second)
            delete rts;
    }

    return scnrs;
}

}
