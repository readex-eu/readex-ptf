#include "application.h"
#include "rts.h"
#include "readex_configuration.h"
#include "psc_errmsg.h"
#include "selective_debug.h"
#include "stringutil.h"
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <cstdio>
#include <iostream>
#include <list>
#include <sstream>
#include <boost/property_tree/json_parser.hpp>

const std::string Application::DEFAULT_REGION_NAME = "psc_reg_name_none";
const std::string Application::DEFAULT_FILE_NAME   = "psc_file_name_none";


Application* Application::singleInstance;


Application& Application::instance() {
    if( !singleInstance ) {
        singleInstance = new Application();
        std::cout <<  "Hint: Creating new application instance: "  <<  singleInstance  <<  std::endl;
    }

    return *singleInstance;
}


Application::Application() :
    mpiprocs( 0 ),
    ompthreads( 0 ),
    phase_region( NULL ),
    main_region( NULL ),
    last_string_id( 1 ) {
}


Application::~Application() {
}


std::string Application::get_app_name() const {
    return app_name;
}


void Application::set_app_name( const std::string name ) {
    app_name = name;
}


std::list<Region*> Application::get_regions() const {
    std::list<Region*> regions;

    typedef std::map<std::string, Region*>::const_iterator Iterator;
    for( Iterator i = code_regions.begin(); i != code_regions.end(); ++i ) {
        regions.push_back( i->second );
    }

    return regions;
}

std::list<Region*> Application::get_significant_regions() const{
    //NOT USED
    std::list<Region*> regions;

    typedef std::map<std::string, Region*>::const_iterator Iterator;
    for( Iterator i = code_regions.begin(); i != code_regions.end(); ++i ) {
        if( i->second->is_significant )
            regions.push_back( i->second );
    }

    return regions;

}


std::string Application::get_app_param() const {
    return app_param;
}


void Application::set_app_param( const std::string param ) {
    app_param = param;
}


Region* Application::searchRegion( int fileId, int rfl ) {
    // TODO: THIS FUNCTION IS DEPRECATED; LOOK UP REGIONS BY THEIR ID
    typedef std::map<std::string, Region*>::iterator Iterator;

    for( Iterator reg = code_regions.begin(); reg != code_regions.end(); ++reg ) {
        if( reg->second->get_ident().file_id == fileId   &&   reg->second->get_ident().rfl == rfl ) {
            return reg->second;
        }
    }

    return NULL;
}


Region* Application::searchRegion( const std::string& regionId ) {
    std::map<std::string, Region*>::iterator i = code_regions.find( regionId );
    return i == code_regions.end()  ?  NULL : i->second;
}


void Application::markSignificantRegions( const std::list<std::string>& names ) {
    std::map<std::string, Region*>::iterator it;
    sig_regions.clear();
    for( std::list<std::string>:: const_iterator i =  names.begin(); i != names.end(); ++i)
    {
        for( it = code_regions.begin(); it != code_regions.end(); ++it ) {
            if( strcasecmp(  it->second->get_name().c_str(), i->c_str() ) == 0 ) {
                it->second->is_significant = true;
                sig_regions.push_back( it->second );
            }
        }
    }
}


std::list<Region*> Application::get_sig_regions_list() const {
    return sig_regions;
}


Region* Application::get_phase_region() const {
    return phase_region;
}

void Application::set_phase_region( Region* region ) {
    if( region   &&   !getRegionByID( region->getRegionID(), true ) ) {
        psc_errmsg( "INVALID PARAMETER to Application::set_phase_region(): The region is unknown to the application (id = %s).\n", region->getRegionID().c_str() );
        abort();
    }

    phase_region = region;
}

void Application::set_main_region( Region* r ) {
    main_region = r;
}
Region* Application::get_main_region() {
    return main_region;
}


void Application::print_region_list() {
    typedef std::map<std::string, Region*>::iterator Iterator;

    std::cerr << '\n' << "The Complete list of regions:\n\n";
    for( Iterator it = code_regions.begin(); it != code_regions.end(); ++it ) {
        std::cerr << std::endl;
        it->second->print( true );
    }
    std::cerr << std::endl;
}

void Application::print_application_tree() {
    //print the main region tree
    main_region->print_subregions( " ", file_names );
    std::cerr << std::endl;

    //print the subroutines
    for( std::map<std::string, Region*>::iterator it = code_regions.begin(); it != code_regions.end(); ++it ) {
        if( it->second->get_ident().type == SUB_REGION ) {
            it->second->print_subregions( " ", file_names );
        }
    }

    std::cerr << std::endl;
}

