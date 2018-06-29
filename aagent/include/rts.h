/**
   @file    aagent/include/rts.h
   @ingroup AnalysisAgent
   @brief   Rts call-tree node header
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

#ifndef RTS_H_
#define RTS_H_

#include "Parameter.h"
#include "SCOREP_OA_ReturnTypes.h"
#include <string>
#include <map>
#include <unordered_map>
#include <list>
#include <vector>
#include <set>
#include <stdint.h>
#include "Region.h"
#include "../../autotune/datamodel/include/TuningParameter.h"

/**
 * Call-tree node types
 */

enum RtsNodeType {
    NODE_REGULAR_REGION,
    NODE_PARAMETER
};

/**
 *Represents the settings of the tuning parameter's value and the objective value.
 */

struct TuningResult {
    std::map< TuningParameter*, int >                              tuning_parameter_value_mapping;
    double                                                         objective_value;
    std::map< std::string, double >                                objectives;
    std::map< std::string, double >                                extraInfo;
    std::unordered_map< std::string, std::pair< double, double > > phase_identifier_range;
    std::set< unsigned int >                                       phases_in_cluster;
};

enum ConfigType {
    STATIC_WORST,
    STATIC_BEST,
    RTS_BEST,
    STATIC_NORMALIZED_BEST,
    STATIC_NORMALIZED_WORST,
    RTS_NORMALIZED_BEST,
    TUNING_RESULT_STATIC,
    TUNING_RESULT_RTS
};

/**
 * @class Rts
 * @ingroup AnalysisAgent
 * @brief Identifies a runtime situation of an instrumented code region in the application.
 * Contains methods to initialize and construct the call-tree.
 */

class Rts {
private:
    Rts *                      parent;
    std::vector<Rts*>          children;

    int                        scorep_id;
    int                        parent_scorep_id;

    RtsNodeType                rts_type;

    Region*                    reg;
    mutable std::vector<Parameter_t*>  parameter;

    std::string                callpathstring;

    /* Name of the rts region as returned by Score-P. This is different from rts->reg->getName(), which returns
     * the parent region for a parameter rts. The parent region propagation occurs only in PTF. */
    std::string                region_name;

    /* Unique rts id-starts from 1 and incremented by 1 for every new rts that is created */
    int                        aa_rts_id;

    bool                       isValidRts;

public:
    /* Represents per-plugin tuning results. It is a mapping of the tuning plugin name to the tuning result obtained for it. */
    std::map<std::string, std::map< ConfigType, TuningResult* > > tuning_plugin_result;   //bool=1 means best static configuration

    TuningResult*              default_objective;

protected:

    /* Add node information to each call-tree node */
    void addNodeInfo( Rts* node );

    /* Generates a call-path string for each call-tree node */
    std::string makeCallpathString( std::vector< std::string >& callpath_strings );

    /* Check if the node already exists as the child of the calling node in the frontend call-tree */
    Rts* findFrontendNode( Rts* current );

    /* Extract parameter name and value from the region name of the node */
    void extract_parameter();

    /* Check if the node already exists as the child of the calling node in the call-tree */
    Rts* findNode( Rts* newnode );

public:

    Rts( SCOREP_OA_CallTreeDef*, Region* rtsreg );

    Rts(): scorep_id( 0 ), parent_scorep_id( 0 ), parent( NULL ), reg( NULL ), aa_rts_id( 0 ), isValidRts( false ), default_objective( NULL ) {
    }

    /* Generate a unique aagent rts ID for each rts  */
    int generateRtsID();

    void reassignScorepID( Rts* oldnode,
                           Rts* newnode );

    /* Construct the call-tree in the analysis agent */
    static void construct_aagent_calltree( SCOREP_OA_CallTreeDef*                                        buffer_casted,
                                           int                                                           callpath_buffer_size,
                                           std::map< uint64_t, std::map< uint32_t, std::list< int > > >* scorep_regionid_mapping );

    /* Recursive insertion of node into the call-tree in the aagent */
    void insertNode( Rts*                                                          parent,
                     SCOREP_OA_CallTreeDef*                                        buffer_data,
                     int                                                           buffer_size,
                     std::map< uint64_t, std::map< uint32_t, std::list< int > > >* scorep_region_id_mappings );


    /* Recursive generation of the mapping of the nodes with their call-path strings */
    void setCallPaths( Rts*                           node,
                       std::vector< std::string >&    callpath_strings,
                       std::map< Rts*, std::string >& rtscallpath_mapping );

    void setValidRtss( Rts* node );

    /* Set the regular region and the last parameter node as valid rts' */
    void setValidRts( Rts* node );

    /* Serializes call-tree nodes to send to the frontend */
    std::string toXMLdata();

    /* Frontend deserialization of the received serialized rts */
    static Rts* fromXMLdata( std::string& calltreeData );

    /* Recursive insertion of node into the call-tree at the front-end */
    void insertFrontendNode( Rts* parent, Rts* current );

