/**
   @file	VariantSpace.h
   @ingroup Autotune
   @brief   Variant Space
   @author	Houssam Haitof
   @verbatim
        Revision:       $Revision$
        Revision date:  $Date$
        Committed by:   $Author$

        This file is part of the Periscope performance measurement tool.
        See http://www.lrr.in.tum.de/periscope for details.

        Copyright (c) 2005-2012, Technische Universitaet Muenchen, Germany
        See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#ifndef VARIANTSPACE_H_
#define VARIANTSPACE_H_

#include <list>
#include <map>

#include "TuningParameter.h"
#include "Variant.h"
#include "VariantSpaceConstraint.h"

using namespace std;


class VariantSpace {
    vector<TuningParameter*> tuningParameters;
    // TODO check if getter/setters needed for vsConstraints
    list<VariantSpaceConstraint> vsConstraints;

public:
    VariantSpace() {
    }

    virtual ~VariantSpace() {
    }

    void addTuningParameter( TuningParameter* tp );

    void addTuningParametersAsVector( vector<TuningParameter*> tps );

    void clear() {
        tuningParameters.clear();
    }

    // TODO instead of exposing the internal representation (a map in this case),
    // the class should implement its own iterator. I will recommend Boost.Iterator -Isaias
    vector<TuningParameter*>getTuningParameters() const;
};

#endif /* VARIANTSPACE_H_ */