std::list<Region*> Application::get_subroutines() {
    std::list<Region*>                       subprograms;
    std::map<std::string, Region*>::iterator it;

    subprograms.push_back( main_region );
    for( it = code_regions.begin(); it != code_regions.end(); ++it ) {
        if( it->second->get_ident().type == SUB_REGION ) {
            subprograms.push_back( it->second );
        }
    }

    return subprograms;
}


void Application::mark_all_potential_parallel_regions() {
    Region* mainRegion = get_main_region();

    if( mainRegion ) {
        //psc_dbgmsg( 1, "Propagating parallel execution information.\n");
        mainRegion->set_rra_recursive_to_RUNS_AS_THREADED( RUNS_AS_SERIAL );
        //psc_dbgmsg( 1, "Propagation of parallel execution information done.\n");
    }
}

Region* Application::get_subroutine( std::string sub_nm ) {
    typedef std::map<std::string, Region*>::iterator Iterator;

    for( Iterator it = code_regions.begin(); it != code_regions.end(); ++it ) {
        if( it->second->get_name() == sub_nm && it->second->get_ident().type == SUB_REGION ) {
            return it->second;
        }
    }

    return NULL;
}


void extract_region_id( char* str,
                        int*  file_id,
                        int*  rfl ) {
    char* str_reg_id;
    char* str_line_nr;
    char* token;

    str_reg_id  = str;
    token       = strchr( str, '-' );
    *token      = '\0';    //make the string compete by terminating with 0
    str_line_nr = ++token; // in a string "42-15" str_line_nr points now to "15"
    *file_id    = atoi( str_reg_id );
    *rfl        = atoi( str_line_nr );
    return;
}


void Application::setOmpThreads( int n ) {
    ompthreads = n;
}

int Application::getOmpThreads() {
    return ompthreads;
}

void Application::setMpiProcs( int n ) {
    mpiprocs = n;
}

int Application::getMpiProcs() {
    return mpiprocs;
}

/**
 * Returns the unique ID of the string
 *
 * @param str string
 * @return	unique ID of the string
 */
int Application::getIDbyString( std::string str ) {
    if( ids_by_string.count( str ) > 0 ) {
        return ids_by_string[ str ];
    }
    else {
        return -1;
    }
}

/**
 * Returns the string registered under the provided ID
 *
 * @param string_id id of the string
 * @return string
 */
std::string Application::getStringByID( int string_id ) {
    std::string result_val = "";
    if( strings_by_id.count( string_id ) > 0 ) {
        return strings_by_id[ string_id ];
    }
    else {
        return result_val;
    }
}

/**
 * Registers new string and assigns an unique id to it. If the string is already registered, its id is returned
 *
 * @param str string
 * @return unique ID of the string
 */
int Application::addString( std::string str ) {
    int string_id = getIDbyString( str );

    if( string_id != -1 ) {
        return string_id;
    }

    last_string_id++;

    while( strcmp( getStringByID( last_string_id ).c_str(), "" ) != 0 ) {
        ;
    }

    last_string_id++;
    strings_by_id[ last_string_id ] = str;
    ids_by_string[ str ]            = last_string_id;

    return last_string_id;
}

/**
 * Generates 64 bit integer id of the region specified by name id, file id and region first line
 *
 * @param name_id name id
 * @param file_id file id
 * @param rfl region first line
 * @return 64bit integer region id
 */
uint64_t Application::hash_region_ident( int name_id,
                                         int file_id,
                                         int rfl ) {
    //return ( ( uint64_t )file_id << 48 ) | ( ( uint64_t )name_id << 32) | ( uint64_t )rfl;
    uint64_t key = ( uint64_t )file_id * 10000000 + ( uint64_t )name_id * 10000 + ( uint64_t )rfl;
    //printf("key generated:%d,%d,%d -> %" PRIu64 "\n", name_id, file_id, rfl, key );
    return key;

    // key description: xxxx|yyy|zzzz : x-file_id, y-name_id, z-rfl
}

/**
 * Returns a pointer to the region object specified by its id
 *
 * @param region_id region id
 * @return pointer to the region object
 */
Region* Application::getRegionByKey( uint64_t region_id ) {
    if( region_by_regionKey.count( region_id ) > 0 ) {
        return region_by_regionKey[ region_id ];
    }
    else {
        return NULL;
    }
}

/**
 * Returns an id of the region specified by its name, file name and region first line
 *
 * @param region_name region name
 * @param file_name file name
 * @param rfl region first line
 * @return id of the region
 */
