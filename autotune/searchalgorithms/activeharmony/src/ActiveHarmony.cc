/**
   @file    ActiveHarmony.cc
   @ingroup ActiveHarmony
   @brief   Interface to a search algorithms of Active Harmony
   @author  Isaias Compres
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

#include "ActiveHarmony.h"

#include "hclient.h"

static int               scenarioCounter            = 0;
int                      hserver_pid                = 0;
hdesc_t*                 hd                         = NULL;
int                      current_value_from_hserver = 0;
int                      scenario_id_counter        = 0;
vector<TuningParameter*> tuningParameters;

/**
 * @brief Constructor
 * @ingroup ActiveHarmony
 *
 **/
ActiveHarmony::ActiveHarmony() {
}

/**
 * @brief Destructor
 * @ingroup ActiveHarmony
 *
 **/
ActiveHarmony::~ActiveHarmony() {
}

/**
 * @brief Initialize description
 * @ingroup ActiveHarmony
 *
 * Long description.
 *
 */
void ActiveHarmony::initialize( DriverContext*,
                                ScenarioPoolSet* ) {
    psc_dbgmsg( PSC_AUTOTUNE_ALL_DEBUG, "Starting the ActiveHarmony server\n" );

    int pid;

    // TODO uncomment and debug this (hserver starts but does not work) - Isaias
    //FILE* fp;
    //char result [ 1000 ];
    //fp = popen( "which hserver", "r" );
    //fread( result, 1, sizeof( result ), fp );
    //fclose( fp );

    //if ( ( pid = fork()) == 0 ) // child
    //{
    //  //if ( execv( "hserver", NULL ) < 0 )
    //  //{
    //  execv( "/home/compres/workspace/external/build/build-activeharmony/bin/hserver", NULL );
    //  perror( "ActiveHarmony server (hserver) not available in this system.\n" );
    //  throw PTF_SEARCH_ERROR( PROGRAM_NOT_FOUND );
    //}
    //else // parent
    //{
    //  hserver_pid = pid;
    //}

    hd = harmony_init( "AutoTune", HARMONY_IO_POLL );
    if( hd == NULL ) {
        psc_errmsg( "Failed to initialize a harmony session.\n" );
        throw PTF_SEARCH_ERROR( INITIALIZATION_ERROR );
    }
}

/**
 * @brief setObjective description
 * @ingroup ActiveHarmony
 *
 * Long description.
 *
 */
void ActiveHarmony::setObjective( int obj ) {
    objective = obj;
}

/**
 * @brief getObjective description
 * @ingroup ActiveHarmony
 *
 * Long description.
 *
 */
int ActiveHarmony::getObjective() {
    return objective;
}

/**
 * @brief searchFinished description
 * @ingroup ActiveHarmony
 *
 * Long description.
 *
 */
bool ActiveHarmony::searchFinished() {
    int    best_scenario;
    double best_value;
    double current;

    best_scenario = 0;
    best_value    = objectiveFunction( 0 );
    current       = best_value;

    for( int scenario_id = 1; scenario_id < srp->size(); scenario_id++ ) {
        current = objectiveFunction( scenario_id );
        if( best_value > current ) {
            best_scenario = scenario_id;
            best_value    = current;
        }
    }

    optimum              = best_scenario;
    path[ search_steps ] = best_value;
    search_steps++;

    // report the result to the harmony server
    if( harmony_report( hd, ( int )( best_value ) ) < 0 ) {
        psc_errmsg( "Failed to report performance to server.\n" );
        throw PTF_SEARCH_ERROR( CONNECTION_ERROR );
    }

    if( scenarioCounter < 10 ) {
        return false;
    }
    else {
        return true;
    }
}

/**
 * @brief getOptimum description
 * @ingroup ActiveHarmony
 *
 * Long description.
 *
 */
int ActiveHarmony::getOptimum() {
    return optimum;
}

/**
 * @brief getSearchPath description
 * @ingroup ActiveHarmony
 *
 * Long description.
 *
 */
map<int, double > ActiveHarmony::getSearchPath() {
    return results;
}

/**
 * @brief addSearchSpace description
 * @ingroup ActiveHarmony
 *
 * Long description.
 *
 */
