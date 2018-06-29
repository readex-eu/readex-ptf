/**
   @file    rts.cc
   @ingroup AnalysisAgent
   @brief   Rts call-tree implementation
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


#include "global.h"
#include "rts.h"

#include <string>
#include <stdio.h>
#include <map>
#include <list>
#include <algorithm>
#include <iostream>
#include <vector>
#include <stdint.h>
#include <regex>
#include <utility>

#include <boost/serialization/vector.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional.hpp>
#include <boost/regex.hpp>
#include "string_helper.h"
#include "psc_errmsg.h"
#include "xml_psc_tags.h"
#include "TuningParameter.h"

using namespace rtstree;
Rts*       root;
static int buffer_index;
int        last_aa_rts_id = 0;

/* Mapping of the call-tree node to its call-path string in the aagent */
std::map<Rts*, std::string > aa_rts_callpath_mapping;

/* Mapping of the score-p id of the rts call-tree node to the pointer to the corresponding rts in the aagent */
std::map<int, Rts*> scorepid_to_rts_mapping;

std::string RtsNodeTypetoString[] = { "NODE_REGULAR_REGION", "NODE_PARAMETER" };
std::string ParameterNodeTypetoString[] = { "NODE_PARAMETER_STRING", "NODE_PARAMETER_INTEGER" };

/* Vector containing the rtss that must be serialized from the aagent to the frontend */
std::vector<Rts*> rts_to_serialize;


Rts::Rts( SCOREP_OA_CallTreeDef* buffer_data, Region* rtsreg ): parent( NULL ), aa_rts_id( 0 ), default_objective( NULL ) {
    scorep_id        = buffer_data->scorep_id;
    parent_scorep_id = buffer_data->parent_scorep_id;
    callpathstring   = "";
    reg              = rtsreg;
    region_name      = buffer_data->name;
    isValidRts       = false;
}

std::map<Rts*, std::string>& rtstree::getRtsCallpathMapping() {
    return aa_rts_callpath_mapping;
}

std::map<int, Rts*> rtstree::get_scorepid_to_rts_mapping() {
    return scorepid_to_rts_mapping;
}

std::vector<Rts*> rtstree::get_rts_to_serialize() {
    return rts_to_serialize;
}

void rtstree::clear_rts_to_serialize() {
    rts_to_serialize.clear();
}

void rtstree::clear_scorepid_to_rts_mapping() {
    scorepid_to_rts_mapping.clear();
}


int Rts::generateRtsID() {
    ++last_aa_rts_id;
    return last_aa_rts_id;
}


/**
 *@brief Construct the call-tree in the analysis agent
 *@param buffer_casted  Call-tree definitions buffer containing the new incoming node that must be added to the tree
 *@param callpath_buffer_size  Size of the call-tree definitions buffer
 *@param scorep_regionid_mapping  Mapping of PTF generated region ID to the map of Score-P region id mapping
 *       to a list of processes on which the id was recorded
 */
void Rts::construct_aagent_calltree( SCOREP_OA_CallTreeDef* buffer_casted, int callpath_buffer_size,
                                     std::map<uint64_t, std::map<uint32_t, std::list<int> > > *scorep_regionid_mapping ) {

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( RtsInfo ), "Rts: call to construct_aagent_tree() \n" );
    uint64_t                                                                 ptf_id;
    std::map<uint64_t, std::map<uint32_t, std::list<int> > >::const_iterator it;
    std::map<uint32_t, std::list<int> >::const_iterator                      it2;

    for( it = scorep_regionid_mapping->begin(); it != scorep_regionid_mapping->end(); ++it ) {
        for( it2 = it->second.begin(); it2 != it->second.end(); ++it2 ) {
            if( it2->first == buffer_casted[0].region_id ) {
                ptf_id = it->first;
            }
        }
    }

    Region* const rtsRegion = appl->getRegionByKey( ptf_id );

    Rts*          current   = new Rts( &buffer_casted[0], rtsRegion );
    if( current ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( RtsInfo ), "Rts: Assigned root node of the call-tree \n" );
        if( !root ) {
            root = current;
            appl->setCalltreeRoot(root);
            root->aa_rts_id = root->generateRtsID();
            rts_to_serialize.push_back( root );
        }
        else {
            root->reassignScorepID( root, current );
        }

        buffer_index = 1;          //1st node processed. It is always the root.

        scorepid_to_rts_mapping.insert( std::make_pair( root->scorep_id, root ) );

        root->insertNode( root, buffer_casted, callpath_buffer_size, scorep_regionid_mapping );

        root->extract_parameter();

        root->addNodeInfo( root );

        //Print the aagent call-tree
        //psc_dbgmsg( 8, "Printing rts tree in aagent \n");
        //printTree( root );
    }
}


void Rts::reassignScorepID( Rts* returnval, Rts* newnode ) {
    returnval->scorep_id        = newnode->scorep_id;
    returnval->parent_scorep_id = newnode->parent_scorep_id;
}


/** Add information to the rts node, such as the call-path string, parameter propagation and setting the isValidFlag **/
void Rts::addNodeInfo( Rts* node ) {
    std::vector<std::string> callpath_strings;
    setCallPaths( node, callpath_strings, rtstree::getRtsCallpathMapping() );
}


/**
 *@brief Check if the node already exists as the child of the caller node in the call-tree. TODO: Perform this when we request for new metrics from Score-P.
 *We compare each child's PTF generated region id with PTF generated id of the new node and assign the new score-p
 *region id to the found node
 *@param newnode Node to search for
 *@return The node, if found. Otherwise, NULL
 */
Rts* Rts::findNode( Rts* newnode ) {
    for( int i = 0; i < children.size(); i++ ) {
//        if( children[i]->reg->getLocalRegionID() == newnode->reg->getLocalRegionID() ) {
        if( (children[i]->getScorepID() == newnode->getScorepID()) || (children[i]->region_name == newnode->region_name) ) {
            return children[i];
        }
    }
    return NULL;
}


/**
 *@brief Recursive insertion of node into the call-tree in the aagent
 *@param node  Node to which the child node must be added
 *@param buffer_data  Call-tree definitions buffer containing the new incoming node that must be added to the tree
 *@param callpath_buffer_size  Size of the call-tree definitions buffer
 *@param scorep_regionid_mapping  Mapping of PTF generated region ID to the map of Score-P region id mapping
 *       to a list of processes on which the id was recorded
 */
