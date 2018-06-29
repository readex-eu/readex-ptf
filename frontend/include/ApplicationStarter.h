#ifndef APPLICATION_STARTER_H_
#define APPLICATION_STARTER_H_

#include "regxx.h"
#include <list>
#include <string>




/**
 * @class ApplicationStarter
 * @ingroup Frontend
 * @brief The application starter provides an interface for Periscope to start and re-start applications.
 */
class ApplicationStarter {
public:
    ApplicationStarter();
    ~ApplicationStarter();

    void runAgents();
    void runApplication();
    void rerunApplication();

private:
    struct AgentDetails {
        char          tag[ 100 ];
        char          host[ 100 ];
        char          port[ 100 ];
        char          parent[ 100 ];
        char          applIds[ 4000 ];
        char          children[ 4000 ];
        AgentDetails* nextAgent;
    };

    struct LevelInfo {
        int                  numberOfAgents;
        struct AgentDetails* agents;
        LevelInfo*           nextLevel;
        LevelInfo*           prevLevel;
    };

    typedef std::string (*StarterPluginFunction)(char*);

    StarterPluginFunction loadPluginFunction( const char* );
    void startApplication( bool isFirstStart );
    LevelInfo* computeAgentHierarchy();
    void runHlAgent( AgentDetails* agent );
    void runAnalysisAgent( AgentDetails* agent );
    void printAgentHierarchy( LevelInfo* levels ) const;
    void instrumentRequiredRegions();

    int                    processes;
    int                    idmap_f[ 8192 ];
    int                    idmap_t[ 8192 ];
    int                    idmap_h_num[ 19 ];
    int                    idmap_h[ 19 ][ 8192 ];
    std::list< EntryData > processList;
    void*                  pluginHandle;
};

#endif /* APPLICATION_STARTER_H_ */
