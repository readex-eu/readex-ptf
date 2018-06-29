#ifndef _AUTOTUNE_GLOBAL_FIXTURE_
#define _AUTOTUNE_GLOBAL_FIXTURE_

#include <boost/test/included/unit_test.hpp>
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/unit_test_log.hpp>
#include <boost/test/results_reporter.hpp>
#include <ostream>

struct GlobalFixture {
    std::ofstream logStream;
    std::ofstream reportStream;

    GlobalFixture() {
        logStream.open( "log.xml" );
        boost::unit_test::unit_test_log.set_stream( logStream );
        reportStream.open( "report.xml" );
        boost::unit_test::results_reporter::set_stream( reportStream );
    }


    ~GlobalFixture() {
        // TODO set log back to defaults
    }
};

#endif