void Rts::insertNode( Rts* node, SCOREP_OA_CallTreeDef* buffer_data, int callpath_buffer_size,
                      std::map<uint64_t, std::map<uint32_t, std::list<int> > > *scorep_regionid_mapping ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( RtsInfo ), "Rts: calling insertNode() \n" );
    if( buffer_index >= callpath_buffer_size )
        return;

    /* If we have multiple threads, the buffer contains the results from all the threads.
     * After each thread's records, the next thread's records immediately start.
     * So, when the next thread's phase region is encountered, start the tree creation from the phase region.
     * Else, the recursion goes till the root without a matching parent node and crashes. */
    if( buffer_data[buffer_index].parent_scorep_id == 0 ) {
        root->scorep_id        = buffer_data[buffer_index].scorep_id;
        root->parent_scorep_id = buffer_data[buffer_index].parent_scorep_id;
        buffer_index          += 1;
        insertNode( root, buffer_data, callpath_buffer_size, scorep_regionid_mapping );
    }

    else if( buffer_data[buffer_index].parent_scorep_id == node->scorep_id ) {

        uint64_t                                                                 ptf_id;
        std::map<uint64_t, std::map<uint32_t, std::list<int> > >::const_iterator ptf_it;
        std::map<uint32_t, std::list<int> >::const_iterator                      scorep_map_it;

        for( ptf_it = scorep_regionid_mapping->begin(); ptf_it != scorep_regionid_mapping->end(); ++ptf_it ) {
            for( scorep_map_it = ptf_it->second.begin(); scorep_map_it != ptf_it->second.end(); ++scorep_map_it ) {
                if( scorep_map_it->first == buffer_data[buffer_index].region_id ) {
                    ptf_id = ptf_it->first;
                }
            }
        }

        Region* const rtsRegion = appl->getRegionByKey( ptf_id );
        Rts *         newnode   = new Rts( &buffer_data[buffer_index], rtsRegion  );

        //Check if the node already exists in the call-tree
        Rts*          returnval = node->findNode( newnode );
        if( returnval ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( RtsInfo ), "Rts: Node already exists! Reassigning node IDs \n" );
            //Assign the new score-p id to the found node
            reassignScorepID( returnval, newnode );
            scorepid_to_rts_mapping.insert( std::make_pair( returnval->scorep_id, returnval ) );
            buffer_index++;
            insertNode( returnval, buffer_data, callpath_buffer_size, scorep_regionid_mapping );
        }
        else {

            node->children.push_back( newnode );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( RtsInfo ), "Rts: Added rts to the tree \n" );

            newnode->parent = node;
            newnode->aa_rts_id = newnode->generateRtsID();

            newnode->extract_parameter();

            scorepid_to_rts_mapping.insert( std::make_pair( newnode->scorep_id, newnode ) );
            rts_to_serialize.push_back( newnode );

            buffer_index++;
            insertNode( newnode, buffer_data, callpath_buffer_size, scorep_regionid_mapping );
        }
    }
    else {
        insertNode( node->parent, buffer_data, callpath_buffer_size, scorep_regionid_mapping );
    }
}


/**
 *@brief Extract the parameter name and value from the region name and assign the Parameter_t struct
 */
void Rts::extract_parameter() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( RtsInfo ), "Rts: call to extract_parameter() \n" );
    std::string reg_name = region_name;
    std::size_t pos      = reg_name.find('=');

    /* Region name is in the format:name=value. So, find the position of "=" in string
     * Some regions(overloaded operators) may have ==, /=, *=, +=, -= in their name. Don't accidentally mark it as a parameter
     * Also, don't mark regions with " = " or an operator overloading function with the name "operator"
     */
    if( pos != std::string::npos && !std::regex_search(reg_name,std::regex("(\\s+=)|(=\\s+)|(={2,})|(=$)"))
    && !std::regex_search(reg_name,std::regex("operator(\\s)*((\\\\)|(!)|(-)|(\\+)|(\\*)|(/))*=")) ){
        std::string param_name  = reg_name.substr(0,pos);
        std::string param_value = reg_name.substr(pos+1);
        rts_type = NODE_PARAMETER;

        Parameter_t* user_parameter = new Parameter_t;
        user_parameter->param_name  = param_name;
        user_parameter->param_value = param_value;

        if( std::find_if( param_value.begin(), param_value.end(), (int (*)(int)) std::isalpha  ) != param_value.end() ) {
            user_parameter->param_type = NODE_PARAMETER_STRING;
        }
        else {
            user_parameter->param_type = NODE_PARAMETER_INTEGER;
        }

        parameter.push_back(user_parameter);
        // The parameter's region is now the same as its parent node's region
        reg = parent->reg;
    }
    else {
        rts_type = NODE_REGULAR_REGION;
    }
}

double Rts::getDefaultEnergy() {
    if(default_objective != nullptr)
        return default_objective->objective_value;
    else return 0.0;
}


/**
 *@brief Print the entire rts tree
 *@param node  Node to print
 */
void rtstree::printTree( Rts * node ) {
    cout << "call-path: " << node->getCallPath() <<endl;
    cout << "\t aa_rts_id: " << node->getRtsID() <<endl;
    cout << "\t a_ptf_region_id: "<< node->getRegion()->getRegionID() <<endl;
    cout << "\t isValidRts: " << node->ValidRts()  << endl;
    cout << "\t score-p id: " << node->getScorepID() <<endl;
    cout << "\t parent score-p id: " << node->getParentScorepID()  << endl;
    cout << "\t score-p region name: " << node->getName() << endl;
    cout << "\t parent region: " << node->getRegion()->get_name() << endl;
    cout << "\t node type: " << RtsNodeTypetoString[node->getRtsType()] << endl;

    if( !node->getParameter().empty() ) {
        std::vector<Parameter_t*> param_list = node->getParameter();
        for( std::vector<Parameter_t*>::const_iterator iter = param_list.begin(); iter != param_list.end(); iter++ ) {
            cout << "\t parameter name = " << (*iter)->param_name << "\t parameter value = " << (*iter)->param_value
                 << "\t parameter type = " << (*iter)->param_type << endl;
       }
    }

    cout << "\t default energy: " << node->getDefaultEnergy() << endl;

    if( !( node->tuning_plugin_result.empty() ) ) {
         std::map<std::string, std::map<ConfigType, TuningResult* > >::const_iterator pluginResult_iter = node->tuning_plugin_result.begin();
         std::map<ConfigType, TuningResult*>::const_iterator        config;

         for( ; pluginResult_iter != node->tuning_plugin_result.end(); pluginResult_iter++ ) {
             cout << "\t plugin: " << pluginResult_iter->first << endl;

             for(config=pluginResult_iter->second.begin(); config != pluginResult_iter->second.end(); config++ ) {
                 cout << "\t\t configuration:" << endl;
                 cout << "\t\t\t type: " << config->first << endl;
                 for(auto &tp : config->second->tuning_parameter_value_mapping ) {
                     cout << "\t\t\t " << tp.first->getName() <<":\t" << tp.second << endl;
                 }

                 cout << "\t\t\t objectives:" << endl;

                 cout << "\t\t\t\t objective value = " << std::setprecision(12) << std::setw(14) << config->second->objective_value << endl;

                 for (auto &objective : config->second->objectives ){
                     cout << "\t\t\t\t " << objective.first <<":\t" << std::setprecision(12) << std::setw(14) << (double) objective.second << endl;
                 }

                 cout << "\t\t\t extra info:" << endl;
                 for (auto &info : config->second->extraInfo ){
                     cout << "\t\t\t\t " << info.first <<":\t" << std::setprecision(12) << std::setw(14) << info.second << endl;
                 }

                 cout << "\t\t\t phase features range:" << endl;
                 for( auto &ph_r : config->second->phase_identifier_range) {
                     cout << "\t\t\t\t " << ph_r.first << ":\t" << ph_r.second.first << " - " << ph_r.second.second << endl;
                 }

                 cout << "\t\t\t phases in cluster:  ";
                 for( auto &ph_c : config->second->phases_in_cluster) {
                     cout << ph_c << "  ";
                 }
                 cout << endl;
             }
         }
     }
     cout << endl << endl;
     for( int i = 0; i < node->getChildren().size(); i++ ) {
         printTree( node->getChildren()[i]);
     }
}


