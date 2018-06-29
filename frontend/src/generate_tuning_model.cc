#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __sparc
#include "getopt.h"
#elif __p575
#include "getopt.h"
#else
#include <getopt.h>
#endif

#include "frontend.h"
#include "regxx.h"
#include "psc_errmsg.h"
#include "selective_debug.h"
#include "timing.h"
#include "psc_config.h"
#include "application.h"
#include "selective_debug.h"
#include "frontend_accl_statemachine.h"
#include "autotune_services.h"
#include "psc_errmsg.h"

#ifdef HAVE_CEREAL
#include "tuning_model.h"
#endif


void generate_tuning_model() {
    psc_dbgmsg(2, "Generating tuning model\n" );
#ifdef HAVE_CEREAL
    std::unordered_map <std::string, std::string> iids;

    appl->fillBestInputIdentifiers( iids );

    std::unordered_map< std::string, std::string >::const_iterator got;
    got = iids.find( "collect_num_threads" );

    if ( got == iids.end() || !got->second.compare( "true" ) ) {
        iids.insert( { "Thread", std::to_string( appl->getOmpThreads() ) } );
    }

    if ( got != iids.end() ) {
        iids.erase( got );
    }

    got = iids.find( "collect_num_processes" );

    if ( got == iids.end() || !got->second.compare( "true" ) ) {
        iids.insert( { "Process", std::to_string( appl->getMpiProcs() ) } );
    }

    if ( got != iids.end() ) {
        iids.erase( got );
    }

    std::string file_path( getenv( "PWD" ) );
    //file_path += "/tuning_model.json";
    try {
        std::string file_p = configTree.get < std::string > ( "Configuration.periscope.tuningModel.file_path" );
        if( file_p.length() == 0 ) {
            file_path += "/tuning_model.json";
        } else {
            file_path = file_p;
        }
    } catch (exception &e) {
        file_path += "/tuning_model.json";
    }

    std::ofstream ofs( file_path.c_str() );
    if ( ofs.is_open() ) {
        tmg::cluster::equality_clusterer c;
        auto tm = tmg::generate_tuning_model( appl->getCalltreeRoot(), iids, c, tmg::random_selector );
        ofs << tm;
        ofs.close();
    }
    /* FIXME: print error message if file cannot be opened */
#endif
    psc_dbgmsg( 2,"Finished tuning model generation\n" );
}
