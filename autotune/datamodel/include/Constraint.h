/**
   @file	Constraint.h
   @ingroup Autotune
   @brief   Constraint
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

#ifndef AUTOTUNE_CONSTRAINT_H_
#define AUTOTUNE_CONSTRAINT_H_

#include <list>

using namespace std;

class Constraint {
    int       leftBoundary;
    int       rightBoundary;
    list<int> elements;

public:
    bool assertValue( int );

    Constraint() {
    }

    ~Constraint() {
    }

    list<int>getElements();

    int getLeftBoundary();

    int getRightBoundary();

    void setElements( list<int> elements );

    void setLeftBoundary( int leftBoundary );

    void setRightBoundary( int rightBoundary );
};



#endif /* CONSTRAINT_H_ */