uint64_t Application::getRegionKeyByDescr( std::string region_name,
                                           std::string file_name,
                                           int         rfl ) {
    if( file_name.empty() ) {
        file_name = DEFAULT_FILE_NAME;
    }
    if( region_name.empty() ) {
        region_name = DEFAULT_REGION_NAME;
    }
    int file_id = getIDbyString( file_name );
    if( file_id == -1 ) {
        return 0;
    }
    int name_id = getIDbyString( region_name );
    if( name_id == -1 ) {
        return 0;
    }
    return hash_region_ident( name_id, file_id, rfl );
}

/**
 * Returns a pointer to the region object specified by its name, file name and region first line
 *
 * @param region_name region name
 * @param file_name file name
 * @param rfl region first line
 * @return pointer to the region object
 */
Region* Application::getRegionByDescr( std::string region_name,
                                       std::string file_name,
                                       int         rfl ) {
    if( file_name.empty() ) {
        file_name = DEFAULT_FILE_NAME;
    }
    if( region_name.empty() ) {
        region_name = DEFAULT_REGION_NAME;
    }
    uint64_t region_key = getRegionKeyByDescr( region_name, file_name, rfl );
    return getRegionByKey( region_key );
}


Region* Application::getRegionByID( const std::string& id ) {
    return getRegionByID( id, false );
}


Region* Application::getRegionByID( const std::string& id,
                                    bool               allowNull ) {
    typedef std::map<std::string, Region*>::iterator Iterator;

    Iterator i      = code_regions.find( id );
    Region*  result = ( i == code_regions.end() )  ?  NULL : i->second;

    if( result == NULL   &&   !allowNull ) {
        psc_errmsg( "Required code region not found: %s.  %d regions are known:\n", id.c_str(), code_regions.size() );
        for( Iterator i = code_regions.begin(); i != code_regions.end(); ++i ) {
            psc_errmsg( "    %s\n", i->first.c_str() );
        }

        abort();
    }

    return result;
}

/**
 * Creates a new region and registers it with the application. Also an unique id for
 * the region is generated which could be later used to reference the region. If the
 * region already registered, the existing region is returned.
 *
 * @param rfl region first line
 * @param file_name file name
 * @param reg_name name of the region
 * @param region_type region type
 * @param start_position first line of the region
 * @param end_position end line of the region.
 * @return id of the region
 */
Region* Application::addSCOREPRegion( int         rfl,
                                      std::string file_name,
                                      std::string reg_name,
                                      RegionType  region_type,
                                      int         start_position,
                                      int         end_position ) {
    if( file_name.empty() ) {
        file_name = DEFAULT_FILE_NAME;
    }
    if( reg_name.empty() ) {
        reg_name = DEFAULT_REGION_NAME;
    }

    Region* existing = getRegionByDescr( reg_name, file_name, rfl );
    if( existing ) {
        return existing;
    }

    int      name_id        = addString( reg_name );
    int      file_id        = addString( file_name );
    uint64_t new_region_key = hash_region_ident( name_id, file_id, rfl );
    Region*  new_region     = new Region( new_region_key, region_type, file_id, rfl, start_position, end_position, file_name, reg_name );
    code_regions.insert( std::make_pair( new_region->getRegionID(), new_region ) );
    region_by_regionKey[ new_region_key ] = new_region;

    Region* test = getRegionByID( new_region->getRegionID(), true );
    if( test != new_region ) {
        psc_errmsg( "Region with region_id %s was not stored correctly!\n", new_region->getRegionID().c_str() );
        abort();
    }

    return new_region;
}


void Application::addRegion( const std::string& region_name,
                             int                rfl,
                             const std::string& file_name,
                             RegionType         type,
                             int                start,
                             int                end ) {
    const std::string& regionName = region_name.empty()  ?  DEFAULT_REGION_NAME : region_name;
    const std::string& fileName   = file_name.empty()  ?  DEFAULT_FILE_NAME : file_name;

    int     file_id = addString( fileName );
    Region* region  = new Region( type, file_id, rfl, start, end, fileName, regionName );
    code_regions.insert( std::make_pair( region->getRegionID(), region ) );

    if( type == USER_REGION   &&   !phase_region ) {
        psc_dbgmsg( 1, "Setting phase region automatically: %s\n", region->getRegionID().c_str() );
        set_phase_region( region );
    }
}

Rts* Application::getCalltreeRoot() {
    return calltree_root;
}

void Application::setCalltreeRoot( Rts* node ) {
    if( node ) {
        calltree_root = node;
    }
}

void Application::construct_frontend_calltree( Rts* current ) {
    if( current->getParentScorepID() == 0) {
        if( !calltree_root ) {
            setCalltreeRoot( current );
        }
        else {
            calltree_root->reassignScorepID( calltree_root, current );
        }
        return;
    }
    calltree_root->insertFrontendNode( calltree_root, current );
}