/**
 *@brief Serializes individual rts' to send to the frontend
 *@return The stringstream of a serialized rts
 */
std::string Rts::toXMLdata() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( RtsInfo ), "Rts: call to toXMLdata(): serializing node data to XML string \n" );
    std::stringstream xmlData;

    //Replace whitespace, <, &, and > with &ws;, &lt;, &amp;, and &gt;
    std::vector< boost::regex > reg_expr;
    std::vector< std::string >  format;
    boost::regex                reg_expr1( PSC_XML_WS );
    boost::regex                reg_expr2( PSC_XML_LT );
    boost::regex                reg_expr3( PSC_XML_AMP );
    boost::regex                reg_expr4( PSC_XML_GT );

    reg_expr.push_back( reg_expr1 );
    reg_expr.push_back( reg_expr2 );
    reg_expr.push_back( reg_expr3 );
    reg_expr.push_back( reg_expr4 );
    format.push_back( XML_PSC_WS );
    format.push_back( XML_PSC_LT );
    format.push_back( XML_PSC_AMP );
    format.push_back( XML_PSC_GT );

    // Start of a call-tree node
    // <Node>
    xmlData << "<" << XML_PSC_NODE_TAG << ">" << std::endl;

    // Node's scorepID
    xmlData << "\t<" << XML_PSC_NODE_SCOREPID_TAG << ">" << scorep_id << "</"
            << XML_PSC_NODE_SCOREPID_TAG << ">" << std::endl;

    // Node's parent scorepid
    xmlData << "\t<" << XML_PSC_NODE_PARENT_SCOREPID_TAG << ">" << parent_scorep_id << "</"
            << XML_PSC_NODE_PARENT_SCOREPID_TAG << ">" << std::endl;

    std::string aa_region_id = this->reg->getRegionID();
    aa_region_id             = formatString( aa_region_id, reg_expr, format );

    std::string region_name  = this->reg->get_name();
    region_name              = formatString( region_name, reg_expr, format );
    // Aagent region name
    xmlData << "\t<" << XML_PSC_PROP_CONTEXT_REGION_NAME_TAG << ">" << region_name << "</"
            << XML_PSC_PROP_CONTEXT_REGION_NAME_TAG << ">" << std::endl;

    // Aagent id in string
    xmlData << "\t<" << XML_PSC_NODE_AAREGIONID_STRING_TAG << ">" << aa_region_id << "</"
            << XML_PSC_NODE_AAREGIONID_STRING_TAG << ">" << std::endl;

    // Aagent region file name
    xmlData << "\t<" <<  XML_PSC_PROP_CONTEXT_FILENAME_TAG << ">" << this->reg->getFileName() << "</"
            << XML_PSC_PROP_CONTEXT_FILENAME_TAG << ">" << std::endl;

    // Aagent region rfl
    xmlData << "\t<" << XML_PSC_PROP_CONTEXT_RFL_TAG << ">" << this->reg->getFirstLine() << "</"
            << XML_PSC_PROP_CONTEXT_RFL_TAG << ">" << std::endl;

    // Aagent region type
    xmlData << "\t<" << XML_PSC_PROP_CONTEXT_REGION_TYPE_TAG << ">" << static_cast<int>(this->reg->get_type()) << "</"
            << XML_PSC_PROP_CONTEXT_REGION_TYPE_TAG << ">" << std::endl;

    // RtsNodeType
    xmlData << "\t<" << XML_PSC_NODE_TYPE_TAG << ">" << static_cast<int>( rts_type ) << "</"
            << XML_PSC_NODE_TYPE_TAG << ">" << std::endl;

    std::string callpath_string = callpathstring;
    callpath_string             = formatString( callpath_string, reg_expr, format );

    // Callpath String
    xmlData << "\t<" << XML_PSC_NODE_CALLPATH_TAG << ">" << callpath_string << "</"
            << XML_PSC_NODE_CALLPATH_TAG << ">" << std::endl;

    std::string reg_name       = this->region_name;
    reg_name                   = formatString( reg_name, reg_expr, format );
    // Region Name
    xmlData << "\t<" << XML_PSC_NODE_REGIONNAME_TAG << ">" << reg_name << "</"
            << XML_PSC_NODE_REGIONNAME_TAG << ">" << std::endl;

    if( RtsNodeTypetoString[rts_type].find("PARAMETER") != std::string::npos ) {
        // <Parameters>
        xmlData << "\t<" << XML_PSC_CALLTREE_PARAMETERS_TAG << ">" << std::endl;

        std::vector<Parameter_t*>::iterator param_it;
        for( param_it = parameter.begin(); param_it != parameter.end(); param_it++ ) {

            // <Parameter>
            xmlData << "\t<" << XML_PSC_CALLTREE_PARAMETER_TAG << ">" << std::endl;

            std::string parameter_name  = (*param_it)->param_name;
            parameter_name              = formatString( parameter_name, reg_expr, format );
            std::string parameter_value = (*param_it)->param_value;
            parameter_value             = formatString( parameter_value, reg_expr, format );
            // Parameter name
            xmlData << "\t\t<" << XML_PSC_CALLTREE_PARAMETER_NAME_TAG << ">" << parameter_name << "</"
                    << XML_PSC_CALLTREE_PARAMETER_NAME_TAG << ">" << std::endl;

            // Parameter value
            xmlData << "\t\t<" << XML_PSC_CALLTREE_PARAMETER_VALUE_TAG << ">" << parameter_value << "</"
                    << XML_PSC_CALLTREE_PARAMETER_VALUE_TAG << ">" << std::endl;

            // ParameterNodeType
            xmlData << "\t<" << XML_PSC_NODE_PARAMETER_TYPE_TAG << ">" << static_cast<int>( (*param_it)->param_type ) << "</"
                    << XML_PSC_NODE_PARAMETER_TYPE_TAG << ">" << std::endl;

            // </Parameter>
            xmlData << "\t</" << XML_PSC_CALLTREE_PARAMETER_TAG << ">" << std::endl;
        }
        // </Parameters>
        xmlData << "\t</" << XML_PSC_CALLTREE_PARAMETERS_TAG << ">" << std::endl;
    }

    // </Node>
    xmlData << "</" << XML_PSC_NODE_TAG << ">" << std::endl;
    // End of a call-tree node

    return xmlData.str();
}


