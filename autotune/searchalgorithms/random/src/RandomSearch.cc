/**
   @file    RandomSearch.cc
   @ingroup RandomSearch
   @brief   Random sampling search algorithm
   @author  Miklos Homolya
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope Tuning Framework.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2014, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include "RandomSearch.h"
#include "UniformDistributionModel.h"
#include "IndependentParametersModel.h"
#include "TuningDatabase.h"
#include "search_common.h"

using std::list;

namespace {
bool areTuningSpecificationListsEqual( list<TuningSpecification*> const& tsl1,
                                       list<TuningSpecification*> const& tsl2 ) {
    if( tsl1.size() != tsl2.size() ) {
        return false;
    }
    list<TuningSpecification*>::const_iterator it1, it2;
    for( it1 = tsl1.begin(), it2 = tsl2.begin(); it1 != tsl1.end() && it2 != tsl2.end(); ++it1, ++it2 ) {
        if( *( *it1 ) != *( *it2 ) ) {
            return false;
        }
    }
    return true;
}

double featureDistance( ProgramSignature f1,
                        ProgramSignature f2 ) {
    double sum = 0;
    for( ProgramSignature::const_iterator it = f1.begin(); it != f1.end(); ++it ) {
        string key    = *it;
        INT64  value1 = f1[ key ];

        ProgramSignature::const_iterator search = find( f2.begin(), f2.end(), key );
        if( search != f2.end() ) {
            INT64 value2 = f2[ *search ];
            sum += ( value1 - value2 ) * ( value1 - value2 );
        }
    }
    return sqrt( sum );
}

std::vector<TuningConfiguration>getNearestGoodConfigurations( ProgramSignature signature ) {
    int    bestId;
    double bestDist = std::numeric_limits<double>::infinity();

    boost::scoped_ptr< Iterator<int> > it( tdb->queryPrograms() );
    if( !it->hasNext() ) {
        return std::vector<TuningConfiguration>();
    }

    while( it->hasNext() ) {
        int    id   = it->next();
        double dist = featureDistance( signature, tdb->querySignature( id ) );
        if( dist < bestDist ) {
            bestId   = id;
            bestDist = dist;
        }
    }
    return tdb->queryConfigurationsByRatio( bestId, 1.02 );
}
} /* unnamed namespace */


void RandomSearch::addObjectiveFunction(ObjectiveFunction *obj){
   objectiveFunctions.push_back(obj);
}



/**
 * @brief Constructor description
 * @ingroup RandomSearch
 */
RandomSearch::RandomSearch() : ISearchAlgorithm(),
    pool_set( NULL ), optimumScenario( -1 ), sampleCount( 2 ), optimumValue( std::numeric_limits<double>::max() ),
    worst( -1 ), worstValue( std::numeric_limits<double>::min() ) {
}

/**
 * @brief Destructor description
 * @ingroup RandomSearch
 */
RandomSearch::~RandomSearch() {
}

/**
 * @brief Initialize description
 * @ingroup RandomSearch
 *
 * Long description.
 */
void RandomSearch::initialize( DriverContext* context, ScenarioPoolSet* poolSet ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "RandomSearch: call to initialize()\n" );

    this->pool_set = poolSet;
}

/**
 * @brief Search Finished description
 * @ingroup RandomSearch
 *
 * Long description.
 */
bool RandomSearch::searchFinished() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "RandomSearch: call to searchFinished()\n" );
 	while (!scenarioIds.empty()) {
 		int scenario_id = scenarioIds.front();
 		scenarioIds.pop();
// 		for (int i = 0; i < objectiveFunctions.size(); i++) {
                        int i=0;
 			double objValue = objectiveFunctions[i]->objective(scenario_id, pool_set->srp);
 			path[scenario_id] = objValue;
// 			Scenario* scenario= pool_set->fsp->getScenarioByScenarioID(scenario_id);
// 			scenario->addResult(objectiveFunctions[i]->getName(), objValue);

 			if (optimumScenario== -1 || objValue < optimumValue) {
 				optimumScenario = scenario_id;
 				optimumValue = objValue;
 			}

            if (worst == -1 || objValue > worstValue) {
                worst = scenario_id;
                worstValue = objValue;
            }
// 		}
 	}

   return true;
}

/**
 * @brief Get Optimum description
 * @ingroup RandomSearch
 *
 * Long description.
 */
int RandomSearch::getOptimum() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "RandomSearch: call to getOptimum()\n" );

    return optimumScenario;
}


int RandomSearch::getWorst() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "Exhaustive Search: call to getWorst()\n" );

    if( worst == -1 ) {
        psc_abort( "Error: No worst scenario has been determined yet." );
    }
    assert( path.find( worst ) != path.end() );
    return worst;
}

/**
 * @brief Clear description
 * @ingroup RandomSearch
 *
 * Long description.
 */
