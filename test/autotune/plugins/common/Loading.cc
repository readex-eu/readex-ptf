#define BOOST_TEST_MODULE Loading

#include "GlobalFixture.h"
#include "FrontendFixture.h"
#include "IPlugin.h"
#include "autotune_services.h"

#define LOAD_PLUGIN( _name_ ) \
    { \
        bool loaded     = false; \
        bool terminated = false; \
        try{ \
            fe_context->loadPlugin( _name_, &major, &minor, &name, &description ); \
            plugin = fe_context->getTuningPluginInstance( _name_ ); \
            print_loaded_plugin( major, minor, name, description ); \
            cout << _name_ << " plugin loaded." << endl << endl; \
            loaded = true; \
            plugin->terminate(); \
            delete plugin; \
            terminated = true; \
        } catch( ... ) { \
            cout << _name_ << " failed to load." << endl << endl; \
            loaded = false; \
        } \
        BOOST_REQUIRE( loaded ); \
        BOOST_REQUIRE( terminated ); \
    }

struct LoadingFixture : FrontendFixture {
    IPlugin* plugin;
    int      major, minor;      // version information
    string   name, description; // version information

    LoadingFixture() : FrontendFixture() {
    }

    ~LoadingFixture() {
    }
};

BOOST_GLOBAL_FIXTURE( GlobalFixture );

BOOST_FIXTURE_TEST_SUITE( loading_plugins, LoadingFixture )

// this test suite requires the 'make install' target to be completed before 'make check'

BOOST_AUTO_TEST_CASE( loading_compilerflags ) {
    LOAD_PLUGIN( "compilerflags" );
}

BOOST_AUTO_TEST_CASE( loading_demo ) {
    LOAD_PLUGIN( "demo" );
}

BOOST_AUTO_TEST_CASE( loading_dvfs ) {
    LOAD_PLUGIN( "dvfs" );
}

BOOST_AUTO_TEST_CASE( loading_masterworker ) {
    LOAD_PLUGIN( "masterworker" );
}

BOOST_AUTO_TEST_CASE( loading_mpiparameters ) {
    LOAD_PLUGIN( "mpiparameters" );
}

//BOOST_AUTO_TEST_CASE (loading_mpit_custom) {
//	LOAD_PLUGIN("mpit-custom");
//}

BOOST_AUTO_TEST_CASE( loading_pcap ) {
    LOAD_PLUGIN( "pcap" );
}

BOOST_AUTO_TEST_CASE( loading_pipeline ) {
    LOAD_PLUGIN( "pipeline" );
}

//BOOST_AUTO_TEST_CASE (loading_user) {
//	LOAD_PLUGIN("user");
//}

BOOST_AUTO_TEST_SUITE_END()