/**
 *@brief Frontend deserialization of the received serialized rts information
 *@param calltreeData The stringstream of a serialized rts
 *@return The deserialized rts
 */
Rts* Rts::fromXMLdata( std::string& calltreeData ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( RtsInfo ), "Rts: call to fromXMLdata(): deserializing XML data to node \n" );
    try {
        using boost::property_tree::ptree;

        ptree                     tree;
        std::istringstream        stream( calltreeData );

        //Replace &ws;, &lt;, &amp;, and &gt; with whitespace, <, &, and >
        std::vector< boost::regex > reg_expr;
        std::vector< std::string >  format;
        boost::regex                reg_expr1( XML_PSC_WS );
        boost::regex                reg_expr2( XML_PSC_LT );
        boost::regex                reg_expr3( XML_PSC_AMP );
        boost::regex                reg_expr4( XML_PSC_GT );

        reg_expr.push_back( reg_expr1 );
        reg_expr.push_back( reg_expr2 );
        reg_expr.push_back( reg_expr3 );
        reg_expr.push_back( reg_expr4 );
        format.push_back( PSC_XML_WS );
        format.push_back( PSC_XML_LT );
        format.push_back( PSC_XML_AMP );
        format.push_back( PSC_XML_GT );

        // parse the XML
        read_xml( stream, tree );

        // read the node
        const ptree& calltree_node        = tree.get_child( XML_PSC_NODE_TAG );

        int           scorep_id           = calltree_node.get<int>( XML_PSC_NODE_SCOREPID_TAG );
        int           parent_scorep_id    = calltree_node.get<int>( XML_PSC_NODE_PARENT_SCOREPID_TAG );

        //region name
        std::string   aaregion_name  = calltree_node.get<std::string>( XML_PSC_PROP_CONTEXT_REGION_NAME_TAG );
        aaregion_name                = formatString( aaregion_name, reg_expr, format );


        //regionid
        std::string   aaregion_id_string  = calltree_node.get<std::string>( XML_PSC_NODE_AAREGIONID_STRING_TAG );
        aaregion_id_string                = formatString( aaregion_id_string, reg_expr, format );

        //filename
        std::string   aaregion_filename   = calltree_node.get<std::string>( XML_PSC_PROP_CONTEXT_FILENAME_TAG );

        //rfl
        int           aaregion_rfl        = calltree_node.get<int>( XML_PSC_PROP_CONTEXT_RFL_TAG );

        //type
        RegionType   aaregion_type        = static_cast<RegionType>( calltree_node.get<int>( XML_PSC_PROP_CONTEXT_REGION_TYPE_TAG ) );

        appl->addRegion( aaregion_name, aaregion_rfl, aaregion_filename, aaregion_type, aaregion_rfl, aaregion_rfl );

        Region* const node_reg            = appl->getRegionByID( aaregion_id_string);

        RtsNodeType   rts_type            = static_cast<RtsNodeType>( calltree_node.get<int>( XML_PSC_NODE_TYPE_TAG ) );

        std::string   node_callpathstring = calltree_node.get<std::string>( XML_PSC_NODE_CALLPATH_TAG );
        node_callpathstring               = formatString( node_callpathstring, reg_expr, format );

        std::string   reg_name            = calltree_node.get<std::string>( XML_PSC_NODE_REGIONNAME_TAG );
        reg_name                          = formatString( reg_name, reg_expr, format );

        Rts*          node                = new Rts();
        node->scorep_id                   = scorep_id;
        node->parent_scorep_id            = parent_scorep_id;
        node->rts_type                    = rts_type;
        if( node_reg != NULL ) node->reg  = node_reg;
        node->callpathstring              = node_callpathstring;
        node->region_name                 = reg_name;

        if( calltree_node.get_child_optional( XML_PSC_CALLTREE_PARAMETERS_TAG ) ) {
            //        if( parameter_info ) {
            BOOST_FOREACH( const ptree::value_type& param_info, calltree_node.get_child( XML_PSC_CALLTREE_PARAMETERS_TAG ) ) {
                std::string       parameter_name  = param_info.second.get<std::string>( XML_PSC_CALLTREE_PARAMETER_NAME_TAG);
                std::string       parameter_value = param_info.second.get<std::string>( XML_PSC_CALLTREE_PARAMETER_VALUE_TAG);
                ParameterNodeType parameter_type  = static_cast<ParameterNodeType>( param_info.second.get<int>( XML_PSC_NODE_PARAMETER_TYPE_TAG ) );
                parameter_name                    = formatString( parameter_name, reg_expr, format );
                parameter_value                   = formatString( parameter_value, reg_expr, format );

                Parameter_t* user_parameter       = new Parameter_t;
                user_parameter->param_name        = parameter_name;
                user_parameter->param_value       = parameter_value;
                user_parameter->param_type        = parameter_type;
                node->parameter.push_back(user_parameter);
            }
        }
        return node;
    }
    catch( std::exception& ex ) {
        psc_errmsg( "Unable to parse the property XML: %s\n", ex.what() );
        abort();
    }
}


/**
 *@brief Check if the node already exists as the child of the caller node in the frontend call-tree
 *We compare each child's region id string with region id string of the new node and assign the new score-p region id to the found node
 *@param newnode Node to search for
 *@return The node, if found. Otherwise, NULL
 */
