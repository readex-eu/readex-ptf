/**
   @file    ScenarioPoolSet.h
   @ingroup Autotune
   @brief   Tuning Parameter header
   @author  Isaias Compres
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

#ifndef SCENARIOPOOLSET_H_
#define SCENARIOPOOLSET_H_

#include "ScenarioPool.h"
#include "ScenarioResultsPool.h"
#include "AnalysisResultsPool.h"

class ScenarioPoolSet {
public:
    ScenarioPool*        csp;
    ScenarioPool*        psp;
    ScenarioPool*        esp;
    ScenarioPool*        fsp;
    ScenarioResultsPool* srp;
    AnalysisResultsPool* arp;

    ScenarioPoolSet();

    virtual ~ScenarioPoolSet();
};
#endif /* SCENARIOPOOLSET_H_ */
