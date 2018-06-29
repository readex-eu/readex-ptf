/**
   @file    IPlugin.h
   @ingroup AutotunePlugins
   @brief   Plugin interface
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

/**
   @defgroup AutotunePlugins Autotune Plugins
   @ingroup Autotune
 */


#ifndef IPLUGIN_H_
#define IPLUGIN_H_

// Autotune includes
#include "ISearchAlgorithm.h"
#include "ScenarioPool.h"
#include "ScenarioPoolSet.h"
#include "DriverContext.h"
#include "StrategyRequest.h"
#include "PropertyID.h"
#include "Advice.h"

// Periscope includes
// TODO including everything from the Frontend may be unsafe and unnecessary,
// do includes selectively -Isaias
#include "config.h" // configure data
#include "selective_debug.h"

// STL includes
#include <vector>
#include <queue>

#define PSC_AUTOTUNE_ALL_DEBUG PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAll )
#define PSC_AUTOTUNE_PLUGIN_DEBUG PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins )

class DriverContext;

enum PTF_PLUGIN_ERROR {
    TUNING_PARAMETERS_NOT_FOUND,
    UNDEFINED_OBJECTIVE,
    NULL_REFERENCE
};

//! [iplugin_h]
class IPlugin {
protected:
    DriverContext*   context;
    ScenarioPoolSet* pool_set;

public:
    virtual ~IPlugin() = 0;

    // Plugins are required to implement the full API
    // TODO doxygen documentation important here -IC
    virtual void initialize( DriverContext*   context,
                             ScenarioPoolSet* pool_set ) = 0;

    virtual void startTuningStep( void ) = 0;

    virtual bool analysisRequired( StrategyRequest** strategy ) = 0;

    virtual void createScenarios( void ) = 0;

    virtual void prepareScenarios( void ) = 0;

    virtual void defineExperiment( int               numprocs,
                                   bool&             analysisRequired,
                                   StrategyRequest** strategy ) = 0;

    virtual bool restartRequired( std::string& env,
                                  int&         numprocs,
                                  std::string& cmd,
                                  bool&        instrumented ) = 0;

    virtual bool searchFinished( void ) = 0;

    virtual void finishTuningStep( void ) = 0;

    virtual bool tuningFinished( void ) = 0;

    virtual Advice* getAdvice( void ) = 0;

    /** Finalizes the plug-in normally, cleaning up all resources. */
    virtual void finalize( void ) = 0;

    /** Terminates the plug-in due to an error. This method should be able to be executed safely at any point. */
    virtual void terminate( void ) = 0;
};
//! [iplugin_h]

inline IPlugin::~IPlugin() {
}

#endif /* IPLUGIN_H_ */
