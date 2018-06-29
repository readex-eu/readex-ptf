/**
   @file    xml_psc_tags.h
   @ingroup AnalysisAgent
   @brief   XML Tags for property's presentation
   @verbatim
        Revision:       $Revision$
        Revision date:  $Date$
        Committed by:   $Author$

        This file is part of the Periscope performance measurement tool.
        See http://www.lrr.in.tum.de/periscope for details.

        Copyright (c) 2005-2011, Technische Universitaet Muenchen, Germany
        See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#ifndef XML_PSC_TAGS_H_
#define XML_PSC_TAGS_H_

#define XML_PSC_NAMESPACE                    "http://www.lrr.in.tum.de/Periscope"
#define XML_PSC_SCHEMA_LOC                   "http://www.lrr.in.tum.de/Periscope psc_properties.xsd"

#define XML_PSC_EXPERIMENT_TAG               "Experiment"
#define XML_PSC_DATE_TAG                     "date"
#define XML_PSC_TIME_TAG                     "time"
#define XML_PSC_DIR_TAG                      "dir"
#define XML_PSC_REVISION_TAG                 "rev"
#define XML_PSC_NUM_PROPS_TAG                "NumOfProperties"

#define XML_PSC_PROP_TAG                     "property"
#define XML_PSC_PROP_ID_TAG                  "ID"
#define XML_PSC_PROP_CLUSTER_TAG             "cluster"
#define XML_PSC_PROP_NAME_TAG                "name"

#define XML_PSC_PROP_CONTEXT_TAG             "context"
#define XML_PSC_PROP_CONTEXT_FILEID_TAG      "FileID"
#define XML_PSC_PROP_CONTEXT_FILENAME_TAG    "FileName"
#define XML_PSC_PROP_CONTEXT_RFL_TAG         "RFL"
#define XML_PSC_PROP_CONTEXT_CONFIG_TAG      "Config"
#define XML_PSC_PROP_CONTEXT_REGION_TYPE_TAG "RegionType"
#define XML_PSC_PROP_CONTEXT_REGION_NAME_TAG "RegionName"
#define XML_PSC_PROP_CONTEXT_REGIONID_TAG    "RegionId"


#define XML_PSC_PROP_EXECOBJ_TAG             "execObj"
#define XML_PSC_PROP_EXECOBJ_PROC_TAG        "process"
#define XML_PSC_PROP_EXECOBJ_THREAD_TAG      "thread"

#define XML_PSC_PROP_SEVERITY_TAG            "severity"
#define XML_PSC_PROP_CONFIDENCE_TAG          "confidence"

#define XML_PSC_PROP_PURPOSE_TAG             "purpose"

#define XML_PSC_PROP_ADDINFO_TAG             "addInfo"

#define XML_PSC_NODE_TAG                     "Node"
#define XML_PSC_NODE_TYPE                    "NodeType"
#define XML_PSC_CHILDREN_TAG                 "Children"
#define XML_PSC_CHILD_TAG                    "Child"
#define XML_PSC_NODE_SCOREPID_TAG            "ScorepID"
#define XML_PSC_NODE_PARENT_SCOREPID_TAG     "ParentScorepID"
#define XML_PSC_NODE_AAREGIONID_TAG          "AARegionID"
#define XML_PSC_NODE_AAREGIONID_STRING_TAG   "AARegionIDString"
#define XML_PSC_NODE_TYPE_TAG                "RtsNodeType"
#define XML_PSC_NODE_PARAMETER_TYPE_TAG      "ParameterNodeType"
#define XML_PSC_NODE_CALLPATH_TAG            "CallPath"
#define XML_PSC_NODE_REGIONNAME_TAG          "ScorePRegionName"
#define XML_PSC_CALLTREE_PARAMETER_TAG       "Parameter"
#define XML_PSC_CALLTREE_PARAMETERS_TAG      "Parameters"
#define XML_PSC_CALLTREE_PARAMETER_NAME_TAG  "ParameterName"
#define XML_PSC_CALLTREE_PARAMETER_VALUE_TAG "ParameterValue"
#define XML_PSC_NODE_AARTS_ID_TAG            "RtsID"
#define XML_PSC_PROP_RTSBASED_TAG            "RtsBased"

#define XML_PSC_WS                           "&ws;"
#define XML_PSC_LT                           "&lt;"
#define XML_PSC_AMP                          "&amp;"
#define XML_PSC_GT                           "&gt;"

#define PSC_XML_WS                           " "
#define PSC_XML_LT                           "<"
#define PSC_XML_AMP                          "&"
#define PSC_XML_GT                           ">"
#endif /*XML_PSC_TAGS_H_*/
