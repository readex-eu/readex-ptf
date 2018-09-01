/****************************************************************************
**  CUBE        http://www.score-p.org/                                    **
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 1998-2015                                                **
**  Forschungszentrum Juelich GmbH, Juelich Supercomputing Centre          **
**                                                                         **
**  Copyright (c) 2009-2015                                                **
**  German Research School for Simulation Sciences GmbH,                   **
**  Laboratory for Parallel Programming                                    **
**                                                                         **
**  This software may be modified and distributed under the terms of       **
**  a BSD-style license.  See the COPYING file in the package base         **
**  directory for details.                                                 **
****************************************************************************/

/**
 * \file helper.cpp
 * \brief Defines a set of helping functions.
 *
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <iomanip>
#include <string>
#include <algorithm>
#include <numeric>
#include <list>
#include <map>
#include <vector>

#include <cubelib/Cube.h>
#include <cubelib/CubeCnode.h>
#include <cubelib/CubeRegion.h>
#include <cubelib/CubeThread.h>

#include "../common_incl/helper.h"
#include "../common_incl/Predicates.h"

using namespace std;

namespace cube
{
/**
 * Transforms a callpathtype \f$ \longrightarrow \f$ string.
 */
string
Callpathtype2String( CallpathType ct )
{
    switch ( ct )
    {
        case MPI:
            return "MPI";
        case OMP:
            return "OMP";
        case USR:
            return "USR";
        case COM:
            return "COM";
        case REC:
            return "REC";
        case SEN:
            return "SEN";
        case SENREC:
            return "SENREC";
        case COL:
            return "COL";
        case FORK:
            return "FORK";
        case LOCK:
            return "LOCK";
        case CRIT:
            return "CRIT";
        case EPK:
            return "EPK";
        case NUL:
            return "NUL";
        case MPI_RMA_OP:
            return "MPI_RMA_OP";
        case MPI_RMA_GATS:
            return "MPI_RMA_GATS";
        case MPI_RMA_COLL:
            return "MPI_RMA_COLL";
        case MPI_RMA_LOCK:
            return "MPI_RMA_LOCK";
        case MPI_RMA_UNLOCK:
            return "MPI_RMA_UNLOCK";
        default:
            break;
    }
    return "UNDEF";
}
/**
 * Transforms a string \f$ \longrightarrow \f$ callpathtype.
 */
CallpathType
String2Callpathtype( const string& name )
{
    /*
       classify a functionname into two classes:
       MPI (collective, send, receive, MPI) and USR
     */
    string region = lowercase( name );
    // cout << "Callpath name" << region << endl;

    if ( region.length() <= 4 )
    {
        return USR;
    }
    else
    {
        string suffix = name.substr( 4 );

        if ( is_mpi_api( region ) )
        {
            if ( is_mpi_coll( region ) || is_mpi_sync_coll( region ) )
            {
                return COL;
            }
            else if ( is_mpi_rma( region ) )
            {
                return MPI_RMA_OP;
            }
            else if ( region == "mpi_win_fence"  ||
                      region == "mpi_win_create" ||
                      region == "mpi_win_free" )
            {
                return MPI_RMA_COLL;
            }
            else if ( is_mpi_sync_rma_active( region ) )
            {
                // Fence is already handled above
                return MPI_RMA_GATS;
            }
            else if ( region == "mpi_win_lock" )
            {
                return MPI_RMA_LOCK;
            }
            else if ( region == "mpi_win_unlock" )
            {
                return MPI_RMA_UNLOCK;
            }
            else if ( region.substr( 0, 12 ) == "mpi_sendrecv" )
            {
                return SENREC;
            }
            else if ( region.find( "send" ) != string::npos  ||
                      region.find( "start" ) != string::npos )
            {
                return SEN;
            }
            else if ( region.find( "recv" ) != string::npos )
            {
                return REC;
            }
            else
            {
                return MPI;
            }
        }
        else if ( name.compare( 0, 4, "omp_" ) == 0 || name.compare( 0, 5, "!$omp" ) == 0 )
        {
            if ( name.compare( 0, 4, "omp_" ) == 0 && suffix.find( "lock" ) != string::npos )
            {
                return LOCK;
            }
            else if ( name.compare( 0, 5, "!$omp" ) == 0 )
            {
                string construct = name.substr( 5, name.find_first_of( '@', 5 ) - 5 );

                if ( construct.find( "parallel" ) != string::npos )
                {
                    return FORK;
                }
                if ( construct.find( "critical" ) != string::npos )
                {
                    return CRIT;
                }
            }
            return OMP;
        }

        return USR;
    }
}
/**
 * Get a callpath in "directory notation" (like func1/func2/func3/../cnode1).
 */
