/**
   @file    Property.cc
   @ingroup AnalysisAgent
   @brief   Performance property abstraction
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include <iostream>
#include <sstream>
#include <string>
#include <boost/regex.hpp>

#include "string_helper.h"
#include "Property.h"
#include "xml_psc_tags.h"

Context* Property::get_context() const {
    return context;
}

int Property::get_rank() const {
    return context->getRank();
}


int Property::get_thread() const {
    return context->getThread();
}


void Property::set_context( Context* c ) {
    context = c;
}

void Property::set_PhaseContext( Context* c ) {
    phaseContext = c;
}

void Property::add_ScenarioId( int scId ) {
    scenarioIds.push_back( scId );
}

std::list<int>* Property::get_ScenarioIdList() {
    return &scenarioIds;
}

void Property::set_Purpose( PropertyPurpose purps ) {
    purpose = purps;
}

PropertyPurpose Property::get_Purpose( void ) {
    return purpose;
}

Region* Property::get_region() const {
    return context->getRegion();
}

std::string Property::toXMLSerialize() {
    std::stringstream        xmlData;
    bool                     cluster = false;
    std::list<int>::iterator scenarioId;

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

    std::string agent_region_id = context->getRegionId();
    agent_region_id = formatString( agent_region_id, reg_expr, format );

    std::string callpath;
    if (isRtsBased()){
   	 callpath = context->getCallpath();
       callpath = formatString( callpath, reg_expr, format );
    } else {
   	 callpath = "";
    }

    // Property: <property cluster="false" ID="1" >
    xmlData << "  <" << XML_PSC_PROP_TAG << " " << XML_PSC_PROP_CLUSTER_TAG << "=\"" << std::boolalpha << cluster
            << "\" " XML_PSC_PROP_ID_TAG << "=\"" << id() << "-" << subId() << "\" >" << std::endl;

    // Property name: <name>IA64 Pipeline Stall Cycles</name>
    xmlData << "\t<" << XML_PSC_PROP_NAME_TAG << ">" << name() << "</" << XML_PSC_PROP_NAME_TAG << ">" << std::endl;

    // Context: <context FileID="42" FileName="add.f90" RFL="2" Region="USER_REGION" RegionId="1-27">
    xmlData << "\t<" << XML_PSC_PROP_CONTEXT_TAG
            << " " << XML_PSC_PROP_CONTEXT_FILEID_TAG << "=\"" << context->getFileId()
            << "\" " << XML_PSC_PROP_CONTEXT_FILENAME_TAG << "=\"" << context->getFileName()
            << "\" " << XML_PSC_PROP_CONTEXT_RFL_TAG << "=\"" << context->getStartPosition()
            << "\" " << XML_PSC_PROP_CONTEXT_CONFIG_TAG << "=\"" << appl->getMpiProcs()
            << "x" << appl->getOmpThreads() << "\" " << XML_PSC_PROP_CONTEXT_REGION_TYPE_TAG
            << "=\"" << static_cast<int>( context->getRegion()->get_ident().type )
            << "\" " << XML_PSC_PROP_CONTEXT_REGIONID_TAG << "=\"" << agent_region_id
            << "\" " << XML_PSC_PROP_RTSBASED_TAG << "=\"" << isRtsBased()
            << "\" " << XML_PSC_NODE_CALLPATH_TAG << "=\"" << callpath
            << "\" >" << std::endl;

    // Executing object: <execObj process="0" thread="0" />
    xmlData << "\t\t<" << XML_PSC_PROP_EXECOBJ_TAG
            << " " XML_PSC_PROP_EXECOBJ_PROC_TAG << "=\"" << context->getRank()
            << "\" " XML_PSC_PROP_EXECOBJ_THREAD_TAG << "=\"" << context->getThread()
            << "\"/>" << std::endl;

    xmlData << "\t</" << XML_PSC_PROP_CONTEXT_TAG << ">" << std::endl;

    //xmlData.precision(2);  // Set 2 digits after the decimal point
    //xmlData << std::fixed; // Use a fixed point floating point format or std::scientific

    // Severity: <severity>52.30</severity>
    xmlData << "\t<" << XML_PSC_PROP_SEVERITY_TAG << ">" << severity()
            << "</" << XML_PSC_PROP_SEVERITY_TAG << ">" << std::endl;

    // Confidence: <confidence>0.99</confidence>
    xmlData << "\t<" << XML_PSC_PROP_CONFIDENCE_TAG << ">" << confidence()
            << "</" << XML_PSC_PROP_CONFIDENCE_TAG << ">" << std::endl;

    // Information about the purpose of the property, can be analysis or tuning.
    // Purpose: <purpose>0</purpose>
    xmlData << "\t<" << XML_PSC_PROP_PURPOSE_TAG << ">" << get_Purpose()
            << "</" << XML_PSC_PROP_PURPOSE_TAG << ">" << std::endl;

    // Extra information (property specific!)
    xmlData << "\t<" << XML_PSC_PROP_ADDINFO_TAG << ">" << std::endl;
    xmlData << toXMLExtra();
    for( scenarioId = scenarioIds.begin(); scenarioId != scenarioIds.end(); scenarioId++ ) {
        xmlData << "\t\t<ScenarioID>" << *scenarioId << "</ScenarioID>" << std::endl;
    }
    xmlData << "\t</" << XML_PSC_PROP_ADDINFO_TAG << ">" << std::endl;

    // End of the Property
    xmlData << "  </" << XML_PSC_PROP_TAG << ">" << std::endl;

    return xmlData.str();
}


std::string Property::toXML() {
    std::stringstream        xmlData;
    bool                     cluster = false;
    std::list<int>::iterator scenarioId;

    // Property: <property cluster="false" ID="1" >
    xmlData << "  <" << XML_PSC_PROP_TAG << " " << XML_PSC_PROP_CLUSTER_TAG << "=\"" << std::boolalpha << cluster
            << "\" " XML_PSC_PROP_ID_TAG << "=\"" << id() << "-" << subId() << "\" >" << std::endl;

    // Property name: <name>IA64 Pipeline Stall Cycles</name>
    xmlData << "\t<" << XML_PSC_PROP_NAME_TAG << ">" << name() << "</" << XML_PSC_PROP_NAME_TAG << ">" << std::endl;

    // Context: <context FileID="42" FileName="add.f90" RFL="2" Region="USER_REGION" RegionId="1-27">
    xmlData << "\t<" << XML_PSC_PROP_CONTEXT_TAG
            << " " << XML_PSC_PROP_CONTEXT_FILEID_TAG << "=\"" << context->getFileId()
            << "\" " << XML_PSC_PROP_CONTEXT_FILENAME_TAG << "=\"" << context->getFileName()
            << "\" " << XML_PSC_PROP_CONTEXT_RFL_TAG << "=\"" << context->getStartPosition()
            << "\" " << XML_PSC_PROP_CONTEXT_CONFIG_TAG << "=\"" << appl->getMpiProcs()
            << "x" << appl->getOmpThreads() << "\" " << XML_PSC_PROP_CONTEXT_REGION_TYPE_TAG
            << "=\"" << static_cast<int>( context->getRegion()->get_ident().type )
            << "\" " << XML_PSC_PROP_CONTEXT_REGIONID_TAG << "=\"" << context->getRegionId()
            << "\" >" << std::endl;

    // Executing object: <execObj process="0" thread="0" />
    xmlData << "\t\t<" << XML_PSC_PROP_EXECOBJ_TAG
            << " " XML_PSC_PROP_EXECOBJ_PROC_TAG << "=\"" << context->getRank()
            << "\" " XML_PSC_PROP_EXECOBJ_THREAD_TAG << "=\"" << context->getThread()
            << "\"/>" << std::endl;

    xmlData << "\t</" << XML_PSC_PROP_CONTEXT_TAG << ">" << std::endl;

    //xmlData.precision(2);  // Set 2 digits after the decimal point
    //xmlData << std::fixed; // Use a fixed point floating point format or std::scientific

    // Severity: <severity>52.30</severity>
    xmlData << "\t<" << XML_PSC_PROP_SEVERITY_TAG << ">" << severity()
            << "</" << XML_PSC_PROP_SEVERITY_TAG << ">" << std::endl;

    // Confidence: <confidence>0.99</confidence>
    xmlData << "\t<" << XML_PSC_PROP_CONFIDENCE_TAG << ">" << confidence()
            << "</" << XML_PSC_PROP_CONFIDENCE_TAG << ">" << std::endl;

    // Information about the purpose of the property, can be analysis or tuning.
    // Purpose: <purpose>0</purpose>
    xmlData << "\t<" << XML_PSC_PROP_PURPOSE_TAG << ">" << get_Purpose()
            << "</" << XML_PSC_PROP_PURPOSE_TAG << ">" << std::endl;

    // Extra information (property specific!)
    xmlData << "\t<" << XML_PSC_PROP_ADDINFO_TAG << ">" << std::endl;
    xmlData << toXMLExtra();
    for( scenarioId = scenarioIds.begin(); scenarioId != scenarioIds.end(); scenarioId++ ) {
        xmlData << "\t\t<ScenarioID>" << *scenarioId << "</ScenarioID>" << std::endl;
    }
    xmlData << "\t</" << XML_PSC_PROP_ADDINFO_TAG << ">" << std::endl;

    // End of the Property
    xmlData << "  </" << XML_PSC_PROP_TAG << ">" << std::endl;

    return xmlData.str();
}
