#ifndef REGION_H_
#define REGION_H_

#include <string>
#include <stdio.h>
#include <map>
#include <list>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
using namespace std;

/// Have to know whether a region is executed outside an OMP PARALLEL or inside
typedef enum RegionRunsAs {
    RUNS_AS_SERIAL   = 0,
    RUNS_AS_THREADED = 1
} RegionRunsAs;

/// Region types
typedef enum RegionType {
    //Sequential regions
    REGION_NONE       =   0,
    MAIN_REGION       =   1,
    SUB_REGION        =   2,
    LOOP_REGION       =   3,
    NESTEDLOOP_REGION =   4,
    VECTOR_REGION     =   5,
    FORALL_REGION     =   6,
    CALL_REGION       =   7,
    IO_REGION         =   8,
    //OpenMP regions
    BARRIER_REGION          =   9,
    PARALLEL_REGION         =  10,
    PARALLEL_REGION_BODY    =  11,
    DO_REGION               =  12,
    SECTIONS_REGION         =  13,
    SECTION_REGION          =  14,
    SECTION_REGION_BODY     =  15,
    SINGLE_REGION           =  16,
    SINGLE_REGION_BODY      =  17,
    CRITICAL_REGION         =  18,
    CRITICAL_REGION_BODY    =  19,
    WORKSHARE_REGION        =  20,
    WORKSHARE_REGION_BODY   =  21,
    ATOMIC_REGION           =  22,
    ORDERED_REGION          =  23,
    ORDERED_REGION_BODY     =  24,
    MASTER_REGION_BODY      =  25,
    IMPLICIT_BARRIER_REGION =  26,
    USER_REGION             =  27,
    WORKSHARE               =  28,
    WORKSHARE_SECTIONS      =  29,
    WORKSHARE_DO            =  30,
    //MPI regions (Not yet in MRI or EPC_Monitoring Library)
    MPI_CALL =  31,
    //Other regions
    TASK_REGION      =  32,
    TASK_REGION_BODY =  33,
    //Tune Region
    TUNE_REGION =  34,
    //OpenCL Region
    OPENCL_MEM_CMD_REGION     =  35,
    OPENCL_KERNEL_CALL_REGION =  36,
    //CUDA Region
    CUDA_MEM_TRANSFER_REGION =  37,
    CUDA_KERNEL_CALL_REGION  =  38,
    //Vienna pattern - pipeline
    VIE_PIPE_INBUFFER_REGION  =  39,
    VIE_PIPE_OUTBUFFER_REGION =  40,
    VIE_PIPE_STAGE_REGION     =  41,
    VIE_PIPELINE_REGION       =  42,
    UNSPECIFIED               =  43,
    UNKNOWN                   =  44,
    OMP_FLASH_REGION          =  45,
    DATA_STRUCTURE            = 100
} RegionType;


/**
 * Identifies a code region in the monitored application. Do NOT serialize this information and send it to
 * other agents! Some of the data stored is valid only on the local system, such as the Score-P region id.
 */
typedef struct RegionIdent {
    RegionType  type;
    int         file_id;
    int         rfl;
    int         start_position;
    int         end_position;
    std::string file_name;
    uint64_t    local_region_id;

    bool operator==( const struct RegionIdent& in ) const;

    bool operator!=( const struct RegionIdent& in ) const;
} RegionIdent;

typedef enum PluginIdEnum {
    PLUGIN_UNKNOWN = 100
}PluginIdType;

typedef enum TargetEnum {
    TARGET_OpenCL  =   1,
    TARGET_CUDA    =   2,
    TARGET_CPU     =   3,
    TARGET_UNKNOWN = 100
}TargetType;

class Target {
protected:
    TargetType TargetId;

public:
    Target();
    Target( TargetType targetId );
    ~Target();
};


class CodeVariant {
protected:
    std::string        Name;
    std::string        Group;
    std::list<Target*> Targets;

public:
    CodeVariant();

    CodeVariant( std::string name,
                 std::string group );

    ~CodeVariant();
};


typedef enum runtimeTuningActionEnum {
    TUNING_ACTION_NONE = 0,
    TUNING_ACTION_VARIABLE_INTEGER,
    TUNING_ACTION_VARIABLE_STRING,
    TUNING_ACTION_FUNCTION_POINTER
}runtimeTuningActionType;

/**
 * @class Plugin
 * @ingroup AnalysisAgent
 *
 * @brief Represents the application's tuned region plugin information
 *
 */
