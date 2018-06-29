#include "Region.h"
#include "application.h"

extern Application* appl;

//Implementation of class Region
//
Region::Region() {
    region_ident.type            = UNSPECIFIED;
    region_ident.file_id         = 0;
    region_ident.rfl             = 0;
    region_ident.start_position  = 0;
    region_ident.end_position    = 0;
    region_ident.local_region_id = 0;
    region_runs_as               = RUNS_AS_SERIAL; //default
    is_significant               = 0;
    parent                       = NULL;
}

Region::Region( RegionType t, int fileId, int rfl, int startPosition, int endPosition, std::string fileName, std::string regName, bool is_sig ) {
    region_ident.type            = t;
    region_ident.file_id         = fileId;
    region_ident.rfl             = rfl;
    region_ident.start_position  = startPosition;
    region_ident.end_position    = endPosition;
    region_ident.file_name       = fileName;
    region_ident.local_region_id = 0;
    region_runs_as               = RUNS_AS_SERIAL; //default
    std::string tmp 					= regName;
    is_significant					= is_sig;

    //std::transform (tmp.begin(),tmp.end(), tmp.begin(), ::tolower);
    name   = tmp;
    parent = NULL;
}
Region::Region( uint64_t local_region_key, RegionType t, int fileId, int rfl,
                int startPosition, int endPosition, std::string fileName,
                std::string regName, bool is_sig ) {
    region_ident.type            = t;
    region_ident.file_id         = fileId;
    region_ident.rfl             = rfl;
    region_ident.start_position  = startPosition;
    region_ident.end_position    = endPosition;
    region_ident.file_name       = fileName;
    region_ident.local_region_id = local_region_key;
    region_runs_as               = RUNS_AS_SERIAL; //default
    is_significant					= is_sig;
    std::string tmp = regName;
    //std::transform (tmp.begin(),tmp.end(), tmp.begin(), ::tolower);
    name   = tmp;
    parent = NULL;
}


std::string Region::getRegionID() const {
    std::stringstream region_id_stream;
    region_id_stream <<  region_ident.file_name  <<  "*"  <<  name  <<  "*"  <<  region_ident.rfl;
    return region_id_stream.str();
}

uint64_t Region::getLocalRegionID() const {
    return region_ident.local_region_id;
}

std::string Region::getFileName() const {
    return region_ident.file_name;
}

int Region::getFirstLine() const {
    return region_ident.start_position;
}

int Region::getLastLine() const {
    return region_ident.end_position;
}

void Region::set_parent( Region* reg ) {
    parent = reg;
}

void Region::add_subregion( Region* reg ) {
    subregions.push_back( reg );
}

void Region::set_ident( const RegionIdent id ) {
    region_ident = id;
}

void Region::set_rra( const RegionRunsAs rra ) {
    region_runs_as = rra;
}

void Region::set_rra_recursive( const RegionRunsAs rra ) {
    region_runs_as = rra;
    for( std::list<Region*>::iterator rit = subregions.begin(); rit != subregions.end(); rit++ ) {
        ( *rit )->set_rra_recursive( rra );
    }
}

void Region::set_rra_recursive_to_RUNS_AS_THREADED( RegionRunsAs rra ) {
    if( is_parallel_region( this->get_type() ) ) {
        rra = RUNS_AS_THREADED;
    }
    if( region_runs_as != RUNS_AS_THREADED ) {
        region_runs_as = rra;
    }

    for( std::list<Region*>::iterator rit = subregions.begin(); rit != subregions.end(); rit++ ) {
        Region* reg = ( *rit );

        reg->set_rra_recursive_to_RUNS_AS_THREADED( rra );

        if( reg->get_type() == CALL_REGION && rra == RUNS_AS_THREADED ) {
            //psc_dbgmsg( 1, "SET RUNS_AS_THREADED call to %s  %x\n", reg->get_name().c_str(),appl);
            Region* calledSubroutine = appl->get_subroutine( reg->get_name() );
            //psc_dbgmsg( 1, "Called subroutine found %s\n", calledSubroutine->get_name().c_str());
            if( calledSubroutine != NULL && calledSubroutine->get_rra() != RUNS_AS_THREADED ) {
                //psc_dbgmsg( 1, "SET RUNS_AS_THREADED subroutine %s\n", calledSubroutine->get_name().c_str());
                calledSubroutine->set_rra_recursive_to_RUNS_AS_THREADED( RUNS_AS_THREADED );
            }
        }
    }
}


RegionIdent Region::get_ident() const {
    return region_ident;
}

Region* Region::get_parent() const {
    return parent;
}

std::list< Region* > Region::get_subregions() const {
    return subregions;
}

RegionType Region::get_type() {
    return region_ident.type;
}

RegionRunsAs Region::get_rra() const {
    return region_runs_as;
}

void Region::set_name( const std::string nm ) {
    std::string tmp = nm;
    //std::transform (tmp.begin(),tmp.end(), tmp.begin(), ::tolower);
    name = tmp;
}