Rts* Rts::findFrontendNode( Rts* newnode ) {
    for( int i = 0; i < children.size(); i++ ) {
        if( ( children[i]->callpathstring.compare( newnode->callpathstring ) ) == 0 ) {
            return children[i];
        }
    }
    return NULL;
}


/**
 *@brief Recursive insertion of node into the call-tree in the frontend
 *@param parent Node to which the child node must be added
 *@param current The new incoming node that must be added to the tree
 */
void Rts::insertFrontendNode( Rts* parent, Rts* current ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( RtsInfo ), "Rts: call to insertFrontendNode(): inserting node in the frontend call-tree \n" );

    if( current->callpathstring.compare( ( parent->callpathstring ) +  "/" + current->region_name ) == 0 ) {
        //    if( current->scorep_id = parent->scorep_id ) {

        //Check if the node already exists in the call-tree
        Rts* foundnode = parent->findFrontendNode( current );

        if( foundnode ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( RtsInfo ), "Rts: Node with the same call-path already exists! \n" );
            return;
        }
        parent->children.push_back(current);
        current->parent = parent;
        return;
    }

    for( int i = 0; i < parent->children.size(); i++ ) {
        insertFrontendNode( parent->children[i], current );
    }
}


bool Rts::hasParameterChild() {
    for( int i = 0; i < children.size(); i++ ) {
        if( children[i]->rts_type != NODE_REGULAR_REGION )
            return true;
    }
    return false;
}


/**
 *@brief Generates a call-path string from the root node to each call-tree node.
 *Parameters that are not leaf nodes are ignored in the call-path string of a node
 *@param callpath_strings  Vector containing the region name of each rts region traversed
 *@return The string of the call-path of the node from the root
 */
std::string Rts::makeCallpathString( std::vector<std::string>& callpath_strings ) {
    std::string callpath_string = "";
    for( int i = 0; i < callpath_strings.size(); i++ ) {
            callpath_string += "/" + callpath_strings[i];
    }
    return callpath_string;
}


/**
 *@brief Recursive generation of the mapping of each node with its call-path string
 *@param node Rts to be mapped to its call-path string
 *@param callpath_strings Vector containing the region name of each rts region traversed
 *@param rtscallpath_mapping Mapping of the call-tree node to its call-path string
 */
void Rts::setCallPaths( Rts* node, std::vector<std::string>& callpath_strings, std::map<Rts*, std::string>& rtscallpath_mapping ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( RtsInfo ), "Rts: call to setCallPaths(): generating call-path string of the node \n" );
    if( !node ) {
        return;
    }

    callpath_strings.push_back( node->region_name );

    std::string test_callpathstring = makeCallpathString( callpath_strings );
    node->callpathstring            = test_callpathstring;

    rtscallpath_mapping.insert( std::pair<Rts*, std::string>( node, test_callpathstring ) );

    // If the node is a parameter node, insert the parameter information
    // Propagate the parameter list from the parameters above this node
    if( node->parent != NULL ) {
        if( ( node->parent->rts_type != NODE_REGULAR_REGION ) && ( node->rts_type != NODE_REGULAR_REGION ) ) {
            std::vector<Parameter_t*>::iterator param_it;
            for( param_it = node->parent->parameter.begin(); param_it != node->parent->parameter.end(); param_it++ ) {
                std::vector<Parameter_t*>::iterator it = std::find(node->parameter.begin(), node->parameter.end(), *param_it );
                if( it == node->parameter.end() ) {
                    node->parameter.insert( node->parameter.begin(), *param_it );
                }
            }
        }
    }

    if( node->children.size() == 0 ) {
        callpath_strings.pop_back();
    }
    setValidRts( node );

    int node_index = 0;
    int i          = 0;

    while( i < node->children.size() ) {
        setCallPaths( node->children[i], callpath_strings, rtscallpath_mapping );
        node_index = ++i;
        if( node_index == node->children.size() ) {

            if( callpath_strings.size() != 0 ) {
                callpath_strings.pop_back();
            }
            return;
        }
    }
}


void Rts::setValidRtss( Rts* node ) {
    if( !node ) {
        return;
    }
    setValidRts( node );
    for( int i = 0; i < node->children.size(); i++ ) {
        setValidRtss( node->children[i] );
    }
}


void Rts::setValidRts( Rts* node ) {
    // Set the node as a valid rts if it is a regular region or the last parameter node before a regular region
    if( node->children.size() == 0 || !( node->hasParameterChild() ) ) {
        node->isValidRts = true;
    }
    else
        node->isValidRts = false;
}

/**
 *@brief Recursive insertion of the per plugin tuning results for each node
 *@param plugin_name Name of the plugin
 *@param obj_value The measured value of the objective
 *@param tuning_parameter_value_mapping Mapping of the tuning parameters with their values
 *@param objectives Objectives
 *@param extraInfo extra info
 *@param config config
 */
void Rts::insertPluginResult( const std::string plugin_name, const double obj_value, std::map<TuningParameter*, int>& tuning_parameter_value_mapping,
            std::map<std::string, double>& objectives, std::map<std::string, double>& extraInfo, ConfigType config ) {
    if( !plugin_name.empty() ) {
        std::map< ConfigType, TuningResult* > tuning_result_config;
        tuning_result_config.insert(std::make_pair(config, insertTuningResult(obj_value, tuning_parameter_value_mapping, objectives, extraInfo)));
        if( tuning_plugin_result.empty() ) {
            tuning_plugin_result.insert(std::make_pair( plugin_name, tuning_result_config));
            return;
        }
        tuning_plugin_result[plugin_name].insert(tuning_result_config.begin(), tuning_result_config.end());
    }
}


void Rts::insertPluginResult( const std::string plugin_name, const double obj_value, std::map<TuningParameter*, int>& tuning_parameter_value_mapping,
        std::map<std::string, double>& objectives, std::map<std::string, double>& extraInfo, ConfigType config,
        const std::unordered_map<std::string, std::pair<double,double> >& phase_identifier_range, const std::set<unsigned int>& phases_in_cluster ) {
    if( !plugin_name.empty() ) {
        std::map< ConfigType, TuningResult* > tuning_result_config;
        tuning_result_config.insert(std::make_pair(config, insertTuningResult(obj_value, tuning_parameter_value_mapping, objectives, extraInfo,
                phase_identifier_range, phases_in_cluster)));
        if( tuning_plugin_result.empty() ) {
            tuning_plugin_result.insert(std::make_pair( plugin_name, tuning_result_config));
            return;
        }
        tuning_plugin_result[plugin_name].insert(tuning_result_config.begin(), tuning_result_config.end());
    }
}


