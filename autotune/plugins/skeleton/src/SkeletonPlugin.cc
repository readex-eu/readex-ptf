/**
   @file    SkeletonPlugin.cc
   @ingroup SkeletonPlugin
   @brief   Skeleton of a Plugin
   @author  Author's name
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include "SkeletonPlugin.h"

/**
 * @brief Initialize the plugin's data structures.
 *
 * The tuning parameter list needs to be created.
 *
 * Search algorithms are loaded here when required. This can be done as follows:
 *
 *    searchAlgorithm = loadSearchAlgorithm("name");
 *
 * where "name" refers to one of the available search algorithms (currently only exhaustive).
 *
 * @ingroup SkeletonPlugin
 *
 * @param context a pointer to a context for a plugin
 * @param pool_set a pointer to a set of pools for a plugin
 */
void SkeletonPlugin::initialize( DriverContext*   context,
                                 ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "SkeletonPlugin: call to initialize()\n" );

    psc_errmsg( "SkeletonPlugin: initialize() not implemented\n" );
    throw 0;
}

/**
 * @brief Operations to be done at the start of a tuning step.
 *
 * The tuning parameters that were created at initialization are used to create a variant space.
 * The variant space and the regions are then used to create a search space that is passed to the
 * search algorithm.
 *
 * @ingroup SkeletonPlugin
 *
 */
void SkeletonPlugin::startTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "SkeletonPlugin: call to startTuningStep()\n" );

    psc_errmsg( "SkeletonPlugin: startTuningStep() not implemented\n" );
    throw 0;
}

/**
 * @brief Defines a pre-analysis strategy.
 *
 * Found properties of the pre-analysis strategy are stored in the arp.
 *
 * @ingroup SkeletonPlugin
 *
 * @return true if pre-analysis is required false otherwise
 *
 */
bool SkeletonPlugin::analysisRequired( StrategyRequest** strategy ) {
    return false;
}

/**
 * @brief The Created Scenario Pool (csp) is populated here.
 *
 * The scenarios need to be created and added to the first pool. To create the scenarios, a
 * search algorithm can be used or it can be done directly by the plugin.
 *
 * After this step, the Periscope will verify that scenarios were added to the csp.
 *
 * @ingroup SkeletonPlugin
 *
 */
void SkeletonPlugin::createScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "SkeletonPlugin: call to createScenarios()\n" );

    psc_errmsg( "SkeletonPlugin: createScenarios() not implemented\n" );
    throw 0;
}

/**
 * @brief Preparatory steps for the scenarios are done here.
 *
 * If there are any preparatory steps required by some or all scenarios in the csp (for example:
 * the project may need to be re-compiled), then they are to be performed here. After each
 * scenario is prepared, they are migrated from the csp to the Prepared Scenario Pool (psp).
 *
 * In some cases, no preparation may be necessary and the plugin can simply move all scenarios
 * from the csp to the psp.
 *
 * After this step, the Periscope will verify that scenarios were added to the psp.
 *
 * @ingroup SkeletonPlugin
 *
 */
void SkeletonPlugin::prepareScenarios( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "SkeletonPlugin: call to prepareScenarios()\n" );

    psc_errmsg( "SkeletonPlugin: prepareScenarios() not implemented\n" );
    throw 0;
}


/**
 * @brief Populate the Experiment Scenario Pool (esp) for the next experiment.
 *
 * This is the final step before the experiments are executed. Scenarios are moved from the
 * psp to the esp, depending on the number of processes and whether they can be executed
 * in parallel.
 *
 * After this step, the Periscope will verify that scenarios were added to the esp.
 *
 * @ingroup SkeletonPlugin
 *
 * @param numprocs         Number of processes required for the experiment
 * @param analysisRequired Defines is per-experiment analysis required
 * @param strategy         Strategy request for per-experiment analysis
 *
 */
void SkeletonPlugin::defineExperiment( int               numprocs,
                                       bool&             analysisRequired,
                                       StrategyRequest** strategy ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "SkeletonPlugin: call to defineExperiment()\n" );

    psc_errmsg( "SkeletonPlugin: defineExperiment() not implemented\n" );
    throw 0;
}

/**
 * @brief Returns does the application has to be restarted for the next experiment.
 *
 * Return true if a restart of the application is required for the next experiment,
 * false otherwise.
 *
 * @ingroup SkeletonPlugin
 *
 * @return true if an application has to be restarted false otherwise
 *
 */
bool SkeletonPlugin::restartRequired( std::string& env,
                                      int&         numprocs,
                                      std::string& command,
                                      bool&        is_instrumented ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "SkeletonPlugin: call to restartRequired()\n" );

    psc_errmsg( "SkeletonPlugin: restartRequired() not implemented\n" );
    throw 0;
    return false; // no restart required
}

