/**
   @file    ConfigModel.h
   @ingroup READEX
   @brief   Configuration sent by tuning_potential
   @author  Anamika Chowdhury
   @verbatim
   Revision:       $Revision$
   Revision date:  $Date$
   Committed by:   $Author$

   This file is part of READEX.

   Copyright (c) 2016, Technische Universitaet Muenchen, Germany
   See the COPYING file in the base directory of the package for details.
   @endverbatim
 */


#ifndef READEX_PRELUDE_CONFIGURATION_H
#define READEX_PRELUDE_CONFIGURATION_H

#include <iostream>
#include <list>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/detail/xml_parser_writer_settings.hpp>

#include "SignificantRegion.h"


using namespace std;
using boost::property_tree::ptree;
using boost::property_tree::xml_writer_settings;

class ConfigModel {
private:
    ptree internalData;
    ptree XMLConfigTree;
public:
    ConfigModel();
    ConfigModel( bool                              isExist,
                 const list< SignificantRegion* >& significant_regions,
                 vector< string >&                 sig_region_names,
                 const PhaseRegion*                phase,
                 double                            weight,
                 double                            time_variation,
                 double                            compute_intensity_variation,
                 double                            granularity_sig_region,
                 vector< string >&                 t_variation_reg,
                 vector< string >&                 com_intensity_variation_reg );
    void toXML( const std::string& );
    void createTemplate();
    void updateTree( ptree& configTree );

};

#endif //READEX_PRELUDE_CONFIGURATION_H
