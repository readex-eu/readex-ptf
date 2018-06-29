#include "ExhaustiveATPSearch.h"
#include "search_common.h"
#include <cassert>
#include <vector>




ExhaustiveATPSearch::ExhaustiveATPSearch() : ISearchAlgorithm(), pool_set( NULL ), optimum( -1 ), optimumValue( std::numeric_limits<double>::max() ),
                                       worst( -1 ), worstValue( std::numeric_limits<double>::max() ), atpServc(NULL){ }


void ExhaustiveATPSearch::initialize( DriverContext*   context,
                                   ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "ExhaustiveATP Search Algorithm selected\n" );
    this->pool_set = pool_set;
}

void ExhaustiveATPSearch::addObjectiveFunction(ObjectiveFunction *obj){
   objectiveFunctions.push_back(obj);
}



bool ExhaustiveATPSearch::searchFinished() {
	psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotuneSearch), "ExhaustiveATP Search: call to searchFinished()\n");

	while (!scenarioIds.empty()) {
		int scenario_id = scenarioIds.front();
		scenarioIds.pop();
		//for (int i = 0; i < objectiveFunctions.size(); i++) {
		int i=0;
			double objValue = objectiveFunctions[i]->objective(scenario_id, pool_set->srp);
			path[scenario_id] = objValue;
			Scenario* scenario= pool_set->fsp->getScenarioByScenarioID(scenario_id);
			//scenario->addResult(objectiveFunctions[i]->getName(), objValue);

			if (optimum == -1 || objValue < optimumValue) {
				optimum = scenario_id;
				optimumValue = objValue;
			}

			if (worst == -1 || objValue > worstValue) {
			    worst = scenario_id;
			    worstValue = objValue;
			}
		//}
	}

	return true;
}


int ExhaustiveATPSearch::getOptimum() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "ExhaustiveATP Search: call to getOptimum()\n" );
    if( optimum == -1 ) {
        psc_abort( "Error: No optimum scenario has been determined yet." );
    }

    assert( path.find( optimum ) != path.end() );
    return optimum;
}


int ExhaustiveATPSearch::getWorst() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "ExhaustiveATP Search: call to getWorst()\n" );
    if( worst == -1 ) {
        psc_abort( "Error: No worst scenario has been determined yet." );
    }

    assert( path.find( worst ) != path.end() );
    return worst;
}


void ExhaustiveATPSearch::clear() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "ExhaustiveATP Search: call to clear()\n" );
    path.clear();
    searchSpaces.clear();
    optimum      = -1;
    optimumValue = std::numeric_limits<double>::max();
    worst        = -1;
    worstValue   = std::numeric_limits<double>::max();
    objectiveFunctions.clear();
}


map<int, double > ExhaustiveATPSearch::getSearchPath() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "ExhaustiveATP Search: call to getSearchPath()\n" );
    return path;
}

void ExhaustiveATPSearch::addSearchSpace( SearchSpace* searchSpace ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "ExhaustiveATP Search: call to addSearchSpace()\n" );
    searchSpaces.push_back( searchSpace );
}

void ExhaustiveATPSearch::createScenarios() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "ExhaustiveATP Search: call to createScenarios()\n" );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAll ), "ExhaustiveATP Search: Generating multiple variants.\n" );

    if (objectiveFunctions.size()==0){
   	 addObjectiveFunction(new PTF_minObjective(""));
    }

    iterate_SS( 0, searchSpaces.size(), new list<TuningSpecification*>() );
}

void ExhaustiveATPSearch::iterate_SS( int                         SS_index,
                                   int                         n_SS,
                                   list<TuningSpecification*>* ts ) {
    if( SS_index >= n_SS ) {
        generatescenario( ts );
        return;
    }
    std::unordered_map<int, std::vector<int32_t>> validCombinations;
    vector<TuningParameter*> tuningParameters;
    /* Query for valid configurations with default domain. Currently the first domain is the default domain */
    std::string domain_name = searchSpaces[ SS_index ]->getDomain();
    if(!domain_name.empty())
        validCombinations = atpServc->getValidConfigurations(domain_name.c_str());
    tuningParameters = searchSpaces[ SS_index ]->getVariantSpace()->getTuningParameters();
    map<TuningParameter*, int>* variant_SS = new map<TuningParameter*, int>;
    iterate_valid_combination( 0, tuningParameters.size(), validCombinations, &tuningParameters,  variant_SS, ts, SS_index, n_SS ); //0,6,..,..,..,0,1
}

