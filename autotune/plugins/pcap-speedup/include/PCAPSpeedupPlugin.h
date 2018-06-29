/**
   @defgroup PCAPSimplePlugin OpenMP Parallelism Capping Plugin
   @ingroup AutotunePlugins
 */

#ifndef PCAP_SIMPLE_PLUGIN_H_
#define PCAP_SIMPLE_PLUGIN_H_

#include "AutotunePlugin.h"




class PCAPSpeedupPlugin : public IPlugin {
public:
    PCAPSpeedupPlugin();

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
    void addRegionToSearchAlgorithm(double execTime);
    void parse_opts( int argc, char* argv[] );
    void parseConfigFile();
    void writePCAPresults();

    Application& app;
    std::string  searchAlgorithmName;
    bool         has_pcap_config;
    std::string  pcap_config_file;
    std::map<string, Region*>               code_region_candidates;
    std::vector<std::pair<string, double> > regionExectimeMap;
    Region*                                 selected_region;
    bool                                    firstTuningStep;
    double                                  firstCutOff;
    double                                  secondCutOff;
    int                                     minNoRegions;
    int                                     maxNoRegions;
    ISearchAlgorithm*                       searchAlgorithm;
    TuningParameter*                        numberOfThreadsTP;
};

#endif
