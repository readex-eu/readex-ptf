/**
   @file    PerfDynamicsAnalysis.cc
   @ingroup PerfDynamicsAnalysis
   @brief   Performance Dynamics Analysis strategy
   @author  Yury Oleynik
   @verbatim
    Revision:       $Revision$
    Revision date:  Dec 10, 2013
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2015, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#include <iostream>

#include "PerfDynamicsAnalysis.hpp"
#include "psc_errmsg.h"
#include "analysisagent.h"
#include "ScorepMPIStrategy.h"
#include "OpenMPAnalysisScoreP.h"
#include "Importance.h"


#include "PerformanceDynamics.hpp"

#include <TDA_Vector.hpp>
#include <TDA_ScaleSpaceAnalyzer.hpp>
#include <TDA_IOUtils.hpp>

using namespace std;

/**
 * @brief Analysis procedure of the Dynamic Strategy
 *
 * Here is the analysis procedure of the Dynamic Strategy (DS):
 * 1.  Create Static Strategy (SS) in the constructor of this strategy. Static Strategy can be
 * any one-step strategy and is specified by name
 * 2.  DS lets the SS to configure the first experiment and request all the needed metrics
 * 3.  DS configure the DP to perform a burst of iterations of the predefined length
 * 4.  DS checks whether DP is finished and collected all the data
 * 5.  DS sets PDB time access window equal to the burst of iterations just completed
 * 6.  DS lets SS to check for the data and evaluate Static Properties (SP) on the configured time window.
 * 7.  DS asks SS for found SP
 * 8.  DS goes over the iterations in the burst one by one and sets PDB time access window equal to the current iteration
 * 9.  DS re-evaluates each SP found previously by the SS for the current iteration
 * 10. DS request SP severity for the current iteration and stores it into the corresponding vector
 * 11. DS creates Dynamic Properties (DP) for each vector of severity values of the found SPs
 * 12. DS evaluates candidate DPs
 * 13. Found DPs are reported
 */
PerfDynamicsStrategy::PerfDynamicsStrategy( string       staticStrategyName,
                                            Application* application,
                                            int          duration,
                                            bool         pedantic )
    : Strategy( pedantic ) {
    if( duration < 16 ) {
        psc_errmsg( "Provided analysis duration %d is too short. Minimum duration is 16 iterations", duration );
        agent->terminate_analysis();
    }
    /*
     * 1.  Create Static Strategy (SS) in the constructor of this strategy. Static Strategy can be
     * any one-step strategy and is specified by name
     *
     * Currently only MPI and OMP strategies are supported
     */
    if( staticStrategyName == "MPI" ) {
        staticStrategy = new ScorepMPIStrategy( pedantic );
    }
    else if( staticStrategyName == "OMP" ) {
        staticStrategy = new OpenMPAnalysisScoreP( pedantic );
    }
    else if( staticStrategyName == "Importance" ) {
        staticStrategy = new Importance( pedantic );
    }
    else {
        cout << "Unknown static strategy name! MPI will be used by default" << endl;
        staticStrategy = new ScorepMPIStrategy( pedantic );
    }
    cout << endl << "1.  Create Static Strategy (SS) in the constructor of this strategy." << endl;
    this->staticStrategyName = staticStrategyName;

    burst_begin = 0;
    phaseRegion = NULL;
    test_mode   = false;
    if( psc_get_debug_level() >= 7 ) {
        std::cout << "Running performance dynamics analysis in the test mode!" << std::endl;
        test_mode = true;
    }
    if( !test_mode ) {
        burst_length = duration;
    }
    else {
        burst_length = 1;
    }
}


std::list<Property*> PerfDynamicsStrategy::create_initial_candidate_properties_set(
    Region* initial_region ) {
    std::list <Property*> candidates;
    return candidates;     //empty
}

