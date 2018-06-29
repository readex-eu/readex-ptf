/**
   @file  PerformanceDataBase.cc
   @ingroup AnalysisAgent
   @brief   Internal data storage
   @author  Yury Oleynik
   @verbatim
    Revision:       $Revision$
    Revision date:  Oct 28, 2013
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2011, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */


#include <stdio.h>
#include <sstream>
#include <string>
#include <iostream>
#include <math.h>
#include <boost/assert.hpp>
#include <algorithm>

#include "PerformanceDataBase.h"
#include "psc_errmsg.h"


PerformanceDataBase::PerformanceDataBase( DataProvider* provider ) {
    this->provider         = provider;
    last_written_iteration = 0;
    default_op             = PDB_REDUCTION_SUM;
    interp_type            = PDB_INTERPOLATION_ZEROS;
    timeWindowLeft         = 0;
    timeWindowRight        = 0;
}

PerformanceDataBase::~PerformanceDataBase() {
    delete provider;
}

void PerformanceDataBase::print_db() {
    printf( "==============PERFORMANCE DATA BASE CONTENT==============\n" );
    std::map<std::string, std::map<int, INT64> >::iterator iter1;
    for( iter1 = data.begin(); iter1 != data.end(); iter1++ ) {
        printf( "Entry: %s:\n", iter1->first.c_str() );
        std::map<int, INT64>::iterator iter2;
        int                            iterations_till_now = provider->getCurrentIterationNumber();

        int prev_iter_number = 0;
        for( iter2 = iter1->second.begin(); iter2 != iter1->second.end(); iter2++ ) {
            INT64 value = iter2->second;
            for( int i = prev_iter_number; i < iter2->first; i++ ) {
                printf( "x " );
            }
            prev_iter_number = iter2->first + 1;
            printf( "%lld", value );
        }
        for( int i = prev_iter_number; i < iterations_till_now; i++ ) {
            printf( "x " );
        }
        printf( "\n" );
    }
    printf( "============================END==========================\n" );
}

