/**
   @file    region.h
   @ingroup Frontend
   @brief   Tuning header
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

#ifndef TUNING_H_INCLUDED
#define TUNING_H_INCLUDED

#include <iostream>

namespace tmg {

class tuning_parameter final {
public:
    inline
    tuning_parameter(const std::string & name, int start, int step, int end)
    : end_(std::max(start, end))
    , step_(std::abs(step))
    , start_(std::min(start, end))
    , name_(name)
    {
        int range = end_ - start_;
        end_ = end_ - (range % step);
    }

    inline const std::string &
    name() const noexcept
    {
        return name_;
    }

    inline int
    start() const noexcept
    {
        return start_;
    }

    inline int
    step() const noexcept
    {
        return step_;
    }

    inline int
    end() const noexcept
    {
        return end_;
    }

private:
    int end_;
    int step_;
    int start_;
    std::string name_;
};

class tuning_value final {
public:
    inline
    tuning_value(const std::shared_ptr<tuning_parameter> & p, int value)
    : value_(value)
    , parameter_(p)
    {
        if (value < p->start() || value > p->end())
            std::cerr << "TM ERROR: Tuning value out of range:"
                      << value << "-> (" << p->start() << ", " << p->step() << ", " << p->end()
                      << ").\n";
        if (((value - p->start()) % p->step()) != 0)
            std::cerr << "TM ERROR: Tuning value is not a multiple of step:"
                      << value << "-> (" << p->start() << ", " << p->step() << ", " << p->end()
                      << ").\n";
    }

    inline bool
    operator==(const tuning_value & other) const noexcept
    {
        return parameter_ == other.parameter_ && value() == other.value();
    }

    inline bool
    operator!=(const tuning_value & other) const noexcept
    {
        return !(*this == other);
    }

    const std::shared_ptr<tuning_parameter>
    parameter() const noexcept
    {
        return parameter_;
    }

    int
    value() const noexcept
    {
        return value_;
    }

private:
    int value_;
    std::shared_ptr<tuning_parameter> parameter_;
};

}

#endif