void ExhaustiveATPSearch::iterate_valid_combination(  int TP_index,
                                                      int n_TP,
                                                      std::unordered_map<int, std::vector<int32_t>>& validCombinations,
                                                      vector<TuningParameter*>* tuningParameters,
                                                      map<TuningParameter*, int>* VariantSS,
                                                      list<TuningSpecification*>* ts,
                                                      int SS_index,
                                                      int n_SS ) {
    if( TP_index >= n_TP ) {
        Variant* variant;
        variant = new Variant( *VariantSS );
        //To get the the valid rtss I have to get rtss for the significant regions
        if( withRtsSupport() ){
            std::list<Rts*>* rtsList;
            rtsList = new list<Rts*>;
            vector<Rts*> rtsVector = searchSpaces[ SS_index ]->getRts();
            if (rtsVector.size()>0)
                rtsList->push_back(rtsVector[0] );
            ts->push_back( new TuningSpecification( variant, rtsList ) );

        } else {
            std::list<Region*>* regions;
            regions = new list<Region*>;
            regions->push_back( searchSpaces[ SS_index ]->getRegions()[ 0 ] );
            ts->push_back( new TuningSpecification( variant, regions ) );
        }
        iterate_SS( SS_index + 1, n_SS, ts );
        ts->pop_back(); //?
        return;
    }
    if (!validCombinations.empty())
    {
        for ( auto it = validCombinations.begin(); it!= validCombinations.end(); ++it ){
            for (int j = 0; j < n_TP; ++j) { //elems_per_combination = no. of atps = n_TP
                TuningParameter* tp = ( *tuningParameters )[ j ];
                //psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(ApplTuningParameter), "ATP NAME: %s \n", tp->getName().c_str());
                //psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(ApplTuningParameter), "ATP VALUE: %d \n", it->second.at(j));
                ( *VariantSS )[ tp ] = it->second.at(j);
            }
            iterate_valid_combination( n_TP, n_TP, validCombinations, tuningParameters,  VariantSS, ts, SS_index, n_SS );
        }
    }
    else
    {
        for (int j = 0; j < n_TP; ++j) {
            TuningParameter* tp = ( *tuningParameters )[ j ];
            Restriction* r = tp->getRestriction();
            if (r->getType() == 2)
            {
                //VectorRangeRestriction
                vector<int> v = reinterpret_cast<Restriction*>( r )->getElements();
                for( int i = 0; i < v.size(); i++ ) {
                    ( *VariantSS )[ tp ] = v[ i ];
                    //psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "ExhaustiveATP Search: v[ i ] %d \n", v[ i ] );
                }
            }
        }
        iterate_valid_combination( n_TP, n_TP, validCombinations, tuningParameters, VariantSS, ts, SS_index, n_SS );
    }
}

void ExhaustiveATPSearch::generatescenario( std::list<TuningSpecification*>* ts ) {
    Scenario*                                 scenario;
    std::list<TuningSpecification*>*          ts1 = new list<TuningSpecification*> ();
    std::list<TuningSpecification*>::iterator TS_iterator;

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "ExhaustiveATP Search: call to generatescenario()\n" );
    for( TS_iterator = ts->begin(); TS_iterator != ts->end(); TS_iterator++ ) {
        TuningSpecification* tuningSpec;
        tuningSpec = ( *TS_iterator );
        VariantContext       context = tuningSpec->getVariantContext();
        Variant*             var     = const_cast<Variant*>( tuningSpec->getVariant() );
        TuningSpecification* temp    = new TuningSpecification( var, context.context_union.entity_list );
        ts1->push_back( temp );
    }

    scenario = new Scenario( ts1 );
    scenarioIds.push( scenario->getID() );
    //scenario->print();
    pool_set->csp->push( scenario );
}

void ExhaustiveATPSearch::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "ExhaustiveATP Search: call to terminate()\n" );
}

void ExhaustiveATPSearch::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "ExhaustiveATP Search: call to finalize()\n" );
    terminate();
}

ISearchAlgorithm* getSearchAlgorithmInstance( void ) {
    return new ExhaustiveATPSearch();
}

int getVersionMajor( void ) {
    return 1;
}

int getVersionMinor( void ) {
    return 0;
}

string getName( void ) {
    return "Exhaustive ATP Search";
}

string getShortSummary( void ) {
    return "Explores the full space spanned by all tuning parameters.";
}
