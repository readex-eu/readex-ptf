/**
   @file    SearchSpace.h
   @ingroup Autotune
   @brief   Search Space header
   @author  Houssam Haitof
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#ifndef SEARCHSPACE_H_
#define SEARCHSPACE_H_

#include <list>
#include <map>
#include <vector>

#include "application.h"
#include "VariantSpace.h"

using namespace std;


class SearchSpace {
    vector<Region*> regions;
    vector<Rts*> rtsVector;
    // TODO check if getter/setters needed for vsConstraints
    VariantSpace* variantSpace;
    string domainName;

public:
    SearchSpace() {
    }

    virtual ~SearchSpace() {
    }

    void addRegion( Region* reg );

    vector<Region*> getRegions();

    void addRts( Rts* rts );

    vector<Rts*> getRts();

    void addDomain( std::string name );

    std::string getDomain ();

    void setVariantSpace( VariantSpace* vs );

    VariantSpace* getVariantSpace();
};

#endif /* SEARCHSPACE_H_ */
