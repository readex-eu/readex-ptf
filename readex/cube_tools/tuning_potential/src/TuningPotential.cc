/**
 * TuningPotential.cc
 * Computes tuning potential for all regions
 * @ param1:
 */

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <algorithm>
#include <numeric>
#include <boost/format.hpp>
#include <vector>
#include <list>
#include <map>
#include <iomanip>



#include "../include/TuningPotential.h"
#include "../include/ProcessingIntensity.h"
#include "../../common_incl/helper.h"

using namespace std;
using namespace cube;
using namespace services;
using namespace boost;


/**
 * @Brief: Computes and detect dynamism for all significant regions
 * @param phase_node
 * @param cnodes list of significant call nodes
 * @param rgion_names list of significant region names
 * @param in_cube name of the cube file
 * @param threshold Threshold of Dynamism depending on avg of phase
 * @return
 */
list< SignificantRegion* >
ComputeIntraPhaseTP( Cnode*            phase_node,
                     vector< Cnode* >& cnodes,
                     vector< string >& region_names,
                     Cube*             in_cube ) {
    vector< Cnode* >           sig_nodes;
    list< SignificantRegion* > sig_region_list;
    const vector< Process* >&  processes = in_cube->get_procv();
    Metric*                    met_any   = in_cube->get_met( "time" );

    for( const auto& cnode : cnodes )
    {
        string c_name = cnode->get_callee()->get_name();
        for( const auto& region : region_names )
        {
            if( strcasecmp( region.c_str(), c_name.c_str() ) == 0 )
                sig_nodes.push_back( cnode );
        }
    }

    //double avg_phase_transfer_intnsty = getPhaseNodeAvgProcIntnsty( phase_node, in_cube, processes[0] );

    vector< Cnode* > variant_nodes;

    for( const auto& sig_node : sig_nodes )
    {
        Region *region = sig_node->get_callee();

        Value* met_tau_val = in_cube->get_sev_adv( met_any,
                                                   CUBE_CALCULATE_INCLUSIVE,
                                                   sig_node,
                                                   CUBE_CALCULATE_EXCLUSIVE,
                                                   processes[ 0 ],
                                                   CUBE_CALCULATE_INCLUSIVE );

        if( met_tau_val->myDataType() != CUBE_DATA_TYPE_TAU_ATOMIC )
        {
            std::cout << "Please instrument the application with CUBE_TUPLE profiling format( Intra-phase )" << endl;

            return sig_region_list;
        }

        /* Casting from Value to TauAtomicValue */
        TauAtomicValue* tau_atomic_tuple = ( TauAtomicValue* ) met_tau_val;
        string stat_tuple                = tau_atomic_tuple->getString();
        vector< string > tau_atomic_val  = ValueParser( stat_tuple );
        double stand_deviation           = tau_atomic_val.at( 4 ) == "-nan" ? 0.0 : stod( tau_atomic_val.at( 4 ) );
        uint64_t N                       = stod( tau_atomic_val.at( 0 ) );
        double mean                      = stod( tau_atomic_val.at( 3 ) );
        double minValue                  = tau_atomic_tuple->getMinValue().getDouble();
        double maxValue                  = tau_atomic_tuple->getMaxValue().getDouble();

        if ( N != 0 )
        {
            double phasetime = getPhaseNodeTime( phase_node, in_cube, met_any, processes[ 0 ] );
            /*std::cout << "REGION NAME: " << sig_node->get_callee()->get_name() << endl;
            std::cout << "Exec_time: " << mean * N << endl;
            std::cout << "Exec_time of Phase: " << phasetime << endl;*/
            double dyn_avg_phase = ( mean ) * N / getPhaseNodeTime( phase_node, in_cube, met_any, processes[ 0 ] ) * 100; // weight
            double dev_perc_reg  = 0.0;

            if ( mean != 0.0 )
            {
                dev_perc_reg = ( stand_deviation / mean ) * 100 ;  // coefficient of variation OR degree of spreadness
            }
            double dev_perc_phase = ( stand_deviation / getPhaseNodeAvgValue( phase_node, in_cube, met_any, processes[ 0 ] ) ) * 100;

            DynamismMetric dyn_ti = DynamismMetric( met_any->get_uniq_name(), minValue, maxValue, ( mean ) * N, dev_perc_reg, dev_perc_phase, dyn_avg_phase );
            vector< DynamismMetric > dyn_metrics;
            dyn_metrics.push_back( dyn_ti );
            double avg_compute_intensity = getProcessingIntensity( sig_node, in_cube );

            if( avg_compute_intensity != -1 )
            {
                double avg_phase_compute_intensity = getPhaseNodeAvgProcIntnsty( phase_node, in_cube, processes[ 0 ] );
                double variation_compute_intensity = ( avg_compute_intensity / avg_phase_compute_intensity ) * 100;
                std::string str                    = "Transfer Intensity";
                DynamismMetric dyn_t_i             = DynamismMetric( str, 0.0, 0.0, 0.0, 0.0, 0.0, variation_compute_intensity );
                dyn_metrics.push_back( dyn_t_i );
            }
            else
            {
                std::cout << " Please run the application with PAPI Metric plugin to get intensity information" << endl;
                std::string str        = "Transfer Intensity";
                DynamismMetric dyn_t_i = DynamismMetric( str, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 );
                dyn_metrics.push_back( dyn_t_i );
            }

            if( !dyn_metrics.empty() )
            {
                SignificantRegion* sig_region = new SignificantRegion;

                sig_region->name            = region->get_name();
                //sig_region->source_location = region->get_mod();
                //sig_region->line_no         = region->get_begn_ln();
                sig_region->granularity     = computeGranularity( sig_node, in_cube );

                sig_region->dynamism_metrics.assign( dyn_metrics.begin(), dyn_metrics.end() );
                sig_region_list.push_back( sig_region );
                variant_nodes.push_back( sig_node );
            }
        }
    }

    if ( !variant_nodes.empty() )
    {
        printPotentialResults( sig_region_list );
    }
    else
    {
        std::cout << endl << " There is no intra-phase dynamism" << endl;
    }

    return sig_region_list;
}

