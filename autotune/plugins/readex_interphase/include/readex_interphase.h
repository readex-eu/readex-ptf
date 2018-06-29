/**
   @file    readex_interphase.h
   @ingroup ReadexInterphasePlugin
   @brief   READEX interphase tuning plugin for inter-phase tuning
   @author  Madhura Kumaraswamy
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2018, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

/**
   @defgroup ReadexInterphasePlugin READEX InterPhase Plugin
   @ingroup AutotunePlugins
 */

#ifndef READEX_INTERPHASE_PLUGIN_H_
#define READEX_INTERPHASE_PLUGIN_H_

#include "AutotunePlugin.h"
#include <unordered_set>
#include <unordered_map>
#include <tuple>
#include <memory>

#include <boost/property_tree/ptree.hpp>

// uncomment the line below if your plugin will load search algorithms
//#include "ISearchAlgorithm.h"

class ReadexInterphasePlugin : public IPlugin {
        vector<TuningParameter*>        tuningParameters;
        ISearchAlgorithm*               searchAlgorithm;
        std::list<Region*>              code_significant_regions;
        std::vector<ObjectiveFunction*> objectives;
        int                             tuningStep;

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
        bool restartRequired( std::string& env,
                              int&         numprocs,
                              std::string& cmd,
                              bool&        instrumented );
        bool searchFinished( void );
        void finishTuningStep( void );
        bool tuningFinished( void );
        Advice* getAdvice( void );
        void finalize( void );
        void terminate( void );
        map<TuningParameter*, int> getTuningParameters(int scenario_id);

    private:
        int min_freq;
        int max_freq;
        int freq_step;
        int current_step;

        static double objectiveFunction_Energy( int scenario_id,
                                                ScenarioResultsPool* );
        int getDefaultTuningValue( std::string tp_name );

        void insertPhaseData( int scenario_id, ScenarioPoolSet* pool_set );

        void insertPhaseData( unsigned int&             curr_phase,
                              int&                      scenario_id,
                              std::vector<std::string>& tags,
                              std::vector<std::string>& obj_tags,
                              std::vector<double>&      phase_ident,
                              std::vector<double>&      phase_obj );

        void normalizeDefaultObjectives();
};

namespace interph {
#define UNCLASSIFIED -1
#define MIN_POINTS 3
#define NOISE -2
struct default_obj {
    double def_time;
    double def_node_energy;
    double def_cpu_energy;
    double def_edp;
    double def_ed2p;
};

struct rts_info {
    std::string                             callpath;
    std::map<std::string, double>           objValues;
    std::map<std::string,double>            defaultObjValues;
    std::unordered_map<std::string, double> normalizedObj;
    std::map<std::string, double>           phaseIdentifiers;
    std::map<std::string, double>           defaultPhaseIdentifiers;
};

struct phaseInfo {
    int scenarioID;
    std::map<TuningParameter*, int>         scenarioConfig;
    std::unordered_map<std::string, double> phaseIdentifiers;
    std::map<std::string, double>           objValues;
    std::unordered_map<std::string, double> normalizedObj;
    std::pair<double, double>               normalizedFeatures;
    bool                                    visited;
    std::vector<unsigned int>               neighbors;
    int                                     clusterID;
    std::unordered_map<std::string, double> defaultPhaseIdentifiers;
    std::map<std::string,double>            defaultObjValues;
    std::unordered_map<std::string,double>  normalizedDefaultObj;
    std::unordered_set<std::unique_ptr<rts_info> > rtsInfo;
    phaseInfo():visited(false),clusterID(UNCLASSIFIED){ }
};

std::map<unsigned int, phaseInfo*> dtaPhases;
std::unordered_map<unsigned int, double> normalizedCompIntensity;
std::unordered_map<unsigned int, double> normalizedBranchInstr;
/* Map containing the pair of best phase number(from which the config is obtained) for all phases, the phases belonging to a cluster,
 * best phase number for each rts for each cluster. In the form <Cluster_num,<phase number,phases_in_cluster>,<rts callpath,total_obj,phase number>>
 */
std::map<unsigned int,std::pair<unsigned int, std::set<unsigned int> > > configForClusters;
std::vector<std::unordered_map<std::string,std::pair<double,unsigned int> > > rts_best_objs;  //<rts_callpath,<obj_value,best phase config>>

void getMeasurements(list<MetaProperty> &properties, std::vector<double> &ident, std::vector<std::string> &tags, unsigned int &phase_iter );
void insertRtsData(int &scenario_id, ScenarioPoolSet* pool_set, std::map<unsigned int, phaseInfo*>::iterator phase_pos,
        std::list<Region*> &code_significant_regions, std::vector<ObjectiveFunction*> &objectives, std::vector<std::string> &tags,
        unsigned int &phase_iter, int tuningStep);
void calcComputeIntensity();
void normalizeIdentifiers();
void printPhaseData();
void normalizeObjectives(std::vector<ObjectiveFunction*> &objectives);
void computeBestConfigForCluster(std::vector<ObjectiveFunction*> objectives);
void insertTuningResultForNode(std::vector<ObjectiveFunction*> objectives);
void insertResultForNode(std::vector<ObjectiveFunction*> objectives);
void exportData(std::vector<TuningParameter*> tuningParameters, std::vector<ObjectiveFunction*> objectives);
void generateXML();
void createXMLFile();
boost::property_tree::ptree cluster_tree;

inline namespace clust {
unsigned int num_clusters;
void cluster();
std::map<std::pair<unsigned int, unsigned int>,double> dist_matrix;
std::vector<std::pair<unsigned int,double> > k_neigh_dist;
double calculateEps(std::vector<std::pair<unsigned int,double> > &k_nn_dist);
double calculateEps1(std::vector<std::pair<unsigned int,double> > &k_nn_dist);
double calculateEps2(std::vector<std::pair<unsigned int,double> > &k_nn_dist);
double calcEps(std::map<std::pair<unsigned int, unsigned int>,double> dist);
double findKNearestNeighbors(std::vector<double>& distances_from_point );
void calcDistanceMatrix(const std::map<unsigned int,phaseInfo*> pt_x );
int clusterPhases(std::map<unsigned int, phaseInfo*> &input_data);
std::vector<unsigned int> checkDistance(std::map<unsigned int, phaseInfo*>::iterator phase_i, std::map<std::pair<unsigned int,
        unsigned int>,double> dist, const double eps);
void expandCluster(std::vector<unsigned int>& neighbors, std::map<unsigned int, phaseInfo*> &phases,
        const double eps, const int &cluster_num, std::map<unsigned int, phaseInfo*>::iterator input_i);
void printClusters(std::map<unsigned int, phaseInfo*> &phases);

void exportClusters(const std::map<unsigned int, phaseInfo*> phases,unsigned int num_clusters);
void generateFile(const std::map<unsigned int, phaseInfo*> results, std::ofstream &result_file);
}  //namespace clust
}  //namespace interph

#endif
