/**
   @file    SCOREP_OA_ReturnTypes.h
   @ingroup AnalysisAgent
   @brief   Defines return types received from ScoreP
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2016, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */
#ifndef SCOREP_OA_RETURNTYPES_H_
#define SCOREP_OA_RETURNTYPES_H_



/**
 * @brief Defines return types received from ScoreP
 *
 */

//#include <stdint.h>
#include <inttypes.h>

#define MAX_COUNTER_NAME_LENGTH                         256
#define MAX_COUNTER_UNIT_LENGTH                         10
#define MAX_REGION_NAME_LENGTH                          500
#define MAX_FILE_NAME_LENGTH                            256

#define SCOREP_OA_COUNTER_TIME                  0


typedef struct SCOREP_OA_CallPathContext_struct {
    uint32_t region_id;
    uint32_t context_id;
    uint32_t parent_context_id;
    uint32_t thread;
    uint64_t rank;
    uint64_t call_count;
}SCOREP_OA_CallPathContext;

typedef struct SCOREP_OA_CallPathMeasurement_struct {
    uint32_t context_id;
    uint64_t sum;
    uint64_t count;
}SCOREP_OA_CallPathMeasurement;

typedef struct SCOREP_OA_CallPathCounterDef_struct {
    char     name[ MAX_COUNTER_NAME_LENGTH ];
    char     unit[ MAX_COUNTER_UNIT_LENGTH ];
    uint32_t status;
}SCOREP_OA_CallPathCounterDef;

typedef struct SCOREP_OA_CallPathRegionDef_struct {
    uint32_t region_id;
    char     name[ MAX_REGION_NAME_LENGTH ];
    char     file[ MAX_FILE_NAME_LENGTH ];
    uint32_t rfl;
    uint32_t rel;
    uint32_t adapter_type;
}SCOREP_OA_CallPathRegionDef;

typedef struct SCOREP_OA_FlatProfileMeasurement_struct {
    uint32_t measurement_id;
    uint64_t rank;
    uint32_t thread;
    uint32_t region_id;
    uint64_t samples;
    uint32_t metric_id;
    uint64_t int_val;
}SCOREP_OA_FlatProfileMeasurement;

typedef struct SCOREP_OA_CallTreeDef_struct {
    uint32_t region_id;
    char     name[ MAX_REGION_NAME_LENGTH ];
    uint32_t scorep_id;
    uint32_t parent_scorep_id;
}SCOREP_OA_CallTreeDef;

typedef struct SCOREP_OA_RtsMeasurement_struct {
    uint64_t rank;
    uint32_t thread;
    uint64_t count;
    uint32_t metric_id;
    uint64_t int_val;
    uint32_t scorep_id;
}SCOREP_OA_RtsMeasurement;

typedef struct SCOREP_OA_ProfileSummary_struct {
    SCOREP_OA_CallPathContext*        context_buffer;
    SCOREP_OA_CallPathMeasurement*    measurement_buffer;
    SCOREP_OA_CallPathCounterDef*     counter_def_buffer;
    SCOREP_OA_CallPathRegionDef*      region_def_buffer;
    SCOREP_OA_FlatProfileMeasurement* static_measurement_buffer;
    SCOREP_OA_CallTreeDef*            calltree_def_buffer;
    SCOREP_OA_RtsMeasurement*         rts_measurement_buffer;
    uint32_t                          context_size;
    uint32_t                          measurement_size;
    uint32_t                          counter_def_size;
    uint32_t                          region_def_size;
    uint32_t                          static_measurement_size;
    uint32_t                          calltree_def_size;
    uint32_t                          rts_measurement_size;
} SCOREP_OA_CallPathSummary;

typedef enum scorep_oaconsumer_data_types {
    FLAT_PROFILE = 0,
    MERGED_REGION_DEFINITIONS,
    REGION_DEFINITIONS,
    COUNTER_DEFINITIONS,
    CALLPATH_PROFILE_CONTEXTS,
    CALLPATH_PROFILE_MEASUREMENTS,
    NUMBER_OF_THREADS,
    CALLTREE_DEFINITIONS,
    RTS_MEASUREMENTS
} scorep_oaconsumer_data_types; // SCOREP_OAConsumer_DataTypes

typedef enum SCOREP_OA_RegionTypes {
    //ScoreP v 2.0
    SCOREP_ADAPTER_USER     = 1,
    SCOREP_ADAPTER_COMPILER = 2,      //TODO verify!!!
    SCOREP_ADAPTER_MPI      = 5,
    SCOREP_ADAPTER_POMP     = 7,
    SCOREP_ADAPTER_PTHREAD  = 999,    //TODO verify!!!
    SCOREP_ADAPTER_CUDA     = 1000    //TODO verify!!!
} SCOREP_OA_RegionTypes;


#endif /*SCOREP_OA_RETURNTYPES_H_*/
