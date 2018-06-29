/**
   @file    application.h
   @ingroup Communication
   @brief   Analysis agent application header
   @author
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2013, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */
#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "Region.h"
#include "readex_configuration.h"
#include <boost/noncopyable.hpp>
#include <boost/property_tree/ptree.hpp>
#include <list>
#include <map>
#include <unordered_map>
#include <stdint.h>
#include <string>

class Rts;

/**
 * Represents the monitored application. The main purpose of this class is to provide information
 * about the application, such as region information. The singleton instance of this class can be
 * retrieved globally and will never change.
 */
class Application : public boost::noncopyable {
public:
    /** Gets the singleton instance of this class. The instance will never change and never be destroyed. */
    static Application& instance();

private:
    std::string                    app_name;         ///< Name of the monitored application
    std::string                    app_param;        ///< Application's parameters
    std::map<std::string, Region*> code_regions;     ///< Instrumented regions
    std::map<int, std::string>     file_names;       ///< Mapping between File names and File IDs (old map)

    std::map<int, std::string>  strings_by_id;       ///< Mapping between strings and their IDs
    std::map<std::string, int>  ids_by_string;       ///< Mapping between string IDs and strings
    int                         last_string_id;      ///<unique file id, incremented by one and assigned to the newly added file
    std::map<uint64_t, Region*> region_by_regionKey; ///< Mapping between region key and Region objects

    std::list<Region*> sig_regions;                  ///< List of significant regions

    Region* phase_region;                            //Initialized to the region specified in the command line
    Region* main_region;
    int     mpiprocs, ompthreads;

    uint64_t hash_region_ident( int name_id,
                                int file_id,
                                int rfl );

    Rts* calltree_root;

    std::map<Rts*,std::string> appl_rtscallpath_mapping; ///< Mapping of the call-tree node to its call-path string

    Readex_Metrics energy_metrics;

    std::map<std::string,std::map<std::string,double> > default_energy_mapping;

    std::map< std::string, std::vector<std::string> > input_identifiers;

public:
    void printDefaultEnergy();

    void generateDefaultEnergyMapping( std::string callpath, double defaultEnergy, double totalInstr );

    std::map<std::string, std::map<std::string, double> > getDefaultEnergyMapping() { return default_energy_mapping; }

    int getIDbyString( std::string str );

    std::string getStringByID( int string_id );

    int addString( std::string str );

    Region* getRegionByKey( uint64_t region_id );

    uint64_t getRegionKeyByDescr( std::string region_name,
                                  std::string file_name,
                                  int         rfl );

    Region* getRegionByDescr( std::string region_name,
                              std::string file_name,
                              int         rfl );


    /** Retrieves the region given by the specified region id. */
    Region* getRegionByID( const std::string& id );

    /** Retrieves the rts given by the specified callpathid. */
    Rts* getRtsByCallpath( const std::string& call_path );

    /** Retrieves the region given by the specified region id. This method can also be used to test if a region is known at all. */
    Region* getRegionByID( const std::string& id,
                           bool               allowNull );

    Region* addSCOREPRegion( int         rfl,
                             std::string file_name,
                             std::string reg_name,
                             RegionType  region_type,
                             int         start_position,
                             int         end_position );

    void addRegion( const std::string& region_name,
                    int                rfl,
                    const std::string& file_name,
                    RegionType         type,
                    int                start,
                    int                end );

    void print_region_list();

    void print_application_tree();

    Region* searchRegion( int fileId,
                          int rfl );           // TODO: THIS FUNCTION IS DEPRECATED; LOOK UP REGIONS BY THEIR ID

    Region* searchRegion( const std::string& regionId );

//    std::list<Region*> searchRegions( const std::list<std::string>& names ); // search for the Region provided by the name of significant region name

    void markSignificantRegions( const std::list<std::string>& names ); // search for the Region provided by the name of significant region name

    std::list<Region*> get_sig_regions_list() const;

    std::string get_app_name() const;

    void set_app_name( const std::string name );

    std::string get_app_param() const;

    void set_app_param( const std::string param );

    Region* get_phase_region() const;

    Region* get_main_region();                     //main

    void mark_all_potential_parallel_regions(); //marks all regions as RUNS_AS_THREADED if they might be called from within an OMP parallel region

    std::list<Region*>get_regions() const;

    /** get the list of significant regions --  NOT USED */
    std::list<Region*>get_significant_regions() const;

    //Get the subroutine Region for a CALL. If there is more than one sub with the same Name
    //returns NULL
    Region* get_subroutine( std::string sub_nm );

    std::list <Region*>get_subroutines();

    void setMpiProcs( int n );

    int getMpiProcs();

    void setOmpThreads( int n );

    int getOmpThreads();

    void set_phase_region( Region* pr );

    Rts* getCalltreeRoot();

    void setCalltreeRoot( Rts* node );

    void construct_frontend_calltree( Rts* node );

    std::map<Rts*,std::string>& getRtsCallpathMapping();

    void set_energy_metrics( Readex_Metrics energy_metrics );

    Readex_Metrics& get_energy_metrics();

    void loadInputIdentifiers( std::string const& file_name );

    void printInputIdentifiers();

    void fillBestInputIdentifiers( std::unordered_map< std::string, std::string >& best_identifiers );
private:
    /** Only used by the static getter. */
    Application();

    /** The private destructor prevents deletion of the singleton instance. */
    ~Application();

    void add_region_in_list();
    void set_main_region( Region* mr );

    void addInputIdentifiers( boost::property_tree::ptree const& pt );

    static Application* singleInstance;

    static const std::string DEFAULT_REGION_NAME;
    static const std::string DEFAULT_FILE_NAME;
};

void extract_region_id( char* str,
                        int*  file_id,
                        int*  start_line_nr );

RegionType extract_region_type( char* str );

//Get the name of the Data Structure
std::string get_ds_name( void* n );

#endif //APPLICATION_H_
