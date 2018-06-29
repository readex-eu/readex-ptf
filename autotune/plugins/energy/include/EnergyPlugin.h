#ifndef ENERGY_PLUGIN_H_
#define ENERGY_PLUGIN_H_

#include "AutotunePlugin.h"
#include "ISearchAlgorithm.h"
#include <string>




/** Implements a plug-in that determines the optimal number of MPI processes for an application. */
class EnergyPlugin : public IPlugin {
public:
    EnergyPlugin();

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
    void parseCPUFreqs();
    void writeResults();
    void writeResults(std::ofstream& resultFile, Scenario&);
    void writeResults(std::ofstream& resultFile, Scenario&, TuningSpecification&);
    void writeResults(std::ofstream& resultFile, Scenario&, const std::map<std::string, double>& results);
    void cleanup();

    Application&      app;
    TuningParameter*  numberOfProcsTP;
    TuningParameter*  numberOfThreadsTP;
    TuningParameter*  frequencyTP;
    ISearchAlgorithm* searchAlgorithm;
    int               nextScenarioNumProcs;
    int               minFreq;
    int               maxFreq;
    int               freqStep;

    static const std::string ENERGY;
    static const std::string TIME;
    static double objectiveFunction_EDP( int scenario_id,
                                         ScenarioResultsPool* );
};

#endif