string
get_callpath_for_cnode( Cnode* cn )
{
    string callpath;
    Cnode* ptr = cn->get_parent();

    while ( true )
    {
        if ( ptr == 0 )
        {
            break;
        }
        else
        {
            const Region* region = ptr->get_callee();
            string        nname  = region->get_name();
            callpath = "/" + nname + callpath;
            ptr      = ptr->get_parent();
        }
    }

    const Region* region = cn->get_callee();
    callpath = callpath + "/" + region->get_name();
    return callpath;
}
/**
 * Does sum all values for given metric in a CUBE.
 */
double
get_atotalt( Cube*         input,
             const string& metricname )
{
    Metric*                  metric  = input->get_met( metricname );

    if ( metric == 0 )
    {
        return -1.0;
    }

    const vector< Cnode* >&  cnodes  = input->get_cnodev();
    const vector< Thread* >& threads = input->get_thrdv();
    double                   total( 0.0 );

    for ( size_t i = 0; i < cnodes.size(); i++ )
    {
        double max( 0.0 );
        for ( size_t j = 0; j < threads.size(); j++ )
        {
            double nval = input->get_sev( metric, cnodes[ i ], threads[ j ] );
            if ( nval > max )
            {
                max = nval;
            }
        }
        total += max;
    }

    return total;
}

/**
 * Gets to every  calltype a factor.
 */
unsigned long long
TypeFactor( const string& name )
{
    const CallpathType cpType = String2Callpathtype( name );
    unsigned long long d;
    switch ( cpType )
    {
        case USR:
            d = 24;
            break;
        case COL:
            d = 40;
            break;
        case SEN:
            d = 50;
            break;
        case REC:
            d = 46;
            break;
        case SENREC:
            d = 72;
            break;
        case MPI:
            d = 24;
            break;
        case OMP:
            d = 24;
            break;
        case FORK:
            d = 44;
            break;
        case LOCK:
            d = 34;
            break;
        case CRIT:
            d = 44;
            break;
        case MPI_RMA_OP:
            d = 72;
            break;
        case MPI_RMA_GATS:
            d = 33;
            break;
        case MPI_RMA_COLL:
            d = 28;
            break;
        case MPI_RMA_LOCK:
            d = 45;
            break;
        case MPI_RMA_UNLOCK:
            d = 44;
            break;
        default:
            d = 0;
            break;
    }
    return d;
}

/**
 * checks for the call node type
 * Returns true if it is a user region
 */
bool
isUserNode( const Cnode* c_node )
{
    Region* r            = c_node->get_callee();
    CallpathType cp_type = String2Callpathtype( r->get_name() );

    switch ( cp_type )
    {
        case MPI:
            return false; // return if MPI
        case OMP:
            return false;
        case USR:
            return true;
        case COM:
            return false;
        case EPK:
            return false;
        case REC:
            return false;
        case SEN:
            return false;
        case SENREC:
            return false;
        case COL:
            return false;
        case FORK:
            return true;
        case LOCK:
            return false;
        case CRIT:
            return false;
        case MPI_RMA_OP:
            return false;
        case MPI_RMA_GATS:
            return false;
        case MPI_RMA_LOCK:
            return false;
        case MPI_RMA_COLL:
            return false;
        case MPI_RMA_UNLOCK:
            return false;
        case NUL:
            return false;
        default:
            return false;
    }
}

/**
 * checks for the call node type
 * Returns true if it is a parallel region
 */
