/**
   @file    ConfigAnalysis.h
   @ingroup ConfigAnalysisStrategy
   @brief   Configurable Analysis header
   @author  Michael Gerndt
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

/**
 * @defgroup ConfigAnalysisStrategy Configurable Analysis Strategy
 * @ingroup Strategies
 */

#ifndef STRATEGY_ConfigAnalysis_H
#define STRATEGY_ConfigAnalysis_H

#include "global.h"
#include "rts.h"
#include "strategy.h"
#include "application.h"
#include "DataProvider.h"
#include "analysisagent.h"
#include <list>

class ConfigAnalysis : public Strategy {
protected:
    Prop_List                    candProperties;
    Region*                      phaseRegion;
    std::list<PropertyRequest*>* requests;


public:
    ConfigAnalysis( std::list<PropertyRequest*>* reqs,
                    bool                         pedantic = false );
    ~ConfigAnalysis();
    std::string name();
    bool isRtsBased();

    bool reqAndConfigureFirstExperiment( Region* initial_region ); // TRUE can start; FALSE not ready
    bool evaluateAndReqNextExperiment();                           // TRUE requires next step; FALSE if done
    void configureNextExperiment();

    void createProperty( int          propId,
                         int          rank,
                         std::string  entityID );  //can be regionId or rts Callpath string
};


#endif
