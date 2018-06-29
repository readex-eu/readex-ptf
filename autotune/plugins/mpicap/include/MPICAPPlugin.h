#ifndef MPI_PROCS_PLUGIN_H_
#define MPI_PROCS_PLUGIN_H_

#include "AutotunePlugin.h"
#include "ISearchAlgorithm.h"
#include <string>
#include <vector>




/** Implements a plug-in that determines the optimal number of MPI processes for an application. */
class MPICAPPlugin : public IPlugin {
public:
    MPICAPPlugin();

    void initialize( DriverContext*   context,
                     ScenarioPoolSet* pool_set );

    bool analysisRequired( StrategyRequest** strategy );

    void startTuningStep();

    void createScenarios();

    void prepareScenarios();

    void defineExperiment( int               numprocs,
                           bool&             analysisRequired,
                           StrategyRequest** strategy );

    bool restartRequired( std::string& env,
                          int&         numprocs,
                          std::string& command,
                          bool&        is_instrumented );

    bool searchFinished();

    void finishTuningStep();

    bool tuningFinished();

    Advice* getAdvice();

    void finalize();
    void terminate();

private:
    void writeResults();
    void cleanup();

    Application&      app;
    TuningParameter*  numberOfProcsTP;
    ISearchAlgorithm* searchAlgorithm;
    int               nextScenarioNumProcs;

    static const std::string ENERGY;
    static const std::string TIME;
    static double objectiveFunction_EDP( int scenario_id,
                                         ScenarioResultsPool* );
};

#endif