void ActiveHarmony::addSearchSpace( SearchSpace* searchSpace ) {
    psc_dbgmsg( PSC_AUTOTUNE_ALL_DEBUG, "ActiveHarmony: adding search space\n" );

    tuningParameters = searchSpace->getVariantSpace()->getTuningParameters();

    if( tuningParameters.size() != 1 ) {
        psc_errmsg( "None or multiple tuning parameters in search space in exhaustive search defined. Exiting.\n" );
        throw 0;
    }

    if( harmony_register_int( hd, "current_value_from_hserver",
                              &current_value_from_hserver,
                              tuningParameters[ 0 ]->getRangeFrom(),
                              tuningParameters[ 0 ]->getRangeTo() - 1, 1 ) < 0 ) {
        psc_errmsg( "Could not register parameter in Harmony server\n" );
        throw 0;
    }

    if( harmony_connect( hd, NULL, 0 ) < 0 ) {
        psc_errmsg( "Could not connect to harmony server.\n" );
        throw PTF_SEARCH_ERROR( INITIALIZATION_ERROR );
    }

    searchSpaces.push_back( searchSpace );
}

/**
 * @brief createScenarios description
 * @ingroup ActiveHarmony
 *
 * Long description.
 *
 */
void ActiveHarmony::createScenarios() {
    psc_dbgmsg( PSC_AUTOTUNE_ALL_DEBUG, "[#### AUTOTUNE ####]: Starting the search\n" );

    ScenarioDescription*        scenario;
    Variant*                    v;
    map<TuningParameter*, int>* value;

    scenario = new ScenarioDescription();
    scenario->setId( scenarioCounter++ );
    value = new map<TuningParameter*, int>();

    psc_dbgmsg( PSC_AUTOTUNE_ALL_DEBUG, "value before calling the hserver: %d\n", current_value_from_hserver );
    int hresult = harmony_fetch( hd );
    if( hresult < 0 ) {
        psc_errmsg( "Failed to fetch values from server.\n" );
        throw 0;
    }
    else if( hresult == 0 ) {
        psc_errmsg( "No value received from hserver\n" );
        throw 0;
    }
    else if( hresult > 0 ) {
        psc_dbgmsg( PSC_AUTOTUNE_ALL_DEBUG, "NEW value received from hserver: %d\n",
                    current_value_from_hserver );
    }

    value->insert( make_pair( tuningParameters[ 0 ], current_value_from_hserver ) );
    v = new Variant( *value );
    scenario->setVariant( v );
    if( searchSpaces[ 0 ]->getRegions().size() != 1 ) {
        psc_errmsg( "None or multiple regions in exhaustive search defined. Exiting.\n" );
        throw 0;
    }
    scenario->setRegion( searchSpaces[ 0 ]->getRegions()[ 0 ] );

    scenario->setObjective( EXECTIME );
    scenario->setComment( "Scenario: " );
    csp->push( scenario );
    psc_dbgmsg( PSC_AUTOTUNE_ALL_DEBUG, "[#### AUTOTUNE ####]: Added Scenario(%d) with variant %d for tuning parameter %d to the scenario pool\n",
                scenario->getId(), scenarioCounter, tuningParameters[ 0 ]->getId() );
}

/**
 * @brief Terminates the search algorithm.
 * @ingroup ActiveHarmony
 *
 **/
void ActiveHarmony::terminate() {
    if( hd != NULL ) {
        if( harmony_disconnect( hd ) < 0 ) {
            psc_errmsg( "Failed to disconnect from harmony server.\n" );
            throw 0;
        }
    }

    if( hserver_pid != 0 ) {
        psc_dbgmsg( PSC_AUTOTUNE_ALL_DEBUG, "Killing the Harmony server with PID: %d...\n", hserver_pid );
        union sigval val;
        sigqueue( hserver_pid, SIGKILL, val );
    }
}

/**
 * @brief Finalizes the search algorithms by calling terminate().
 * @ingroup ActiveHarmony
 *
 **/
void ActiveHarmony::finalize() {
    terminate();
}

/**
 * @brief Returns a pointer to the search object.
 * @ingroup ActiveHarmony
 *
 **/
ISearchAlgorithm* getSearchAlgorithmInstance( void ) {
    return new ActiveHarmony();
}

/**
 * @brief Returns the current major version number.
 * @ingroup ActiveHarmony
 *
 **/
int getVersionMajor( void ) {
    return 1;
}

/**
 * @brief Returns the current minor version number.
 * @ingroup ActiveHarmony
 *
 **/
int getVersionMinor( void ) {
    return 0;
}

/**
 * @brief Returns the name of the search algorithm.
 * @ingroup ActiveHarmony
 *
 **/
string getName( void ) {
    return "ActiveHarmony";
}

/**
 * @brief Returns a short description.
 * @ingroup ActiveHarmony
 *
 **/
string getShortSummary( void ) {
    return "Nelder-Mead Simplex algorithm provided by ActiveHarmony.";
}