std::list<Property*> PerfDynamicsStrategy::create_next_candidate_properties_set(
    std::list<Property*> staticProps ) {
    std::list<Property*>  candidates;

    //DSP_Utils_IO debug_io("./");

    /*
     * if no properties found by the SS, then nothing to be done for the DS
     */
    if( staticProps.empty() ) {
        return candidates;
    }

    /*
     * debug: analyze only one property
     */
    //Property* first_prop=*(staticProps.begin());
    //staticProps.clear();
    //staticProps.push_back(first_prop);

    /*
     * Use clones of the found SS properties since they will have to be reevaluated in the further analysis
     *
     * DOESN'T WORK! SMTH IS WRONG WITH THE CLONE. SOLVING THE ISSUE WITH A WORKAROUND BELOW
     */
    /*std::list<Property*> tempList;
      for( const auto& prop : staticProps ) {
          tempList.push_back( prop->clone() );
      }
      staticProps = tempList;*/
    /*
     * Go over the SSs, compute severities vectors and create DSs
     */
    cout << endl << "8.  DS goes over all SSs, compute severities vectors and create DSs" << endl;
    cout << endl << "9.  DS goes over the iterations in the burst one by one and sets PDB time access window equal to the current iteration" << endl;
    cout << endl << "10. DS re-evaluates each SP found previously by the SS for the current iteration" << endl;
    cout << endl << "11. DS requests SP severity for the current iteration and stores it into the corresponding vector" << endl;
    cout << endl << "12. DS creates DSP engines for the time series of the severities. This will be shared among the DPs created to evaluate the SP" << endl;
    cout << endl << "13. DS creates dynamic property for this time series" << endl;
    cout << endl << "STATIC Properties list size is " << staticProps.size() << endl;
    for( const auto& prop : staticProps ) {
        vector< double > severities, impacts;
        cout << "Property " << prop->name() << " selected for dynamics analysis" << endl;

        /*
         * Time series identifier
         */
        stringstream id;
        id << "timeseries.";
        id << prop->get_region()->get_ident().file_id << ".";
        id << prop->get_region()->get_ident().rfl << ".";
        id << prop->get_region()->get_name() << ".";
        id << prop->get_rank() << ".";
        id << prop->get_thread() << ".";
        id << prop->id();
        id << prop->subId();

        TDA_Vector time_series;

        if( !test_mode ) {
            /*
             * 8.  DS goes over the iterations in the burst one by one and sets PDB time access window equal to the current iteration
             */
            double max_value = 0.0;
            for( int i = burst_begin; i < burst_begin + burst_length; i++ ) {
                pdb->setTimeWindow( i, i + 1 );
                pdb->setInterpolationType( PDB_INTERPOLATION_ZEROS );
                if( prop->id() == USERREGIONEXECTIME ) {
                    Context phaseContext( phaseRegion, prop->get_rank(), prop->get_thread() );
                    double  phase_time = pdb->get( &phaseContext, PSC_EXECUTION_TIME );
                    severities.push_back( phase_time );
                    impacts.push_back( phase_time );
                }
                else {
                    /*
                     * 9.  DS re-evaluates each SP found previously by the SS for the current iteration
                     */
                    prop->evaluate();
                    /*
                     * 10. DS request SP severity for the current iteration and stores it into the corresponding vector
                     */
                    double  sev = prop->severity();
                    Context phaseContext( phaseRegion, prop->get_rank(), prop->get_thread() );
                    double  phase_time = pdb->get( &phaseContext, PSC_EXECUTION_TIME );
                    double  impact     = sev * phase_time;
                    impacts.push_back( impact );
                    severities.push_back( sev );
                }
            }
            time_series  = TDA_Vector( impacts );
            time_series /= time_series.getMax();

            TDA_IOUtils storage( "./" );
            storage.store_vector( time_series, id.str() );
        }
        else {
            TDA_IOUtils loader( "./" );
            loader.read_vector( time_series, id.str() );
        }



        if( tda_stuffs.find( id.str() ) != tda_stuffs.end() ) {
            throw std::runtime_error( "There is already exists a tda_data with the same id" );
        }

        TDA_Stuff* tda_data = new TDA_Stuff( time_series );

        tda_stuffs[ id.str() ] = tda_data;

        candidates.push_back( new PerformanceDynamics( prop, *tda_data ) );
    }
    return candidates;
}

std::string PerfDynamicsStrategy::name() {
    return "Performance dynamics analysis";
}

bool PerfDynamicsStrategy::reqAndConfigureFirstExperiment(
    Region* initial_region ) {
    /*
     * 2.  DS lets the SS to configure the first experiment and request all the needed metrics
     */
    cout << endl << "2.  DS lets the SS to configure the first experiment and request all the needed metrics" << endl;
    staticStrategy->reqAndConfigureFirstExperiment( initial_region );


    /*
     * 3.  DS configure the DP to perform a burst of iterations of the predefined length
     */
    cout << endl << "3.  DS configures the DP to perform a burst of iterations of the predefined length" << endl;


    dp->setIterationBurstLength( burst_length );


    dp->regionTypeSubscribe( this, USER_REGION );

    return true;
}

