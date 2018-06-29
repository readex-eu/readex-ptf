#ifndef DVFS_PLUGIN_H_
#define DVFS_PLUGIN_H_

#include "AutotunePlugin.h"
#include "ISearchAlgorithm.h"




typedef struct {
    int    scenarioId;
    int    fileId;
    string regionFileNameStartPos;
    int    freq;
    string governor;
    double time;
    double energy;
    double energydelay;
    double energynode;
    double powernode;
    double tco;

    int rfl;
} outputDVFS;


class DVFSPlugin : public IPlugin {
public:
    DVFSPlugin();

    /*
     * @brief Initialize the plugin's data structures.
     */
    void initialize( DriverContext*   context,
                     ScenarioPoolSet* pool_set );

    /*
     * @brief Preparation before starting a tuning step.
     */
    void startTuningStep( void );

    /*
     * @brief Defines a pre-analysis strategy.
     */
    bool analysisRequired( StrategyRequest** strategy );

    /*
     * @brief Population of the "Created Scenario Pool" (csp)
     */
    void createScenarios( void );

    /*
     * @brief Method to prepare the scenarios.
     */
    void prepareScenarios( void );

    /*
     * @brief Method to populate the Experiment Scenario Pool (esp)
     * for the next experiment.
     */
    void defineExperiment( int               numprocs,
                           bool&             analysisRequired,
                           StrategyRequest** strategy );

    /*
     * @brief Method to know if the restart procedure is required or not.
     */
    bool restartRequired( std::string& env,
                          int&         numprocs,
                          std::string& command,
                          bool&        is_instrumented );

    /*
     * @brief Method to know if the search procedure is finished or not.
     */
    bool searchFinished( void );

    /*
     * @brief The method performs the final operation of a tuning step.
     */
    void finishTuningStep( void );

    /*
     * @brief Returns true whether the plugin finished the tuning process.
     */
    bool tuningFinished( void );

    /*
     * @brief Prints the tuning advice.
     */
    Advice* getAdvice();

    /*
     * @brief prints advice to be used by enopt stand alone
     */
    void createEnoptAdvice( const map<string, outputDVFS*>& outputvalues,
                            map<string, outputDVFS*>&       optimum );

    /*
     * @brief optimize the power
     */
    void optimizeForPower( const map<string, outputDVFS*>& outputvalues,
                           map<string, outputDVFS*>&       optimum );

    /*
     * @brief optimize the power
     */
    void minimize( const map<string, outputDVFS*>& outputvalues,
                   map<string, outputDVFS*>&       optimum );

    /*
     * @brief Method to finalize the plugin normally.
     */
    void finalize( void );

    /*
     * @brief Standard termination of the plugin
     */
    void terminate( void );

    /*
     * @brief Energy predictor model
     */
    //int model(double, double, double, double, double, double);

    /*
     * @brief Energy predictor models
     */
    int model_prediction( float   cycles,
                          float   instructions,
                          float   cache2,
                          float   cache3,
                          float   eRef,
                          float   tRef,
                          float   pwrRef,
                          float** outvec,
                          float&  optFreq,
                          float&  optval );

    float model_compute( float* c,
                         float  ymin,
                         float  ymax,
                         float* invec );

    float model_energy1( float eRef,
                         float factor );

    float model_energy2( float pwrRef,
                         float tRef,
                         float factor1,
                         float factor2 );

    float model_delay( float pwrRef,
                       float tRef,
                       float factor1,
                       float factor2,
                       int   iexp );

    float model_TCO( float pwrRef,
                     float tRef,
                     float factor1,
                     float factor2 );

    float model_powercapping( float pwrRef,
                              float factor1 );

    float model_policy1( float freq1,
                         float freq2,
                         float factor1,
                         float factor2 );

    float model_policy2( float freq1,
                         float freq2,
                         float factor1,
                         float factor2 );

    float model_policy3( float pwrRef,
                         float factor1,
                         float factor2 );

    float model_policy4( float factor1,
                         float Freq );

    double getObjectiveFunctionValue( outputDVFS* dvfsData );

private:
    const float                   fRef;
    std::vector<TuningParameter*> tuningParameters;
    ISearchAlgorithm*             searchAlgorithm;
    Region*                       tuningRegion;
    Application&                  app;
    VariantSpace                  variantSpace;
    SearchSpace                   searchSpace;
    std::list<Region*>            suited_regions;
    bool                          define_params;
    int                           model_method;
    int                           set_freq_node;
    int                           freq_neighbours;

    static const int POWERCAP = 110;
};

#endif
