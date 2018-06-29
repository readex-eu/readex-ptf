#include "ExhaustiveSearch.h"
#include "search_common.h"
#include <cassert>
#include <vector>




ExhaustiveSearch::ExhaustiveSearch() : ISearchAlgorithm(), pool_set( NULL ), optimum( -1 ), optimumValue( std::numeric_limits<double>::max() ),
                                       worst( -1 ), worstValue( std::numeric_limits<double>::min() ){ }


void ExhaustiveSearch::initialize( DriverContext*   context,
                                   ScenarioPoolSet* pool_set ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "Exhaustive Search Algorithm selected\n" );
    this->pool_set = pool_set;
}

void ExhaustiveSearch::addObjectiveFunction(ObjectiveFunction *obj){
   objectiveFunctions.push_back(obj);
}



bool ExhaustiveSearch::searchFinished() {
	psc_dbgmsg(PSC_SELECTIVE_DEBUG_LEVEL(AutotuneSearch), "Exhaustive Search: call to searchFinished()\n");

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


int ExhaustiveSearch::getOptimum() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "Exhaustive Search: call to getOptimum()\n" );
    if( optimum == -1 ) {
        psc_abort( "Error: No optimum scenario has been determined yet." );
    }

    assert( path.find( optimum ) != path.end() );
    return optimum;
}


int ExhaustiveSearch::getWorst() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "Exhaustive Search: call to getWorst()\n" );
    if( worst == -1 ) {
        psc_abort( "Error: No worst scenario has been determined yet." );
    }

    assert( path.find( worst ) != path.end() );
    return worst;
}


void ExhaustiveSearch::clear() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "Exhaustive Search: call to clear()\n" );
    path.clear();
    searchSpaces.clear();
    optimum      = -1;
    optimumValue = std::numeric_limits<double>::max();
    worst        = -1;
    worstValue   = std::numeric_limits<double>::min();
    objectiveFunctions.clear();
}


map<int, double > ExhaustiveSearch::getSearchPath() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "Exhaustive Search: call to getSearchPath()\n" );
    return path;
}

void ExhaustiveSearch::addSearchSpace( SearchSpace* searchSpace ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "Exhaustive Search: call to addSearchSpace()\n" );
    searchSpaces.push_back( searchSpace );
}

void ExhaustiveSearch::createScenarios() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "Exhaustive Search: call to createScenarios()\n" );
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneAll ), "Exhaustive Search: Generating multiple variants.\n" );

    if (objectiveFunctions.size()==0){
   	 addObjectiveFunction(new PTF_minObjective(""));
    }

    if (searchSpaces.size()==0){
        psc_errmsg( "Fatal: No search space passed to exhaustive search.\n" );
        abort();
    }

    iterate_SS( 0, searchSpaces.size(), new list<TuningSpecification*>() );
}

void ExhaustiveSearch::iterate_SS( int                         SS_index,
                                   int                         n_SS,
                                   list<TuningSpecification*>* ts ) {
    if( SS_index >= n_SS ) {
        generatescenario( ts );
        return;
    }

    vector<TuningParameter*> tuningParameters;
    tuningParameters = searchSpaces[ SS_index ]->getVariantSpace()->getTuningParameters();
    map<TuningParameter*, int>* variant_SS = new map<TuningParameter*, int>;
    iterate_TP( 0, tuningParameters.size(), &tuningParameters,  variant_SS, ts, SS_index, n_SS ); //0,2,..,..,..,0,1
}


void ExhaustiveSearch::iterate_TP( int TP_index,
                                   int n_TP,
                                   vector<TuningParameter*>* tuningParameters,
                                   map<TuningParameter*, int>* VariantSS,
                                   list<TuningSpecification*>* ts,
                                   int SS_index,
                                   int n_SS ) {
    if( TP_index >= n_TP ) {
        //printf("The value of the TP_index is %d", TP_index);
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
    TuningParameter* tp = ( *tuningParameters )[ TP_index ];

    Restriction* r = tp->getRestriction();
    if( r == NULL || r->getType() != 2 ) {
        //No VectorRangeRestriction
        for( int i = tp->getRangeFrom(); i <= tp->getRangeTo(); i += tp->getRangeStep() ) {
            ( *VariantSS )[ tp ] = i;
            iterate_TP( TP_index + 1, n_TP, tuningParameters, VariantSS, ts, SS_index, n_SS );
        }
    }
    else {
        //VectorRangeRestriction
        vector<int> v = reinterpret_cast<Restriction*>( r )->getElements();
        for( int i = 0; i < v.size(); i++ ) {
            ( *VariantSS )[ tp ] = v[ i ];
            iterate_TP( TP_index + 1, n_TP, tuningParameters, VariantSS, ts, SS_index, n_SS );
        }
    }
}

void ExhaustiveSearch::generatescenario( std::list<TuningSpecification*>* ts ) {
    Scenario*                                 scenario;
    std::list<TuningSpecification*>*          ts1 = new list<TuningSpecification*> ();
    std::list<TuningSpecification*>::iterator TS_iterator;

    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "Exhaustive Search: call to generatescenario()\n" );
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

void ExhaustiveSearch::terminate() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "Exhaustive Search: call to terminate()\n" );
}

void ExhaustiveSearch::finalize() {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "Exhaustive Search: call to finalize()\n" );
    terminate();
}


ISearchAlgorithm* getSearchAlgorithmInstance( void ) {
    return new ExhaustiveSearch();
}

int getVersionMajor( void ) {
    return 1;
}

int getVersionMinor( void ) {
    return 0;
}

string getName( void ) {
    return "Exhaustive Search";
}

string getShortSummary( void ) {
    return "Explores the full space spanned by all tuning parameters.";
}
