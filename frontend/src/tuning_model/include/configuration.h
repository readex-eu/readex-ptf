/**
   @file    region.h
   @ingroup Frontend
   @brief   Region header
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

#ifndef CONFIGURATION_H_INCLUDED
#define CONFIGURATION_H_INCLUDED

#include <algorithm>
#include <numeric>
#include <unordered_map>
#include <unordered_set>

#include <boost/functional/hash.hpp>

#include "common.h"
#include "tuning.h"

namespace tmg {

class configuration final {
private:
    class const_iterator final {
    public:
        inline constexpr
        const_iterator(const std::unordered_map<
            std::string,
            std::unique_ptr<tuning_value>
        >::const_iterator & it)
        : it_(it)
        {}

        inline const const_iterator &
        operator++() noexcept
        {
            it_ = std::next(it_);
            return *this;
        }

        inline bool
        operator==(const const_iterator & other) const noexcept
        {
            return it_ == other.it_;
        }

        inline bool
        operator!=(const const_iterator & other) const noexcept
        {
            return !(*this == other);
        }

        inline const tuning_value &
        operator*() noexcept
        {
            return *(it_->second);
        }

        inline const std::string &
        id() const noexcept
        {
            return it_->second->parameter()->name();
        }

        inline int
        value() const noexcept
        {
            return it_->second->value();
        }

    private:
        std::unordered_map<std::string, std::unique_ptr<tuning_value>>::const_iterator it_;
    };

public:
    configuration() = default;

    inline
    configuration(const std::vector<tuning_value> & tvs)
    {
        for (const auto & tv : tvs)
            add_tuning_value(tv);
    }

    ~configuration() = default;

    inline
    configuration(const configuration & other)
    {
        for (const auto & tv : other)
            tvs_[tv.parameter()->name()] = std::make_unique<tmg::tuning_value>(tv);
    }

    configuration(configuration && other) = default;

    inline configuration &
    operator=(const configuration & other)
    {
        tvs_.clear();
        for (const auto & tv : other)
            tvs_[tv.parameter()->name()] = std::make_unique<tmg::tuning_value>(tv);

        return *this;
    }

    bool
    operator==(const configuration & other) const noexcept
    {
        if (nvalues() != other.nvalues())
            return false;

        for (const auto & tv : other) {
            auto it = tvs_.find(tv.parameter()->name());
            if (it == tvs_.end())
                return false;

            if ((*it).second->value() != tv.value())
                return false;
        }

        return true;
    }

    inline configuration::const_iterator
    begin() const noexcept
    {
        return const_iterator(tvs_.begin());
    }

    inline configuration::const_iterator
    end() const noexcept
    {
        return const_iterator(tvs_.end());
    }

    inline size_t
    nvalues() const noexcept
    {
        return tvs_.size();
    }

    inline void
    add_tuning_value(const tuning_value & tv)
    {
        TUNING_MODEL_DEBUG_ASSERT(tvs_.find(tv.parameter()->name()) == tvs_.end());
        tvs_[tv.parameter()->name()] = std::make_unique<tmg::tuning_value>(tv);
    }

    /*
        FIXME: introduce tuning parameter id
    */
    inline const tmg::tuning_value *
    tuning_value(const std::string & id) const noexcept
    {
        auto it = tvs_.find(id);
        return it != tvs_.end() ? it->second.get() : nullptr;
    }

    inline const tmg::tuning_value *
    tuning_value(const std::shared_ptr<tmg::tuning_parameter> & p) const noexcept
    {
        auto it = tvs_.find(p->name());
        return it != tvs_.end() ? it->second.get() : nullptr;
    }

    inline std::vector<double>
    to_vector() const noexcept
    {
        std::vector<double> v;
        for (const auto & tv : tvs_)
            v.push_back(tv.second->value());

        return v;
    }

private:
    std::unordered_map<std::string, std::unique_ptr<tmg::tuning_value>> tvs_;
};

typedef tmg::configuration
    (*configuration_selector_t)(const std::unordered_set<const configuration*> & rtss);

static inline tmg::configuration
random_selector(const std::unordered_set<const configuration*> & cs)
{
    TUNING_MODEL_DEBUG_ASSERT(!cs.empty());

    tmg::configuration c(*(*cs.begin()));
    return c;
}

static inline tmg::configuration
average_selector(const std::unordered_set<const configuration*> & cs)
{
    TUNING_MODEL_DEBUG_ASSERT(!cs.empty());

    std::unordered_set<std::shared_ptr<tuning_parameter>> tps;
    for (const auto v : *(*cs.begin()))
        tps.insert(v.parameter());

    tmg::configuration c;
    for (const auto & tp : tps) {
        double sum = std::accumulate(cs.begin(), cs.end(), 0.0,
            [&](double sum, const configuration * c){return sum + c->tuning_value(tp)->value();});
        int avg = (sum*10.0) / cs.size();
        int rest = avg % (tp->step() * 10);
        if (rest < (tp->step()*10) / 2.0)
            c.add_tuning_value(tuning_value(tp, (avg-rest)/10.0));
        else
            c.add_tuning_value(tuning_value(tp, ((avg-rest)/10.0)+tp->step()));
    }

    return c;
}

}

namespace std
{
	template<>
	struct hash<tmg::configuration>
	{
		size_t operator()(const tmg::configuration & c) const
		{
			std::size_t seed = 0;

			for (const auto & tv: c) {
				boost::hash_combine(seed, boost::hash_value(tv.parameter()->name()));
				boost::hash_combine(seed, boost::hash_value(tv.value()));
			}

			return seed;
		}
	};
}

#endif
