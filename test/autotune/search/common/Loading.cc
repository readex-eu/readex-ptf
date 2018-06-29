#define BOOST_TEST_MODULE Loading

#include "GlobalFixture.h"
#include "FrontendFixture.h"
#include "IPlugin.h"
#include "autotune_services.h"

#define LOAD_PLUGIN( _name_ ) \
    { \
        bool loaded = false; \
        try{ \
            fe_context->loadSearchAlgorithm( _name_, &major, &minor, &name, &description ); \
            search_algorithm = fe_context->getSearchAlgorithmInstance( _name_ ); \
            print_loaded_plugin( major, minor, name, description ); \
            cout << _name_ << " search algorithm loaded." << endl << endl; \
            loaded = true; \
        } catch( ... ) { \
            cout << _name_ << " failed to load." << endl << endl; \
            loaded = false; \
        } \
        BOOST_REQUIRE( loaded ); \
    }

struct SearchLoadingFixture : FrontendFixture {
    ISearchAlgorithm* search_algorithm;
    int               major, minor;      // version information
    string            name, description; // version information

    SearchLoadingFixture() : FrontendFixture() {
    }

    ~SearchLoadingFixture() {
    }
};

BOOST_GLOBAL_FIXTURE( GlobalFixture );

BOOST_FIXTURE_TEST_SUITE( loading_search_algorithms, SearchLoadingFixture )

// this test suite requires the 'make install' target to be completed before 'make check'

BOOST_AUTO_TEST_CASE( loading_compilerflags ) {
    LOAD_PLUGIN( "exhaustive" );
}

BOOST_AUTO_TEST_CASE( loading_demo ) {
    LOAD_PLUGIN( "individual" );
}

BOOST_AUTO_TEST_CASE( loading_dvfs ) {
    LOAD_PLUGIN( "gde3" );
}

BOOST_AUTO_TEST_SUITE_END()