void PerformanceDataBase::get_reductions( int left,
                                          int right,
                                          DataBaseInterpolationType interp_type,
                                          std::map<int, INT64>      dataEntry,
                                          double mean,
                                          double&                   sum_out,
                                          double&                   sum_sqr_out,
                                          double&                   stddev_out ) {
    int                            num_elements = 0;
    int                            last_left_x  = left;
    double                         last_left_y  = 0;
    double                         sum          = 0.0;
    double                         sum_sqr      = 0.0;
    double                         stddev       = 0.0;
    std::map<int, INT64>::iterator iter;
    bool                           do_printing = false;

    /*
     * roll to the first element in the range which is inside of the requested interval
     */
    for( iter = dataEntry.begin(); iter != dataEntry.end(); iter++ ) {
        if( iter->first >= left ) {
            break;
        }
        else {
            last_left_y = ( double )iter->second;
            if( do_printing ) {
                std::cout << "rolled over " << iter->first << "-" << iter->second << std::endl;
            }
        }
    }

    /*
     * in case there is a gap in the beginning of the interval
     */
    int    right_limit = right;
    double right_y     = 0.0;
    if( iter != dataEntry.end() ) {
        right_y     = ( double )iter->second;
        right_limit = std::min( iter->first, right );
    }

    if( do_printing ) {
        std::cout << "there is a gap in the beginning from "
                  << left << " to " << right_limit << std::endl;
    }
    // means that the gap is in the beginning of the map
    sum          += interpolate_and_reduce( left, last_left_y, right_limit, right_y, PDB_REDUCTION_SUM, interp_type, 0 );
    sum_sqr      += interpolate_and_reduce( left, last_left_y, right_limit, right_y, PDB_REDUCTION_SUM_SQR, interp_type, 0 );
    stddev       += interpolate_and_reduce( left, last_left_y, right_limit, right_y, PDB_REDUCTION_STDDEV, interp_type, mean );
    num_elements += right_limit - left;
    last_left_x   = right_limit;
    if( do_printing ) {
        std::cout << "increased num_elements by " << right_limit - left << std::endl;
    }

    /*
     * iterate over the elements of the map that are within the range
     */
    for(; iter != dataEntry.end(); iter++ ) {
        if( iter->first >= left && iter->first < right ) {
            /*
             * if there is a gap in the middle of the interval
             */
            if( iter->first - last_left_x > 1 ) {
                if( do_printing ) {
                    std::cout << "there is a gap in the middle from" << last_left_x << " to " << iter->first << std::endl;
                }
                sum          += interpolate_and_reduce( last_left_x, last_left_y, iter->first - 1, ( double )iter->second, PDB_REDUCTION_SUM, interp_type, 0 );
                sum_sqr      += interpolate_and_reduce( last_left_x, last_left_y, iter->first - 1, ( double )iter->second, PDB_REDUCTION_SUM_SQR, interp_type, 0 );
                stddev       += interpolate_and_reduce( last_left_x, last_left_y, iter->first - 1, ( double )iter->second, PDB_REDUCTION_STDDEV, interp_type, mean );
                num_elements += iter->first - last_left_x - 1;
                if( do_printing ) {
                    std::cout << "increased num_elements by " << iter->first - last_left_x - 1 << std::endl;
                }
            }

            num_elements++;
            sum        += ( double )iter->second;
            sum_sqr    += ( double )iter->second * ( double )iter->second;
            stddev     += ( ( double )iter->second - mean ) * ( ( double )iter->second - mean );
            last_left_y = ( double )iter->second;
            last_left_x = iter->first;
            if( do_printing ) {
                std::cout << "reduced element " << iter->first << "-" << iter->second << std::endl;
            }
        }
        if( iter->first > right ) {
            break;
        }
    }
    /*
     * if there is a gap at the end of the interval then interpolate
     */
    right_y = 0.0;
    if( iter != dataEntry.end() ) {
        right_y = ( double )iter->second;
    }
    if( right - last_left_x > 1 ) {
        // include counting of the left border if it was not yet counted
        sum          += interpolate_and_reduce( last_left_x, last_left_y, right - 1, last_left_y, PDB_REDUCTION_SUM, interp_type, 0 );
        sum_sqr      += interpolate_and_reduce( last_left_x, last_left_y, right - 1, last_left_y, PDB_REDUCTION_SUM_SQR, interp_type, 0 );
        stddev       += interpolate_and_reduce( last_left_x, last_left_y, right - 1, last_left_y, PDB_REDUCTION_STDDEV, interp_type, mean );
        num_elements += right - last_left_x - 1;
        if( do_printing ) {
            std::cout << "there is a gap at the end from " << last_left_x << " to " << right << std::endl;
        }
        if( do_printing ) {
            std::cout << "increased num_elements by " << right - last_left_x - 1 << std::endl;
        }
    }
    if( num_elements != right - left ) {
        sum_out     = 0.0;
        sum_sqr_out = 0.0;
        stddev_out  = 0.0;
        std::cout << "WARNING!!! Error in reduction!!! num_elements=" << num_elements << " left="
                  << left << " right=" << right << " size=" << dataEntry.size() << std::endl;
        BOOST_ASSERT( false );
    }
    psc_dbgmsg( 7, "Reduced %d values of the measurements vector of length %d over the window %d-%d\n",
                num_elements, dataEntry.size(), left, right );
    sum_out     = sum;
    sum_sqr_out = sum_sqr;
    stddev_out  = stddev;
}


