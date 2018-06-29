#include "ATPService.h"
#include "psc_errmsg.h"
#include <list>
#include <unordered_map>
#include <vector>
#include <algorithm>


//atpService* atp_srvc;

int atpService::startATPServer( RegistryService* registry ) {
    std::list< EntryData > results;
    EntryData              atp_query;
    atp_query.app = "ATPServer";
    atp_query.tag = "none";

    psc_init_start_time();

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "Trying to start ATP Server within timeout %f \n", get_atp_server_timeout () );
    int plugin_pid = fork();
    if ( plugin_pid == 0 ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL ( ApplTuningParameter ), "Starting ATP Server \n" );

        char*       cmd     = ( char * )malloc( 2000 * sizeof( char ) );
        const char* regHost = registry->get_reghost();
        const int   regPort = registry->get_regport();

        std::sprintf( cmd, "atpserver %s %d", regHost, regPort );
        int retVal = system( cmd );
        free( cmd );
        // Terminate the child process
        _exit( retVal );
        if ( retVal != 0 )
        {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "ATP Server Can not be started\n" );
            return -1;
        }
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL ( ApplTuningParameter ), "Started ATP Server \n" );
    }
    else if ( plugin_pid < 0 ) {
        psc_errmsg( "Error forking child process!\n" );
        abort();
    }
    sleep( 5 );
    while ( !is_timed_out() ) {
        if ( registry->query_entries( results, atp_query ) == -1 ) {
            psc_errmsg( "Error querying registry for ATP Server\n" );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "ATP Server is not registered in Registry Service \n" );
            sleep( 5 );
        }
        else {
            break;
        }
    }

    if ( results.size() == 1 ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ),
                    "Readex Tuning is connected to %d ATP Server\n",
                    results.size() );

        EntryData atps_ = results.front();
        port_           = atps_.port;
        atpServerID     = atps_.id;
        app_            = atps_.app;
        tag_            = atps_.tag;

        //entry should be the information in the registry for the current process
        host_ = atps_.node;
        sock_ = socket_client_connect( ( char* )atps_.node.c_str(), port_ );
        std::cout << "SOCKET #: " << sock_ <<endl;
        if ( sock_ == -1 ) {
            psc_abort( "Error: Unable to retrieve ATP Server entry from Registry!\n" );
            return -1;
        }
    }
    else {
        psc_errmsg( "Timeout querying registry for ATP Server |%s| and component |%s| \n", atp_query.tag.c_str(), atp_query.app.c_str() );
        return -1;
    }

    return 1;
}