class Plugin {
protected:
    PluginIdType            PluginId;
    std::string             TuningActionName;
    runtimeTuningActionType TuningActionType;
    int                     NumberOfVariants;
    std::list<CodeVariant*> CodeVariants;
    int                     Rmin;
    int                     Rmax;
    int                     Rstep;

public:
    Plugin();
    Plugin( PluginIdType            pluginId = PLUGIN_UNKNOWN,
            std::string             tuningActionName = "",
            runtimeTuningActionType tuningActionType = TUNING_ACTION_NONE,
            int                     numberOfVariants = 0 );
    ~Plugin();

    void setPluginId( PluginIdType pluginId );

    PluginIdType getPluginId() const;

    void setTuningActionName( const std::string& tuningActionName );

    std::string getTuningActionName() const;

    runtimeTuningActionType getTuningActionType() const;

    void setNumberOfVariants( int numberOfVariants );

    int getNumberOfVariants() const;

    void setRmin( int rmin );

    int getRmin() const;

    void setRmax( int rmax );

    int getRmax() const;

    void setRstep( int rstep );

    int getRstep() const;
};




/**
 * @class Region
 * @ingroup AnalysisAgent
 *
 * @brief Represents a code region in the monitored application.
 *
 * Represents a code region in the monitored application. Do NOT serialize this information and send it to
 * other agents! Region objects contain local data, such as the Score-P region id. This information does
 * not make sense for a remote agent. A global region id can be used to exchange information about regions
 * in remote agents.
 */
class Region {
public:
    Region();
    Region( RegionType  t,
            int         fileId,
            int         rfl,
            int         startPosition = 0,
            int         endPosition = 0,
            std::string fileName = "",
            std::string regName = "",
            bool        is_sig = false);
    Region( uint64_t    local_region_key,
            RegionType  t,
            int         fileId,
            int         rfl,
            int         startPosition = 0,
            int         endPosition = 0,
            std::string fileName = "",
            std::string regName = "",
            bool        is_sig = false);

    std::string getRegionID() const;

    uint64_t getLocalRegionID() const;

    std::string getFileName() const;

    int getFirstLine() const;

    int getLastLine() const;

    void set_parent( Region* reg );

    void add_subregion( Region* reg );

    void set_ident( const RegionIdent id );

    void set_rra( const RegionRunsAs rra );

    void set_rra_recursive( const RegionRunsAs rra );

    void set_rra_recursive_to_RUNS_AS_THREADED( RegionRunsAs rra );

    void set_is_significant( bool is_sig );

    bool get_is_significant( ) const;

    RegionIdent get_ident() const;

    Region* get_parent() const;

    std::list< Region* >get_subregions() const;

    RegionType get_type();

    RegionRunsAs get_rra() const;

    void print_subregions( std::string token,
                           bool        print_ds = false );

    void print_subregions( std::string token,
                           std::map<int, std::string> fnames,
                           bool print_ds = false );

    void print( bool print_ds = false );

    //std::string str_print( bool print_ds = false );

    void print( std::map<int, std::string> fnames,
                bool print_ds = false );

    std::string str_print( bool print_ds = false );

    void set_name( const std::string nm );

    std::string get_name() const;

    long get_key();      //File_id_line_nr

    std::string getIdForPropertyMatching() {
        ///TODO: THIS IS A WORKAROUND, has to be refactored
        return getRegionID();
        sprintf( id_for_property_matching, "%d-%d", region_ident.file_id, region_ident.rfl );
        return string( id_for_property_matching );
    }

    std::list<Plugin*> plugins;

    bool operator==( const Region& in ) const;

    bool operator!=( const Region& in ) const;

    string toString( int indent, string indentation_character ) const {
        string base_indentation;
        for( int i = 0; i < indent; i++ ) {
            base_indentation.append( indentation_character );
        }

        stringstream temp;
        temp << base_indentation << "Region: " << name << endl;
        temp << base_indentation << indentation_character << "Region identity: (" << region_ident.file_id << "," << region_ident.type << "," << region_ident.rfl << ")" << endl;
        return temp.str();
    }

    string toString() const {
        return toString( 0, "\t" );
    }

    bool  				  is_significant; ///< If Region is significant or insignificant

private:
    std::string        name;           ///< Region name
    RegionIdent        region_ident;   ///< Ident of the region
    RegionRunsAs       region_runs_as; ///< ??
    std::list<Region*> subregions;     ///< Subregions of the current region
    Region*            parent;         ///< Parent region
    char               id_for_property_matching[ 100 ];
};


bool is_parallel_region( RegionType rt );

std::string region_type_to_name( RegionType rt );


#endif /* REGION_H_ */
