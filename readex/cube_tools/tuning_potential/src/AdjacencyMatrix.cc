#include <algorithm>
#include <vector>
#include <iterator>
#include <string>

using namespace std;

#include "../include/AdjacencyMatrix.h"
#include "../include/DetectRegion.h"
#include "../../common_incl/helper.h"
 
/**
 * @brief Create two dimensional adjacency matrix of call-tree
 * @param n
 * @return
 */
size_t**
AdjacencyMatrix( size_t n )
{
    size_t** adjacent_matrix = new size_t*[ n ];
    for( size_t i = 0; i < n; i++ )
    {
        adjacent_matrix[ i ] = new size_t[ n ];
        for( size_t j = 0; j < n; j++ )
        {
            adjacent_matrix[ i ][ j ] = 0;
        }
    }
    return adjacent_matrix;
}

void
updateMatrix( Cnode*                       call_node,
              size_t**                     adjMatrix,
              const map< size_t, string >& can_sig_regions )
{
    string node_name = call_node->get_callee()->get_name();
    size_t from_idx  = getIndex( node_name, can_sig_regions );
    if( from_idx == -1 )
        return;
    size_t n                          = can_sig_regions.size();
    vector< Cnode* >& c_subtree_nodes = call_node->get_whole_subtree();
    for( const auto& cnode : c_subtree_nodes )
    {
        const Region* region = cnode->get_callee();
        size_t to_idx        = getIndex( region->get_name(), can_sig_regions );
        if( to_idx == -1 )
            continue;
        addEdge( from_idx, to_idx, n, adjMatrix );
    }
}


/**
 * @brief Construct transitive closure of adjMatrix[][] using Floyd Warshall algorithm
 * @param size
 * @param adjMatrix Two dimensional array of adjacency matrix
 */
void
transitiveClosure( size_t   size,
                   size_t** adjMatrix )
{
    for( size_t k = 0; k < size; k++ )
        for( size_t i = 0; i < size; i++ )
            for( size_t j = 0; j < size; j++ )
                adjMatrix[ i ][ j ] = adjMatrix[ i ][ j ] || ( adjMatrix[ i ][ k ] && adjMatrix[ k ][ j ] );
}

void
addEdge( size_t   src,
         size_t   to,
         size_t   n,
         size_t** adjMatrix )
{
    if( src < n && to < n )
        *( *( adjMatrix + src ) + to ) = 1;
    else
        return;

}
/**
 * @brief Get non-nested candidate for significant regions
 * @param size  size of can_sig_regions
 * @param a_m Two dimensional array of adjacency matrix
 * @param can_sig_regions  maps of significant region with index
 * @param candidate_list candidate list
 * @return
 */
void
getCandidateRegions( Cnode*                 phase_node,
                     size_t                 size,
                     size_t**&              a_m,
                     map< size_t, string >& can_sig_regions,
                     vector< string >&      candidate_list )
{
    vector< string > can_r_names;
    vector< string > parent_names;

    // Now check the strongly connected components with more than one region
    for( size_t i = 0; i < size; i++ )
    {
        for( size_t j = 0; j < size; j++ )
        {
            if( i != j && a_m[ i ][ j ] && a_m[ j ][ i ] )
            {
                string name_i = getRegionName( i, can_sig_regions );
                string name_j = getRegionName( j, can_sig_regions );

                if( can_r_names.empty() )
                {
                    can_r_names.push_back( name_i );
                    can_r_names.push_back( name_j );
                }
                if( !checkExistingName( name_i, can_r_names ) )
                    can_r_names.push_back( name_i );
                if( !checkExistingName( name_j, can_r_names ) )
                    can_r_names.push_back( name_j );
            }
        }
    }

    //delete strongly connected components with more than two nodes
    if( !can_r_names.empty() )
    {
        cout << "\n\n";
        cout << "The following regions are strongly connected components:" << endl;
        for( const auto& cnode : can_r_names )
            cout << cnode << endl;
        getRestCandidates( can_r_names, parent_names, candidate_list );
        //reconstruct the whole matrix
        reconstructMatrix( size, a_m, phase_node, candidate_list, can_sig_regions );
    }
}


/**
 * get the names of regions except leaf nodes and parent nodes to them.
 * @param sig_nodes
 * @param parent_nodes
 * @param can_sig_regions
 * @return
 */
