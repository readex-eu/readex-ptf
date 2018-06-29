/**
   @file    tmmerger.cc
   @ingroup Frontend
   @brief   Tuning Model Merger
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
#include "clustering/distance.h"
#include "clustering/vector.h"
#include "merge.h"
#include "serialization.h"

#include <fstream>
#include <iostream>

static void
print_usage(const std::string & app)
{
    std::cerr << "Tuning Model Merger\n";
    std::cerr << "Usage: " << app << " FILE [FILE]\n";
}

int
main(int argc, char * argv[])
{
    if (argc < 2)
        print_usage(argv[0]);

    std::vector<tmg::scenario_set> scnrvector;
    for (ssize_t n = 1; n < argc; n++) {
        std::ifstream is(argv[n]);
        if (is.fail()) {
            std::cerr << "Cannot open tuning model " << argv[n];
            exit(1);
        }

        scnrvector.push_back(deserialize(is, tmg::random_selector));
    }

    using namespace tmg::cluster;

    hierarchical_clusterer c(euclidean_distance, centroid);
    auto scenarios = tmg::merge(scnrvector, tmg::average_selector, c);
    std::cout << serialize(scenarios, {});

    return 0;
}
