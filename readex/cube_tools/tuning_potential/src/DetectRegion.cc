/**
 * \file Detect_Region.cc
 * \brief It detects the program regions by reading the cube file and outputs
 */
/******************************************
 *
 */
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <iomanip>
#include <algorithm>
#include <boost/format.hpp>
#include <boost/regex.hpp>
#include <boost/utility.hpp>
#include <list>
#include <map>
#include <numeric>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "../include/DetectRegion.h"
#include "../include/AdjacencyMatrix.h"
#include "../include/TuningPotential.h"
#include "../../datamodel/include/ConfigModel.h"
#include "../../datamodel/include/SignificantRegion.h"

#include "../../common_incl/helper.h"
#include "../../common_incl/Predicates.h"
#include "../../common_incl/enums.h"

#include "config.h"

#define YorN( x ) ( x ? "yes" : "no" )

using namespace std;
using namespace cube;
using namespace services;
using boost::property_tree::ptree;

bool
isCoarseCnode( Cnode*                  cnode,
               const vector< Cnode* >& filt_cnodes );
/**
 *  @brief Prints out the help how to use this tool.
 * @param filename
 */
void
usage_dyn( const char* filename )
{
    cout << "Usage: " << filename << " [-h] -p <phase name> [-t <threshold>] [-c <compute_intensity variation threshold>] [-v <time variation threshold>] [-w <weight threshold>] [-r <file name>] [-f <.tex Tex file name>] <cubefile>\n";
    cout << "  -h    Help; Output this help message\n";
    cout << "  -p    <phase name>  Phase Name\n";
    cout << "  -t    Threshold_value  Choose threshold for granularity (default value 100 ms)\n";
    cout << "  -c    Compute Intensity deviation threshold (absolute default value 10 #)\n";
    cout << "  -v    %  Variation threshold( default value 10% ) of region and phase\n";
    cout << "  -w    %  weight( default value 10% ) across phase \n";
    cout << "  -r    File name without extension\n";
    cout << "  -f    <.tex file name> Tex File name without extension\n";
    exit( -1 );
}

/**
 * Return codes
 * 0 = no inter-phase or intra-phase dynamism
 * 1 = inter-phase dynamism due to variation in execution time only
 * 2 = intra-phase dynamism due to variation in execution time only
 * 3 = intra-phase dynamism due to variation in compute intensity only
 * 4 = 2 + 3
 * 5 = 1 + 2
 * 6 = 1 + 3
 * 7 = 1 + 2 + 3
 */