/**
 *@brief Setting of the tuning parameters and their values along with the objective value
 *@param obj_value The measured value of the objective
 *@param tuning_param_value_mapping Mapping of the tuning parameters with their values
 *@param objectives Objectives
 *@param extraInfo extraInfo
 */
TuningResult* Rts::insertTuningResult( const double obj_value, std::map<TuningParameter*, int>& tuning_param_value_mapping,
            std::map<std::string, double>& objectives, std::map<std::string, double>& extraInfo) {
    TuningResult* tuning_result                   = new TuningResult;
    tuning_result->tuning_parameter_value_mapping = tuning_param_value_mapping;
//    if( this->getParentScorepID() != 0 ) {
    tuning_result->objectives                 = objectives;
//    if( this->getParentScorepID() != 0 ) {
        tuning_result->extraInfo                  = extraInfo;
//    }
    tuning_result->objective_value                = obj_value;
    return tuning_result;
}


TuningResult* Rts::insertTuningResult( const double obj_value, std::map<TuningParameter*, int>& tuning_param_value_mapping,
            std::map<std::string, double>& objectives, std::map<std::string, double>& extraInfo,
            const std::unordered_map<std::string, std::pair<double,double> >& phase_identifier_range, const std::set<unsigned int>& phases_in_cluster) {
    TuningResult* tuning_result                   = new TuningResult;
    tuning_result->tuning_parameter_value_mapping = tuning_param_value_mapping;
//    if( this->getParentScorepID() != 0 ) {
    tuning_result->objectives                     = objectives;
//    if( this->getParentScorepID() != 0 ) {
        tuning_result->extraInfo                  = extraInfo;
//    }
    tuning_result->objective_value                = obj_value;
    tuning_result->phase_identifier_range         = phase_identifier_range;
    tuning_result->phases_in_cluster              = phases_in_cluster;
    return tuning_result;
}


/**
 *@brief Generates a list of rts for a given region
 *@param search_reg Region for which the rts are determined
 *@param rtsList List of rts of that region
 */

list<Rts*> Rts::getRtsOfRegion( Region* search_reg, list<Rts*>* rtsList ){
    if( rtsList == NULL ){
        rtsList = new list<Rts*>;
    }

    if( reg == search_reg ){
        //add rts to the list
        rtsList->push_back(this);
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( RtsInfo ), "Rts: call to getRtsOfRegion() \n" );
    }

   for( int i = 0; i < children.size(); i++ ) {
        list<Rts*> rl = children[i]->getRtsOfRegion( search_reg, rtsList );
    }

   return *rtsList;
}


/**
 *@brief Generates a list of valid rts's for a given region
 *@param search_reg Region for which the rts are determined
 *@param rtsList List of valid rts's of that region
 */

list<Rts*> Rts::getValidRtsOfRegion( Region* search_reg, list<Rts*>* rtsList ){
    if( rtsList == NULL ){
        rtsList = new list<Rts*>;
    }

    if( isValidRts && reg == search_reg ){
        //add rts to the list
        rtsList->push_back(this);
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( RtsInfo ), "Rts: call to getValidRtsOfRegion() \n" );
    }

   for( int i = 0; i < children.size(); i++ ) {
        list<Rts*> rl = children[i]->getValidRtsOfRegion( search_reg, rtsList );
    }

   return *rtsList;
}


Rts* Rts::getRtsByCallpath( std::string callpath ) {
    Rts* rts;
    //printf("rts callpath: %s\n", callpathstring.c_str());fflush(stdout);
    if( callpathstring == callpath ) {
        psc_dbgmsg( 8, "Rts::getRtsByCallpath: rts for callpath <%s> found.\n", callpath.c_str() );
        return this;
    }
    else {
        for( int i = 0; i < children.size(); i++ ) {
            rts = children[i]->getRtsByCallpath(callpath);
            if(rts) {
                return rts;
            }
        }
    }
    if( this == appl->getCalltreeRoot()) psc_dbgmsg( 8, "Rts::getRtsByCallpath: no rts found for callpath <%s>.\n",callpath.c_str() );
    return NULL;
}


//void rtstree::insertDefaultEnergy(Rts* node) {
//    std::map<std::string,double> default_map = appl->getDefaultEnergyMapping();
//    for( std::map<std::string,double>::iterator default_it=default_map.begin(); default_it!=default_map.end();default_it++ ) {
//        Rts* rts_match = node->getRtsByCallpath(default_it->first);
//        TuningResult* default_result               = new TuningResult;
//        default_result->objective_value            = default_it->second;
//        rts_match->default_energy                  = default_result;
//    }
//}

//void rtstree::insertDefaultEnergy(Rts* node) {
//    std::map<std::string,double> energy_map = appl->getDefaultEnergyMapping().find("Energy")->second;
//    std::map<std::string, std::map<std::string,double> > default_map = appl->getDefaultEnergyMapping();
//    //First insert energy
//    for( auto energy_it : energy_map ) {
//        Rts* rts_match = node->getRtsByCallpath(energy_it.first);
//        TuningResult* energy_result                = new TuningResult;
//        energy_result->objectives.insert(std::make_pair("Energy",energy_it.second));
//        rts_match->default_energy                  = energy_result;
//    }
//    //Compute normalized energy
//    if(appl->getDefaultEnergyMapping().find("TotalInstr")!=appl->getDefaultEnergyMapping().end()) {
//        std::map<std::string,double> instr_map = appl->getDefaultEnergyMapping().find("TotalInstr")->second;
//        for( auto instr_it : instr_map ) {
//            Rts* rts_match = node->getRtsByCallpath(instr_it.first);
//            double normalized_energy = (rts_match->default_energy->objectives.find("Energy")->second)/instr_it.second;
//            rts_match->default_energy->objectives.insert(std::make_pair("NormalizedEnergy",normalized_energy));
//        }
//    }
//}


void Rts::insertDefaultObjValue( std::string obj, double &obj_value ) {
    //psc_dbgmsg( 8, " Inserting default obj values %s, %1.13f.\n",obj.c_str(),obj_value);
    default_objective = new TuningResult;
    default_objective->objectives.insert(std::make_pair(obj,obj_value));
    default_objective->objective_value = obj_value;
}


