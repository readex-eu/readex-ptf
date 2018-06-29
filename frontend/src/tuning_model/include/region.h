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


#ifndef REGION_H_INCLUDED
#define REGION_H_INCLUDED

#include "common.h"

#include <boost/functional/hash.hpp>

namespace tmg {

class region_id final {
public:
    inline
    region_id(const std::string & file, size_t line, const std::string & name)
    : line_(line)
    , file_(file)
    , name_(name)
    {}

    region_id(const region_id & other) = default;

    region_id(region_id && other) = default;

    region_id &
    operator=(const region_id & other) = default;

    bool
    operator==(const region_id & other) const noexcept
    {
        return name_ == other.name_ && file_ == other.file_ && line_ == other.line_;
    }

    bool
    operator!=(const region_id & other) const noexcept
    {
        return !(*this == other);
    }

    inline size_t
    line() const noexcept
    {
        return line_;
    }

    inline const std::string &
    file() const noexcept
    {
        return file_;
    }

    inline const std::string &
    name() const noexcept
    {
        return name_;
    }

    inline std::string
    debug_string() const
    {
        return strfmt(file(), ":", line(), ":", name());
    }

private:
    size_t line_;
    std::string file_;
    std::string name_;
};

}

namespace std
{
    template<>
    struct hash<tmg::region_id>
    {
        size_t
        operator()(const tmg::region_id & rid) const
        {
            std::size_t seed = 0;
            boost::hash_combine(seed, boost::hash_value(rid.file()));
            boost::hash_combine(seed, boost::hash_value(rid.line()));
            boost::hash_combine(seed, boost::hash_value(rid.name()));
            return seed;
        }
    };

}

namespace tmg {

class region final {
public:
    inline
    region(const region_id & id)
    : id_(id)
    {}

    region(const region & other) = default;

    region(region && other) = default;

    region &
    operator=(const region & other) = default;

    inline bool
    operator==(const region & other) const noexcept
    {
        return id_ == other.id_;
    }

    inline bool
    operator!=(const region & other) const noexcept
    {
        return !(*this == other);
    }

    const region_id &
    id() const noexcept
    {
        return id_;
    }

    inline size_t
    line() const noexcept
    {
        return id().line();
    }

    inline const std::string &
    file() const noexcept
    {
        return id().file();
    }

    inline const std::string &
    name() const noexcept
    {
        return id().name();
    }

    inline std::string
    debug_string() const
    {
        return id().debug_string();
    }

private:
    region_id id_;
};

}

#endif