int
main( int argc, char* argv[] )
{
    string phase_name( "undef" );
    //TODO: tex_filename should be char since ofstream accepts it
    string tex_filename( "undef" );
    string config_file_name( "readex_config.xml" );
    double gran_threshold( 0.1 );
    double thrshld_var_t( 10 );
    double thrshld_var_w( 10 );
    double thrshld_var_com_intnsty( 10 );
    int    argcount( 1 );
    bool   do_phase( false );
    bool   has_thrshld( false );
    bool   gen_tex( false );
    bool   has_config_file( false );
    bool   coarse_regions_only( false );
    int    c;

    while ( ( c = getopt( argc, argv, "t:p:c:v:w:r:f:nh?" ) ) != -1 )
    {
        switch ( c )
        {
            case 'h':
            case '?':
                usage_dyn( argv[ 0 ] );
                break;
            case 'p':
                argcount++;
                phase_name  = string( optarg );
                do_phase    = true;
                argcount   += 2;
                break;
            case 't':
                gran_threshold  = stod( optarg );
                has_thrshld     = true;
                argcount       += 2;
                break;
            case 'c':
                thrshld_var_com_intnsty  = stod( optarg );
                argcount                += 2;
                break;
            case 'v':
                thrshld_var_t  = stod( optarg );
                argcount      += 2;
                break;
            case 'w':
                thrshld_var_w  = stod( optarg );
                argcount      += 2;
                break;
            case 'n':
                coarse_regions_only  = true;
                argcount            += 1;
                break;
            case 'r':
                config_file_name = string( optarg ) + ".xml";
                has_config_file  = true;
                argcount        += 2;
                break;
            case 'f':
                tex_filename  = string( optarg ) + ".tex";
                gen_tex       = true;
                //tex_filename, ".tex" );
                argcount     +=2;
                break;

        }
    }
    if ( !do_phase )
    {
        usage_dyn( argv[ 0 ] );
        cout << "\nError: Wrong arguments.\n";
        exit( -1 );
    }

    if ( ( argc - argcount ) != 0  )
    {
        usage_dyn( argv[ 0 ] );
        cout << "\nError: Wrong arguments.\n";
        exit( -1 );
    }


    int returnCode( 0 );
    if ( argcount == argc )
    {
        Cube* input_cube = NULL;
        try
        {
            string c_filename = string( argv[ argcount - 1 ] );
            input_cube = new Cube();
             // Read input files
            cout << "Reading " << c_filename << "..." << endl;
            //checks if the phasename and cubex file names are same
            if ( phase_name == c_filename )
            {
                usage_dyn( argv[ 0 ] );
                cerr << "\nError: Phase region name and .cubex file name can not be same.\n";
                exit( -1 );
            }

            input_cube->openCubeReport( c_filename );
            cerr << " Done." << endl;

            const vector< Cnode* >& candidate_nodes = input_cube->get_cnodev(); // without further filteration
            vector< Cnode* >        coarse_cnodes;
            vector< Cnode* >        rest_nodes;
            Cnode*                  phase_node;

            cerr << "Granularity threshold: " << gran_threshold << '\n';
            cerr << candidate_nodes.size() << "candidate_nodes" << '\n';

            phase_node = checkPhaseRegion( candidate_nodes, phase_name );
            if ( phase_node )
            {
                cerr << "Phase node " << phase_node->get_callee()->get_name() << " found in candidate regions. It has " << phase_node->num_children() << " children.\n";
            }

            if ( has_thrshld )
            {
                for ( const auto& node : candidate_nodes )
                {
                    double gran = computeGranularity( node, input_cube );
                    cerr << "Granularity of " << node->get_callee()->get_name() << ": " << gran << ", Number of children: " << node->num_children() << '\n';
                    if ( gran > gran_threshold )
                        rest_nodes.push_back( node );
                }
                cerr << rest_nodes.size() << " rest_nodes\n";
                //just debug output
                for ( const auto& node : rest_nodes )
                {
                    double gran = computeGranularity( node, input_cube );
                    //cout << "Granularity of " << node->get_callee()->get_name() << ": " << gran << '\n';
                }

                //Cnode* phase_node = checkPhaseRegion( rest_nodes, phase_name );
                phase_node = checkPhaseRegion( rest_nodes, phase_name );

                /*check for the phase node. If the phase region is not in rest nodes, discard the rest nodes and
                 * initialize with the candidate cnodes.
                 */
                if ( phase_node == NULL )
                {
                    cout << "Phase node " << phase_name << " not found in coarse granular regions.\n";
                    coarse_cnodes.assign( candidate_nodes.begin(), candidate_nodes.end() );
                }
                else
                    coarse_cnodes.assign( rest_nodes.begin(), rest_nodes.end() );
            }
            else
                coarse_cnodes.assign( candidate_nodes.begin(), candidate_nodes.end() );

            cout << coarse_cnodes.size() << " coarse_nodes\n";

            //check for phase region
            phase_node = checkPhaseRegion( coarse_cnodes, phase_name );

            if ( phase_node != NULL )
            {
                cout << "Phase node " << phase_node->get_callee()->get_name() << '\n';
                unsigned int no_child = phase_node->num_children();
                if ( no_child == 0 )
                    cout << "Phase region doesn't have any child\n";
                else
                {
                    cout << "There is a phase region\n" << endl;

                    //Traverse the phase node in order to compute granularity of each sub nodes
                    vector< string >      can_r_names;
                    vector< Cnode* >      candidateNodes;
                    map< size_t, string > l_cnames;
                    traverseTree( phase_node, coarse_cnodes, input_cube, can_r_names, l_cnames, candidateNodes, gran_threshold, coarse_regions_only );

                    if ( can_r_names.empty() )
                    {
                        cout << "No significant candidate region in Progress loop\n" << endl;
                    }
                    else
                    {
                        size_t** a_m = generateAdjacencyMatrix( l_cnames.size(),  phase_node, l_cnames );

                        getCandidateRegions( phase_node, l_cnames.size(), a_m, l_cnames, can_r_names );

                        cout << "\n\nCandidate regions are:" << endl;
                        for ( const auto& region : can_r_names )
                            cout << "    " << region << ' ' << endl;
                        cout << "\n\n\n";

                        /* check if candidate region contain more than one region other than the phase region */
                        if ( can_r_names.size() == 1 )
                        {
                            cout << "There is no candidate region other than the Phase Region. Please check your .cubex output" << endl;
                            //exit(-1);
                        }

                        vector< string > sig_region_names = getSignificantRegions( can_r_names, candidateNodes, a_m, l_cnames,  phase_node, input_cube );
                      //  freeMatrix( l_cnames.size(), a_m );

                        if ( sig_region_names.empty() )
                        {
                            cout << "\n\nThere is no significant region in the progress loop\n\n" << endl;
                        }
                        else
                        {
                            cout << "\n\nSignificant regions are:" << endl;
                            for ( const auto& region : sig_region_names )
                                cout <<"    " << region << ' ' << endl;
                            cout << "\n\n\n";

                            // Compute Intra-phase Tuning Potential
                            list< SignificantRegion* > sig_regions = ComputeIntraPhaseTP( phase_node, candidateNodes, sig_region_names, input_cube );
                            // Compute Inter-phase Tuning Potential
                            PhaseRegion* phase_region = ComputeInterPhaseTP( phase_node, input_cube, thrshld_var_t );

                            vector< string > time_variation_reg;
                            vector< string > com_intensity_variation_reg;
                            //print summery
                            returnCode = printSummary( phase_region, sig_regions, thrshld_var_t, thrshld_var_w, thrshld_var_com_intnsty, time_variation_reg, com_intensity_variation_reg, gen_tex, tex_filename, gran_threshold );

                            if ( !sig_regions.empty() || true ) // Generate configuration file anyway
                            {
                                ConfigModel* model;
                                ptree        configTree;
                                cout << "Writing into the configuration file..." << endl;
                                try
                                {
                                  struct stat st;
                                  if (stat(config_file_name.c_str(), &st) == 0) {
                                     //std::cout << "file exists\n";
                                  } else {
                                     //std::cout << "file does not exist\n";
                                     std::stringstream cmd;
                                     cmd << "cp " << PERISCOPE_INSTALL_DIRECTORY << "/templates/readex_config.xml.default " << config_file_name << std::endl;
                                     system(cmd.str().c_str());
                                     cout<< "Config file did not exist. Copied template with " << cmd.str() << std::endl;
                                  }

                                    boost::property_tree::xml_parser::read_xml( config_file_name, configTree, boost::property_tree::xml_parser::trim_whitespace );

                                    //isExist = true
                                    //cout << "read_xml\n"; cout.flush();
                                    model = new ConfigModel( true, sig_regions, sig_region_names, phase_region, thrshld_var_w, thrshld_var_t, thrshld_var_com_intnsty, gran_threshold, time_variation_reg, com_intensity_variation_reg );
                                    //cout << "generated new model\n"; cout.flush();
                                    model->updateTree( configTree );
                                    //cout << "updated tree\n"; cout.flush();
                                    model->toXML( config_file_name );
                                }
                                catch ( exception &e )
                                {
                                    //serialize significant region information
                                    model = new ConfigModel( false, sig_regions, sig_region_names, phase_region, thrshld_var_w, thrshld_var_t, thrshld_var_com_intnsty, gran_threshold, time_variation_reg, com_intensity_variation_reg );
                                    cout << "exception create template\n"; cout.flush();
                                    model->createTemplate();
                                    model->toXML( config_file_name );
                                }
                                delete model;
                            }
                            delete phase_region;
                            for( const auto region : sig_regions )
                            {
                                delete region;
                            }
                        }

                    }
                }

            }
            else
            {
                cout << "Either phase region name doesn't match with the provided name OR\n";
                cout << "Design time tuning requires at least one phase region"<< endl;
            }

        }
        catch ( const RuntimeError& err )
        {
            cerr << err.get_msg() << endl;
        }
        if ( input_cube != NULL )
        {
            input_cube->closeCubeReport();
            delete input_cube;
        }
    }
    return returnCode;
}

