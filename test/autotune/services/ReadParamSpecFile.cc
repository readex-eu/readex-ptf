#define BOOST_TEST_MODULE ReadParamSpecFile


#include <boost/test/included/unit_test.hpp>
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/unit_test_log.hpp>
#include <boost/test/results_reporter.hpp>
#include <vector>

#include "autotune_services.h"
#include "GlobalFixture.h"
#include "FrontendFixture.h"
#include "config.h"
using namespace std;

struct SpecFileFixture : FrontendFixture {
    string                 file_path;
    vector<MPIParameterTP> conftps;
    mpiImplementationName  MPIType;
    string                 param_spec_file;
    SpecFileFixture() {
        //string s(string("cp -a ") + string(PERISCOPE_SOURCE_DIRECTORY) +
        //		string("/test/autotune/services/param_spec.conf ."));
        //system(s.c_str());
        param_spec_file = string( PERISCOPE_SOURCE_DIRECTORY ) + string( "/test/autotune/services/param_spec.conf" );
    }

    ~SpecFileFixture() {
    }
};

BOOST_GLOBAL_FIXTURE( GlobalFixture );

BOOST_FIXTURE_TEST_SUITE( parameter_specification_file_parsing, SpecFileFixture )

BOOST_AUTO_TEST_CASE( read_parameter_specification_file ) {
    cout << "Using file: " << param_spec_file << endl;
    if( !param_spec_file.empty() ) {
        conftps = getTuningParameters( param_spec_file.c_str(), MPIType );
    }
    else {
        perror( "No parameter specification file found, set PSC_PARAM_SPEC_FILE with path of file.\n" );
    }
    //conftps = getTuningParameters(file_path.c_str());

    /*
     *  Configuration file contents
     *  MP_BUFFER_MEM=16M,32M,64M;
     *	MP_EAGER_LIMIT=12:2:16;
     *	MP_TASK_PER_NODE= 4:4:12;
     *	MP_TASK_AFFINITY=mcm,core;
     */

    BOOST_REQUIRE( conftps.size() == 4 );
    BOOST_CHECK_EQUAL( MPIType, OPENMPI );
    BOOST_CHECK_EQUAL( conftps.at( 0 ).env_var_name.c_str(), "MP_BUFFER_MEM" );
    BOOST_CHECK( !conftps.at( 0 ).ranged );
    BOOST_CHECK_EQUAL( conftps.at( 1 ).env_var_name.c_str(), "MP_EAGER_LIMIT" );
    BOOST_CHECK( !conftps.at( 1 ).ranged );
    BOOST_CHECK_EQUAL( conftps.at( 2 ).env_var_name.c_str(), "MP_TASK_PER_NODE" );
    BOOST_CHECK( conftps.at( 2 ).ranged );
    BOOST_CHECK_EQUAL( conftps.at( 3 ).env_var_name.c_str(), "MP_TASK_AFFINITY" );
    BOOST_CHECK( !conftps.at( 3 ).ranged );
}

BOOST_AUTO_TEST_SUITE_END()
