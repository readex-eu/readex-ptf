/**
 * ProcessingIntensity.cc
 * Computes transfer intensity
 * transfer_intensity = PAPI_TOT_INS/PAPI_L3_TCM
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


#include"../include/TuningPotential.h"
#include "../include/ProcessingIntensity.h"
#include "../../common_incl/helper.h"



using namespace std;
using namespace cube;
using namespace services;
using namespace boost;


double
getProcessingIntensity( Cnode* sig_node,
                        Cube*  in_cube )
{
    /* Get PAPI_TOT_INS and PAPI_L3_TCM metrices */
    Metric* met_papi_ins = in_cube->get_met( "PAPI_TOT_INS" );
    Metric* met_papi_tcm = in_cube->get_met( "PAPI_L3_TCM" );

    if ( met_papi_ins  == NULL || met_papi_tcm  == NULL )
        return -1;

    const vector< Process* >& processes = in_cube->get_procv();

    Region *region     = sig_node->get_callee();
    Value *met_tau_ins = in_cube->get_sev_adv( met_papi_ins,
                                               CUBE_CALCULATE_INCLUSIVE,
                                               sig_node,
                                               CUBE_CALCULATE_EXCLUSIVE,
                                               processes[ 0 ],
                                               CUBE_CALCULATE_INCLUSIVE );
    Value *met_tau_tcm = in_cube->get_sev_adv( met_papi_tcm,
                                               CUBE_CALCULATE_INCLUSIVE,
                                               sig_node,
                                               CUBE_CALCULATE_EXCLUSIVE,
                                               processes[ 0 ],
                                               CUBE_CALCULATE_INCLUSIVE );

    if( met_tau_ins->myDataType() != CUBE_DATA_TYPE_TAU_ATOMIC )
    {
        cout << "Please instrument the application with CUBE_TUPLE profiling format( Intra-phase )" << endl;
        return -1;
    }

    /* Casting from Value to TauAtomicValue */
    TauAtomicValue* tau_tuple_ins = ( TauAtomicValue* ) met_tau_ins;
    TauAtomicValue* tau_tuple_tcm = ( TauAtomicValue* ) met_tau_tcm;

    vector< string > tau_val_ins = ValueParser( tau_tuple_ins->getString() );
    vector< string > tau_val_tcm = ValueParser( tau_tuple_tcm->getString() );

    double avg_ins = stod( tau_val_ins.at( 3 ) );
    double avg_tcm = stod( tau_val_tcm.at( 3 ) );

    double N_tcm = stod( tau_val_tcm.at( 0 ) );
    if ( N_tcm == 0 )
        avg_tcm = 1.0;

    double avg_trans_intensity = avg_ins / avg_tcm ;

    //cout << "Region Name:: " << region->get_name()  << "    Trans_intensity:: " << avg_trans_intensity << endl;

    return avg_trans_intensity;
}

double
getPhaseNodeAvgProcIntnsty( Cnode*   pnode,
                            Cube*    input,
                            Process* proc )
{
    /* Get PAPI_TOT_INS and PAPI_L3_TCM metrices */
    Metric* met_papi_ins = input->get_met( "PAPI_TOT_INS" );
    Metric* met_papi_tcm = input->get_met( "PAPI_L3_TCM" );

    if ( met_papi_ins  == NULL || met_papi_tcm == NULL )
        return -1;

    Value* complex_value_ins = input->get_sev_adv( met_papi_ins,
                                                   CUBE_CALCULATE_INCLUSIVE,
                                                   pnode,
                                                   CUBE_CALCULATE_EXCLUSIVE,
                                                   proc,
                                                   CUBE_CALCULATE_INCLUSIVE );
    Value* complex_value_tcm = input->get_sev_adv( met_papi_tcm,
                                                   CUBE_CALCULATE_INCLUSIVE,
                                                   pnode,
                                                   CUBE_CALCULATE_EXCLUSIVE,
                                                   proc,
                                                   CUBE_CALCULATE_INCLUSIVE );

    /* Casting from Value to TauAtomicValue */
    TauAtomicValue* tau_tuple_ins = ( TauAtomicValue* )complex_value_ins;
    TauAtomicValue* tau_tuple_tcm = ( TauAtomicValue* )complex_value_tcm;

    vector< string > tau_val_ins = ValueParser( tau_tuple_ins->getString() );
    vector< string > tau_val_tcm = ValueParser( tau_tuple_tcm->getString() );

    double average_ins = stod( tau_val_ins.at( 3 ) );
    double average_tcm = stod( tau_val_tcm.at( 3 ) );

    double N_tcm = stod( tau_val_tcm.at( 0 ) );
    if( N_tcm == 0 )
        average_tcm = 1;

    double avg_trans_intensity = average_ins / average_tcm;

    return avg_trans_intensity;
}