DataBaseQueryResult PerformanceDataBase::getReducedValueInWindow( DataBaseReductionOperation op,
                                                                  DataBaseInterpolationType  interp_type,
                                                                  int                        left,
                                                                  int                        right,
                                                                  Context*                   ct,
                                                                  Metric                     m,
                                                                  INT64&                     result ) {
    std::map<int, INT64> dataEntry;

    DataBaseQueryResult error = find_data_entry( ct, m, dataEntry );

    if( error != PDB_SCOREP_SUCCESS ) {
        return error;
    }

    double sum          = 0.0;
    double sum_sqr      = 0.0;
    double stddev       = 0.0;
    double mean         = 0.0;
    int    num_elements = right - left + 1;

    get_reductions( left, right, interp_type, dataEntry, mean, sum, sum_sqr, stddev );


    switch( op ) {
    case PDB_REDUCTION_AVG:
        result = sum / ( double )num_elements;
        break;
    case PDB_REDUCTION_STDDEV:
        mean = sum / ( double )num_elements;
        get_reductions( left, right, interp_type, dataEntry, mean, sum, sum_sqr, stddev );
        result = sqrt( stddev / ( double )num_elements );
        break;
    case PDB_REDUCTION_SUM:
        result = ( double )sum;
        break;
    case PDB_REDUCTION_SUM_SQR:
        result = ( double )sum_sqr;
    }



    return PDB_SCOREP_SUCCESS;
}



DataBaseQueryResult PerformanceDataBase::getLastIterationValue(
    Context* ct, Metric m, INT64& result ) {
    INT64 out = 0;
    result = out;

    std::map<int, INT64> dataEntry;

    DataBaseQueryResult error = find_data_entry( ct, m, dataEntry );

    if( error != PDB_SCOREP_SUCCESS ) {
        return error;
    }

    if( dataEntry.empty() ) {
        psc_dbgmsg( 5, "Data entry %s found, but it is empty\n", ct_2_string( ct, m ).c_str() );
        return PDB_SCOREP_NOT_FOUND;
    }

    result = dataEntry.rbegin()->second;

    return PDB_SCOREP_SUCCESS;
}

DataBaseQueryResult PerformanceDataBase::getSpecificIterationValue( Context* ct,
                                                                    Metric   m,
                                                                    int      iteration,
                                                                    INT64&   result ) {
    INT64 out = 0;
    result = out;

    std::map<int, INT64> dataEntry;

    DataBaseQueryResult error = find_data_entry( ct, m, dataEntry );

    if( error != PDB_SCOREP_SUCCESS ) {
        return error;
    }

    if( dataEntry.empty() ) {
        psc_dbgmsg( 5, "Data entry %s found, but it is empty\n", ct_2_string( ct, m ).c_str() );
        return PDB_SCOREP_NOT_FOUND;
    }

    if( dataEntry.find( iteration ) == dataEntry.end() ) {
        psc_dbgmsg( 5, "Data entry %s found, but iteration %d is not found\n",
                    ct_2_string( ct, m ).c_str(), iteration );
        return PDB_SCOREP_NOT_FOUND;
    }

    result = dataEntry[ iteration ];

    return PDB_SCOREP_SUCCESS;
}

DataBaseQueryResult PerformanceDataBase::getAllIterationsValues( Context*            ct,
                                                                 Metric              m,
                                                                 MissingDataFill     fill,
                                                                 std::vector<INT64>& result ) {
    std::vector<INT64> out;
    result = out;

    std::map<int, INT64> dataEntry;

    DataBaseQueryResult error = find_data_entry( ct, m, dataEntry );

    if( error != PDB_SCOREP_SUCCESS ) {
        return error;
    }


    std::map<int, INT64>::iterator iter;
    int                            iterations_till_now = provider->getCurrentIterationNumber();

    int   prev_iter_number = 0;
    INT64 last_value       = 0;
    for( iter = dataEntry.begin(); iter != dataEntry.end(); iter++ ) {
        INT64 value = iter->second;
        for( int i = prev_iter_number; i < iter->first; i++ ) {
            out.push_back( last_value );
        }
        out.push_back( value );
        prev_iter_number = iter->first + 1;
        last_value       = value;
    }
    for( int i = prev_iter_number; i < iterations_till_now; i++ ) {
        out.push_back( last_value );
    }
    result = out;
    return PDB_SCOREP_SUCCESS;
}

