/*
 * FilterRegion.cc
 *
 *  Created on: Feb 11, 2016
 *      Author: anamika
 */
/**
 * \file FilterRegion.cc
 * \brief It detects the program regions by reading the cube file and outputs
 */
/******************************************
 *
 */
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
#include <boost/regex.hpp>


#include "../include/FilterRegion.h"
#include "../../common_incl/helper.h"
#include "../../common_incl/enums.h"

using namespace std;
using namespace cube;
using namespace services;

bool
predicateFunc( const string& a,
               const string& b )
{
    return ( strcasecmp( a.c_str(), b.c_str() ) == 0 ) ? true : false;
}

int
main( int argc, char* argv[] )
{
    string       cube_file_name( "undefined" );
    string       filter_file_name( "scorep.filt" );
    string       filter_file_name_i( "intel-tcollect.filt" );
    double       threshold( 0.1 );
    boost::regex reg( "\\s" );
    string       format( "*" );
    boost::regex reg_i{ "[\\*\\[\\]]" };
    string       format_i{ "\\\\$&" };
    int          argcount( 1 );
    bool         has_intel_filt_file( false );
    int          c;

    while ( ( c = getopt( argc, argv, "t:f:i:h?" ) ) != -1 )
    {
        switch ( c )
        {
            case 'h':
            case '?':
                usage( argv[ 0 ] );
                exit( 0 );
                break;
            case 't':
                threshold  = stod( optarg );
                argcount  += 2;
                break;
            case 'f':
                filter_file_name = string( optarg ) + ".filt";
                argcount        += 2;
                break;
            case 'i':
                filter_file_name_i   = string( optarg ) + ".filt";
                has_intel_filt_file  = true;
                argcount            += 2;
                break;
            default:
                usage( argv[ 0 ] );
                cout << "\nError: Wrong arguments.\n";
                exit( 0 );
        }
    }

    if ( ( argc - argcount ) != 1 )
    {
        usage( argv[ 0 ] );
        cout << "\nError: Wrong arguments.\n";
        exit( 0 );
    }

    if ( argcount < argc )
    {
        Cube* in_cube = NULL;
        try
        {
            cube_file_name = string( argv[ argcount ] );
            Cube* in_cube  = new Cube();
            // Read input files
            cerr << "Reading " << cube_file_name << "...";
            in_cube->openCubeReport( cube_file_name.c_str() );
            cerr << " Done." << endl;

            //get all call tree nodes
            const vector< Cnode* >& cnodes = in_cube->get_cnodev();
            vector< Cnode* >        filtered_cnodes;

            //Traverse the phase node in order to compute granularity of each sub nodes
            FilterCNodes( cnodes, in_cube, threshold, filtered_cnodes );

            vector< string > filter_node_names;
            cout << "--------------------------------------------------------------" << endl;
            for ( const auto& cnode : filtered_cnodes )
            {
                if ( !isAllowedforFilter( cnode ) )
                {
                    continue;
                }
                cout  << "--Allowed filtered node name--" << cnode->get_callee()->get_name() << endl;

                filter_node_names.push_back( cnode->get_callee()->get_mangled_name() );
            }
            sort( filter_node_names.begin(), filter_node_names.end() );
            vector< string >::iterator it = unique( filter_node_names.begin(), filter_node_names.end() );

            filter_node_names.resize( distance( filter_node_names.begin(), it ) );

            //using predicate comparison
            unique( filter_node_names.begin(), filter_node_names.end(), predicateFunc );

            cout << "Writing into the filter file " << endl;

            ofstream ofs( filter_file_name, ios_base::out );
            if ( ofs.is_open() )
            {
                ofs << "SCOREP_REGION_NAMES_BEGIN\n";
                ofs << "EXCLUDE MANGLED\n";
                for ( const auto& node : filter_node_names )
                {
                     string fn_name = boost::regex_replace( node, reg, format );
		     if (fn_name.find("opari") == string::npos) {
		       ofs << fn_name;
		       ofs << "\n";
		     }
                }
                ofs << "SCOREP_REGION_NAMES_END\n";
                ofs.close();
            }

            if ( has_intel_filt_file )
            {
                cout << "Writing intel compiler filter file " << endl;
                ofstream ofsi( filter_file_name_i, ios_base::out );
                if ( ofsi.is_open() )
                {
                    for ( const auto& node : filter_node_names )
                    {
                        string fn_name = boost::regex_replace( node, reg_i, format_i );
			//dont filter opari regions
                        if ( !fn_name.empty() && fn_name.find("opari") == string::npos)
                        {
                            ofsi << ".*:";
                            ofsi << fn_name;
                            ofsi << " off\n";
                        }
                    }
		    vector<string> ext_names = {".cpp", ".cc", ".c", ".CPP", ".CC", ".C", ".h", ".H"};
		    for (const auto& ext : ext_names) 
		    {
		        ofsi << "^.*\\";
			ofsi << ext;
			ofsi << ":$ off\n";
		    }
                    ofsi.close();
                }
            }

            //delete in_cube;
            cout << "DONE" << endl;
        } catch ( const RuntimeError& err )
        {
            cerr << err.what() << endl;
        }
        if ( in_cube != NULL )
        {
            delete in_cube;
        }
    }
}


bool
isAllowedforFilter( const Cnode* c_node )
{
//    size_t no_child = c_node->num_children();
//    if ( no_child > 0 ) {
//        if ( isUserNode( c_node ) )
//        {
//            for( size_t cn_i = 0; cn_i < no_child; cn_i++ )
//            {
//                const Cnode* cnode = c_node->get_child( cn_i );
//                if ( isUserNode( cnode ) )
//                    isAllowedforFilter( cnode );
//                else
//                    return false;
//            }
//        }
//        else
//            return false;
//    }
//    else
    return isUserNode( c_node );
}

void
FilterCNodes( const vector< Cnode* >& call_nodes,
              Cube*                   in_cube,
              double                  threshold,
              vector< Cnode* >&       filt_nodes )
{
    for ( size_t i = 0; i < call_nodes.size(); i++ )
    {
        Cnode*  call_node = call_nodes[ i ];
        Region* p         = call_node->get_callee();
        double gran_val   = computeGranularity( call_node, in_cube );
        if ( gran_val < threshold )
        {
            filt_nodes.push_back( call_node );
            cout  << "--Call node name--" << call_node->get_callee()->get_name() << "    Granularity Value: " << gran_val << endl;
        }
        else
        {
            //cout  << "--Call node name not filtered--" << call_node->get_callee()->get_name() << "    Granularity Value: " << gran_val << endl;
        }
    }
}