std::string Region::get_name() const {
    return name;
}

void Region::set_is_significant( bool is_sig ){
	is_significant=is_sig;
}

bool Region::get_is_significant( ) const {
	return is_significant;
}



void Region::print_subregions( std::string token, bool print_ds ) {
    if( ( region_ident.type == DATA_STRUCTURE || region_ident.type == VECTOR_REGION ) && !print_ds ) {
        return;
    }
    std::cerr << token << "|_"
              << ( ( region_runs_as == RUNS_AS_SERIAL ) ? "S " : "P " )
              << ( ( name.length() > 0 ) ? name : "" ) << "( "
              << region_type_to_name( region_ident.type ) << ", "
              << region_ident.file_id << ", "
              << region_ident.rfl
    //<< ", " << region_ident.end_position
              << " )"
              << std::endl;
    token += "  ";      //Lower Level
    for( std::list<Region*>::iterator rit = subregions.begin(); rit != subregions.end(); rit++ ) {
        ( *rit )->print_subregions( token, print_ds );
    }
}

void Region::print_subregions( std::string token, std::map<int, std::string> fnames, bool print_ds ) {
    if( ( region_ident.type == DATA_STRUCTURE || region_ident.type == VECTOR_REGION ) && !print_ds ) {
        return;
    }
    std::cerr << token << "|_"
              << ( ( region_runs_as == RUNS_AS_SERIAL ) ? "S " : "P " )
              << ( ( name.length() > 0 ) ? name : "" ) << "( "
              << region_type_to_name( region_ident.type ) << ", "
              << region_ident.file_name << ", "
              << region_ident.rfl
    //<< ", " << region_ident.end_position
              << " )"
              << std::endl;
    token += "  ";      //Lower Level
    for( std::list<Region*>::iterator rit = subregions.begin(); rit != subregions.end(); rit++ ) {
        ( *rit )->print_subregions( token, fnames, print_ds );
    }
}

void Region::print( bool print_ds ) {
    if( ( region_ident.type == DATA_STRUCTURE || region_ident.type == VECTOR_REGION ) && !print_ds ) {
        return;
    }
    std::cout << ( ( region_runs_as == RUNS_AS_SERIAL ) ? "S " : "P " )
              << ( ( name.length() > 0 ) ? name : "" ) << "( "
              << region_type_to_name( region_ident.type ) << ", "
              << region_ident.file_id << ", "
              << region_ident.rfl
    //<< ", "  << region_ident.end_position
              << " )";
}

//  std::string Region::str_print(bool print_ds)
//  {
//    std::ostringstream ost;
//    if((region_ident.type == DATA_STRUCTURE || region_ident.type == VECTOR_REGION) && !print_ds)
//      return ost.str();
//    ost   << ((region_runs_as == RUNS_AS_SERIAL)? "S ":"P ")
//      << ((name.length() > 0)? name: "") << "( "
//      << region_type_to_name(region_ident.type) << ", "
//      << region_ident.file_id << ", "
//      << region_ident.rfl
//      //<< ", "  << region_ident.end_position
//      << " )";
//    return ost.str();
//  }

void Region::print( std::map<int, std::string> fnames, bool print_ds ) {
    if( ( region_ident.type == DATA_STRUCTURE || region_ident.type == VECTOR_REGION ) && !print_ds ) {
        return;
    }
    std::cerr <<  "( " << region_type_to_name( region_ident.type ) << ", "
              << region_ident.file_name << ", "
              << region_ident.rfl
    //<< ", " << region_ident.end_position
              << " )";
}

std::string Region::str_print( bool print_ds ) {
    std::ostringstream ost;
    if( ( region_ident.type == DATA_STRUCTURE || region_ident.type == VECTOR_REGION ) && !print_ds ) {
        return ost.str();
    }
    ost <<   "( "
        << region_type_to_name( region_ident.type ) << ", "
        << region_ident.file_name << ", "
        << region_ident.start_position
        << " )";
    return ost.str();
}

long Region::get_key() {
    long  file_id_line_nr;
    float temp;

    file_id_line_nr = ( long )( region_ident.file_id );
    temp            = ( float )( region_ident.rfl );

    while( temp >= 1 ) {
        temp            /= 10;
        file_id_line_nr *= 10;
    }

    file_id_line_nr += ( long )( region_ident.rfl );

    return file_id_line_nr;
}

bool is_parallel_region( RegionType rt ) {
    return ( rt == PARALLEL_REGION || rt == DO_REGION || rt == SECTIONS_REGION || rt == WORKSHARE_REGION || rt == TASK_REGION ) ? true : false;
}