std::unordered_map< std::string, std::vector< TuningParameter* > > atpService::getATPSpecs() {
    const char* req_message = "getatpspecs;\n";

    socket_write_line( sock_, req_message );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "Sending line to ATPServer: <%s> via socket# %d\n", req_message, sock_ );

    int length;
    sleep( 5 );
    /*Read number of domains, number of params & total number of param values*/
    int32_t param_num_details[ 3 ];
    if ( length = socket_blockread( sock_, ( char* )( param_num_details ), ( sizeof( int32_t ) * 3 ) ) != ( sizeof( int32_t ) * 3 ) ) {
        psc_errmsg( "wrong number of bytes read: %d!=%d\n", length, ( sizeof( int32_t ) * 3 ) );
        abort();
    }

    int32_t number_of_domains  = param_num_details[ 0 ];
    int32_t number_of_params   = param_num_details[ 1 ];
    int32_t total_param_values = param_num_details[ 2 ];

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ),
                "Received from ATP Server [%d] domains [%d] parameters and [%d] total param values\n",
                number_of_domains,
                number_of_params,
                total_param_values );

    //if ( number_of_params == 0 )
    //   return;

    //Read number of params per domain
    int32_t domain_details[ number_of_domains ];
    if ( length = socket_blockread( sock_, ( char* )( domain_details ), ( number_of_domains * sizeof( int32_t ) ) ) != ( number_of_domains * sizeof( int32_t ) ) ) {
        psc_errmsg( "wrong number of bytes read: %d!=%d\n", length, ( number_of_domains * sizeof( int32_t ) ) );
        abort();
    }
    for( int32_t i = 0; i < number_of_domains; i++ ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ),
                    "Domain[%d]: # of parameters[%d]\n", i, domain_details[ i ] );
    }

    // Read domain names

    int   buf_size = ( number_of_domains * 100 ) + 1;
    char* buf      = ( char* )malloc( buf_size );

    int ret = socket_read_line( sock_, buf, buf_size );
    if ( ret > 0 ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "Received domain names from ATPServer: <%s>\n", buf );
    }

    //split param names for more than 1 parameters
    char** domain_names = ( char** )malloc( number_of_domains );

    buf_size = ( number_of_params * 100 ) + 1;
    char* buf1 = ( char* )malloc( buf_size );

    //read names of the parameters
    ret = socket_read_line( sock_, buf1, buf_size );
    if ( ret > 0 ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "Received param names from ATPServer: <%s>\n", buf1 );
    }

    //split param names for more than 1 parameters
    char** param_names = ( char** )malloc( number_of_params );

    for( int32_t i = 0, k = 0; i < number_of_domains; i++ ) {
        domain_names[ i ] = ( char* )malloc( 101 );
        strncpy( domain_names[ i ], &buf[ i * 100 ], 100 );
        std::vector< std::string > paramNames_per_domain( domain_details[ i ] );
        std::string str = domain_names[ i ];

        if ( domain_details[ i ] == 0 ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "Domain #: %d Domain Name: %s Param Name: EMPTY\n", i, domain_names[ i ] );
            //k -= 1;
            continue;
        }
        for( int32_t j = 0; j < domain_details[ i ]; j++ ) {
            param_names[ k ] = ( char* )malloc( 101 );
            strncpy( param_names[ k ], &buf1[ k * 100 ], 100 );
            paramNames_per_domain.at( j ) = param_names[ k ];
            //paramNames_per_domain.push_back( param_names[ k ] );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "Domain #: %d Domain Name: %s Param Name: %s\n", i, domain_names[ i ], param_names[ k ] );
            k += 1;
            //delete param_names[ k ];
        }
        //store domain, TuningParameter name pair
        domain_paramNames_mapping_.insert( std::make_pair( domain_names[ i ], paramNames_per_domain ) );
    }
    //Read param details
    buf_size = ( ( number_of_params * 3 ) + total_param_values ) * sizeof( int32_t );
    int32_t* param_details = ( int32_t* )malloc( buf_size );

    if ( length = socket_blockread( sock_, ( char* )( param_details ), buf_size ) != buf_size ) {
        free( param_details );
        psc_errmsg( "wrong number of bytes read: %d!=%d\n", length, buf_size );
        abort();
    }

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "Received param details from ATPServer: \n" );

    std::unordered_map< std::string, std::vector< TuningParameter* > > atps;

    int idx = 0;
    for( int32_t i = 0, j = 0; i < number_of_params; i++, j += 100 ) {
        //param_names[ i ] = ( char* )malloc( 101 );
        //strncpy( param_names[ i ], &buf1[ j ], 100 );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "PARAMETER #: %d \n", i );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "PARAMETER NAME[%d]: %s \n", i, param_names[ i ] );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "PARAMETER [%d]: type - %d \n", i, param_details[ idx + 0 ] );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "PARAMETER [%d]: default_value - %d \n", i, param_details[ idx + 1 ] );
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "PARAMETER [%d]: max_values - %d \n", i, param_details[ idx + 2 ] );

        int num_values = ( param_details[ idx + 0 ] == 1) ? 3 : param_details[ idx + 2 ];

        TuningParameter* a_tp = new TuningParameter();
        a_tp->setName( param_names[ i ] );
        a_tp->setPluginType( Readex_Intraphase );
        a_tp->setRuntimeActionType( TUNING_ACTION_FUNCTION_POINTER );

        for ( int j = 0; j < num_values; ++j ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "PARAMETER [%d][%d] = %d \n", i, (j + 3), param_details[ idx + 3 + j ] );
        }

        //set range type details min, max, step
        if ( param_details[ idx + 0 ] == 1 ) {
            a_tp->setRange( param_details[ idx + 3 + 0 ], param_details[ idx + 3 + 1 ], param_details[ idx + 3 + 2 ] );
        }

        idx = idx + 3 + num_values;
        std::string domain_name = getDomainByParamName( param_names[ i ] );
        atps[ domain_name ].push_back( a_tp );
        //delete a_tp;
        delete param_names[ i ];
    }
    //print domain information
    /*for ( const auto& apt : atps ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "DOMAIN NAME: %s \n", atp.first.c_str() );
        for ( const auto& tuning_parameter : atp.second ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "TP NAME: %s \n", tuning_parameter->getName().c_str() );
        }

    }*/
    //free param_names
    free( buf );
    free( buf1 );
    //free( param_names );
    free( param_details );
    free( domain_names );
    //free( domain_details );
    return atps;
}