bool PerfDynamicsStrategy::evaluateAndReqNextExperiment() {
    /*
     * 4.  DS checks whether DP is finished and collected all the data
     */
    /*
     * Note: during getResult DataProvider will notify this strategy about measured metrics and regions using
     * callback functions. Callback functions will instantiate candidate properties based on the metric type
     * and other conditions.
     */
    cout << endl << "4.  DS checks whether DP is finished and collected all the data" << endl;
    if( dp->getResults() == ALL_INFO_GATHERED ) {
        /* Information was measured. Now the properties can be evaluated. */
    }
    else {
        /* Information is missing */
        return true;
    }

    /*
     * 5.  DS sets PDB time access window equal to the burst of iterations just completed
     */
    cout << endl << "5.  DS sets PDB time access window equal to the burst of iterations just completed" << endl;
    pdb->setTimeWindow( burst_begin, burst_begin + burst_length );
    pdb->setDefaultReductionOperation( PDB_REDUCTION_SUM );
    pdb->setInterpolationType( PDB_INTERPOLATION_ZEROS );

    /*
     * 6.  DS lets SS to check for the data and evaluate Static Properties (SP) on the configured time window.
     */
    cout << endl << "6.  DS lets SS to check for the data and evaluate Static Properties (SP) on the configured time window." << endl;
    staticStrategy->evaluateAndReqNextExperiment();

    /*
     * 7.  DS asks SS for found SP
     */
    cout << endl << "7.  DS requests SS for found SPs" << endl;
    Prop_List static_props = foundProperties;
    agent->print_property_set( static_props, "SET OF FOUND STATIC PROPERTIES", true, false );


    /*
     * create DP properties out of the SPs, steps 8-11
     */
    candProperties = create_next_candidate_properties_set( static_props );


    /* TEMPORAL WORKAROUND FOR THE ISSUE ABOVE: re-evaluate SPs on the whole time interval and store them to the
     * found props list
     *
     */
    pdb->setTimeWindow( burst_begin, burst_begin + burst_length );
    pdb->setDefaultReductionOperation( PDB_REDUCTION_SUM );
    pdb->setInterpolationType( PDB_INTERPOLATION_ZEROS );
    Prop_List::iterator prop_it;
    for( const auto& prop : static_props ) {
        prop->evaluate();
    }
    agent->print_property_set( static_props, "SET OF FOUND STATIC PROPERTIES", true, false );

    /*
     * 12. DS evaluates and refine candidate DPs
     */
    cout << endl << "14. DS evaluates and refines candidate DPs";
    Prop_List new_candidates;
    int       refinement_iter = 0;
    do {
        Prop_List::iterator prop_it;
        if( psc_get_debug_level() >= 2 ) {
            stringstream title;
            title << "SET OF CANDIDATE DYNAMIC PROPERTIES, REFINEMENT ITERATION " << refinement_iter;
            agent->print_property_set( candProperties, title.str().c_str(), false, false );
        }
        for( const auto& prop : candProperties ) {
            prop->evaluate();
            if( prop->condition() ) {
                foundPropertiesLastStep.push_back( prop );
                foundProperties.push_back( prop );
            }
            Prop_List new_props = prop->next();
            new_candidates.insert( new_candidates.end(), new_props.begin(), new_props.end() );
        }
        candProperties = new_candidates;
        new_candidates.clear();
        refinement_iter++;
    }
    while( !candProperties.empty() );
    /*
     * 13. Found DPs are reported
     */
    if( psc_get_debug_level() >= 2 ) {
        agent->print_property_set( foundProperties, "SET OF FOUND DYNAMIC PROPERTIES", true, false );
    }

    return false;
}


void PerfDynamicsStrategy::configureNextExperiment() {
    psc_dbgmsg( 1, "Calling dp->transfer_requests_to_processes();\n", strategy_steps );
    dp->transfer_requests_to_processes();
}

void PerfDynamicsStrategy::region_definition_received_callback( Region* reg ) {
    if( reg->get_type() == USER_REGION ) {
        phaseRegion = reg;
        cout << "DP received phase region" << endl;
    }
}