/**
 *
 * @Brief: Print the tuning potential results
 * @param
 * @s_regions: pointer to dnyamism metric
 * @size: no of significant regions
 * @no_m: # of metrices
 */
void
printPotentialResults( list< SignificantRegion* >& regions )
{
    std::cout << "Significant region information\n";
    std::cout << "==============================" << endl;

    std::cout << format( "%-28s %-15s %-12s %-15s %-15s %-15s %-15s\n" )
            % "Region name"  %  "Min(t)" % "Max(t)" % "Time" % "Time Dev.(%Reg)" % "Ops/L3miss" %  "Weight(%Phase)"     ;
    for ( const auto& region : regions )
    {
        if ( region->dynamism_metrics.size() > 0 )
        {
            std::cout << format( "%-25s %8.3f          %8.3f     %8.3f         %5.1f       %7.0f            %5.0f \n" ) %
                         region->name.substr( 0, 25 ) %
                         region->dynamism_metrics[ 0 ].min %
                         region->dynamism_metrics[ 0 ].max %
                         region->dynamism_metrics[ 0 ].execTime %
                         region->dynamism_metrics[ 0 ].dev_perc_reg %
                         region->dynamism_metrics[ 1 ].dyn_perc_phase %
                         region->dynamism_metrics[ 0 ].dyn_perc_phase;
        }
    }
}

/**
 * @brief Computes and detect dynamism across all phases
 * @param phase_node
 * @param in_cube name of the cube file
 * @param threshold  Threshold of Dynamism
 * @return
 */
