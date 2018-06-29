/**
   @file    common.h
   @ingroup Frontend
   @brief   Tuning model common header
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

#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <sstream>
#include <stdexcept>
#include <string>

#include <assert.h>

#ifdef TUNING_MODEL_DEBUG
# define TUNING_MODEL_DEBUG_ASSERT(x) assert(x)
#else
# define TUNING_MODEL_DEBUG_ASSERT(x) (void)(x)
#endif

template<typename T> static inline T
convert(const std::string & s)
{
    T r;
    std::stringstream sstream;
    sstream << s;
    sstream >> r;
    return r;
}

namespace tmg {

template <typename... Args>
static inline void
format_to_stream(std::ostream & os, Args... args);

template <typename Arg>
static inline void
format_to_stream(std::ostream & os, const Arg & arg)
{
    os << arg;
}

template <typename Arg, typename... Args>
static inline void
format_to_stream(std::ostream & os, const Arg & arg, Args... args)
{
    os << arg;
    format_to_stream(os, args...);
}

template <typename... Args>
static inline std::string
strfmt(Args... args)
{
    std::ostringstream os;
    format_to_stream(os, args...);
    return os.str();
}

class exception : public std::runtime_error {
public:
    virtual
    ~exception() noexcept;

    inline
    exception(const std::string & s)
    : std::runtime_error(s)
    {}
};

}

#endif /* COMMON_H_INCLUDED */