    /* Returns true if the node has a parameter child */
    bool hasParameterChild();

    /* Recursive insertion of the per plugin tuning results for each node */
    void insertPluginResult( const std::string                  plugin_name,
                             const double                       obj_value,
                             std::map< TuningParameter*, int >& tuning_parameter_value_mapping,
                             std::map< std::string, double >&   objectives,
                             std::map< std::string, double >&   extraInfo,
                             ConfigType                         config );

    void insertPluginResult( const std::string                                                 plugin_name,
                             const double                                                      obj_value,
                             std::map< TuningParameter*, int >&                                tuning_parameter_value_mapping,
                             std::map< std::string, double >&                                  objectives,
                             std::map< std::string, double >&                                  extraInfo,
                             ConfigType                                                        config,
                             const std::unordered_map<std::string, std::pair<double,double> >& phase_identifier_range,
                             const std::set<unsigned int>&                                     phases_in_cluster );

    /* Setting of the tuning parameters and their values along with the objective value */
    TuningResult* insertTuningResult( const double                       obj_value,
                                      std::map< TuningParameter*, int >& tuning_param_value_mapping,
                                      std::map< std::string, double >&   objectives,
                                      std::map< std::string, double >&   extraInfo );

    TuningResult* insertTuningResult( const double                                                      obj_value,
                                      std::map<TuningParameter*, int>&                                  tuning_param_value_mapping,
                                      std::map<std::string, double>&                                    objectives,
                                      std::map<std::string, double>&                                    extraInfo,
                                      const std::unordered_map<std::string, std::pair<double,double> >& phase_identifier_range,
                                      const std::set<unsigned int>&                                     phases_in_cluster );

    Rts* getRtsByCallpath(std::string callpath);

    bool ValidRts() const {
        return isValidRts;
    }

    std::string getName() {
        return region_name;
    }

    std::vector<Rts*> getChildren() const {
        return this->children;
    }

    const int getParentScorepID() const {
        return parent_scorep_id;
    }

    const int getScorepID() const {
        return scorep_id;
    }

    const RtsNodeType getRtsType() const {
        return rts_type;
    }

    Region* getRegion() const {
        return reg;
    }

    std::vector<Parameter_t*> getParameter() const {
        return parameter;
    }

    const std::map< std::string, std::map< ConfigType, TuningResult* > >& getTuningResults() const {
        return tuning_plugin_result;
    }

    std::string & getCallPath() {
        return callpathstring;
    }

    const Rts* getParent() const {
        return parent;
    }

    const int getRtsID() const {
        return aa_rts_id;
    }

    const std::string RtsRegionName() const {
        return region_name;
    }

    /* Generates a list of rts for a region */
    list< Rts* > getRtsOfRegion( Region*       reg,
                                 list< Rts* >* rtsList );

    /* Generates the list of valid rts's of a region */
    list< Rts* > getValidRtsOfRegion( Region*       reg,
                                      list< Rts* >* rtsList);

    void delete_aagent_tree( Rts* node );

    void insertDefaultObjValue( std::string obj,
                                double&     obj_value );

    double getDefaultEnergy();

    double getDefaultNormalizedEnergy();

    void copyData( Rts* node, Rts* clone, Rts* parent, unsigned int& curr_cluster, unsigned int& next_cluster );

    void modifyNode( Rts* newClusterNode, Rts* node, unsigned int& curr_cluster );

//    double getDefaultEnergy() {
//        return default_energy->objective_value;
//    }

    void setCallPaths( Rts* node, std::vector<std::string>& callpath_strings );

    /* Insert default TP values */
    void insertDefaultTPValues( std::vector<TuningParameter*>& tuningParameters );

    ~Rts() {
        while( !parameter.empty() ) {
            delete parameter.back();
            parameter.pop_back();
        }
        reg = NULL;
//        if( parameter )
//        {
//            delete parameter;
//        }
    }
};


namespace rtstree {

    void printTree( Rts*                           node,
                    std::map< Rts*, std::string >& rtscallpath_mapping );
    void printTree( Rts* node);
    void assignNewData( Rts* node, unsigned int& cluster_num );

    std::map< Rts*, std::string >& getRtsCallpathMapping();
    std::map< int, Rts* > get_scorepid_to_rts_mapping();
    void clear_scorepid_to_rts_mapping();
    std::vector< Rts* > get_rts_to_serialize();
    void clear_rts_to_serialize();
    void modifyTree( Rts* node, unsigned int num_clusters );

    void cloneTree(Rts* parent, Rts* node, unsigned int& curr_cluster, unsigned int& next_cluster);
    std::string modifyString(Rts* node, std::string& target, unsigned int& cluster_num);

    void insertDefaultEnergy( Rts* node );

    /* Display percentage of energy savings */
    void displaySavings();

    /* Display true percentage of energy savings after applying the best configurations for the rts's */
    void displayTrueSavings();
}

#endif /* RTS_H_ */
