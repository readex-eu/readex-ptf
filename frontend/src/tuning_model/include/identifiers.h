/**
   @file    identifiers.h
   @ingroup Frontend
   @brief   Identifiers header
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

#ifndef IDENTIFIERS_H_INCLUDED
#define IDENTIFIERS_H_INCLUDED

#include "common.h"

#include <boost/functional/hash.hpp>

#include <string>
#include <unordered_set>

namespace tmg {

template<typename T>
class identifier final {
public:
    identifier(const std::string & name, const T & value)
    : value_(value)
    , name_(name)
    {}

    identifier(const identifier & other) = default;

    identifier(identifier && other) = default;

    identifier &
    operator=(const identifier & other) = default;

    bool
    operator==(const identifier<T> & other) const noexcept
    {
        return name_ == other.name_ && value_ == other.value_;
    }

    bool
    operator!=(const identifier<T> & other) const noexcept
    {
        return !(*this == other);
    }

    inline const std::string &
    name() const noexcept
    {
        return name_;
    }

    inline const T &
    value() const noexcept
    {
        return value_;
    }

    inline std::string
    debug_string() const
    {
        return strfmt("<", name(), ", ", value(), ">");
    }

private:
    T value_;
    std::string name_;
};

}

namespace std
{
    template<typename T>
    struct hash<tmg::identifier<T>>
    {
        size_t
        operator()(const tmg::identifier<T> & id) const
        {
            std::size_t seed = 0;
            boost::hash_combine(seed, boost::hash_value(id.name()));
            boost::hash_combine(seed, boost::hash_value(id.value()));
            return seed;
        }
    };
}

namespace tmg {

class identifiers final {
public:
    template <typename T>
    class const_iterator final {
    public:
        inline constexpr
        const_iterator(const typename std::unordered_set<tmg::identifier<T>>::const_iterator & it)
        : it_(it)
        {}

        inline const const_iterator<T> &
        operator++() noexcept
        {
            it_ = std::next(it_);
            return *this;
        }

        inline const const_iterator<T>
        operator++(int) noexcept
        {
            auto tmp = it_;
            it_ = std::next(it_);
            return tmp;
        }

        inline bool
        operator==(const const_iterator<T> & other) const noexcept
        {
            return it_ == other.it_;
        }

        inline bool
        operator!=(const const_iterator<T> & other) const noexcept
        {
            return !(*this == other);
        }

        inline const tmg::identifier<T>
        operator*() noexcept
        {
            return std::move(*it_);
        }

    private:
        typename std::unordered_set<tmg::identifier<T>>::const_iterator it_;
    };

    identifiers() = default;

    identifiers(const identifiers & other) = default;

    identifiers(identifiers && other) = default;

    identifiers &
    operator=(const identifiers & other) = default;

    inline bool
    operator==(const identifiers & other) const noexcept
    {
        return ints_ == other.ints_
            && uints_ == other.uints_
            && strings_ == other.strings_;
    }

    inline bool
    operator!=(const identifiers & other) const noexcept
    {
        return !(*this == other);
    }

    inline identifiers::const_iterator<int64_t>
    begin_int() const noexcept
    {
        return const_iterator<int64_t>(ints_.begin());
    }

    inline identifiers::const_iterator<int64_t>
    end_int() const noexcept
    {
        return const_iterator<int64_t>(ints_.end());
    }

    inline identifiers::const_iterator<uint64_t>
    begin_uint() const noexcept
    {
        return const_iterator<uint64_t>(uints_.begin());
    }

    inline identifiers::const_iterator<uint64_t>
    end_uint() const noexcept
    {
        return const_iterator<uint64_t>(uints_.end());
    }

    inline identifiers::const_iterator<std::string>
    begin_string() const noexcept
    {
        return const_iterator<std::string>(strings_.begin());
    }

    inline identifiers::const_iterator<std::string>
    end_string() const noexcept
    {
        return const_iterator<std::string>(strings_.end());
    }

    inline size_t
    nints() const noexcept
    {
        return ints_.size();
    }

    inline size_t
    nuints() const noexcept
    {
        return uints_.size();
    }

    inline size_t
    nstrings() const noexcept
    {
        return strings_.size();
    }

    inline size_t
    nidentifiers() const noexcept
    {
        return nints() + nuints() + nstrings();
    }

    inline bool
    empty() const noexcept
    {
        return nidentifiers() == 0;
    }

    inline void
    add_int(const identifier<int64_t> & id)
    {
        ints_.insert(id);
    }

    inline void
    add_uint(const identifier<uint64_t> & id)
    {
        uints_.insert(id);
    }

    inline void
    add_string(const identifier<std::string> & id)
    {
        strings_.insert(id);
    }

    inline std::string
    debug_string() const
    {
        std::string s("(");
        for (const auto & id : ints_)
            s += id.debug_string() + ",";
        for (const auto & id : uints_)
            s += id.debug_string() + ", ";
        for (const auto & id : strings_)
            s += id.debug_string() + ", ";

        return s + ")";
    }

private:
    std::unordered_set<identifier<int64_t>> ints_;
    std::unordered_set<identifier<uint64_t>> uints_;
    std::unordered_set<identifier<std::string>> strings_;
};

class input_id final {
public:
    inline
    input_id()
    {
        ids_ = std::make_unique<tmg::identifiers>();
    }

    inline
    input_id(std::unique_ptr<tmg::identifiers> ids)
    : ids_(std::move(ids))
    {}

    inline const tmg::identifiers &
    ids() const noexcept
    {
        return *ids_;
    }

    inline bool
    operator==(const input_id & other)
    {
        return *ids_ == *other.ids_;
    }

    inline bool
    operator!=(const input_id & other)
    {
        return !(*this == other);
    }

private:
    std::unique_ptr<tmg::identifiers> ids_;
};

}

#endif
