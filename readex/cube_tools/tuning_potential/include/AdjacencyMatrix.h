#ifndef __ADJ_MATRIX_H
#define __ADJ_MATRIX_H

#include <iostream>

#include <cubelib/Cube.h>

using namespace cube;
using namespace std;


size_t**
AdjacencyMatrix( size_t n );

void
updateMatrix( Cnode*                       call_node,
              size_t**                     adjMatrix,
              const map< size_t, string >& can_sig_regions );

void
transitiveClosure( size_t   size,
                   size_t** adjMatrix );

size_t
getIndex( const string&                node_name,
          const map< size_t, string >& can_sig_regions );

void
addEdge( size_t   src,
         size_t   to,
         size_t   n,
         size_t** adjMatrix );

void
getCandidateRegions( Cnode*                 phase_node,
                     size_t                 size,
                     size_t**&              a_m,
                     map< size_t, string >& can_sig_regions,
                     vector< string >&      candidate_list );

vector< string >
getSignificantRegions( vector< string >&      candidate_list,
                       vector< Cnode* >&      can_cnodes,
                       size_t**               a_m,
                       map< size_t, string >& can_region_mapping,
                       Cnode*                 phase_node,
                       Cube*                  input_cube );

vector< string >
getParentRegName( size_t                       to,
                  size_t                       size,
                  size_t**                     a_m,
                  const map< size_t, string >& can_sig_regions,
                  const vector< string >&      parent_names );

void
getRestCandidates( vector< string >& sig_nodes,
                   vector< string >& parent_nodes,
                   vector< string >& can_sig_regions );

string
getRegionName( const size_t                idx,
               const map< size_t, string>& can_sig_regions );

void
printMatrix( size_t   n,
             size_t** adjMatrix );

void
freeMatrix( size_t   n,
            size_t** adjMatrix );

Cnode*
getCnode( string           c_name,
          vector< Cnode* > s_nodes );

string
getInfluencedRegion( Cnode*            phasenode,
                     Cnode*            callnode,
                     Cube*             input,
                     vector< string >& parent_names );

bool
checkAllParents( vector< string >& sig_nodes,
                 Cnode*            phasenode,
                 Cnode*            callnode );

void
reconstructMatrix( size_t                 size,
                   size_t **&             adjMatrix,
                   Cnode*                 phase_node,
                   vector< string >&      candidateNodes,
                   map< size_t, string >& list_cnames );

double
computeExclusiveValueOfParent( Cube*  input,
                               Cnode* parent );

#endif
