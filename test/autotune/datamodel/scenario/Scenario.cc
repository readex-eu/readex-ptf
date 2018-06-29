#define BOOST_TEST_MODULE Scenario

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/serialization/vector.hpp>
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
#include "GlobalFixture.h"
#include "FrontendFixture.h"

using namespace std;
using namespace boost::iostreams;
using namespace boost::archive;

//BOOST_TEST_DONT_PRINT_LOG_VALUE(RegionIdent);
BOOST_TEST_DONT_PRINT_LOG_VALUE( Region );
BOOST_TEST_DONT_PRINT_LOG_VALUE( TuningParameter );
//BOOST_TEST_DONT_PRINT_LOG_VALUE(std::list<Region*>);

struct ScenarioSerializationFixture : FrontendFixture {
    map<TuningParameter*, int>* value;
    Variant*                    variant;
    list<TuningSpecification*>* ts_in;
    list<PropertyRequest*>*     pr_in;
    Scenario*                   original_scenario;

    ScenarioSerializationFixture() : FrontendFixture() {
        value = new map<TuningParameter*, int>();

        Region* pr = new Region();

        TuningParameter* tp1 = new TuningParameter();
        tp1->setId( 0 );
        tp1->setName( "tp1" );
        tp1->setPluginType( MPI );
        tp1->setRuntimeActionType( TUNING_ACTION_NONE );
        tp1->setRange( 2, 4, 1 );
        Restriction* res = new Restriction();
        res->setRegion( pr );
        res->setRegionDefined( true );
        tp1->setRestriction( res );

        TuningParameter* tp2 = new TuningParameter();
        tp2->setId( 1 );
        tp2->setName( "tp2" );
        tp2->setPluginType( CFS );
        tp2->setRuntimeActionType( TUNING_ACTION_VARIABLE_INTEGER );
        tp2->setRange( 20, 50, 10 );
        tp2->setRestriction( res );

        //
        // WARNING: Do not add more than one value here, because the order of TPs is determined by
        // the object's address; this means, in the final evaluation, the order can be different!
        //
        // (The real fix would be to make the comparison more intelligent)
        //
        value->insert( make_pair( tp2, 2344 ) );
        variant = new Variant( *value );
        ts_in   = new list<TuningSpecification*>();
        ts_in->push_back( new TuningSpecification( variant ) );
        pr_in = new list<PropertyRequest*>();
        pr_in->push_back( new PropertyRequest( new list<int>) );
        original_scenario = new Scenario( new Region(), ts_in, pr_in );
    }

    ~ScenarioSerializationFixture() {
        delete original_scenario;
    }
};

BOOST_GLOBAL_FIXTURE( GlobalFixture );

BOOST_FIXTURE_TEST_SUITE( scenario_serialization, ScenarioSerializationFixture )

BOOST_AUTO_TEST_CASE( serialization_to_and_from_a_file ) {
    Scenario*   restored_scenario;
    const char* fileName = "test_scenario_serialized_data";

    {
        ofstream      ofs( fileName );
        text_oarchive oar( ofs );
        oar&          original_scenario;
    }

    {
        ifstream      ifs( fileName );
        text_iarchive iar( ifs );
        iar&          restored_scenario;
    }

    map<TuningParameter*, int>           variant          = original_scenario->getTuningSpecifications()->front()->getVariant()->getValue();
    map<TuningParameter*, int>           variant_restored = restored_scenario->getTuningSpecifications()->front()->getVariant()->getValue();
    map<TuningParameter*, int>::iterator tp               = variant.begin();
    map<TuningParameter*, int>::iterator tp_restored      = variant_restored.begin();

    for(;
        tp != variant.end() && tp_restored != variant_restored.end();
        tp++, tp_restored++ ) {
        TuningParameter* tp1          = tp->first;
        TuningParameter* tp1_restored = tp_restored->first;

        BOOST_REQUIRE_EQUAL( *tp1, *tp1_restored );
        BOOST_REQUIRE_EQUAL( tp1->getId(), tp1_restored->getId() );
        BOOST_REQUIRE_EQUAL( tp1->getPluginType(), tp1_restored->getPluginType() );
        BOOST_REQUIRE_EQUAL( tp1->getRuntimeActionType(), tp1_restored->getRuntimeActionType() );
        BOOST_REQUIRE_EQUAL( tp1->getName(), tp1_restored->getName() );
        BOOST_REQUIRE_EQUAL( tp1->getRangeFrom(), tp1_restored->getRangeFrom() );
        BOOST_REQUIRE_EQUAL( tp1->getRangeTo(), tp1_restored->getRangeTo() );
        BOOST_REQUIRE_EQUAL( tp1->getRangeStep(), tp1_restored->getRangeStep() );
        BOOST_REQUIRE_EQUAL( *( tp1->getRestriction()->getRegion() ), *( tp1_restored->getRestriction()->getRegion() ) );
        BOOST_REQUIRE_EQUAL( tp->second, tp_restored->second );
    }

    BOOST_REQUIRE( *original_scenario == *restored_scenario );

    delete restored_scenario;
}

BOOST_AUTO_TEST_SUITE_END()
