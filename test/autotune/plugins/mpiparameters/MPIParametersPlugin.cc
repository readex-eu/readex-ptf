#define BOOST_TEST_MODULE MPIParameterPlugin

#include "GlobalFixture.h"
#include "FrontendFixture.h"
#include "IPlugin.h"
#include "autotune_services.h"
#include "MPIParametersPlugin.h"




struct MPIParametersPluginFixture : FrontendFixture {
    MPIParametersPluginFixture() : FrontendFixture() {
        std::string cmd = std::string( "cp -a " ) + PERISCOPE_SOURCE_DIRECTORY + "/test/autotune/plugins/mpiparameters/param_spec.conf .";
        if( system( cmd.c_str() ) != 0 ) {
            perror( "error while copying the param_spec.conf file for this unit test" );
        }

        // add an arbitrary user region that will be used as the phase region
        appl->addRegion( "user", 1, "foo", USER_REGION, 1, 10 );

        fe_context->loadPlugin( "mpiparameters", &major, &minor, &name, &description );
        plugin = ( MPIParametersPlugin* )fe_context->getTuningPluginInstance( "mpiparameters" );
    }


    ~MPIParametersPluginFixture() {
        plugin->terminate();
        fe_context->unloadPlugins();
    }


    MPIParametersPlugin* plugin;
    int                  major, minor;
    string               name, description;
};




BOOST_GLOBAL_FIXTURE( GlobalFixture );

BOOST_FIXTURE_TEST_SUITE( mpiparameters_plugin_interface, MPIParametersPluginFixture )

BOOST_AUTO_TEST_CASE( initialize_plugin ) {
    printf( "initialize_plugin test case\n" );
    plugin->initialize( plugin_context, fe->frontend_pool_set.get() );
    BOOST_CHECK_EQUAL( plugin->getNumParameters(), 4 );
    BOOST_CHECK_EQUAL( plugin->getNumValues( 0 ), 3 );
    BOOST_CHECK_EQUAL( plugin->getNumValues( 1 ), 3 );
    BOOST_CHECK_EQUAL( plugin->getNumValues( 2 ), 3 );
    BOOST_CHECK_EQUAL( plugin->getNumValues( 3 ), 2 );
}

BOOST_AUTO_TEST_CASE( scenario_creation ) {
    printf( "scenario_creation test case\n" );
    plugin->initialize( plugin_context, fe->frontend_pool_set.get() );
    BOOST_CHECK_EQUAL( plugin->getNumParameters(), 4 );
    plugin->startTuningStep();
    plugin->createScenarios();

    BOOST_CHECK_EQUAL( fe->frontend_pool_set->csp->size(), 54 );
}

BOOST_AUTO_TEST_CASE( scenario_preparation ) {
    printf( "scenario_preparation test case\n" );
    plugin->initialize( plugin_context, fe->frontend_pool_set.get() );
    plugin->startTuningStep();
    plugin->createScenarios();
    plugin->prepareScenarios();

    BOOST_CHECK_EQUAL( fe->frontend_pool_set->csp->size(), 53 );
    BOOST_CHECK_EQUAL( fe->frontend_pool_set->psp->size(), 1 );
}

BOOST_AUTO_TEST_SUITE_END()
