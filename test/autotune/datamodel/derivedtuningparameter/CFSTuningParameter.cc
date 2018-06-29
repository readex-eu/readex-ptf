#define BOOST_TEST_MODULE TuningParameter

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/unit_test_log.hpp>
#include <boost/test/results_reporter.hpp>
#include <ostream>
#include <vector>

#include "application.h"
#include "frontend.h"
#include "Scenario.h"
#include "CFSTuningParameter.h"
#include "GlobalFixture.h"
#include "FrontendFixture.h"

using namespace std;
using namespace boost::iostreams;
using namespace boost::archive;

//BOOST_TEST_DONT_PRINT_LOG_VALUE(RegionIdent);
//BOOST_TEST_DONT_PRINT_LOG_VALUE(Region);
BOOST_TEST_DONT_PRINT_LOG_VALUE( CFSTuningParameter );
//BOOST_TEST_DONT_PRINT_LOG_VALUE(std::list<Region*>);

struct TuningParameterSerializationFixture : FrontendFixture {
    CFSTuningParameter* original_tuning_parameter;

    TuningParameterSerializationFixture() : FrontendFixture() {
        Region* pr = new Region();

        CFSTuningParameter* tp1 = new CFSTuningParameter();
        tp1->setId( 0 );
        tp1->setName( "tp1" );
        tp1->setPluginType( MPI );
        tp1->setRuntimeActionType( TUNING_ACTION_NONE );
        tp1->setRange( 2, 4, 1 );
        Restriction* res = new Restriction();
        res->setRegion( pr );
        res->setRegionDefined( true );
        tp1->setRestriction( res );
        string* fs1 = new string( "Flag1" );
        string* vs1 = new string( "Value1" );
        //tp1->setFlagString(fs1);
        //tp1->addValueString(vs1);

        original_tuning_parameter = tp1;
    }

    ~TuningParameterSerializationFixture() {
        delete original_tuning_parameter;
    }
};

BOOST_GLOBAL_FIXTURE( GlobalFixture );

BOOST_FIXTURE_TEST_SUITE( tuningparameter_serialization, TuningParameterSerializationFixture )

BOOST_AUTO_TEST_CASE( serialization_to_and_from_a_file ) {
    CFSTuningParameter* restored_tuning_parameter;
    const char*         fileName = "test_derivedtuningparameterserialization_data";

    cout << "About to serialize" << endl;
    {
        ofstream      ofs( fileName );
        text_oarchive oar( ofs );
        oar&          original_tuning_parameter;
    }

    cout << "About to deserialize" << endl;
    {
        ifstream      ifs( fileName );
        text_iarchive iar( ifs );
        iar&          restored_tuning_parameter;
    }

    CFSTuningParameter* tp          = original_tuning_parameter;
    CFSTuningParameter* tp_restored = restored_tuning_parameter;

    BOOST_CHECK_EQUAL( *tp, *tp_restored );
    BOOST_CHECK_EQUAL( tp->getId(), tp_restored->getId() );
    BOOST_CHECK_EQUAL( tp->getPluginType(), tp_restored->getPluginType() );
    BOOST_CHECK_EQUAL( tp->getRuntimeActionType(), tp_restored->getRuntimeActionType() );
    BOOST_CHECK_EQUAL( tp->getName(), tp_restored->getName() );
    BOOST_CHECK_EQUAL( tp->getRangeFrom(), tp_restored->getRangeFrom() );
    BOOST_CHECK_EQUAL( tp->getRangeTo(), tp_restored->getRangeTo() );
    BOOST_CHECK_EQUAL( tp->getRangeStep(), tp_restored->getRangeStep() );
    BOOST_CHECK_EQUAL( *( tp->getRestriction()->getRegion() ), *( tp_restored->getRestriction()->getRegion() ) );

    BOOST_REQUIRE( *original_tuning_parameter == *restored_tuning_parameter );

    delete restored_tuning_parameter;
}

BOOST_AUTO_TEST_SUITE_END()