/**
 * @brief Check the name of the phase region
 * @param call_nodes
 * @param ct_count
 * @param phase_name
 * @return if phase name matches returns the phase node otherwise NULL
 */
Cnode*
checkPhaseRegion( const vector< Cnode* >& call_nodes,
                  const string&           phase_name )
{
    size_t ct_count = call_nodes.size();
    vector< string > names( ct_count, "no name" );
    for ( size_t i = 0; i < ct_count; i++ )
    {
        Cnode* cnode   = call_nodes[ i ];
        Region* region = cnode->get_callee();
        names[ i ]     = region->get_name();
        if ( names[ i ] == phase_name )
        {
            return cnode;
        }
    }
    return NULL;
}

/**
 *@brief Traverse the whole call tree and remove those nodes which are in the filtered node list
 * @param call_node
 * @param coarse_cnodelist
 * @param in_cube
 * @param sig_region_names
 * @param list_cnames
 * @param sign_nodes
 * @param gran_threshold
 * @param coarse_regions_only
 */
void
traverseTree( Cnode*                  call_node,
              const vector< Cnode* >& coarse_cnodelist,
              Cube*                   in_cube,
              vector< string >&       sig_region_names,
              map< size_t, string >&  list_cnames,
              vector< Cnode* >&       sign_nodes,
              double                  gran_threshold,
              bool                    coarse_regions_only )
{

    // if the tree has omp/mpi events
    if ( !isUserNode( call_node ) || !isCoarseCnode( call_node, coarse_cnodelist ) || ( !coarse_regions_only && isForkNode( call_node ) ) )
    {
        return;
    }
    else
    {
    	// check for MPI/OMP parallel events of all the children of the current node's parent
    	Cnode* p_node = call_node->get_parent();
    	bool has_p_event = false;
    	//cerr << "Callr node " << call_node->get_callee()->get_name() << '\n';
        checkParallelEvents (p_node, call_node, &has_p_event);
        if (has_p_event)
    	    return;
        Region* p      = call_node->get_callee();
        string  region = p->get_name();
        double gran = computeGranularity( call_node, in_cube );
        cerr << "Granularity of " << call_node->get_callee()->get_name() << ": " << gran << '\n';
        if ( gran < gran_threshold ) return;

        if ( !region.empty() && !checkExistingName( region, sig_region_names ) ) {
            size_t cname_cnt = list_cnames.size();
            sig_region_names.push_back( region );
            list_cnames.insert( make_pair( cname_cnt, region ) );
            // cout << " Traverse Tree   NAME: " << region << endl;
            sign_nodes.push_back( call_node );
        }
    }
    size_t no_child = call_node->num_children();

    if ( ( !coarse_regions_only && no_child > 0 && !isForkNode( call_node ) ) || ( coarse_regions_only && no_child > 0 ) )
    {
        for ( size_t cn_i = 0; cn_i < no_child; cn_i++ )
        {
            Cnode* cnode = call_node->get_child( cn_i );
            traverseTree( cnode, coarse_cnodelist, in_cube, sig_region_names, list_cnames, sign_nodes, gran_threshold, coarse_regions_only );
        }
    }
    else
        return;
}