bool
isForkNode( const Cnode* c_node )
{
    Region* r            =  c_node->get_callee();
    CallpathType cp_type = String2Callpathtype( r->get_name() );

    switch ( cp_type )
    {
        case FORK:
            return true;
        default:
            return false;
    }
}

/**
 * Compute Granularity(Time/OCC)
 * @param ct_node
 * @param input
 * @return
 */
double
computeGranularity( Cnode* ct_node,
                    Cube*  input )
{
    Metric* met_t     = input->get_met( "time" );
    Metric* met_visit = input->get_met( "visits" );
    Region* region    = ct_node->get_callee();

    //Anamika, this is strange. The correct value is returned with EXCLUSIVE, but it should be INCLUSIVE
    //Modified code according to hint to take only first thread.

    Value* met_tau_t = input->get_sev_adv( met_t,
                                           CUBE_CALCULATE_INCLUSIVE,
                                           region,
                                           CUBE_CALCULATE_EXCLUSIVE,
                                           input->get_thrdv()[0],
                                           CUBE_CALCULATE_EXCLUSIVE );

    double time, time1=0.0;
    double occ, occ1=0.0;
    if( met_tau_t->myDataType() == CUBE_DATA_TYPE_TAU_ATOMIC )
    {
        /* Casting from Value to TauAtomicValue */
        TauAtomicValue* tau_tuple_t = ( TauAtomicValue* ) met_tau_t;
        string stat_tuple_t         = tau_tuple_t->getString();
        //cout << stat_tuple_t << endl;
        vector< string > tau_val_t = ValueParser( stat_tuple_t );
        //cout << ct_node->get_callee()->get_name() <<"  " << tau_val_t.at( 3 ) <<"  "<< tau_val_t.at( 0 ) << endl;
        double avg_t;
        double N_time;
        try {
             avg_t               = stod( tau_val_t.at( 3 ) );
             N_time              = stod( tau_val_t.at( 0 ) );
        } catch (exception &e) {
             avg_t               = 0.0;
             N_time              = 1;
        }

        time = N_time * avg_t;
        occ =  N_time;
    }
    else
    {
      time = input->get_sev( met_t, CUBE_CALCULATE_INCLUSIVE, region, CUBE_CALCULATE_INCLUSIVE , input->get_thrdv()[0], CUBE_CALCULATE_EXCLUSIVE);
      occ  = input->get_sev( met_visit, CUBE_CALCULATE_INCLUSIVE, region, CUBE_CALCULATE_EXCLUSIVE, input->get_thrdv()[0], CUBE_CALCULATE_EXCLUSIVE );
      //Modified code according to hint to take only first thread.
      //time1 = input->get_sev( met_t, CUBE_CALCULATE_INCLUSIVE, region, CUBE_CALCULATE_INCLUSIVE );
      //occ1  = input->get_sev( met_visit, CUBE_CALCULATE_INCLUSIVE, region, CUBE_CALCULATE_EXCLUSIVE );
  }
  //cout << ct_node->get_callee()->get_name() <<"  " << "TIME: " << time << ", OCC: " << occ <<  endl;
  //cout << ct_node->get_callee()->get_name() <<"  " << "TIME: " << time1 << ", OCC: " << occ1 <<  endl;
    double gran_sev = time / occ;
    delete met_tau_t;
    //cout << "Granularity Value: " << gran_sev << endl;
    return gran_sev;
}

/**
* Parse to read the tuple stream (N, min, max):avg, std
* @param value
* @return
*/
vector< string >
ValueParser( string value )
{
    string delimiter[ 5 ] = { "(", ",", ",", "):", "," };
    vector< string > values;
    size_t pos = 0, prev_pos = 0, i = 0 ;
    string token;
    while( i != 5 )
    {
        pos   = value.find( delimiter[ i ] );
        token = value.substr(prev_pos, pos );
        if( !token.empty() )
            values.push_back( token );
        value.erase( prev_pos, pos + delimiter[ i ].length() );
        i++;
    }
    if ( !value.empty() )
    {
        values.push_back( value );
    }
    return values;
}

}
