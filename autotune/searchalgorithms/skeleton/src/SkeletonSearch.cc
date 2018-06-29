/**
   @file    SkeletonSearch.cc
   @ingroup SkeletonSearch
   @brief   Skeleton of a search algorithm
   @author  Author's name
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

#include <SkeletonSearch.h>

/**
 * @brief Constructor description
 * @ingroup SkeletonSearch
 *
 **/
SkeletonSearch::SkeletonSearch() : ISearchAlgorithm() {
}

/**
 * @brief Destructor description
 * @ingroup SkeletonSearch
 *
 **/
SkeletonSearch::~SkeletonSearch() {
}

/**
 * @brief Initialize description
 * @ingroup SkeletonSearch
 *
 * Long description.
 *
 */
void SkeletonSearch::initialize( DriverContext*, ScenarioPoolSet* ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "SkeletonSearch: call to initialize()\n" );

    psc_errmsg( "SkeletonSearch: initialize() not implemented\n" );
    throw 0;
}

/**
 * @brief Search Finished description
 * @ingroup SkeletonSearch
 *
 * Long description.
 *
 */
bool SkeletonSearch::searchFinished() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "SkeletonSearch: call to searchFinished()\n" );

    psc_errmsg( "SkeletonSearch: searchFinished() not implemented\n" );
    throw 0;
}

/**
 * @brief Get Optimum description
 * @ingroup SkeletonSearch
 *
 * Long description.
 *
 */
int SkeletonSearch::getOptimum() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "SkeletonSearch: call to getOptimum()\n" );

    psc_errmsg( "SkeletonSearch: getOptimum() not implemented\n" );
    throw 0;
}

/**
 * @brief Clear description
 * @ingroup SkeletonSearch
 *
 * Long description.
 *
 */
void SkeletonSearch::clear() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "SkeletonSearch: call to clear()\n" );

    psc_errmsg( "SkeletonSearch: clear() not implemented\n" );
    throw 0;
}

/**
 * @brief Get Search Path description
 * @ingroup SkeletonSearch
 *
 * Long description.
 *
 */
map<int, double> SkeletonSearch::getSearchPath() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "SkeletonSearch: call to getSearchPath()\n" );

    psc_errmsg( "SkeletonSearch: getSearchPath() not implemented\n" );
    throw 0;
}

/**
 * @brief Add Search Space description
 * @ingroup SkeletonSearch
 *
 * Long description.
 *
 */
void SkeletonSearch::addSearchSpace( SearchSpace* ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "SkeletonSearch: call to addSearchSpace()\n" );

    psc_errmsg( "SkeletonSearch: addSearchSpace() not implemented\n" );
    throw 0;
}

/**
 * @brief Create Scenarios description
 * @ingroup SkeletonSearch
 *
 * Long description.
 *
 */
void SkeletonSearch::createScenarios() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "SkeletonSearch: call to createScenarios()\n" );

    psc_errmsg( "SkeletonSearch: createScenarios() not implemented\n" );


    if (objectiveFunctions.size()==0){
   	 addObjectiveFunction(new PTF_minObjective(""));
    }


    throw 0;
}

/**
 * @brief Terminate description
 * @ingroup SkeletonSearch
 *
 * Long description.
 *
 */
void SkeletonSearch::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "SkeletonSearch: call to terminate()\n" );

    psc_errmsg( "SkeletonSearch: terminate() not implemented\n" );
    throw 0;
}

/**
 * @brief Finalize description
 * @ingroup SkeletonSearch
 *
 * Long description.
 *
 */
void SkeletonSearch::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "SkeletonSearch: call to finalize()\n" );

    psc_errmsg( "SkeletonSearch: finalize() not implemented\n" );
    throw 0;
    //terminate();
}

/**
 * @brief Get Search Algorithm Instance description
 * @ingroup SkeletonSearch
 *
 * Long description.
 *
 */
ISearchAlgorithm* getSearchAlgorithmInstance( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "SkeletonSearch: call to getSearchAlgorithmInstance()\n" );

    psc_errmsg( "SkeletonSearch: getSearchAlgorithmInstance() not implemented\n" );
    throw 0;
    //return new SkeletonSearch();
}

/**
 * @brief Return the current major version number.
 * @ingroup SkeletonSearch
 *
 **/
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "SkeletonSearch: call to getVersionMajor()\n" );

    psc_errmsg( "SkeletonSearch: getVersionMajor() not implemented\n" );
    throw 0;
    //return 1;
}

/**
 * @brief Return the current minor version number.
 * @ingroup SkeletonSearch
 *
 **/
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "SkeletonSearch: call to getVersionMinor()\n" );

    psc_errmsg( "SkeletonSearch: getVersionMinor() not implemented\n" );
    throw 0;
    //return 0;
}

/**
 * @brief Return the name of the search algorithm.
 * @ingroup SkeletonSearch
 *
 **/
string getName( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "SkeletonSearch: call to getName()\n" );

    psc_errmsg( "SkeletonSearch: getName() not implemented\n" );
    throw 0;
    //return "Skeleton Search";
}

/**
 * @brief Return a short description.
 * @ingroup SkeletonSearch
 *
 **/
string getShortSummary( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "SkeletonSearch: call to getShortSummary()\n" );

    psc_errmsg( "SkeletonSearch: getShortSummary() not implemented\n" );
    throw 0;
    //return "Skeleton of an Autotune search algorithm class.";
}