void RandomSearch::clear() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "RandomSearch: call to clear()\n" );

    models.clear();
}

/**
 * @brief Get Search Path description
 * @ingroup RandomSearch
 *
 * Long description.
 */
map<int, double> RandomSearch::getSearchPath() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "RandomSearch: call to getSearchPath()\n" );

    return path;
}

/**
 * @brief Add ProbabilityModel for searching
 * @ingroup RandomSearch
 *
 * Please note that the ownership of the object is transferred to RandomSearch.
 */
void RandomSearch::addProbabilityModel( ProbabilityModelPtr model ) {
    models.push_back( model );
}

/**
 * @brief Add Search Space description
 * @ingroup RandomSearch
 *
 * Long description.
 */
void RandomSearch::addSearchSpace( SearchSpace* searchSpace ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "RandomSearch: call to addSearchSpace()\n" );

    models.push_back( ProbabilityModelPtr( new UniformDistributionModel( searchSpace ) ) );
}

/**
 * @brief Add SearchSpace with ProgramSignature.
 * @ingroup RandomSearch
 */
void RandomSearch::addSearchSpaceWithSignature( SearchSpace*            ss,
                                                ProgramSignature const& signature ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "RandomSearch: call to addSearchSpaceWithSignature()\n" );

    // Described in 5.6 of Miklos' thesis
    // TODO list in final chapter of thesis
    // TODO(random): configurable probability model
    //models.push_back(ProbabilityModelPtr(IndependentParametersModel::learnLinearSVM(ss, signature)));
    models.push_back( ProbabilityModelPtr( IndependentParametersModel::fromConfigurationSamples( ss, getNearestGoodConfigurations( signature ) ) ) );
}

/**
 * @brief Create Scenarios description
 * @ingroup RandomSearch
 *
 * Long description.
 */
void RandomSearch::createScenarios() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "RandomSearch: call to createScenarios()\n" );


    if (objectiveFunctions.size()==0){
   	 addObjectiveFunction(new PTF_minObjective(""));
    }


    std::vector<list<TuningSpecification*>*> scenarios;
    for( int i = 0; scenarios.size() < sampleCount && i < 3 * sampleCount; i++ ) {
        list<TuningSpecification*>* tuningSpecList = new list<TuningSpecification*>();
        for( size_t i = 0; i < models.size(); i++ ) {
            tuningSpecList->push_back( models[ i ]->sample() );
//      /* TODO(random): compare with the approach of Exhaustive Search */
//      TuningSpecification *tuningSpec = models[i]->sample();
//      Variant *variant = new Variant(tuningSpec->getVariant()->getValue());
//      VariantContext context = tuningSpec->getVariantContext();
//      tuningSpecList->push_back(new TuningSpecification(variant, context.context_union.region_list));
        }

        // eliminate duplicates
        bool skip = false;
        for( size_t i = 0; i < scenarios.size(); i++ ) {
            if( areTuningSpecificationListsEqual( *scenarios[ i ], *tuningSpecList ) ) {
                skip = true;
            }
        }
        if( !skip ) {
            scenarios.push_back( tuningSpecList );
        }
    }

    for( size_t i = 0; i < scenarios.size(); i++ ) {
        Scenario* scenario = new Scenario( scenarios[ i ] );
        pool_set->csp->push( scenario );
        scenarioIds.push( scenario->getID() );
    }
}

/**
 * @brief Terminate description
 * @ingroup RandomSearch
 *
 * Long description.
 */
void RandomSearch::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "RandomSearch: call to terminate()\n" );
}

/**
 * @brief Finalize description
 * @ingroup RandomSearch
 *
 * Long description.
 */
void RandomSearch::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "RandomSearch: call to finalize()\n" );
    terminate();
}

/**
 * @brief Get Search Algorithm Instance description
 * @ingroup RandomSearch
 *
 * Long description.
 */
ISearchAlgorithm* getSearchAlgorithmInstance( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "RandomSearch: call to getSearchAlgorithmInstance()\n" );

    return new RandomSearch();
}

/**
 * @brief Return the current major version number.
 * @ingroup RandomSearch
 **/
int getVersionMajor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "RandomSearch: call to getVersionMajor()\n" );

    return 1;
}

/**
 * @brief Return the current minor version number.
 * @ingroup RandomSearch
 **/
int getVersionMinor( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "RandomSearch: call to getVersionMinor()\n" );

    return 0;
}

/**
 * @brief Return the name of the search algorithm.
 * @ingroup RandomSearch
 **/
string getName( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "RandomSearch: call to getName()\n" );

    return "Random Search";
}

/**
 * @brief Return a short description.
 * @ingroup RandomSearch
 **/
string getShortSummary( void ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "RandomSearch: call to getShortSummary()\n" );

    return "Randomly samples a probability model";
}