/**
 * @brief Returns the status of the current search iteration.
 *
 * Returns true if the current search iteration is finished (by either the plugin itself or as
 * a result of the search algorithm), false otherwise.
 *
 * @ingroup SkeletonPlugin
 *
 * @return true if the plugin has finished search false otherwise
 *
 */
bool SkeletonPlugin::searchFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "SkeletonPlugin: call to searchFinished()\n" );

    psc_errmsg( "SkeletonPlugin: searchFinished() not implemented\n" );
    throw 0;
    return true;
}

/**
 * @brief Final operation of a tuning step.
 *
 * If any post-processing is necessary before entering the next tuning iteration, it is to be
 * done here.
 *
 * @ingroup SkeletonPlugin
 *
 */
void SkeletonPlugin::finishTuningStep( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "SkeletonPlugin: call to processResults()\n" );

    psc_errmsg( "SkeletonPlugin: processResults() not implemented\n" );
    throw 0;
}

/**
 * @brief Returns true if the plugin finished the tuning process, false otherwise.
 *
 * @ingroup SkeletonPlugin
 *
 * @return true if the plugin has finished tuning false otherwise
 *
 */
bool SkeletonPlugin::tuningFinished( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "SkeletonPlugin: call to tuningFinished()\n" );

    psc_errmsg( "SkeletonPlugin: tuningFinished() not implemented\n" );
    throw 0;
    return true;
}

/**
 * @brief Prints to the screen (and to a file, where necessary) the tuning advice.
 *
 * @ingroup SkeletonPlugin
 */
Advice* SkeletonPlugin::getAdvice( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "SkeletonPlugin: call to getAdvice()\n" );

    psc_errmsg( "SkeletonPlugin: getAdvice() not implemented\n" );
    throw 0;
    // return new Advice();
}

/**
 * @brief Finalize the plugin normally.
 *
 * Remove any allocated memory, objects, file descriptors, etc.
 *
 * @ingroup SkeletonPlugin
 *
 */
void SkeletonPlugin::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "SkeletonPlugin: call to finalize()\n" );

    psc_errmsg( "SkeletonPlugin: finalize() not implemented\n" );
    throw 0;
}

/**
 * @brief Terminate the plugin due to error.
 *
 * Safely remove any allocated memory, objects, file descriptors, etc. This method should
 * be able to be executed safely at any point.
 *
 * @ingroup SkeletonPlugin
 *
 */
void SkeletonPlugin::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "SkeletonPlugin: call to terminate()\n" );

    psc_errmsg( "SkeletonPlugin: terminate() not implemented\n" );
    throw 0;
}

/*
 * The following are C functions that belong to the Plugin Management Interface, and not to the
 * IPlugin class. Currently the interface includes information and instance creation functions.
 *
 * These are defined in PluginManagement.h .
 */

/**
 * @brief Returns an instance of this particular plugin implementation.
 *
 * Typically, a simple return with new is enough. For example:
 *
 * return a pointer to a new SkeletonPlugin();
 *
 * @ingroup SkeletonPlugin
 *
 * @return A pointer to a new SkeletonPlugin
 */
IPlugin* getPluginInstance( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "SkeletonPlugin: call to getComponentInstance()\n" );

    psc_errmsg( "SkeletonPlugin: getComponentInstance() not implemented\n" );
    throw 0;
    return ( IPlugin* )NULL;
}

/**
 * @brief Returns the major plugin interface version used by this plugin (example: the 1 in 1.0).
 *
 * @ingroup SkeletonPlugin
 *
 * @return The major plugin interface version used by the plugin
 */
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "SkeletonPlugin: call to getInterfaceVersionMajor()\n" );

    psc_errmsg( "SkeletonPlugin: getInterfaceVersionMajor() not implemented\n" );
    throw 0;
    return 1;
}

/**
 * @brief Returns the minor plugin interface version used by this plugin (example: the 0 in 1.0).
 *
 * @ingroup SkeletonPlugin
 *
 * @return The minor plugin interface version used by the plugin
 *
 */
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "SkeletonPlugin: call to getInterfaceVersionMinor()\n" );

    psc_errmsg( "SkeletonPlugin: getInterfaceVersionMinor() not implemented\n" );
    throw 0;
    return 0;
}

/**
 * @brief Returns a string with the name of the plugin.
 *
 * @ingroup SkeletonPlugin
 *
 * @return Returns a string with the name of the plugin
 *
 */
string getName( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "SkeletonPlugin: call to getName()\n" );

    psc_errmsg( "SkeletonPlugin: getName() not implemented\n" );
    throw 0;
    return "";
}

/**
 * @brief Returns a string with a short description of the plugin.
 *
 * @ingroup SkeletonPlugin
 *
 * @return A string with a short description of the plugin
 *
 */
string getShortSummary( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotunePlugins ), "SkeletonPlugin: call to getShortSummary()\n" );

    psc_errmsg( "SkeletonPlugin: getShortSummary() not implemented\n" );
    throw 0;
    return "";
}
