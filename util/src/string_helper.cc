/**
   @file    string_helper.cc
   @brief   String modification utilities
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


#include <string>
#include <iostream>
#include <vector>
#include <boost/algorithm/string_regex.hpp>
#include <boost/regex.hpp>
#include "string_helper.h"

/**
 *@brief Format the string using boost regex
 *@param str String to check for matches
 *@param reg_expr Vector of regular expressions to match
 *@param format Vector of strings to replace the corresponding regular expressions matched in the string
 *@return The formatted string
 */
std::string formatString( std::string &str, std::vector<boost::regex> reg_expr, std::vector<std::string> format ) {
    for ( int i = 0; i < reg_expr.size() && i < format.size(); i++ ){
        str = boost::regex_replace( str, reg_expr[i], format[i] );
    }
    return str;
}
