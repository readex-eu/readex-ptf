/**
   @file  PerformanceDataBase.h
   @ingroup AnalysisAgent
   @brief   Internal data storage header
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

#ifndef PERFORMANCEDATABASE_H
#define PERFORMANCEDATABASE_H

#include <map>
#include <string>
#include <vector>

#include "Context.h"
#include "Metric.h"
#include "DataProvider.h"

typedef enum MissingDataFill {
    PDB_SCOREP_WITH_ZEROS,
    PDB_SCOREP_WITH_CONSTANT_FROM_LEFT,
    PDB_SCOREP_WITH_LINEAR_INTERPOLATION
} MissingDataFill;

typedef enum DataBaseQueryResult {
    PDB_SCOREP_SUCCESS,
    PDB_SCOREP_NOT_FOUND,
} DataBaseQueryResult;

typedef enum DataBaseReductionOperation {
    PDB_REDUCTION_SUM,
    PDB_REDUCTION_SUM_SQR,
    PDB_REDUCTION_AVG,
    PDB_REDUCTION_STDDEV
}DataBaseReductionOperation;

typedef enum DataBaseInterpolationType {
    PDB_INTERPOLATION_CONSTANT,
    PDB_INTERPOLATION_LINEAR,
    PDB_INTERPOLATION_ZEROS
}DataBaseInterpolationType;

class PerformanceDataBase {
    std::map<std::string, std::map<int, INT64> > data;
    DataProvider*                                provider;

    int                        timeWindowLeft;
    int                        timeWindowRight;
    DataBaseReductionOperation default_op;
    DataBaseInterpolationType  interp_type;
    int                        last_written_iteration;

    std::string ct_2_string( Context* ct,
                             Metric   m );


    double interpolate_and_reduce( int                        left_x,
                                   INT64                      left_y,
                                   int                        right_x,
                                   INT64                      right_y,
                                   DataBaseReductionOperation op,
                                   DataBaseInterpolationType  interp_type,
                                   double                     extra_param );
    void get_reductions( int left,
                         int right,
                         DataBaseInterpolationType interp_type,
                         std::map<int, INT64> dataEntry,
                         double mean,
                         double& sum_out,
                         double& sum_sqr_out,
                         double& stddev_out );
    DataBaseQueryResult find_data_entry( Context* ct,
                                         Metric m,
                                         std::map<int, INT64>& result );

public:
    PerformanceDataBase( DataProvider* provider );
    ~PerformanceDataBase();

    void print_db();

    void clean();

    DataBaseQueryResult setTimeWindow( int begin,
                                       int end );
    void setDefaultReductionOperation( DataBaseReductionOperation op );

    void setInterpolationType( DataBaseInterpolationType interp_type );

    // TODO: use these getters instead of the old getter functions; after that, commit
    // 102851dbbdef05ca469a0f5607354817696be259 can be reverted or amended accordingly
    DataBaseQueryResult getLastIterationValue( Context* ct,
                                               Metric   m,
                                               INT64&   result );
    DataBaseQueryResult getSpecificIterationValue( Context* ct,
                                                   Metric   m,
                                                   int      iteration,
                                                   INT64&   result );
    DataBaseQueryResult getAllIterationsValues( Context*            ct,
                                                Metric              m,
                                                MissingDataFill     fill,
                                                std::vector<INT64>& result );
    DataBaseQueryResult getReducedValueInWindow( DataBaseReductionOperation op,
                                                 DataBaseInterpolationType  interp_type,
                                                 int                        left,
                                                 int                        right,
                                                 Context*                   ct,
                                                 Metric                     m,
                                                 INT64&                     result );

    void store( Context* ct,
                Metric   m,
                INT64    value );

    /* APIs from the old PDB implementation. Are here only for backward compatibility reasons */
    void store( int    file_id,
                int    rfl,
                int    rank,
                int    thread,
                Metric m,
                INT64  val );

    void erase( int    file_id,
                int    rfl,
                int    rank,
                int    thread,
                Metric m );

    INT64 get_by_CTdescr( int    file_id,
                          int    rfl,
                          int    rank,
                          int    thread,
                          Metric m );

    INT64 get( Context* ct,
               Metric   m );

    int request( Context* ct,
                 Metric   m );

    INT64 try_get( Context* ct,
                   Metric   m );

    void setDataProvider( DataProvider* prov_p );
};

#endif /* PERFORMANCEDATABASE_H */