std::unordered_map< int, std::vector< int32_t > > atpService::getValidConfigurations( const char* dname ) {
    std::unordered_map< int, std::vector< int32_t > > valid_combination;
    int length;
    int32_t status(0), no_combinations(0), elems_per_combination(0);
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "Call to getValidConfigurations(%s)\n", dname );

    std::stringstream ss;
    ss << "getvalidparams," << dname << ";\n";
    std::string req_message( ss.str() );

    socket_write_line (sock_, req_message.c_str());
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "Sending line to ATPServer: <%s> via socket# %d\n", req_message.c_str(), sock_ );

    sleep( 5 );

    /* Read status, number of valid combinations, number of elements of each combination*/
    int32_t *info = ( int32_t* )malloc( sizeof( int32_t ) * 3 );
    if ( length = socket_blockread( sock_, ( char* )( info ), ( sizeof( int32_t ) * 3 ) ) != ( sizeof( int32_t ) * 3 ) ) {
        psc_errmsg( "wrong number of bytes read: %d!=%d\n", length, ( sizeof( int32_t ) * 3 ) );
        abort();
    }
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ) , "Received from ATP Server [%d] status  [%d] no of valid combinations and [%d] elements per combination \n", info[ 0 ], info[ 1 ], info[ 2 ] );

    status = info[ 0 ];

    if ( status == 0 ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ) , "ATP Server returns [%d] status  [%d] no of valid combinations with no valid combination \n", info[ 0 ] );
        free( info );
        return valid_combination;
    }
    constraint_               = status;
    no_combinations           = info[ 1 ];
    elements_per_combination_ = info[ 2 ]; //no. of application tuning parameters

    if ( no_combinations == 0 ) {
        return valid_combination;
    }

    int buf_size = no_combinations * elements_per_combination_ * sizeof( int32_t );
    int32_t *point_combination = ( int32_t* )malloc( buf_size );

    if ( length = socket_blockread( sock_, ( char* )( point_combination ), buf_size ) != buf_size ) {
        free( point_combination );
        psc_errmsg( "wrong number of bytes read: %d!=%d\n", length, buf_size );
        abort();
    }
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "Receive valid combination of points from ATPServer: \n" );

    /*Print valid combinations*/
    for( int i = 0; i < no_combinations; i++ ) {
        std::vector< int32_t >     elems_values_per_combination;
        std::vector< std::string > param_names( elements_per_combination_ );
        param_names = getParamNamesbyDomain( dname );
        for ( int j = 0; j < elements_per_combination_; ++j ) {
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "COMBINATION #:[%d]  PARAM VALUE: [%d] = %d \n", i, j, point_combination[ i * elements_per_combination_ + j ] );
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "COMBINATION #:[%d]  PARAM NAME: [%d] = %s \n", i, j, param_names.at( j ).c_str() );
            elems_values_per_combination.push_back( point_combination[ i * elements_per_combination_ + j ] );
            //valid_atp_mapping_[ param_names.at( j ) ].push_back( point_combination[ i * elements_per_combination_ + j ] );
        }
        valid_combination.insert( { i, elems_values_per_combination } );
    }
    //print the information
    /*for ( const auto& it : valid_combination ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "COMB#: %d \n", it->first );
        for ( const auto& local_it : it )
            psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "VALID POINT: %d \n", local_it );
    }*/
    return valid_combination;
}
std::string atpService::getDomainByParamName( std::string param_name ) {
    for ( const auto& local_it : domain_paramNames_mapping_ ) {
        //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "---DOMAIN NAME-------: %s\n", (local_it->first).c_str() );
        for ( const auto& tuning_parameter : local_it.second ) {
            //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( ApplTuningParameter ), "---TP NAME-------: %s\n", ( *it ).c_str() );
            if ( tuning_parameter.compare( param_name ) == 0 ) {
                return local_it.first;
            }
        }
    }
}

std::vector< std::string > atpService::getParamNamesbyDomain( std::string domain_name ) {
    std::unordered_map< std::string, std::vector< std::string > >::const_iterator found = domain_paramNames_mapping_.find( domain_name );
    if ( found != domain_paramNames_mapping_.end() ) {
        return found->second;
    }
    else {
        return std::vector<std::string>();
    }
}
