/**
   @file    ROMIOPlugin.h
   @ingroup Autotune
   @brief   ROMIO Plugin
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

#ifndef ROMIO_PLUGIN_H_
#define ROMIO_PLUGIN_H_

#include "AutotunePlugin.h"
//uncomment the line below if your plugin will load search algorithms
#include "ISearchAlgorithm.h"

#define TASKS_PER_NODE 16
#define MAX_COLLECTIVE_BUFFER   128       // 128MB
#define MAX_READ_DATA_SIEVING_BUFFER 128  // 128MB
#define MAX_WRITE_DATA_SIEVING_BUFFER 128 // 128MB

class ROMIOPlugin : public IPlugin {
    list<Region*>           regions;
    list<Region*>::iterator regionIterator;
    int                     diff_regions;
    int                     search_step1;
    int                     search_step2;
    int                     exist_collective_io;

    TuningParameter* tp_romio_cb_read;
    TuningParameter* tp_romio_cb_write;
    TuningParameter* tp_cb_nodes;
    TuningParameter* tp_cb_buffer_size;
    TuningParameter* tp_ind_rd_buffer_size;
    TuningParameter* tp_ind_wr_buffer_size;
    TuningParameter* tp_romio_ds_read;
    TuningParameter* tp_romio_ds_write;


    ISearchAlgorithm* searchAlgorithm;
    Application&      app;

public:
    ROMIOPlugin();

    void initialize( DriverContext*   context,
                     ScenarioPoolSet* pool_set );

    void startTuningStep( void );

    bool analysisRequired( StrategyRequest** strategy );

    void createScenariosForCollectiveReadInStep1( Region* regionPtr );

    void createScenariosForCollectiveWriteInStep1( Region* regionPtr );

    void createScenariosForCollectiveReadInStep2( Region* regionPtr );

    void createScenariosForCollectiveWriteInStep2( Region* regionPtr );

    int searchForTheBestCollectiveBufferSize( Region* regionPtr );

    void createScenariosForNonCollectiveRead( Region* regionPtr );

    void createScenariosForNonCollectiveWrite( Region* regionPtr );

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
};

#endif
