/****************************************************************************
**  CUBE        http://www.score-p.org/                                    **
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 1998-2015                                                **
**  Forschungszentrum Juelich GmbH, Juelich Supercomputing Centre          **
**                                                                         **
**  Copyright (c) 2009-2015                                                **
**  German Research School for Simulation Sciences GmbH,                   **
**  Laboratory for Parallel Programming                                    **
**                                                                         **
**  This software may be modified and distributed under the terms of       **
**  a BSD-style license.  See the COPYING file in the package base         **
**  directory for details.                                                 **
****************************************************************************/

/**
 * \file helper.h
 * \brief Declares a set of functions to handle with CallPath types
    (Transform string->callpathtype), Factors
 */
#ifndef CUBE_HELPER
#define CUBE_HELPER

#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <iostream>
#include <map>

#include <cubelib/CubeRegion.h>
#include <cubelib/Cube.h>
#include <cubelib/CubeCnode.h>
#include "enums.h"

namespace cube
{
std::string
Callpathtype2String( CallpathType ct );

CallpathType
String2Callpathtype( std::string& name );

//std::vector< CallpathType >
//classify_callpaths( Cube* );

std::string
get_callpath_for_cnode( Cnode* cn );

unsigned long long
TypeFactor( std::string& name );

double
get_atotalt( Cube*        input,
             std::string& metricname );


/**
 * Finds a maximum value in a vector and returns a pair (value, index)
 *
 * Note: Similar function is in cube_nodeview.cpp|.h
 */
template< class T >
std::pair< T, size_t >
find_max( const std::vector< T >& v )
{
    typename std::vector< T >::iterator it = std::max_element( v.begin(), v.end() );

    return std::pair< T, size_t >( *it, it - v.begin() );
}
/**
 * Checks the type of each call node
 */
bool
isUserNode( const Cnode* c_node );

/**
 * Checks the type of each call node
 */
bool
isForkNode( const Cnode* c_node );

/*
 * Compute threshold for granularity
 */
double
computeGranularity( Cnode* ct_node,
                    Cube*  input );

/**
 * Parser to read the values from the tuple
 * @param value
 * @return
 */
std::vector< std::string >
ValueParser( std::string value );
}
#endif