void rtstree::displaySavings( ){
    double staticSaving(0.0), dynamicSaving(0.0);
    std::ostringstream savings_oss;
    std::list<Region*> sig_regions = appl->get_sig_regions_list();
    double staticBestObj(0.0), bestObj(0.0), defaultObj(0.0);
    std::string plugin_name;
    for (auto sig_regions_it : sig_regions) {
        //get list of rts
        std::list<Rts*> rtsList = appl->getCalltreeRoot()->getValidRtsOfRegion(sig_regions_it, NULL);
        for (auto rts_it : rtsList) {
            if (!rts_it->tuning_plugin_result.empty()) {
                plugin_name = rts_it->tuning_plugin_result.begin()->first;
                std::map< ConfigType, TuningResult* > plugin_result = rts_it->tuning_plugin_result.begin()->second;
//                psc_dbgmsg( 8, " static best %1.13f.\n",plugin_result[STATIC_BEST]->objective_value);
//                psc_dbgmsg( 8, " rts best %1.13f.\n",plugin_result[RTS_BEST]->objective_value);
//                psc_dbgmsg( 8, " default best %1.13f.\n",rts_it->getDefaultEnergy());
                if( plugin_result[STATIC_BEST] != nullptr ) {
                    if( plugin_result[STATIC_BEST]->objective_value != 0.0 && plugin_result[RTS_BEST]->objective_value != 0.0 && rts_it->getDefaultEnergy() != 0.0 ) {
                        staticBestObj += plugin_result[STATIC_BEST]->objective_value;
                        bestObj += plugin_result[RTS_BEST]->objective_value;
                        defaultObj += rts_it->getDefaultEnergy();
//                        savings_oss << "===============================" << endl;
//                        savings_oss << "Default objective value for Rts = " << rts_it->getCallPath() << ":" << defaultObj << endl;
//                        savings_oss << "Static best objective value for Rts = " << rts_it->getCallPath() << ":" << staticBestObj << endl;
//                        savings_oss << "Best objective value for Rts = " << rts_it->getCallPath() << ":" << bestObj << endl;
                    }
                }
            }
        }
    }
    staticSaving = (defaultObj - staticBestObj) / defaultObj * 100;
    dynamicSaving = (staticBestObj - bestObj) / staticBestObj * 100;
    savings_oss << "===============================" << endl;
    savings_oss << "Static Savings for Rts's = " << staticSaving << endl;
    savings_oss << "Dynamic Savings for Rts's = " << dynamicSaving << endl;
    savings_oss << "------------------------------- \n" << endl << endl;

    double phaseBestObj(0.0), defaultPhaseObj(0.0);
    if( plugin_name == "readex_interphase" ) {
        for(auto ch : appl->getCalltreeRoot()->getChildren()) {
            std::map< ConfigType, TuningResult* > phase_plugin_result = ch->tuning_plugin_result.begin()->second;
            phaseBestObj  += phase_plugin_result[STATIC_BEST]->objective_value;
            defaultPhaseObj  += ch->getDefaultEnergy();
        }
    }
    else {
        std::map< ConfigType, TuningResult* > phase_plugin_result = appl->getCalltreeRoot()->tuning_plugin_result.begin()->second;
        phaseBestObj  = phase_plugin_result[STATIC_BEST]->objective_value;
        defaultPhaseObj  = appl->getCalltreeRoot()->getDefaultEnergy();
    }
    staticSaving = (defaultPhaseObj - phaseBestObj) / defaultPhaseObj * 100;

//    savings_oss << "===============================" << endl;
//    savings_oss << "Default objective value for phase = "<< defaultPhaseObj << endl;
//    savings_oss << "Best objective value for phase = " << phaseBestObj << endl;

    savings_oss << "Static Savings for the whole Phase = " << staticSaving << endl;
    savings_oss << "===============================" << endl;
    cout << savings_oss.str();
}


void rtstree::displayTrueSavings( ){
    double staticSaving(0.0), dynamicSaving(0.0);
    std::ostringstream savings_oss;
    std::list<Region*> sig_regions = appl->get_sig_regions_list();
    double staticBestObj(0.0), bestObj(0.0), defaultObj(0.0);
    std::string plugin_name;
    for (auto sig_regions_it : sig_regions) {
        //get list of rts
        std::list<Rts*> rtsList = appl->getCalltreeRoot()->getValidRtsOfRegion(sig_regions_it, NULL);
        for (auto rts_it : rtsList) {
            if (!rts_it->tuning_plugin_result.empty()) {
                plugin_name = rts_it->tuning_plugin_result.begin()->first;
                std::map< ConfigType, TuningResult* > plugin_result = rts_it->tuning_plugin_result.begin()->second;
                if( plugin_result[TUNING_RESULT_STATIC] != nullptr ) {
                    if( plugin_result[TUNING_RESULT_STATIC]->objective_value != 0.0 && plugin_result[TUNING_RESULT_RTS]->objective_value != 0.0 && rts_it->getDefaultEnergy() != 0.0 ) {
                        staticBestObj += plugin_result[TUNING_RESULT_STATIC]->objective_value;
                        bestObj += plugin_result[TUNING_RESULT_RTS]->objective_value;
                        defaultObj += rts_it->getDefaultEnergy();
                    }
                }
            }
        }
    }
    staticSaving = (defaultObj - staticBestObj) / defaultObj * 100;
    dynamicSaving = (staticBestObj - bestObj) / staticBestObj * 100;

    savings_oss << "\n\n ===============================" << endl;
    savings_oss << "True Static Savings for Rts's = " << staticSaving << endl;
    savings_oss << "True Dynamic Savings for Rts's = " << dynamicSaving << endl;
    savings_oss << "------------------------------- \n" << endl << endl;

    double phaseBestObj(0.0), defaultPhaseObj(0.0);
    if( plugin_name == "readex_interphase" ) {
        for(auto ch : appl->getCalltreeRoot()->getChildren()) {
            std::map< ConfigType, TuningResult* > phase_plugin_result = ch->tuning_plugin_result.begin()->second;
            phaseBestObj  += phase_plugin_result[TUNING_RESULT_STATIC]->objective_value;
            defaultPhaseObj  += ch->getDefaultEnergy();
        }
    }
    else {
        std::map< ConfigType, TuningResult* > phase_plugin_result = appl->getCalltreeRoot()->tuning_plugin_result.begin()->second;
        phaseBestObj  = phase_plugin_result[TUNING_RESULT_STATIC]->objective_value;
        defaultPhaseObj  = appl->getCalltreeRoot()->getDefaultEnergy();
    }
    staticSaving = (defaultPhaseObj - phaseBestObj) / defaultPhaseObj * 100;

    savings_oss << "True Static Savings for the whole Phase = " << staticSaving << endl;
    savings_oss << "===============================" << endl;
    cout << savings_oss.str();
}