std::string PerformanceDataBase::ct_2_string( Context* ct,
                                              Metric   m ) {
    std::stringstream key;
    std::stringstream reg_name_build;
    std::string       reg_name;

    int line_number = ct->getRfl();
    reg_name = ct->getRegion()->get_name();

    /* As of the ScoreP revision 3423 in response to ticket 549, each implicit barrier gets its own region definition, where the region
     * first line is equal to the region end line of the OMP region which created the implicit barrier. This is different to the
     * way it is done in MRI Monitor where the implicit barrier time is identified by the file name and the region FIRST line of the
     * region which created the implicit barrier. Therefore in order to find the implicit barrier time corresponding to an OMP region
     * it has to be looked up using the region END line of the region. */
    if( m == PSC_IMPLICIT_BARRIER_TIME ) {
        line_number = ct->getRegion()->get_ident().end_position;
        reg_name_build << "!$omp implicit barrier @" << ct->getRegion()->get_ident().file_name
                       << ":" << line_number;
        reg_name = reg_name_build.str();
        psc_dbgmsg( 7, "Getting IMPLICIT_BARRIER metric : %s %d-%d\n",
                    reg_name.c_str(), line_number, ct->getRegion()->get_ident().end_position );
    }

    if( ct->isRtsBased() ) {
        key << ":" << ct->getFileId() << ":" << line_number << ":" << reg_name << ":"
            << ct->getRank() << ":" << ct->getThread() << ":" << EventList[ m ].EventName << ":" << ct->getRtsID();
    }
    else {
        key << ":" << ct->getFileId() << ":" << line_number << ":" << reg_name << ":"
            << ct->getRank() << ":" << ct->getThread() << ":" << EventList[ m ].EventName;
    }
    return key.str();
}

DataBaseQueryResult PerformanceDataBase::find_data_entry( Context* ct,
                                                          Metric m,
                                                          std::map<int, INT64>& result ) {
    std::map<int, INT64> out;
    result = out;

    std::string key = ct_2_string( ct, m );

    if( data.find( key ) == data.end() ) {
        psc_dbgmsg( 5, "Data entry %s not found\n", key.c_str() );
        return PDB_SCOREP_NOT_FOUND;
    }

    std::map<int, INT64> dataEntry = data[ key ];
    if( dataEntry.empty() ) {
        psc_dbgmsg( 5, "Data entry %s found, but it is empty\n", ct_2_string( ct, m ).c_str() );
        return PDB_SCOREP_NOT_FOUND;
    }

    result = dataEntry;
    return PDB_SCOREP_SUCCESS;
}

void PerformanceDataBase::clean() {
    data.clear();
}

void PerformanceDataBase::store( Context* ct,
                                 Metric   m,
                                 INT64    value ) {
    std::string key               = ct_2_string( ct, m );
    int         current_iteration = provider->getCurrentIterationNumber();

    //printf("STORE() in PDF (KEY,VALUE) = (%s,%d)\n",key.c_str(), value);//fflush(stdout);
    if( current_iteration >= last_written_iteration ) {
        last_written_iteration = current_iteration;
        /*
         * reset time window to default
         */
        setTimeWindow( last_written_iteration, last_written_iteration + 1 );
    }

    if( data.find( key ) == data.end() ) {
        std::map<int, INT64> new_map;
        data[ key ] = new_map;
    }

    data[ key ][ current_iteration ] = value;
}

void PerformanceDataBase::store( int    file_id,
                                 int    rfl,
                                 int    rank,
                                 int    thread,
                                 Metric m,
                                 INT64  val ) {
    Context ct( file_id, rfl, rank, thread );
    store( &ct, m, val );
}

void PerformanceDataBase::erase( int    file_id,
                                 int    rfl,
                                 int    rank,
                                 int    thread,
                                 Metric m ) {
    INT64                currentValue;
    Context              ct( file_id, rfl, rank, thread );
    std::map<int, INT64> dataEntry;
    DataBaseQueryResult  error = find_data_entry( &ct, m, dataEntry );

    if( error != PDB_SCOREP_SUCCESS ) {
        return;
    }

    int iteration = provider->getCurrentIterationNumber();
    if( dataEntry.find( iteration ) == dataEntry.end() ) {
        return;
    }


    dataEntry.erase( iteration );
}

