/**
   @file	Constraint.h
   @ingroup Autotune
   @brief   Variant Space Constraint
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

#ifndef VARIANTSPACECONSTRAINT_H_
#define VARIANTSPACECONSTRAINT_H_

class VariantSpaceConstraint /*: public Constraint*/
{
public:
    VariantSpaceConstraint() {
    }

    virtual ~VariantSpaceConstraint() {
    }
};

#endif /* VARIANTSPACECONSTRAINT_H_ */