void Rts::delete_aagent_tree( Rts* node ) {
    if( node ) {
    if( node->children.size() == 0 ) {
        delete node;
        return;
    }
    int node_index = 0;
    int i          = 0;
    while( i < node->children.size() ) {
            delete_aagent_tree( node->children[i] );
            node_index = ++i;
            if( node_index == node->children.size() ) {
                node->children.erase( node->children.begin(), node->children.end() );
                delete node;
                return;
            }
        }
    }
}


bool withRtsSupport(){
    return rts_support;
}


std::string rtstree::modifyString(Rts* node, std::string& target, unsigned int& cluster_num) {
    std::string replace_str = "Cluster=" + std::to_string(cluster_num);
    return std::regex_replace(target, std::regex("Cluster=\\d+"), replace_str);
}


void Rts::copyData(Rts* node, Rts* clone_n, Rts* parent, unsigned int& curr_cluster, unsigned int& next_cluster) {
    //Now modify the existing cluster node's data
    clone_n->parent = parent;
    parent->children.push_back(clone_n);
    clone_n->parent_scorep_id = node->parent_scorep_id;
    clone_n->scorep_id = node->scorep_id;

    //Now modify the new cluster node's data
    clone_n->region_name = rtstree::modifyString(clone_n,node->region_name,next_cluster);
    if(clone_n->getName().find("Cluster=") != std::string::npos){
       clone_n->extract_parameter();
    }
    else  {
        clone_n->reg = node->reg;
        clone_n->rts_type = NODE_REGULAR_REGION;
    }
}

void rtstree::modifyTree(Rts* node, unsigned int num_clusters) {
    psc_dbgmsg( 10, "modifyTree entered \n" );

    unsigned int curr_cluster(1), next_cluster(curr_cluster);

    //First create a new node for the 1st cluster
    Rts* newClusterNode = new Rts();
    newClusterNode->modifyNode(newClusterNode,node,curr_cluster);

    for(unsigned int ch = 0; ch < node->getChildren().size(); ch++) {
        if(++next_cluster <= num_clusters) {
            rtstree::cloneTree(node,node->getChildren()[ch],curr_cluster,next_cluster);
            curr_cluster = next_cluster;
            if(next_cluster == num_clusters) break;
        }
    }

    std::vector<std::string> callpath_strings;
    node->setCallPaths( node, callpath_strings);
    psc_dbgmsg( 10, "modifyTree finished \n" );
}

void Rts::modifyNode(Rts* newClusterNode,Rts* node, unsigned int& curr_cluster){
    newClusterNode->parent = node;
    newClusterNode->children = std::move(node->children);
    node->children.push_back(newClusterNode);

    newClusterNode->region_name = "Cluster=" + std::to_string(curr_cluster);
    newClusterNode->extract_parameter();
    newClusterNode->parent_scorep_id = node->scorep_id;
    newClusterNode->callpathstring = rtstree::modifyString(node,node->callpathstring,curr_cluster);
}


void rtstree::cloneTree( Rts* parent, Rts* node, unsigned int& curr_cluster, unsigned int& next_cluster) {
    //First correct the cluster number, parameter and region names
    psc_dbgmsg( 10, "cloneTree entered.\n" );
    Rts* clone_n = new Rts();

    //Copy all the data from the node to the clone and correct the region name, parameter info and the callpath
    clone_n->copyData(node,clone_n,parent,curr_cluster,next_cluster);
    for( auto child: node->getChildren() ) {
        rtstree::cloneTree(clone_n,child,curr_cluster,next_cluster);
    }
    psc_dbgmsg( 10, "cloneTree finished.\n" );
}

/**
 *@brief Recursive generation of the mapping of each node with its call-path string
 *@param node Rts to be mapped to its call-path string
 *@param callpath_strings Vector containing the region name of each rts region traversed
 */
void Rts::setCallPaths( Rts* node, std::vector<std::string>& callpath_strings ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( RtsInfo ), "Rts: call to setCallPaths(): generating call-path string of the node \n" );
    if( !node ) {
        return;
    }

    callpath_strings.push_back( node->region_name );

    std::string test_callpathstring = makeCallpathString( callpath_strings );
    node->callpathstring            = test_callpathstring;

    // If the node is a parameter node, insert the parameter information
    // Propagate the parameter list from the parameters above this node
    if( node->parent != NULL ) {
        if( ( node->parent->rts_type != NODE_REGULAR_REGION ) && ( node->rts_type != NODE_REGULAR_REGION ) ) {
            std::vector<Parameter_t*>::iterator param_it;
            for( param_it = node->parent->parameter.begin(); param_it != node->parent->parameter.end(); param_it++ ) {
                std::vector<Parameter_t*>::iterator it = std::find(node->parameter.begin(), node->parameter.end(), *param_it );
                if( it == node->parameter.end() ) {
                    node->parameter.insert( node->parameter.begin(), *param_it );
                }
            }
        }
    }

    if( node->children.size() == 0 ) {
        callpath_strings.pop_back();
    }
    setValidRts( node );

    int node_index = 0;
    int i          = 0;

    while( i < node->children.size() ) {
        setCallPaths( node->children[i], callpath_strings );
        node_index = ++i;
        if( node_index == node->children.size() ) {

            if( callpath_strings.size() != 0 ) {
                callpath_strings.pop_back();
            }
            return;
        }
    }
}


/**
 *@brief Inserts the default values of the tuning parameters
 */
void Rts::insertDefaultTPValues( std::vector<TuningParameter*>& tuningParameters ) {
    if( !( tuning_plugin_result.empty() ) ) {
        std::map<std::string, std::map<ConfigType, TuningResult* > >::const_iterator pluginResult_iter = tuning_plugin_result.begin();
        for( auto tuning_res : pluginResult_iter->second ) {
            for( auto tp : tuningParameters ) {
                auto iter = std::find_if(tuning_res.second->tuning_parameter_value_mapping.begin(), tuning_res.second->tuning_parameter_value_mapping.end(),
                        [tp](auto& tp_mapping) {return (tp->getName() == tp_mapping.first->getName()); });
                if( iter == tuning_res.second->tuning_parameter_value_mapping.end() ) {
                    tuning_res.second->tuning_parameter_value_mapping.emplace(std::make_pair(tp,tp->getDefaultValue()));
                }
            }
        }
    }

    for( int i = 0; i < getChildren().size(); i++ ) {
        getChildren()[i]->insertDefaultTPValues( tuningParameters );
    }
}
