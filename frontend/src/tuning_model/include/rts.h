/**
   @file    tuning_model/include/rts.h
   @ingroup Frontend
   @brief   Rts header
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

#ifndef RTS_H_INCLUDED
#define RTS_H_INCLUDED

#include <string>
#include <unordered_map>

#include "callpath.h"
#include "configuration.h"
#include "region.h"

#include <boost/functional/hash.hpp>

namespace tmg {

class rtsid final {
public:
    inline
    rtsid(
        const tmg::callpath & callpath,
        const std::shared_ptr<region> & region,
        const std::shared_ptr<input_id> & iid)
    : callpath_(callpath)
    , iid_(iid)
    , region_(region)
    {}

    rtsid(const rtsid & other) = default;

    rtsid(rtsid && other) = default;

    rtsid &
    operator=(const rtsid & other) = default;

    rtsid &
    operator=(rtsid && other) = default;

    inline bool
    operator==(const rtsid & other) const noexcept
    {
        return *iid_ == *other.iid_
            && *region_ == *other.region_
            && callpath_ == other.callpath_;
    }

    inline bool
    operator!=(const rtsid & other) const noexcept
    {
        return !(*this == other);
    }

    inline const tmg::callpath &
    callpath() const noexcept
    {
        return callpath_;
    }

    inline const tmg::input_id *
    iid() const noexcept
    {
        return iid_.get();
    }

    inline const tmg::region *
    region() const noexcept
    {
        return region_.get();
    }

private:
    tmg::callpath callpath_;
    std::shared_ptr<tmg::input_id> iid_;
    std::shared_ptr<tmg::region> region_;
};

}

namespace std
{
    template <>
    struct hash<tmg::rtsid>
    {
        size_t
        operator()(const tmg::rtsid & id) const
        {
            std::size_t seed = 0;
            /* FIXME: don't use debug string here */
            boost::hash_combine(seed, boost::hash_value(id.callpath().debug_string()));
            boost::hash_combine(seed, boost::hash_value(id.region()->debug_string()));
            boost::hash_combine(seed, boost::hash_value(id.iid()->ids().debug_string()));
            return seed;
        }
    };
}

namespace tmg {

class scenario;

class rts final {
public:
    inline
    rts(
        const tmg::callpath & callpath,
        const std::shared_ptr<region> & region,
        const std::shared_ptr<input_id> & iid,
        const tmg::configuration & configuration,
        double exectime)
    : id_(callpath, region, iid)
    , exectime_(exectime)
    , scenario_(nullptr)
    , configuration_(configuration)
    {}

    rts(const rts & other) = default;

    rts(rts && other) = default;

    rts &
    operator=(const rts & other) = default;

    inline const tmg::region *
    region() const noexcept
    {
        return rtsid().region();
    }

    inline const tmg::input_id *
    iid() const noexcept
    {
        return rtsid().iid();
    }

    inline const tmg::configuration &
    configuration() const noexcept
    {
        return configuration_;
    }

    inline double
    exectime() const noexcept
    {
        return exectime_;
    }

    std::unique_ptr<rts>
    copy() const
    {
        return std::make_unique<rts>(*this);
    }

    tmg::scenario *
    scenario() const noexcept
    {
        return scenario_;
    }

    void
    assign_scenario(tmg::scenario * scenario)
    {
        scenario_ = scenario;
    }

    const tmg::callpath &
    callpath() const noexcept
    {
        return rtsid().callpath();
    }

    inline std::string
    debug_string() const
    {
       return callpath().debug_string() + "/" + region()->debug_string();
    }

    inline const tmg::rtsid &
    rtsid() const noexcept
    {
        return id_;
    }

private:
    tmg::rtsid id_;
    double exectime_;
    tmg::scenario * scenario_;
    tmg::configuration configuration_;
};

}

#endif