void
getRestCandidates( vector< string >& sig_nodes,
                   vector< string >& parent_nodes,
                   vector< string >& can_sig_regions )
{
    vector< string > common;

    // Merging sig_nodes with parent nodes
    parent_nodes.insert( parent_nodes.end(), sig_nodes.begin(), sig_nodes.end() );
    sort( sig_nodes.begin(), sig_nodes.end() );
    sort( can_sig_regions.begin(), can_sig_regions.end() );
    sort( parent_nodes.begin(), parent_nodes.end() );
    set_intersection( can_sig_regions.begin(), can_sig_regions.end(), parent_nodes.begin(), parent_nodes.end(), std::back_inserter( common ) );
    can_sig_regions.erase( set_difference( can_sig_regions.begin(), can_sig_regions.end(), common.begin(), common.end(),
                                           can_sig_regions.begin() ), can_sig_regions.end() );
    // if( !can_sig_regions.empty() )
    // {
    //     cout << "\n\n";
    //     cout << "Remaining candidate regions are: " << endl;
    //     for( const auto& region : can_sig_regions )
    //         cout << "   "<< region << ' ' << endl;
    //     cout << "\n\n\n";
    // }
}

/**
 * returns the call_tree node given by region name
 * @param phase_node
 * @param node_name
 * @return
 */
Cnode* getCnode( string c_name, vector<Cnode*> s_nodes  )
{
    if( !s_nodes.empty() )
    {
        for( vector<Cnode*>::iterator cnode_i = s_nodes.begin(); cnode_i != s_nodes.end(); ++cnode_i )
        {
            string name = (*cnode_i)->get_callee()->get_name();
            if( strcmp( name.c_str(), c_name.c_str() ) == 0 )
                return *cnode_i;
        }
    }
    return NULL;

}

/**
 *
 * @param region_names
 * @param a_m
 * @param phase_node
 * @param input_cube
 * @return
 */
vector< string >
getSignificantRegions( vector< string >&      candidate_list,
                       vector< Cnode* >&      can_cnodes,
                       size_t**               a_m,
                       map< size_t, string >& can_region_mapping,
                       Cnode*                 phase_node,
                       Cube*                  input_cube )
{
    vector< string > sig_r_names;

    while( !candidate_list.empty() )
    {
        vector< string > non_sig_list;
        vector< string > parent_names;
        size_t size = can_region_mapping.size();
        // for( size_t i = 0; i < size; i++ ){
        //   string name  = getRegionName( i, can_region_mapping );
        //   cout << i << "  " << name << endl;
        // }
        //printMatrix(size,a_m);
        // Gets all the leaf nodes which are not nested
        for( size_t i = 0; i < size; i++ )
        {
          string name  = getRegionName( i, can_region_mapping );
          //cout << "current region: " << name << endl;

            bool visited = false;
            for( size_t j = 0; !visited && j < size; j++ )
            {
                if( a_m[ i ][ j ] == 1 ){
                    visited = true;
                    //cout << "Calls: "<< getRegionName( j, can_region_mapping ) << endl;
                }
            }
            if( !visited )
            {
                //cout << "current region is leaf! " << name << endl;
                Cnode* cnode = getCnode( name, can_cnodes );
                //cout <<  "Computing influenced node........." << endl << endl;
                string modified_name = getInfluencedRegion( phase_node, cnode, input_cube, parent_names );
                //cout << "Influenced region: " << modified_name << endl;
                //cout << "Modified name" << modified_name << endl;
                // Check if the influenced node is the leaf node or not
                if ( modified_name.empty() )
                {
                    non_sig_list.push_back( name );
                    //cout << "Continue"<<endl;
                    //cout << "-------------------"<<endl;
                    continue;
                }
                if( sig_r_names.empty() )
                    sig_r_names.push_back( modified_name );

                //remove leaf if it is not in significant list
                if( strcmp( name.c_str(), modified_name.c_str() ) != 0 )
                {
                    non_sig_list.push_back( name );
                }
                size_t i_i = getIndex( modified_name, can_region_mapping );
                // get all the parents of the leaf node
                vector< string > rem_r_names = getParentRegName( i_i, size, a_m, can_region_mapping, parent_names );
                //cout << endl << "Parents of influenced region " <<modified_name <<": "<< endl;
                //for( const auto& region : rem_r_names )
                //    cout <<"   "<< region << ' '<< endl;
                //cout << endl << endl << endl;

                if( !rem_r_names.empty() )
                {
                    parent_names.insert( parent_names.end(), rem_r_names.begin(), rem_r_names.end() );

                }

                if( !checkExistingName( modified_name, sig_r_names ) )
                    sig_r_names.push_back( modified_name );

                // cout << "Significant -- getSigRegiov() regions are: " << endl;
                // for( const auto& region : sig_r_names )
                //     cout << "   "<< region << ' '<< endl;
                // cout << endl << endl << endl;
                //
                // cout << "Non Significant regions are: " << endl;
                // for( const auto& region : non_sig_list )
                //     cout << "   "<< region << ' '<< endl;
                // cout << endl << endl << endl;
                //
                // cout << "Parent regions are: " << endl;
                // for( const auto& region : parent_names )
                //     cout << "   "<< region << ' '<< endl;
                // cout << endl << endl << endl;
                // cout << "-------------------"<<endl;
            }
        }

        if( !non_sig_list.empty() )
        {
            parent_names.insert( parent_names.end(), non_sig_list.begin(), non_sig_list.end() );
        }

        getRestCandidates( sig_r_names, parent_names, candidate_list );
        reconstructMatrix( size, a_m, phase_node, candidate_list, can_region_mapping );

    }
    return sig_r_names;
}