void
checkParallelEvents (Cnode* curr_node, Cnode* call_node, bool* has_p_event)
{
	if( curr_node == NULL )
            curr_node = call_node;
	// check either it's a OMP/MPI node or it is under OMP/MPI region
	if ( curr_node != NULL && !(curr_node == call_node) && strcmp( curr_node->get_callee()->get_name().c_str(), call_node->get_callee()->get_name().c_str() ) == 0
			&& ( isUserNode( curr_node ) || !isUserNode( curr_node->get_parent() ) )
	   )
	{
		*has_p_event = true;
		cout << "  NAME: " << curr_node->get_callee()->get_name() << endl;
	}
        size_t no_child = curr_node->num_children();
	for( size_t cn_i = 0; cn_i < no_child; cn_i++ )
	{
		Cnode* cnode = curr_node->get_child( cn_i );
		checkParallelEvents( cnode, call_node, has_p_event);
	}

}



/**
 * @brief Generate Adjacency matric of call node tree
 * @param matrix_size
 * @param phase_node
 * @param sig_regions
 * @return
 */
size_t**
generateAdjacencyMatrix( size_t                 matrix_size,
                         Cnode*                 phase_node,
                         map< size_t, string >& sig_regions )
{
    //size_t cnt_candRegions = sig_regions->size();
    //phase_node->get_children();
    size_t**        adj_matrix      = AdjacencyMatrix( matrix_size );
    vector<Cnode*>& p_subtree_nodes = phase_node->get_whole_subtree();
    for ( const auto& name : sig_regions )
    {
        if ( strcmp( name.second.c_str(), phase_node->get_callee()->get_name().c_str() ) == 0 )
            updateMatrix( phase_node, adj_matrix, sig_regions );
        else
        {
            Cnode* call_node = getCnode( name.second, p_subtree_nodes );
            updateMatrix( call_node, adj_matrix, sig_regions );
        }
    }
    transitiveClosure( matrix_size, adj_matrix );
    //printMatrix( cnt_candRegions, adj_matrix );
    return adj_matrix;
}

