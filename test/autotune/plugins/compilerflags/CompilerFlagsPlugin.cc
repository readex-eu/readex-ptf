#define BOOST_TEST_MODULE CompilerFlagsPlugin

#include "GlobalFixture.h"
#include "FrontendFixture.h"
#include "IPlugin.h"
#include "autotune_services.h"
#include "CompilerFlagsPlugin.h"

using namespace std;

struct CompilerFlagsPluginFixture : FrontendFixture {
    CompilerFlagsPlugin* plugin;
    int                  major, minor;      // version information
    string               name, description; // version information

    CompilerFlagsPluginFixture() : FrontendFixture() {
        string s( string( "cp -a " ) + string( PERISCOPE_SOURCE_DIRECTORY ) +
                  string( "/test/autotune/plugins/compilerflags/config.cfg ." ) );
        system( s.c_str() );
        plugin = ( CompilerFlagsPlugin* )loadPlugin( "compilerflags", &major, &minor, &name, &description );
    }

    ~CompilerFlagsPluginFixture() {
    }
};

BOOST_GLOBAL_FIXTURE( GlobalFixture );

BOOST_FIXTURE_TEST_SUITE( compilerflags_plugin_interface, CompilerFlagsPluginFixture )

BOOST_AUTO_TEST_CASE( initialize_plugin ) {
    plugin->initialize( "" );
    BOOST_CHECK_EQUAL( plugin->getNumParameters(), 1 );
    BOOST_CHECK_EQUAL( plugin->getNumValues( 0 ), 3 );
}
/*
   BOOST_AUTO_TEST_CASE (scenario_creation) {
    plugin->initialize("");
    BOOST_CHECK_EQUAL(plugin->getNumParameters(), 4);
    plugin->startTuningStep();
    plugin->createScenarios();

    BOOST_CHECK_EQUAL(csp->size(), 54);
   }

   BOOST_AUTO_TEST_CASE (scenario_preparation) {
    plugin->initialize("");
    plugin->startTuningStep();
    plugin->createScenarios();
    plugin->prepareScenarios();

    BOOST_CHECK_EQUAL(csp->size(), 53);
    BOOST_CHECK_EQUAL(psp->size(), 1);
    //cout << "export string: " << plugin->getExportsString() << endl;
    //BOOST_CHECK_EQUAL(plugin->getExportsString(), "export MP_BUFFER_MEM=16M; export MP_EAGER_LIMIT=12; export MP_TASK_PER_NODE=4; export MP_TASK_AFFINITY=mcm; ");
   }
 */
//BOOST_AUTO_TEST_CASE (experiment_definition) {
//  plugin->initialize("");
//  plugin->startTuningStep();
//  plugin->createScenarios();
//  plugin->prepareScenarios();
//  plugin->defineExperiment(4);
//  BOOST_CHECK_EQUAL(esp->size(), 1);
//}

//BOOST_AUTO_TEST_CASE (restart_required) {
//  string env = "";
//  int new_process_count = 4;
//  string command = "";
//  bool is_instrumented = false;
//  plugin->initialize("");
//  plugin->startTuningStep();
//  plugin->createScenarios();
//  plugin->prepareScenarios();
//  plugin->defineExperiment(4);
//  plugin->restartRequired(&env, &new_process_count, &command, &is_instrumented);
//  BOOST_CHECK_EQUAL(env.c_str(), "export MP_BUFFER_MEM=16M; export MP_EAGER_LIMIT=12; export MP_TASK_PER_NODE=4; export MP_TASK_AFFINITY=mcm; ");
//  BOOST_CHECK(is_instrumented);
//}

//BOOST_AUTO_TEST_CASE (t3) {
//  //TODO test code
//}
BOOST_AUTO_TEST_SUITE_END()