/**
 * @brief computes the impacted region. If Incl time of leaf > Excl time of parent. return the leaf node otherwise parent
 * @param phasenode
 * @param callnode
 * @param input
 * @return
 * TO DO check vice versa. check parent's other children as well
 */
string
getInfluencedRegion( Cnode*            phasenode,
                     Cnode*            callnode,
                     Cube*             input,
                     vector< string >& p_names )
{
    Metric* met_t = input->get_met( "time" );

    //get immediate parent
    Cnode* parent = callnode->get_parent();
    string p_name = parent->get_callee()->get_name();
    string c_name = callnode->get_callee()->get_name();
    if( parent == phasenode )
        return c_name;

    Value* met_tau_t_call = input->get_sev_adv( met_t,
                                                CUBE_CALCULATE_INCLUSIVE,
                                                callnode,
                                                CUBE_CALCULATE_EXCLUSIVE );

    double time_call, time_parent;

    if( met_tau_t_call->myDataType() == CUBE_DATA_TYPE_TAU_ATOMIC )
    {
        /* Casting from Value to TauAtomicValue */
        TauAtomicValue* tau_tuple_t_call = ( TauAtomicValue* ) met_tau_t_call;
        string stat_tuple_t_call         = tau_tuple_t_call->getString();
        vector< string > tau_val_t_call  = ValueParser( stat_tuple_t_call );
        double avg_t_call                = stod( tau_val_t_call.at( 3 ) );
        double N_t_call                  = stod( tau_val_t_call.at( 0 ) );
        time_parent = computeExclusiveValueOfParent( input, parent );
        time_call   = N_t_call * avg_t_call;
    }
    else
    {
        time_call = input->get_sev( met_t,
                                    CUBE_CALCULATE_INCLUSIVE,
                                    callnode,
                                    CUBE_CALCULATE_INCLUSIVE );

        time_parent = input->get_sev( met_t,
                                      CUBE_CALCULATE_EXCLUSIVE,
                                      parent,
                                      CUBE_CALCULATE_EXCLUSIVE );

    }
    cout << "Call node: " << c_name << "    Inclusive Time " << time_call << endl;
    cout << "Parent node: " << p_name << "   Exclusive Time " << time_parent << endl;


    //check if influenced node is in already parent list
    if (time_call > time_parent )
        return  c_name;
    else
        return string();
}

