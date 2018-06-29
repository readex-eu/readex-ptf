/**
   @file    tuning_model.cc
   @ingroup Frontend
   @brief   Tuning model
   @author  Nico Reissmann
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2016, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include "clustering/clusterer.h"
#include "common.h"
#include "conversion.h"
#include "phase.h"
#include "scenario.h"
#include "serialization.h"

#include "aagent/include/rts.h"

#include <list>
#include <string>
#include <unordered_set>

namespace tmg {

static std::unordered_set< const Rts* >
filter_rtss( const Rts* root )
{
    std::function< void( const Rts*, std::unordered_set< const Rts* >& ) > filter = [&](
        const Rts*                        root,
        std::unordered_set< const Rts* >& rtss
    ){
        if ( root->ValidRts()
             && !root->getTuningResults().empty()
             && ( ( root->getParentScorepID() == 0 ) || root->getRegion()->get_is_significant() ) ) {
            rtss.insert( root );
        }

        std::vector< Rts* > children = root->getChildren();
        for ( const auto& child : children ) {
            filter( child, rtss );
        }
    };

    std::unordered_set< const Rts* > rtss;
    filter( root, rtss );
    return rtss;
}

static std::unordered_set< std::unique_ptr< scenario > >
create_scenarios( const std::vector< std::unordered_set< tmg::rts* > >& clusters,
                  const tmg::configuration_selector_t&                  selector ) {
    std::unordered_set< std::unique_ptr< scenario > > scenarios;

    for ( const auto& cluster : clusters ) {
        scenarios.insert( std::move( std::make_unique< scenario >( cluster, selector ) ) );
    }

    return scenarios;
}

std::string
generate_tuning_model( const Rts*                                            root,
                       const std::unordered_map< std::string, std::string >& input_ids,
                       const tmg::cluster::clusterer&                        clusterer,
                       const tmg::configuration_selector_t&                  selector ) {
    auto old_rtss  = filter_rtss( root );
    auto new_rtss  = convert_rtss( old_rtss, input_ids );
    auto clusters  = clusterer.cluster( new_rtss );
    auto scenarios = create_scenarios( clusters, selector );

    auto cphases = extract_cluster_phases( root );

    return serialize( scenarios, cphases );
}

}
