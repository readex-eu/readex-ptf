#define BOOST_TEST_MODULE PCAPPlugin

#include "GlobalFixture.h"
#include "FrontendFixture.h"
#include "frontend_statemachine.h"
#include "IPlugin.h"
#include "autotune_services.h"
#include "PCAPPlugin.h"

using namespace std;

struct PCAPPluginFixture : FrontendFixture {
    PCAPPluginFixture() : FrontendFixture() {
        prepared_strategy_request = NULL;
        analysisStrategyRequest   = NULL;

        analysisPerExperimentRequired = false;
        restart_required              = false;
        env                           = "";
        numprocs                      = fe->get_mpinumprocs();
        command                       = string( "./dummy_binary_name" );
        is_instrumented               = true;

        // add an arbitrary user region that will be used as the phase region
        appl->addRegion( "user", 1, "foo", USER_REGION, 1, 10 );

        fe->fe_context->loadPlugin( "pcap", &major, &minor, &name, &description );
        print_loaded_plugin( major, minor, name, description );
        plugin = ( PCAPPlugin* )fe->fe_context->getTuningPluginInstance( "pcap" );
        fe->plugin_context->setApplInstrumented( true );
        fe->plugin_context->setOmpnumthreads( 8 );
        fe->plugin_context->setMPINumProcs( 8 );
        plugin->initialize( fe->plugin_context.get(), fe->frontend_pool_set.get() );
    }


    ~PCAPPluginFixture() {
        plugin->finalize();
    }


    PCAPPlugin*      plugin;
    int              major, minor;      // version information
    string           name, description; // version information
    StrategyRequest* prepared_strategy_request;
    StrategyRequest* analysisStrategyRequest;
    bool             analysisPerExperimentRequired;
    bool             restart_required;
    string           env;
    int              numprocs;
    string           command;
    bool             is_instrumented;
};

BOOST_GLOBAL_FIXTURE( GlobalFixture );

BOOST_FIXTURE_TEST_SUITE( pcap_plugin_interface, PCAPPluginFixture )

BOOST_AUTO_TEST_CASE( strategy_request_serialization ) {
    // in this test, we need to reach the run_phase_experiments_action state
    printf( "call startTuningStep()...\n" );
    plugin->startTuningStep();
    printf( "call createScenarios()...\n" );
    plugin->createScenarios();
    printf( "call prepareScenarios()...\n" );
    plugin->prepareScenarios();
    printf( "call defineExperiment()...\n" );
    plugin->defineExperiment( fe->get_mpinumprocs(), analysisPerExperimentRequired, &analysisStrategyRequest );
    printf( "call restartRequired()...\n" );
    restart_required = plugin->restartRequired( env, numprocs, command, is_instrumented );
    printf( "required state for run_phase_experiments_action() reached, begin testing...\n" );
    printf( "call createStrategyRequest()...\n" );
    prepared_strategy_request = frontend_statemachine::createStrategyRequest( fe );
    prepared_strategy_request->printStrategyRequest();
    printf( "call pushStrategyRequest()...\n" );
    frontend_statemachine::pushStrategyRequest( prepared_strategy_request );
}

BOOST_AUTO_TEST_SUITE_END()