double
computeExclusiveValueOfParent( Cube*  input,
                               Cnode* parent )
{
    Metric* met_t   = input->get_met( "time" );
    size_t no_child = parent->num_children();

    Value* met_tau_time_incl_parent = input->get_sev_adv( met_t,
                                                          CUBE_CALCULATE_INCLUSIVE,
                                                          parent,
                                                          CUBE_CALCULATE_EXCLUSIVE );
    TauAtomicValue* tau_tuple_parent = ( TauAtomicValue* ) met_tau_time_incl_parent;
    double t_parent                  = tau_tuple_parent->getSum().getDouble();
    double t_cnodes( 0.0 );
    for( int i = 0; i < no_child; ++i )
    {
        Cnode* cnode                     = parent->get_child( i );
        Value *met_tau_time_incl_cnode   = input->get_sev_adv( met_t,
                                                             CUBE_CALCULATE_INCLUSIVE,
                                                             cnode,
                                                             CUBE_CALCULATE_EXCLUSIVE );
        TauAtomicValue* tau_tuple_t_call = ( TauAtomicValue* ) met_tau_time_incl_cnode;
        t_cnodes += tau_tuple_t_call->getSum().getDouble();

    }

    return t_parent - t_cnodes;
}

/**
 *
 * @param sig_nodes
 * @param phasenode
 * @param callnode
 * @return
 */
bool
checkAllParents( vector< string >& sig_nodes,
                 Cnode*            phasenode,
                 Cnode*            callnode )
{
    Cnode* parent = callnode->get_parent();

    if( callnode == phasenode )
        return true;

    if( !checkExistingName( parent->get_callee()->get_name(), sig_nodes ) )
        checkAllParents( sig_nodes, phasenode, parent );
    else
        return false;
}
/**
 * @brief Returns all the parents of visited node
 * @param to
 * @param size
 * @param a_m
 * @param regions
 * @param parent_names
 * @return
 */
vector< string >
getParentRegName( size_t                       to,
                  size_t                       size,
                  size_t**                     a_m,
                  const map< size_t, string >& regions,
                  const vector< string >&      parent_names )
{
    vector< string > p_names;
    for( size_t from = 0; from < size; from++ )
    {
        if( a_m[ from ][ to ] == 1 )
        {
            string name = getRegionName( from, regions );
            if( parent_names.empty() )
                p_names.push_back( name );
            else if ( !checkExistingName( name, parent_names )  )
                p_names.push_back( name );
        }
    }

    return p_names;
}

/**
 * Print the adjacency matrix
 * @param n
 * @param adjMatrix
 */
void
printMatrix( size_t   n,
             size_t** adjMatrix )
{
    cout << '\n';
    for( size_t i = 0; i < n; i++ )
    {
        for( size_t j = 0; j < n; j++ )
            cout << adjMatrix[ i ][ j ] << " ";
        cout << endl;
    }
}

/**
 * Get the indices of significant region from map
 * @param node_name
 * @param regions
 * @return
 */
size_t
getIndex( const string&                node_name,
          const map< size_t, string >& regions )
{
    for( const auto& region : regions )
    {
        string sig_r_name = region.second;
        if( strcmp( node_name.c_str(), sig_r_name.c_str() ) == 0 )
            return region.first;
    }

    return -1;
}

/**
 * Returns the region name of respected call sub tree
 * @param idx
 * @param regions
 * @return
 */
string
getRegionName( const size_t                 idx,
               const map< size_t, string >& regions )
{
    for( const auto& region : regions )
    {
        if( idx == region.first )
            return region.second;
    }

    return NULL;
}

/**
 * Reconstruct the adjacency matrix by removing arch from parent to child, which are already in the significant list
 * @param size
 * @param adjMatrix
 * @param phase_node
 * @param candidateNodes
 * @param list_cnames
 */
void
reconstructMatrix( size_t                 size,
                   size_t**&              adjMatrix,
                   Cnode*                 phase_node,
                   vector< string >&      candidateNodes,
                   map< size_t, string >& list_cnames )
{
    if( candidateNodes.empty() )
        return;

    freeMatrix( size, adjMatrix );
    list_cnames.clear();

    //create the mapping
    size_t cname_cnt;
    for( const auto& node : candidateNodes )
    {
        cname_cnt = list_cnames.size();
        list_cnames.insert( make_pair( cname_cnt, node ) );
    }
    cname_cnt += 1;

    adjMatrix = generateAdjacencyMatrix( cname_cnt, phase_node, list_cnames );
}

/**
 * Deallocate the adjacency matrix
 * @param n
 * @param adjMatrix
 */
void
freeMatrix( size_t   n,
            size_t** adjMatrix )
{
    for( size_t i = 0; i< n; ++i )
    {
        delete[] adjMatrix[ i ];
    }
    delete[] adjMatrix;
}
