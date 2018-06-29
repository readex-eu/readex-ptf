/**
   @file    MPITCPlugin.h
   @ingroup MPITCPlugin
   @brief   MPIT Custom Plugin
   @author  Aras Atalar
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

/**
   @defgroup MPITCPlugin MPI-T Custom Plugin
   @ingroup AutotunePlugins
 */

#ifndef MPITC_PLUGIN_H_
#define MPITC_PLUGIN_H_

#include "AutotunePlugin.h"
// uncomment the line below if your plugin will load search algorithms
//#include "ISearchAlgorithm.h"
#include <string.h>
#include <boost/algorithm/string.hpp>
#include <boost/config.hpp>
#include <vector>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/cuthill_mckee_ordering.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/bandwidth.hpp>

using namespace boost;
using namespace std;


#define INTERNODE 0
#define INTRANODE 1

typedef enum tuning_step {
    TUNING_COLLECTIVES      = 0,
    TUNING_P2P              = 1,
    TUNING_COLLECTIVES_LAST = 2,
    TUNING_P2P_LAST         = 3,
    TUNING_MAPPING          = 4,
    TUNING_DONE             = 5
} tuning_step_t;


class MPITCPlugin : public IPlugin {
private:
    int coll_tp_count;
    int p2p_tp_count;

    int coll_tuning_passes;
    int p2p_tuning_passes;

    /*For each collective tuning parameter: 3 doubles are allocated
     * 1) Best algorithm execution time
     * 2) Best algorithm
     * 3) Number of variants evaluated (used to determine next variant)
     * */
    double* coll_best_value_time;

    /*Optimal Intra(inter)threshold and their execution time*/
    double p2p_best_value_time[ 4 ];

    vector<TuningParameter*> p2p_tuning_parameters;
    vector<TuningParameter*> coll_tuning_parameters;
    vector<TuningParameter*> tuningParameters;

    tuning_step_t tuning_step;
    list<Region*> mpi_coll_regions;

    Application app;

    double getRegionTime( int              scenario_id,
                          TuningParameter* tp );

    void createMapping( int proc_no );

    int collectiveRange( string  coll_name,
                         Region* phase_region,
                         Region* coll_region );

public:
    void initialize( DriverContext*   context,
                     ScenarioPoolSet* pool_set );

    void startTuningStep( void );

    bool analysisRequired( StrategyRequest** strategy );

    void createScenarios( void );

    void prepareScenarios( void );

    void defineExperiment( int               numprocs,
                           bool&             analysisRequired,
                           StrategyRequest** strategy );

    bool restartRequired( string* env,
                          int*    np,
                          string* cmd,
                          bool*   instrumented );

    bool searchFinished( void );

    void finishTuningStep( void );

    bool tuningFinished( void );

    Advice* getAdvice( void );

    void finalize( void );

    void terminate( void );
};

#endif
