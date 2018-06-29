/**
   @file	strutil.h
   @brief	String manipulation routines header
   @author	Karl Fuerlinger
   @verbatim
        Revision:       $Revision$
        Revision date:  $Date$
        Committed by:   $Author$

        This file is part of the Periscope performance measurement tool.
        See http://www.lrr.in.tum.de/periscope for details.

        Copyright (c) 2005-2011, Technische Universitaet Muenchen, Germany
        See the COPYING file in the base directory of the package for details.
   @endverbatim
 */


#ifndef STRUTIL_H_INCLUDED
#define STRUTIL_H_INCLUDED


#include <string>
#include <iostream>
#include <utility>


size_t strskip_ws( std::string str,
                   int         pos );

size_t get_token( std::string  str,
                  int          pos,
                  std::string  delim,
                  std::string& tok );

size_t get_key_value_pair( std::string str,
                           int pos,
                           std::pair< std::string, std::string >& kvpair );


#endif /* STRUTIL_H_INCLUDED */
