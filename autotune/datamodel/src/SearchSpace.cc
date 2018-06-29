/**
   @file    SearchSpace.cc
   @ingroup Autotune
   @brief   Search Space
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

#include "SearchSpace.h"
#include "psc_errmsg.h"




void SearchSpace::addRegion( Region* reg ) {
    if( !reg ) {
        psc_errmsg( "INVALID PARAMETER to SearchSpace::addRegion(): The region cannot be a nullptr.\n" );
        abort();
    }

    regions.push_back( reg );
}

// TODO check if to add an iterator instead
vector<Region*> SearchSpace::getRegions() {
    return regions;
}

void SearchSpace::addRts( Rts* rts ) {
    if( !rts ) {
        psc_errmsg( "INVALID PARAMETER to SearchSpace::addRts(): The rts cannot be a nullptr.\n" );
        abort();
    }


    rtsVector.push_back( rts );
}

// TODO check if to add an iterator instead
vector<Rts*> SearchSpace::getRts() {
   return rtsVector;
}

void SearchSpace::setVariantSpace( VariantSpace* vs ) {
    variantSpace = vs;
}


VariantSpace* SearchSpace::getVariantSpace() {
    return variantSpace;
}

void SearchSpace::addDomain( std::string name ){
    domainName = name;
}

string SearchSpace::getDomain (){
    return domainName;
}