/**
   @file    callpath.h
   @ingroup Frontend
   @brief   Callpath header
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

#ifndef CALLPATH_H_INCLUDED
#define CALLPATH_H_INCLUDED

#include "identifiers.h"
#include "region.h"

namespace tmg {

class callpath_element final {
public:
    inline
    callpath_element(const tmg::region_id & id, const tmg::identifiers & ids)
    : id_(id)
    , ids_(ids)
    {}

    callpath_element(const callpath_element & other) = default;

    callpath_element(callpath_element && other) = default;

    callpath_element &
    operator=(const callpath_element & other) = default;

    inline bool
    operator==(const callpath_element & other) const noexcept
    {
        return id_ == other.id_ && ids_ == other.ids_;
    }

    inline bool
    operator!=(const callpath_element & other) const noexcept
    {
        return !(*this == other);
    }

    inline const tmg::region_id &
    region_id() const noexcept
    {
        return id_;
    }

    inline const tmg::identifiers &
    identifiers() const noexcept
    {
        return ids_;
    }

    inline std::string
    debug_string() const
    {
        return region_id().debug_string() + ":" + identifiers().debug_string();
    }

private:
    tmg::region_id id_;
    tmg::identifiers ids_;
};

class callpath final {
private:
    class const_iterator final {
    public:
        inline constexpr
        const_iterator(const std::vector<callpath_element>::const_iterator & it)
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

        inline const callpath_element
        operator*() noexcept
        {
            return std::move(*it_);
        }

    private:
        std::vector<callpath_element>::const_iterator it_;
    };

public:
    callpath() = default;

    inline
    callpath(const std::vector<callpath_element> & elements)
    : elements_(elements)
    {}

    callpath(const callpath & other) = default;

    callpath(callpath && other) = default;

    callpath &
    operator=(const callpath & other) = default;

    inline bool
    operator==(const callpath & other) const noexcept
    {
        return elements_ == other.elements_;
    }

    inline bool
    operator!=(const callpath & other) const noexcept
    {
        return !(*this == other);
    }

    inline callpath::const_iterator
    begin() const noexcept
    {
        return const_iterator(elements_.begin());
    }

    inline callpath::const_iterator
    end() const noexcept
    {
        return const_iterator(elements_.end());
    }

    inline void
    add_element(const callpath_element & cpe)
    {
        elements_.push_back(cpe);
    }

    inline size_t
    nelements() const noexcept
    {
        return elements_.size();
    }

    inline std::string
    debug_string() const
    {
        std::string s;
        for (const auto & e : elements_)
            s += e.debug_string() + "/";

        return s;
    }

private:
    std::vector<tmg::callpath_element> elements_;
};

}

#endif