bool
checkExistingName( const string&           c_name,
                   const vector< string >& cnode_names )
{
    return ( find( cnode_names.begin(), cnode_names.end(), c_name ) != cnode_names.end() ) ? true : false;
}

bool
isCoarseCnode( Cnode*                  cnode,
               const vector< Cnode* >& filt_cnodes )
{
    return ( find( filt_cnodes.begin(), filt_cnodes.end(), cnode ) != filt_cnodes.end() ) ? true : false;
// Old implementation
//    for ( size_t i = 0; i < filt_cnodes.size(); i++ )
//    {
//        Cnode* cnode_i = filt_cnodes[ i ];
//        if ( cnode->get_id() == cnode_i->get_id() )
//            return true;
//    }
//    return false;
}

/**
 * Prints the Tuning Potential summary
 * @param phase Phase Information
 * @param significantRegions list of Significant Regions
 * @param thrshld_var_t
 * @param thrshld_var_w
 * @param thrshld_var_comIntnsty
 * @param time_variation_reg
 * @param com_intensity_variation_reg
 * @param tex_gen
 * @param tex_f_name
 * @param gran_threshold
 */
int
printSummary( PhaseRegion*                      phase,
              const list< SignificantRegion* >& significantRegions,
              double                            thrshld_var_t,
              double                            thrshld_var_w,
              double                            thrshld_var_comIntnsty,
              vector< string >&                 time_variation_reg,
              vector< string >&                 com_intensity_variation_reg,
              bool                              tex_gen,
              const string&                     tex_f_name,
              double                            gran_threshold )
{
    ofstream ofs;

    cout << '\n';

    //.tex file generate
    if ( tex_gen )
    {
        ofs.open( tex_f_name.c_str(), ios_base :: out );
        ofs << "\\section{Readex-dyn-detect Report}\n";
        ofs << "\\label{sec:dyn-detect}\n";
        /*ofs << "\\begin{table}[]\n";
        ofs << "\\centering\n";
        ofs << "\\caption{My caption}\n";
        ofs << "\\label{my-label}\n";*/
        ofs << "\\begin{longtable}[c]{|l|l|l|}\n";
        ofs << "\\hline\n";
        ofs << "\\multicolumn{3}{|l|}{readex-dyn-detect} \\\\ \\hline\n";
        ofs << "Application name & \\multicolumn{2}{l|}{LULESH} \\\\ \\hline\n";
        ofs << "Benchmark ID & \\multicolumn{2}{l|}{Benchmark01} \\\\ \\hline\n";
        ofs << "\\multirow{4}{*}{Thresholds} & Granularity(sec) & " << gran_threshold << " \\\\ \\cline{2-3}\n";
        ofs << "& Compute intensity variation threshold(Absolute threshold) & "<< thrshld_var_comIntnsty << " \\\\ \\cline{2-3} \n";
        ofs << "& Execution time variation(\\%mean) & " << thrshld_var_t << " \\\\ \\cline{2-3}\n";
        ofs << "& Weight(\\%phase) & " << thrshld_var_w << " \\\\ \\hline\n";
        ofs << "\\multicolumn{3}{|l|}{List of significant regions} \\\\ \\hline\n";
    }

    printf( "threshold time variation (percent of mean region time): %f\n", thrshld_var_t );
    printf( "threshold compute intensity deviation (#ops/L3 miss): %f\n", thrshld_var_comIntnsty );
    printf( "threshold region importance (percent of phase exec. time): %f\n\n", thrshld_var_w );

    bool has_interDyn( false );

    cout << "SUMMARY:\n";
    cout << "========\n" << endl;
    if ( phase == NULL )
        cout << "No inter-phase dynamism\n" << endl;
    else if ( !phase->has_dynamism  )
        cout << "No inter-phase dynamism\n" << endl;
    else
    {
        cout << "Inter-phase dynamism due to variation of the execution time of phases\n" << endl;
        has_interDyn = true;
    }

    bool has_intraDyn_t( false );
    bool has_intra_compIn( false );
    if ( significantRegions.empty() )
        cout << "No intra-phase dynamism\n" << endl;
    else
    {
        vector< double > intensity_values;
        vector< string > region_t_variation;
        vector< string > region_intensity;

        for ( const auto& region : significantRegions )
        {
            if ( region->dynamism_metrics[ 0 ].dev_perc_reg > thrshld_var_t && region->dynamism_metrics[ 0 ].dyn_perc_phase > thrshld_var_w ) // && region->dynamism_metrics[ 0 ].dev_perc_phase > 5
            {
                region_t_variation.push_back( region->name );
                region->dynamism_metrics[ 0 ].has_variation = true;
            }

            if ( region->dynamism_metrics[ 0 ].dyn_perc_phase > thrshld_var_w ) {
                region_intensity.push_back( region->name );
                intensity_values.push_back( region->dynamism_metrics[ 1 ].dyn_perc_phase );
                //printf( "Intensity: %f\n", region->dynamism_metrics[ 1 ].dyn_perc_phase );
            }
        }

        //and variation across phases of Execution time
        if ( !region_t_variation.empty() )
        {
            cout << "Intra-phase dynamism due to time variation(%) of the following important significant regions" << endl;
            for ( const auto& region : region_t_variation )
            {
                cout << boost::format( "     %-25s \n" ) % region ;
            }
            time_variation_reg.assign(  region_t_variation.begin(), region_t_variation.end() );
            has_intraDyn_t = true;
        } else {
            cout << "No intra-phase dynamism due to time variation\n" << endl;
        }

        if ( !region_intensity.empty() )
        {
            //compute std_dev of intensity of those regions
            //if std_dev is larger than thrshld_var_comIntnsty report intra-phase dynamism due to compute intensity
            double sum  = std::accumulate( intensity_values.begin(), intensity_values.end(), 0.0 );
            double mean = sum / intensity_values.size();

            double sq_sum                     = std::inner_product( intensity_values.begin(), intensity_values.end(), intensity_values.begin(), 0.0 );
            double stdev_of_compute_intensity = std::sqrt( sq_sum / intensity_values.size() - mean * mean );

            if ( stdev_of_compute_intensity > thrshld_var_comIntnsty )
            {
                cout << "Intra-phase dynamism due to variation in the compute intensity of the following important significant regions" << endl;
                for ( const auto& intensity : region_intensity )
                {
                    cout << boost::format( "     %-25s \n" ) % intensity;
                    com_intensity_variation_reg.push_back( intensity );
                }
                //update has_variation attribute for compute intensity
                for ( const auto& region : significantRegions )
                {
                    if ( checkExistingName( region->name, com_intensity_variation_reg ) )
                        region->dynamism_metrics[ 1 ].has_variation = true;
                }
                has_intra_compIn = true;
            }
            else
            {
                cout << "No intra-phase dynamism due to compute intensity variation\n" << endl;
            }
        }
        else
        {
          cout << "No intra-phase dynamism due to compute intensity variation\n" << endl;
        }

        if ( tex_gen && ofs.is_open() )
        {
            boost::regex expression( "[*_]" );
            //inserts rows for all significant regions
            for ( const auto& region : significantRegions )
            {
                string newString = find_match_offset( region->name , expression );
                ofs << "\\multirow{6}{*}"<< "{" << newString << "}" <<  " & Granularity(seconds) & " << region->granularity << " \\\\ \\cline{2-3}\n";
                ofs << "& Compute Intensity (ops/L3 miss) & " << region->dynamism_metrics[ 1 ].dyn_perc_phase <<"\\\\ \\cline{2-3}\n";
                ofs << "& execution time variation (deviation in \\% of mean region execution time) & " << region->dynamism_metrics[ 0 ].dev_perc_reg << " \\\\ \\cline{2-3}\n";
                ofs << "& weight (\\% of phase time) & " << region->dynamism_metrics[ 0 ].dyn_perc_phase << " \\\\ \\cline{2-3}\n";
                ofs << "& Has significant variation in execution time? & " <<   YorN( region->dynamism_metrics[ 0 ].has_variation ) << " \\\\ \\cline{2-3}\n";
                ofs << "& Has significant variation in compute intensity? & " << YorN( region->dynamism_metrics[ 1 ].has_variation )  << " \\\\ \\hline\n";
            }
            // insert phase information
            ofs << "\\multicolumn{3}{|l|}{} \\\\ \\hline\n"; // empty row

            ofs << "\\multirow{3}{*}{Phase} & Granularity(seconds) & " << phase->granularity << " \\\\ \\cline{2-3}\n";
            ofs << "& execution time variation(deviation in \\% of mean phase execution time) & " << phase->dev_perc << " \\\\ \\cline{2-3}\n";
            ofs << "& has inter-phase dynamism? & " <<  YorN( phase->has_dynamism ) << " \\\\ \\hline\n";

            ofs << "\\multicolumn{3}{|l|}{} \\\\ \\hline\n";

            ofs << "\\multicolumn{3}{|l|}{Summary} \\\\ \\hline\n";
            ofs << "Intra-phase Dynamism? & \\multicolumn{2}{l|}" << "{";

            if ( !com_intensity_variation_reg.empty() || !time_variation_reg.empty() )
                ofs << "yes";
            else
                ofs << "no";
            ofs <<"}" << " \\\\ \\hline\n";

            ofs << "\\multirow" << "{" << 3 + com_intensity_variation_reg.size()+time_variation_reg.size() << "}" << "{*}{} & \\multicolumn{2}{l|}{Regions with significant variation in the execution time of rts'} \\\\ \\cline{2-3}\n";
            for ( const auto& region : time_variation_reg )
            {
                string newString = find_match_offset( region, expression );
                ofs << "& \\multicolumn{2}{l|}" << "{" << newString << "}" << " \\\\ \\cline{2-3}}\n";
            }

            ofs << "& \\multicolumn{2}{l|}{} \\\\ \\cline{2-3}\n"; //empty row
            ofs << "& \\multicolumn{2}{l|}{Regions with significant deviation in compute intensity} \\\\ \\cline{2-3}\n";
            for ( auto region = com_intensity_variation_reg.begin(); region != com_intensity_variation_reg.end(); ++region )
            {
                string newString = find_match_offset( *region, expression );

                ofs << "& \\multicolumn{2}{l|}" << "{" << newString << "}" ;
                if ( boost::next( region ) == com_intensity_variation_reg.end() )
                    ofs << " \\\\ \\hline\n";
                else
                    ofs << " \\\\ \\cline{2-3}\n";
            }
            ofs << "\\multicolumn{3}{|l|}{} \\\\ \\hline\n"; // empty row

            ofs << "Inter-phase Dynamism? & \\multicolumn{2}{l|}" << "{" <<  YorN( phase->has_dynamism )   << "}" << " \\\\ \\hline\n";
        }

    }
    if ( tex_gen && ofs.is_open() )
    {
        ofs << "\\caption{readex-dyn-detect tool report for LULESH benchmark}\n";
        ofs << "\\end{longtable}\n";
        //ofs << "\\end{table}\n";
        ofs.close();
    }

    return has_interDyn << 2 | has_intraDyn_t << 1 | has_intra_compIn; //has_intra_compIn + has_intraDyn_t * 2 + has_interDyn * 4
}

string
find_match_offset( string const&       search_string,
                   boost::regex const& re )
{
    boost::match_results< string::const_iterator > w;
    boost::match_flag_type                         flags = boost::match_default;
    string::const_iterator                         s     = search_string.begin();
    string::const_iterator                         e     = search_string.end();
    char*                                          newString;
    size_t                                         pos   = 0;
    size_t                                         len   = 0;

    newString = ( char* )realloc( NULL, strlen( search_string.c_str() ) + 1  );
    strcpy( newString, search_string.c_str() );

    while( boost::regex_search( s, e, w, re, flags ) )
    {
        len        = strlen( newString );
        int l      = w.length();
        int p      = w.position();
        s         += p + l;
        newString  = ( char* )realloc( newString, strlen( newString ) + 1 + 1 ); // 1 for \ character
        if ( 0 < p && p < len )
        {
            pos += p;
            memmove( newString + pos + 1, newString + pos, len - pos + 1 );
            newString[ pos ]='\\';
            pos += l + 1;
        }
    }
    return newString;
}