void Application::generateDefaultEnergyMapping( std::string callpath, double defaultEnergy, double totalInstr ) {
    std::map<std::string, std::map<std::string,double> >::iterator energy_map_it = default_energy_mapping.find("Energy");
    std::map<std::string, std::map<std::string,double> >::iterator instr_map_it = default_energy_mapping.find("TotalInstr");

    if( energy_map_it != default_energy_mapping.end() && instr_map_it != default_energy_mapping.end() ) {
        std::map<std::string,double>::iterator energy_rts = energy_map_it->second.find(callpath);
        std::map<std::string,double>::iterator instr_rts = instr_map_it->second.find(callpath);
        if(energy_rts != energy_map_it->second.end() && instr_rts != instr_map_it->second.end()) {
            default_energy_mapping.at("Energy").at(callpath) += defaultEnergy;
            default_energy_mapping.at("TotalInstr").at(callpath) += totalInstr;
            return;
        }
        else {
            default_energy_mapping.at("Energy").insert(std::make_pair(callpath, defaultEnergy));
            default_energy_mapping.at("TotalInstr").insert(std::make_pair(callpath, totalInstr));
            return;
        }
    }
    default_energy_mapping["Energy"].insert(std::make_pair(callpath, defaultEnergy));
    default_energy_mapping["TotalInstr"].insert(std::make_pair(callpath, totalInstr));

    //    std::map<std::string,double> ::iterator entry = default_energy_mapping.find(callpath);
//    if( entry != default_energy_mapping.end() ) {
//        default_energy_mapping.at(callpath) += defaultEnergy;
//        return;
//    }
//    default_energy_mapping.insert(std::make_pair(callpath, defaultEnergy));
}


void Application::printDefaultEnergy() {
//    std::map<std::string,double> ::iterator mapping = default_energy_mapping.begin();
//        for(; mapping!=default_energy_mapping.end(); mapping++)
//            psc_dbgmsg( 1, "default energy mapping: %s, %f \n", mapping->first.c_str(),mapping->second);
        for(auto mapping : default_energy_mapping) {
            for(auto i : mapping.second)
            psc_dbgmsg( 4, "Default %s for rts %s : %f \n", mapping.first.c_str(),i.first.c_str(),i.second);
        }
}

std::map<Rts*,std::string>& Application::getRtsCallpathMapping() {
    return appl_rtscallpath_mapping;
}

void  Application::set_energy_metrics( Readex_Metrics _metrics ) {
    energy_metrics = _metrics;
}

Readex_Metrics& Application::get_energy_metrics()
{
    return energy_metrics;
}

void Application::addInputIdentifiers( boost::property_tree::ptree const& pt )
{
    using boost::property_tree::ptree;
    ptree::const_iterator end = pt.end();
    static string parameter_name;
    for ( ptree::const_iterator it = pt.begin(); it != end; ++it ) {
        string key   = it->first;
        string value = it->second.get_value< string >();

        //This should never happen
        if ( key.size() == 0 && value.size() == 0 )
        {
            psc_errmsg( "Error: Both the key and the value were empty! Investige how this could happen!" );
            abort();
        }
        //Array of values has a first node which contains only of a key
        if ( key.size() != 0 && value.size() == 0 )
        {
            parameter_name = key;
            input_identifiers[ parameter_name ];
        }
        //Array of values has later nodes which contains only of a value
        if ( key.size() == 0 && value.size() != 0 )
        {
            input_identifiers[ parameter_name ].push_back( value );
        }
        //Simple key-value pairs
        if ( key.size() != 0 && value.size() != 0 )
        {
            input_identifiers[ key ];
            input_identifiers[ key ].push_back( value );
        }

        addInputIdentifiers( it->second );
    }
}

void Application::loadInputIdentifiers( string const& file_name ) {
    boost::property_tree::ptree pt;

    read_json( file_name, pt );

    addInputIdentifiers( pt );

//    printInputIdentifiers();
}

void Application::printInputIdentifiers() {
    for( auto var_it : input_identifiers ) {
        cout << var_it.first << "=";
        for( auto val_it : var_it.second ) {
            cout << val_it << ",";
        }
        cout << endl;
    }
}

void Application::fillBestInputIdentifiers( std::unordered_map< std::string, std::string >& best_identifiers ) {
    for( auto var_it : input_identifiers ) {
        best_identifiers[ var_it.first ];
        best_identifiers[ var_it.first ] = var_it.second.front();
    }
}
