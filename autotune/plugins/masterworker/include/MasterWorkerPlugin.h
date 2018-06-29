/**
   @file    MasterWorkerPlugin.h
   @ingroup MasterWorkerPlugin
   @brief   Demo Plugin
   @author  Toni Pimenta, Gervjola Saveta, Eduardo César, Ania Sikora
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
   @defgroup MasterWorkerPlugin Master-Worker Plugin
   @ingroup AutotunePlugins
 */

#ifndef MASTERWORKER_PLUGIN_H_
#define MASTERWORKER_PLUGIN_H_

#include "mwparam.h"
#include "AutotunePlugin.h"
// uncomment the line below if your plugin will load search algorithms
#include "ISearchAlgorithm.h"

class MasterWorkerPlugin : public IPlugin {
    vector<TuningParameter*> tuningParameters; ///< Vector of tuning parameters

    vector<MW_TP> conftps;                     ///< Vector of configuration parameters parsed from the file

    TuningParameter* numberOfWorkersTP;        ///< Variable for storing the values calculated in createScenarios ifor the number of workers and used in prepareScenarios and restartRequired

    // Variables for storing the cofiguration parameters
    MW_TP         mpitpnw, mpifunction, mpilatency, mpithreshold, mpilamda, mpitksz;
    vector<MW_TP> mpi_master_recv, mpi_worker_recv;

    int numberOfWorkers;     ///< Variable for storing the number of workers to be used when restarting the application
    int bestPartitionFactor; ///< Variable for storing the estimated partition factor

    VariantSpace variantSpace;
    SearchSpace  searchSpace;

    ISearchAlgorithm* searchAlgorithm;
    void extractTuningParametersFromConfigurationFile();

    int tuningStep;

public:
    void initialize( DriverContext*   context,
                     ScenarioPoolSet* pool_set );
    bool analysisRequired( StrategyRequest** strategy );
    void startTuningStep( void );
    void createScenarios( void );
    void prepareScenarios( void );
    void defineExperiment( int               numprocs,
                           bool&             analysisRequired,
                           StrategyRequest** strategy );
    bool restartRequired( std::string& env,
                          int&         numprocs,
                          std::string& command,
                          bool&        is_instrumented );
    bool searchFinished( void );
    void finishTuningStep( void );
    bool tuningFinished( void );
    Advice* getAdvice( void );
    void finalize( void );
    void terminate( void );

    const int getNumParameters() const {
        return tuningParameters.size();
    }
    const int getNumValues( int i ) const {
        return ( tuningParameters.at( i )->getRangeTo() - tuningParameters.at( i )->getRangeFrom()
                 + tuningParameters.at( i )->getRangeStep() ) / tuningParameters.at( i )->getRangeStep();
    }

private:
    int CalculatePi( int    total_bytes_transferred,
                     double total_comp_time,
                     double lambda,
                     double latency );                                                                       ///< Function for estimating the number of workers using the analytic model
    float
    pfsim( unsigned int num_w,
           unsigned int num_tasks,
           unsigned int avg_answer_size,
           unsigned int task_sz,
           float netl,
           float nets,
           float wmexct[] );                                                                                                                                 ///< Function for estimating the partition factor using analytical simulation
};




vector<MW_TP>getTuningParametersMW( const char* filename );

#endif
