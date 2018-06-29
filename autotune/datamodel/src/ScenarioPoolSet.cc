/**
   @file    ScenarioPoolSet.cc
   @ingroup Autotune
   @brief   Tuning Parameter
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

#include "ScenarioPoolSet.h"

ScenarioPoolSet::ScenarioPoolSet() {
    csp = new ScenarioPool();
    psp = new ScenarioPool();
    esp = new ScenarioPool();
    fsp = new ScenarioPool();
    srp = new ScenarioResultsPool();
    arp = new AnalysisResultsPool();
}

ScenarioPoolSet::~ScenarioPoolSet() {
    delete csp;
    delete psp;
    delete esp;
    delete fsp;
    delete srp;
    delete arp;
}
