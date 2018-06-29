/**
   @file    string_helper.h
   @brief   String modification utilities header
   @author  Madhura Kumaraswamy
   @verbatim
        Revision:       $Revision$
        Revision date:  $Date$
        Committed by:   $Author$

        This file is part of the Periscope performance measurement tool.
        See http://www.lrr.in.tum.de/periscope for details.

        Copyright (c) 2015-2016, Technische Universitaet Muenchen, Germany
        See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#ifndef STRING_HELPER_H_
#define STRING_HELPER_H_

#include <string>
#include <vector>
#include <boost/regex.hpp>


std::string formatString( std::string &str,
                                 std::vector<boost::regex> reg_expr, std::vector<std::string> format);


#endif /* STRING_HELPER_H_ */