INT64 PerformanceDataBase::get_by_CTdescr( int    file_id,
                                           int    rfl,
                                           int    rank,
                                           int    thread,
                                           Metric m ) {
    Context ct( file_id, rfl, rank, thread );
    INT64   currentValue;

    DataBaseQueryResult result = getReducedValueInWindow( default_op, interp_type, timeWindowLeft,
                                                          timeWindowRight, &ct, m, currentValue );

    if( result != PDB_SCOREP_SUCCESS ) {
        return 0;
    }
    else {
        return currentValue;
    }
}

INT64 PerformanceDataBase::get( Context* ct,
                                Metric   m ) {
    INT64               currentValue;
    DataBaseQueryResult result = getReducedValueInWindow( default_op, interp_type, timeWindowLeft,
                                                          timeWindowRight, ct, m, currentValue );
    //printf("Get (%s,%d)\n",ct_2_string( ct, m ).c_str(), currentValue);fflush(stdout);

    if( result != PDB_SCOREP_SUCCESS ) {
        return 0;
    }
    else {
        return currentValue;
    }
}

int PerformanceDataBase::request( Context* ct,
                                  Metric   m ) {
    provider->addMeasurementRequest( ct, m );
    return 1;
}

INT64 PerformanceDataBase::try_get( Context* ct,
                                    Metric   m ) {
    INT64               currentValue;
    DataBaseQueryResult result = getReducedValueInWindow( default_op, interp_type, timeWindowLeft,
                                                          timeWindowRight, ct, m, currentValue );

    if( result != PDB_SCOREP_SUCCESS ) {
        return -1;
    }
    else {
        return currentValue;
    }
}

void PerformanceDataBase::setDataProvider( DataProvider* prov_p ) {
    provider = prov_p;
}

DataBaseQueryResult PerformanceDataBase::setTimeWindow( int begin,
                                                        int end ) {
    if( timeWindowLeft == begin && timeWindowRight == end ) {
        return PDB_SCOREP_SUCCESS;
    }

    psc_dbgmsg( 6, "Setting PDB access time window to [%d-%d]\n", begin, end );

    if( end - begin < 0 || end - 1 > last_written_iteration || begin < 0 ) {
        std::cerr << "Invalid request: window [" << begin << ";" << end
                  << "] last_written_iteration=" << last_written_iteration << std::endl;
        throw std::invalid_argument( "Time window [begin, end] is invalid; please pass a valid time window to this function." );
    }

    timeWindowLeft  = begin;
    timeWindowRight = end;

    return PDB_SCOREP_SUCCESS;
}

double PerformanceDataBase::interpolate_and_reduce( int                        left_x,
                                                    INT64                      left_y,
                                                    int                        right_x,
                                                    INT64                      right_y,
                                                    DataBaseReductionOperation op,
                                                    DataBaseInterpolationType  interp_type,
                                                    double                     extra_param ) {
    /*
     * for now it is a simple constant interpolation
     */
    double result;
    double value = ( left_y + right_y ) / 2;

    switch( interp_type ) {
    case PDB_INTERPOLATION_LINEAR:
        psc_infomsg( "Warning: linear interpolation is not yet implemented, interpolation with zeros will be applied instead\n" );
        value = 0.0;
        break;
    case PDB_INTERPOLATION_CONSTANT:
        value = ( left_y + right_y ) / 2;
        break;
    case PDB_INTERPOLATION_ZEROS:
        value = 0.0;
        break;
    default:
        value = 0.0;
        break;
    }


    int num_points = right_x - left_x;

    if( num_points <= 0 ) {
        return 0.0;
    }

    switch( op ) {
    case PDB_REDUCTION_SUM:
        result = value * num_points;
        break;
    case PDB_REDUCTION_AVG:
        result = value;
        break;
    case PDB_REDUCTION_STDDEV:
        result = ( value - extra_param ) * ( value - extra_param ) * num_points;
        break;
    case PDB_REDUCTION_SUM_SQR:
        result = value * value * num_points;
    }
    return result;
}

void PerformanceDataBase::setDefaultReductionOperation( DataBaseReductionOperation op ) {
    default_op = op;
}

void PerformanceDataBase::setInterpolationType( DataBaseInterpolationType interp_type ) {
    this->interp_type = interp_type;
}
