#ifndef _AUTOTUNE_FRONTEND_FIXTURE_
#define _AUTOTUNE_FRONTEND_FIXTURE_



#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/unit_test_log.hpp>
#include <boost/test/results_reporter.hpp>
#include <ostream>
#include <vector>

#include "application.h"
#include "frontend.h"
#include "Scenario.h"
#include "GlobalFixture.h"
#include "FrontendFixture.h"
#include "DriverContext.h"

Application*        appl;
PeriscopeFrontend*  fe;
ApplicationStarter* starter;
struct cmdline_opts opts;
DriverContext*      fe_context;
DriverContext*      plugin_context;

struct FrontendFixture {
    FrontendFixture() {
        BOOST_TEST_MESSAGE( "FrontendFixture constructor" );

        appl           = &Application::instance();
        fe             = new PeriscopeFrontend( ACE_Reactor::instance() );
        starter        = new ApplicationStarter();
        fe_context     = new DriverContext();
        plugin_context = new DriverContext();
        psc_set_progname( "dummy_frontend" );

        // TODO populate the command line options
    }

    ~FrontendFixture() {
        BOOST_TEST_MESSAGE( "FrontendFixture destructor" );

        delete starter;
        delete fe;
    }
};

#endif