PhaseRegion*
ComputeInterPhaseTP( Cnode* phase_node,
                     Cube*  in_cube,
                     double threshold )
{
    Metric* met_t                       = in_cube->get_met( "time" );
    const vector< Process* >& processes = in_cube->get_procv();

    Value* complex_value = in_cube->get_sev_adv( met_t,
                                                 CUBE_CALCULATE_INCLUSIVE,
                                                 phase_node,
                                                 CUBE_CALCULATE_EXCLUSIVE,
                                                 processes[ 0 ],
                                                 CUBE_CALCULATE_INCLUSIVE );

    if( complex_value->myDataType() != CUBE_DATA_TYPE_TAU_ATOMIC )
    {
        std::cout << "Please instrument the application with CUBE_TUPLE profiling format( Inter-phase ) " << endl;
        return NULL;
    }

    /* Casting from Value to TauAtomicValue */
    TauAtomicValue* tau_atomic_tuple = ( TauAtomicValue* )complex_value;
    vector< string > tau_atomic_val  = ValueParser( tau_atomic_tuple->getString() );
    double standard_deviation        = stod( tau_atomic_val.at( 4 ) );
    double mean                      = stod( tau_atomic_val.at( 3 ) );
    double minValue                  = tau_atomic_tuple->getMinValue().getDouble();
    double maxValue                  = tau_atomic_tuple->getMaxValue().getDouble();
    uint64_t N                       = stod( tau_atomic_val.at( 0 ) );
    if( std::isnan( standard_deviation ) )
        standard_deviation = 0.0;

    double variation_percentage = ( ( maxValue - minValue ) / mean ) * 100;
    double dev_percentage       = ( standard_deviation / mean ) * 100;

    std::cout << endl << "Phase information" << endl;
    std::cout <<         "=================" << endl;
    std::cout << format( "%-20s %-20s %-20s %-20s %-20s %-20s \n" )
                 % "Min" % "Max" % "Mean"% "Time"% "Dev.(% Phase)" % "Dyn.(% Phase)";

    std::cout << format( "%-20s %-20s %-20s %-20s %-20s %-20s  \n" ) %
                 minValue % maxValue %  mean % ( N * mean ) % dev_percentage % variation_percentage;

    PhaseRegion* phase = new PhaseRegion;

    phase->name         = phase_node->get_callee()->get_name();
    phase->granularity  = computeGranularity( phase_node, in_cube );
    phase->min          = minValue;
    phase->max          = maxValue;
    phase->mean         = mean;
    phase->absTime      = N * mean;
    phase->dev_perc     = dev_percentage;
    phase->var_perc     = variation_percentage;
    phase->has_dynamism = variation_percentage > threshold;

    return phase;
}

/**
 * @brief Get the avaergae value of phase node
 * @param pnode
 * @param input
 * @param met_any
 * @param proc
 * @return
 */
double
getPhaseNodeAvgValue( Cnode*   pnode,
                      Cube *   input,
                      Metric*  met_any,
                      Process* proc )
{
    Value* complex_value = input->get_sev_adv( met_any,
                                               CUBE_CALCULATE_INCLUSIVE,
                                               pnode,
                                               CUBE_CALCULATE_EXCLUSIVE,
                                               proc,
                                               CUBE_CALCULATE_INCLUSIVE );

    /* Casting from Value to TauAtomicValue */
    TauAtomicValue* tau_atomic_tuple = ( TauAtomicValue* )complex_value;
    vector< string > tau_atomic_val  = ValueParser( tau_atomic_tuple->getString() );
    double average                   = stod( tau_atomic_val.at( 3 ) );

    return average;
}

/**
 * @brief Get the absolute value of phase node
 * @param pnode
 * @param input
 * @param met_any
 * @param proc
 * @return
 */
double
getPhaseNodeTime( Cnode*   pnode,
                  Cube*    input,
                  Metric*  met_any,
                  Process* proc )
{

    Value* complex_value = input->get_sev_adv( met_any,
                                               CUBE_CALCULATE_INCLUSIVE,
                                               pnode,
                                               CUBE_CALCULATE_EXCLUSIVE,
                                               proc,
                                               CUBE_CALCULATE_INCLUSIVE );

    /* Casting from Value to TauAtomicValue */
    TauAtomicValue* tau_atomic_tuple = ( TauAtomicValue* )complex_value;
    vector< string > tau_atomic_val  = ValueParser( tau_atomic_tuple->getString() );
    double average                   = stod( tau_atomic_val.at( 3 ) );
    uint64_t N                       = stod( tau_atomic_val.at( 0 ) );

    return average * N;
}
