/**
 * Author: Anamika Chowdhury
 * \file DetectRegion.h
 * \brief Contains a detect regions specific calls
 *
 ************************************************/
#ifndef DETECT_REGION_H_
#define DETECT_REGION_H_

#include <iostream>
#include <list>
#include <boost/regex.hpp>
#include <boost/property_tree/ptree.hpp>

#include <cubelib/Cube.h>
#include <cubelib/CubeServices.h>

#include "../../datamodel/include/SignificantRegion.h"

using namespace std;
using namespace cube;
using boost::property_tree::ptree;

/*
 * This function checks for the phase region name equality with the provided name given via command line argument
 */
Cnode*
checkPhaseRegion( const vector< Cnode* >& call_nodes,
                  const string&           phase_name );

/*
 * Checks the type of each call node
 */
//bool checkCallNodeType( Cnode* c_node );

/*
 * Traverse the children of a call node
 */
void
traverseTree( Cnode*                  call_node,
              const vector< Cnode* >& coarse_cnodelist,
              Cube*                   in_cube,
              vector< string >&       sig_region_names,
              map< size_t, string >&  list_cnames,
              vector< Cnode* >&       sign_nodes,
              double                  gran_threshold,
              bool                    coarse_regions_only );
/**
 * @brief check for MPI/OMP parallel events of all the children of the current node's parent
 */
void
checkParallelEvents (Cnode* curr_node, Cnode* call_node, bool* has_p_event);

/*
 * Create adjacency matrix from call tree
 */
size_t**
generateAdjacencyMatrix( size_t                 size,
                         Cnode*                 phase_node,
                         map< size_t, string >& sig_regions );

//void getCandidateList( Cnode* phase_node, Cnode* find_node, vector<Cnode*>* nested_nodes );
/*
 * check list of existing regions
 */
bool
checkExistingName( const string&           c_name,
                   const vector< string >& cnode_names );

/*
 * Matches the id of a cnode with list of filtered cnodes.
 */
bool
isCaorseCnode( Cnode*                  cnode,
               const vector< Cnode* >& filt_cnodes );

/*
 * print summery
 *
 */
int
printSummary( PhaseRegion*                      phase,
              const list< SignificantRegion* >& sign_r_list,
              double                            thrshld_var_t,
              double                            thrshld_var_w,
              double                            thrshld_var_comIntnsty,
              vector<string>&                   time_variation_reg,
              vector<string>&                   com_intensity_variation_reg,
              bool                              tex_gen,
              const string&                     tex_f_name,
              double                            gran_threshold );

/* Find substring that matches to the regular expression*/
std::string
find_match_offset( std::string const&  search_string,
                   boost::regex const& re );
#endif //end of detect region
