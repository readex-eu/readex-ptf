/**
 * Author: Anamika Chowdhury
 * \file DetectRegion.h
 * \brief Contains a detect regions specific calls
 *
 ************************************************/
#ifndef FILTER_REGION_H_
#define FILTER_REGION_H_

#include <iostream>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <iomanip>
#include <string>
#include <algorithm>
#include <numeric>
#include <list>
#include <map>
#include <vector>


#include <cubelib/Cube.h>
#include <cubelib/CubeServices.h>

using namespace std;
using namespace cube;



//bool
//checkRegionType( const Cnode* c_node );
/*
 * openmp or mpi events are not allowed for filtering
 */
bool
isAllowedforFilter( const Cnode* c_node );

void
FilterCNodes( const vector< Cnode* >& call_nodes,
              Cube*                   in_cube,
              double                  threshold,
              vector< Cnode* >&       filt_nodes );

/**
 * Prints out the help how to use this tool.
 *
 */
void
usage( const char* filename )
{
    cout << "Usage: " << filename << " [-t threshold] [-f filter_file_name] [-h] <cubefile>\n";
    cout << "  -h     Help; Output this help message\n";
    cout << "  -t     Choose threshold for granularity in Seconds (defaults to 0.1s)\n";
    cout << "  -f     Name of the filter file without extension (default name scorep.filt)\n";
    cout << "  -i     Name of the intel compiler filter file without extension\n";
    exit( 0 );
}

#endif //end of Filter region