std::string region_type_to_name( RegionType rt ) {
    switch( rt ) {
    case REGION_NONE:
        return "REGION_NONE";
        break;
    case MAIN_REGION:
        return "MAIN_REGION";
        break;
    case SUB_REGION:
        return "SUB_REGION";
        break;
    case LOOP_REGION:
        return "LOOP_REGION";
        break;
    case NESTEDLOOP_REGION:
        return "NESTEDLOOP_REGION";
        break;
    case VECTOR_REGION:
        return "VECTOR_REGION";
        break;
    case FORALL_REGION:
        return "FORALL_REGION";
        break;
    case CALL_REGION:
        return "CALL_REGION";
        break;
    case IO_REGION:
        return "IO_REGION";
        break;
    //OpenMP regions
    case BARRIER_REGION:
        return "BARRIER_REGION";
        break;
    case PARALLEL_REGION:
        return "PARALLEL_REGION";
        break;
    case PARALLEL_REGION_BODY:
        return "PARALLEL_REGION_BODY";
        break;
    case DO_REGION:
        return "DO_REGION";
        break;
    case SECTIONS_REGION:
        return "SECTIONS_REGION";
        break;
    case SECTION_REGION:
        return "SECTION_REGION";
        break;
    case SECTION_REGION_BODY:
        return "SECTION_REGION_BODY";
        break;
    case SINGLE_REGION:
        return "SINGLE_REGION";
        break;
    case SINGLE_REGION_BODY:
        return "SINGLE_REGION_BODY";
        break;
    case CRITICAL_REGION:
        return "CRITICAL_REGION";
        break;
    case CRITICAL_REGION_BODY:
        return "CRITICAL_REGION_BODY";
        break;
    case WORKSHARE_REGION:
        return "WORKSHARE_REGION";
        break;
    case WORKSHARE_REGION_BODY:
        return "WORKSHARE_REGION_BODY";
        break;
    case ATOMIC_REGION:
        return "ATOMIC_REGION";
        break;
    case TASK_REGION:
        return "TASK_REGION";
        break;
    case TASK_REGION_BODY:
        return "TASK_REGION_BODY";
        break;
    case ORDERED_REGION:
        return "ORDERED_REGION";
        break;
    case ORDERED_REGION_BODY:
        return "ORDERED_REGION_BODY";
        break;
    case MASTER_REGION_BODY:
        return "MASTER_REGION_BODY";
        break;
    case IMPLICIT_BARRIER_REGION:
        return "IMPLICIT_BARRIER_REGION";
        break;
    case USER_REGION:
        return "USER_REGION";
        break;
    case WORKSHARE:
        return "WORKSHARE";
        break;
    case WORKSHARE_SECTIONS:
        return "WORKSHARE_SECTIONS";
        break;
    case WORKSHARE_DO:
        return "WORKSHARE_DO";
        break;
    //MPI regions (Not yet in MRI or EPC_Monitoring Library)
    case MPI_CALL:
        return "MPI_CALL";
        break;
    //OpenCL regions
    case OPENCL_MEM_CMD_REGION:
        return "OPENCL_MEM_CMD_REGION";
        break;
    case OPENCL_KERNEL_CALL_REGION:
        return "OPENCL_KERNEL_CALL_REGION";
        break;
    //CUDA regions
    case CUDA_MEM_TRANSFER_REGION:
        return "CUDA_MEM_TRANSFER_REGION";
        break;
    case CUDA_KERNEL_CALL_REGION:
        return "CUDA_KERNEL_CALL_REGION";
        break;
    //VPattern regions
    case VIE_PIPE_INBUFFER_REGION:
        return "VIE_PIPE_INBUFFER_REGION";
        break;
    case VIE_PIPE_OUTBUFFER_REGION:
        return "VIE_PIPE_OUTBUFFER_REGION";
        break;
    case VIE_PIPE_STAGE_REGION:
        return "VIE_PIPE_STAGE_REGION";
        break;
    case VIE_PIPELINE_REGION:
        return "VIE_PIPELINE_REGION";
        break;
    //Other regions
    case UNSPECIFIED:
        return "UNSPECIFIED";
        break;
    case UNKNOWN:
        return "UNKNOWN";
        break;
    case DATA_STRUCTURE:
        return "DATA_STRUCTURE";
        break;
    default:
        return "";
        break;
    }
}
bool Region::operator==( const Region& in ) const {
    if( this == &in ) {
        return true;
    }

    if(
        name.compare( in.name )
        || region_ident != in.region_ident
        || region_runs_as != in.region_runs_as
        || subregions != in.subregions
        || parent != in.parent
        ) {
        return false;
    }

    return true;
}

bool Region::operator!=( const Region& in ) const {
    return !( *this == in );
}

bool RegionIdent::operator==( const struct RegionIdent& in ) const {
    if(
        type != in.type
        || file_id != in.file_id
        || rfl != in.rfl
        || start_position != in.start_position
        || end_position != in.end_position
        || file_name.compare( in.file_name )
        ) {
        return false;
    }
    return true;
}

bool RegionIdent::operator!=( const struct RegionIdent& in ) const {
    return !( *this == in );
}
