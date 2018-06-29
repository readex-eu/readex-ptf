#include "Metric.h"
#include <stdio.h>
#include <string.h>

EventType* EventList;

void setMetricInEventList( Group g, const char* name, Metric m, int CodeID ) {
    int index;

    index = ( int )m;

    //printf("SetMetric %d  %s %d\n", index, name, m);

    EventList[ index ].EventGroup  = g;
    EventList[ index ].EventMetric = m;
    EventList[ index ].EventName   = name;
    EventList[ index ].CodeID      = CodeID;
}

void printEventlist() {
    int i;

    for( i = 0; i < PSC_LAST_METRIC; i++ ) {
        printf( "[%d] %d %s %d %x\n", i, EventList[ i ].EventGroup, EventList[ i ].EventName, EventList[ i ].EventMetric, EventList[ i ].CodeID );
    }
}

void initEventlist( const char* metric_plugin_name ) {
    int i;
    EventList = ( EventType* )calloc( PSC_LAST_METRIC, sizeof( EventType ) );


    for( i = 0; i < PSC_LAST_METRIC; i++ ) {
        EventList[ i ].EventGroup  = GROUP_UNDEFINED;
        EventList[ i ].EventMetric = PSC_UNDEFINED_METRIC;
        EventList[ i ].EventName   = "UNDEFINED";
        EventList[ i ].CodeID      = CID_NO_CODE;
    }

    setMetricInEventList( GROUP_TIME_MEASUREMENT,         "execution_time", PSC_EXECUTION_TIME, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_L1_TCM", PSC_PAPI_L1_TCM, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_L1_DCM", PSC_PAPI_L1_DCM, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_L1_DCA", PSC_PAPI_L1_DCA, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_L1_ICM", PSC_PAPI_L1_ICM, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_L2_DCM", PSC_PAPI_L2_DCM, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_L2_DCA", PSC_PAPI_L2_DCA, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_L3_TCM", PSC_PAPI_L3_TCM, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_L3_TCA", PSC_PAPI_L3_TCA, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_TOT_INS", PSC_PAPI_TOT_INS, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_INT_INS", PSC_PAPI_INT_INS, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_FP_OPS", PSC_PAPI_FP_OPS, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_LD_INS", PSC_PAPI_LD_INS, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_SR_INS", PSC_PAPI_SR_INS, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_TLB_DM", PSC_PAPI_TLB_DM, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_TLB_IM", PSC_PAPI_TLB_IM, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_LST_INS", PSC_PAPI_LST_INS, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_L2_ICR", PSC_PAPI_L2_ICR, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_L2_ICH", PSC_PAPI_L2_ICH, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_TOT_CYC", PSC_PAPI_TOT_CYC, CID_NO_CODE );
//    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_SR_INS", PSC_AVX_INSTS_CALC, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "perf_raw::r04C6", PSC_AVX_INSTS_CALC, CID_NO_CODE );
    //Itanium metrics
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "BACK_END_BUBBLE_ALL", PSC_BACK_END_BUBBLE_ALL, CID_BACK_END_BUBBLE_ALL );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "BACK_END_BUBBLE_FE", PSC_BACK_END_BUBBLE_FE, CID_BACK_END_BUBBLE_FE );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "BE_EXE_BUBBLE_FRALL", PSC_BE_EXE_BUBBLE_FRALL, CID_BE_EXE_BUBBLE_FRALL );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "BE_EXE_BUBBLE_GRALL", PSC_BE_EXE_BUBBLE_GRALL, CID_BE_EXE_BUBBLE_GRALL );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "BE_EXE_BUBBLE_GRGR", PSC_BE_EXE_BUBBLE_GRGR, CID_BE_EXE_BUBBLE_GRGR );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "BE_FLUSH_BUBBLE_BRU", PSC_BE_FLUSH_BUBBLE_BRU, CID_BE_FLUSH_BUBBLE_BRU );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "BE_FLUSH_BUBBLE_XPN", PSC_BE_FLUSH_BUBBLE_XPN, CID_BE_FLUSH_BUBBLE_XPN );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "BE_L1D_FPU_BUBBLE_FPU", PSC_BE_L1D_FPU_BUBBLE_FPU, CID_BE_L1D_FPU_BUBBLE_FPU );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "BE_L1D_FPU_BUBBLE_L1D", PSC_BE_L1D_FPU_BUBBLE_L1D, CID_BE_L1D_FPU_BUBBLE_L1D );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "BE_L1D_FPU_BUBBLE_L1D_FULLSTBUF", PSC_BE_L1D_FPU_BUBBLE_L1D_FULLSTBUF, CID_BE_L1D_FPU_BUBBLE_L1D_FULLSTBUF );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "BE_L1D_FPU_BUBBLE_L1D_DCURECIR", PSC_BE_L1D_FPU_BUBBLE_L1D_DCURECIR, CID_BE_L1D_FPU_BUBBLE_L1D_DCURECIR );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "BE_L1D_FPU_BUBBLE_L1D_HPW", PSC_BE_L1D_FPU_BUBBLE_L1D_HPW, CID_BE_L1D_FPU_BUBBLE_L1D_HPW );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "BE_L1D_FPU_BUBBLE_L1D_TLB", PSC_BE_L1D_FPU_BUBBLE_L1D_TLB, CID_BE_L1D_FPU_BUBBLE_L1D_TLB );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "BE_L1D_FPU_BUBBLE_L1D_L2BPRESS", PSC_BE_L1D_FPU_BUBBLE_L1D_L2BPRESS, CID_BE_L1D_FPU_BUBBLE_L1D_L2BPRESS );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "BE_RSE_BUBBLE_ALL", PSC_BE_RSE_BUBBLE_ALL, CID_BE_RSE_BUBBLE_ALL );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "FP_FALSE_SIRSTALL", PSC_FP_FALSE_SIRSTALL, CID_FP_FALSE_SIRSTALL );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "FP_FLUSH_TO_ZERO", PSC_FP_FLUSH_TO_ZERO, CID_FP_FLUSH_TO_ZERO );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "FP_TRUE_SIRSTALL", PSC_FP_TRUE_SIRSTALL, CID_FP_TRUE_SIRSTALL );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "L1D_READ_MISSES_ALL", PSC_L1D_READ_MISSES_ALL, CID_L1D_READ_MISSES_ALL );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "L2_DATA_REFERENCES_L2_DATA_READS", PSC_L2_DATA_REFERENCES_L2_DATA_READS, CID_L2_DATA_REFERENCES_L2_DATA_READS );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "L2_FORCE_RECIRC_FILL_HIT", PSC_L2_FORCE_RECIRC_FILL_HIT, CID_L2_FORCE_RECIRC_FILL_HIT );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "L2_FORCE_RECIRC_L1W", PSC_L2_FORCE_RECIRC_L1W, CID_L2_FORCE_RECIRC_L1W );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "L2_FORCE_RECIRC_OZQ_MISS", PSC_L2_FORCE_RECIRC_OZQ_MISS, CID_L2_FORCE_RECIRC_OZQ_MISS );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "L2_FORCE_RECIRC_SAME_INDEX", PSC_L2_FORCE_RECIRC_SAME_INDEX, CID_L2_FORCE_RECIRC_SAME_INDEX );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "L2_OZQ_CANCELS1_BANK_CONF", PSC_L2_OZQ_CANCELS1_BANK_CONF, CID_L2_OZQ_CANCELS1_BANK_CONF );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "L3_READS_DATA_READ_ALL", PSC_L3_READS_DATA_READ_ALL, CID_L3_READS_DATA_READ_ALL );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "L3_READS_DATA_READ_HIT", PSC_L3_READS_DATA_READ_HIT, CID_L3_READS_DATA_READ_HIT );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "L3_READS_DATA_READ_MISS", PSC_L3_READS_DATA_READ_MISS, CID_L3_READS_DATA_READ_MISS );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "L2_OZQ_FULL_THIS", PSC_L2_OZQ_FULL_THIS, CID_L2_OZQ_FULL_THIS );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "BE_FLUSH_BUBBLE_ALL", PSC_BE_FLUSH_BUBBLE_ALL, CID_BE_FLUSH_BUBBLE_ALL );                                 //49
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "BE_L1D_FPU_BUBBLE_ALL", PSC_BE_L1D_FPU_BUBBLE_ALL, CID_BE_L1D_FPU_BUBBLE_ALL );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "BE_EXE_BUBBLE_ALL", PSC_BE_EXE_BUBBLE_ALL, CID_BE_EXE_BUBBLE_ALL );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "DATA_EAR_CACHE_LAT4", PSC_DATA_EAR_CACHE_LAT4, CID_DATA_EAR_CACHE_LAT4 );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "DATA_EAR_CACHE_LAT8", PSC_DATA_EAR_CACHE_LAT8, CID_DATA_EAR_CACHE_LAT8 );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "DATA_EAR_CACHE_LAT16", PSC_DATA_EAR_CACHE_LAT16, CID_DATA_EAR_CACHE_LAT16 );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "DATA_EAR_CACHE_LAT32", PSC_DATA_EAR_CACHE_LAT32, CID_DATA_EAR_CACHE_LAT32 );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "DATA_EAR_CACHE_LAT64", PSC_DATA_EAR_CACHE_LAT64, CID_DATA_EAR_CACHE_LAT64 );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "DATA_EAR_CACHE_LAT128", PSC_DATA_EAR_CACHE_LAT128, CID_DATA_EAR_CACHE_LAT128 );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "DATA_EAR_CACHE_LAT256", PSC_DATA_EAR_CACHE_LAT256, CID_DATA_EAR_CACHE_LAT256 );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "DATA_EAR_CACHE_LAT512", PSC_DATA_EAR_CACHE_LAT512, CID_DATA_EAR_CACHE_LAT512 );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "DATA_EAR_CACHE_LAT1024", PSC_DATA_EAR_CACHE_LAT1024, CID_DATA_EAR_CACHE_LAT1024 );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "DATA_EAR_CACHE_LAT2048", PSC_DATA_EAR_CACHE_LAT2048, CID_DATA_EAR_CACHE_LAT2048 );
    setMetricInEventList( GROUP_PERISCOPE_COUNTER,        "DATA_EAR_CACHE_LAT4096", PSC_DATA_EAR_CACHE_LAT4096, CID_DATA_EAR_CACHE_LAT4096 );

    //p575
    setMetricInEventList( GROUP_UNDEFINED,                "PM_START", PSC_PM_START, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_RUN_CYC", PSC_PM_RUN_CYC, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DPU_HELD", PSC_PM_DPU_HELD, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DPU_HELD_FP_FX_MULT", PSC_PM_DPU_HELD_FP_FX_MULT, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DPU_HELD_MULT_GPR", PSC_PM_DPU_HELD_MULT_GPR, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_FPU_FXMULT", PSC_PM_FPU_FXMULT, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_FPU_FXDIV", PSC_PM_FPU_FXDIV, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_FXU_PIPELINED_MULT_DIV", PSC_PM_FXU_PIPELINED_MULT_DIV, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_LD_MISS_L1_CYC", PSC_PM_LD_MISS_L1_CYC, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DATA_FROM_L2_CYC", PSC_PM_DATA_FROM_L2_CYC, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DATA_FROM_L21_CYC", PSC_PM_DATA_FROM_L21_CYC, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DATA_FROM_L3_CYC", PSC_PM_DATA_FROM_L3_CYC, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_LSU_LMQ_FULL_CYC", PSC_PM_LSU_LMQ_FULL_CYC, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DC_PREF_OUT_OF_STREAMS", PSC_PM_DC_PREF_OUT_OF_STREAMS, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DC_PREF_STREAM_ALLOC", PSC_PM_DC_PREF_STREAM_ALLOC, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_LSU_DERAT_MISS_CYC", PSC_PM_LSU_DERAT_MISS_CYC, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DERAT_MISS_4K", PSC_PM_DERAT_MISS_4K, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DERAT_MISS_64K", PSC_PM_DERAT_MISS_64K, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DERAT_MISS_16M", PSC_PM_DERAT_MISS_16M, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DERAT_MISS_16G", PSC_PM_DERAT_MISS_16G, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DERAT_REF_4K", PSC_PM_DERAT_REF_4K, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DERAT_REF_64K", PSC_PM_DERAT_REF_64K, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DERAT_REF_16M", PSC_PM_DERAT_REF_16M, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DERAT_REF_16G", PSC_PM_DERAT_REF_16G, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_L2_ST_REQ_DATA", PSC_PM_L2_ST_REQ_DATA, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_L2_ST_MISS_DATA", PSC_PM_L2_ST_MISS_DATA, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_LSU_REJECT_STQ_FULL", PSC_PM_LSU_REJECT_STQ_FULL, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_ST_REF_L1", PSC_PM_ST_REF_L1, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_LSU_ST_CHAINED", PSC_PM_LSU_ST_CHAINED, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_L2_PREF_LD", PSC_PM_L2_PREF_LD, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_L1_PREF", PSC_PM_L1_PREF, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_LD_REF_L1", PSC_PM_LD_REF_L1, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_L2_PREF_ST", PSC_PM_L2_PREF_ST, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DPU_WT", PSC_PM_DPU_WT, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_BR_MPRED", PSC_PM_BR_MPRED, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_BR_PRED", PSC_PM_BR_PRED, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DPU_WT_IC_MISS", PSC_PM_DPU_WT_IC_MISS, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_LSU_REJECT_LHS", PSC_PM_LSU_REJECT_LHS, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DPU_HELD_GPR", PSC_PM_DPU_HELD_GPR, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DPU_HELD_FPQ", PSC_PM_DPU_HELD_FPQ, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DPU_HELD_FPU_CR", PSC_PM_DPU_HELD_FPU_CR, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DPU_HELD_SMT", PSC_PM_DPU_HELD_SMT, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_FPU_FCONV", PSC_PM_FPU_FCONV, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_FPU_FSQRT_FDIV", PSC_PM_FPU_FSQRT_FDIV, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_FPU_FMA", PSC_PM_FPU_FMA, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DPU_HELD_FXU_MULTI", PSC_PM_DPU_HELD_FXU_MULTI, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DPU_HELD_INT", PSC_PM_DPU_HELD_INT, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DPU_HELD_CR_LOGICAL", PSC_PM_DPU_HELD_CR_LOGICAL, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_L3SA_REF", PSC_PM_L3SA_REF, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_L3SA_MISS", PSC_PM_L3SA_MISS, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_L3SA_HIT", PSC_PM_L3SA_HIT, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_L3SB_REF", PSC_PM_L3SB_REF, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_L3SB_MISS", PSC_PM_L3SB_MISS, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_L3SB_HIT", PSC_PM_L3SB_HIT, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_FPU_ISSUE_0", PSC_PM_FPU_ISSUE_0, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_FPU_ISSUE_1", PSC_PM_FPU_ISSUE_1, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_FPU_ISSUE_2", PSC_PM_FPU_ISSUE_2, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_FPU_ISSUE_DIV_SQRT_OVERLAP", PSC_PM_FPU_ISSUE_DIV_SQRT_OVERLAP, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_FPU_ISSUE_STALL_FPR", PSC_PM_FPU_ISSUE_STALL_FPR, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_FPU_ISSUE_STALL_ST", PSC_PM_FPU_ISSUE_STALL_ST, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_FPU_FLOP", PSC_PM_FPU_FLOP, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DATA_FROM_L2MISS", PSC_PM_DATA_FROM_L2MISS, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DATA_FROM_L2", PSC_PM_DATA_FROM_L2, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DATA_FROM_L3MISS", PSC_PM_DATA_FROM_L3MISS, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DATA_FROM_L3", PSC_PM_DATA_FROM_L3, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DATA_FROM_L21", PSC_PM_DATA_FROM_L21, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_DATA_FROM_LMEM", PSC_PM_DATA_FROM_LMEM, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_L2_LD_REQ_DATA", PSC_PM_L2_LD_REQ_DATA, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_LD_REQ_L2", PSC_PM_LD_REQ_L2, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_L2_LD_MISS_DATA", PSC_PM_L2_LD_MISS_DATA, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_INST_DISP", PSC_PM_INST_DISP, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_RUN_INST_CMPL", PSC_PM_RUN_INST_CMPL, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_1PLUS_PPC_CMPL", PSC_PM_1PLUS_PPC_CMPL, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_1PLUS_PPC_DISP", PSC_PM_1PLUS_PPC_DISP, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_POWER6_COUNTER,      "PM_LD_MISS_L1", PSC_PM_LD_MISS_L1, CID_NO_CODE );
    setMetricInEventList( GROUP_UNDEFINED,                "PM_END", PSC_PM_END, CID_NO_CODE );
    //p575

    // added for benchmarking strategy
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_BR_CN", PSC_PAPI_BR_CN, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_BR_INS", PSC_PAPI_BR_INS, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_BR_MSP", PSC_PAPI_BR_MSP, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_L1_DCH", PSC_PAPI_L1_DCH, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_L2_DCH", PSC_PAPI_L2_DCH, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_L3_DCA", PSC_PAPI_L3_DCA, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_L3_DCH", PSC_PAPI_L3_DCH, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_L3_DCM", PSC_PAPI_L3_DCM, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_RES_STL", PSC_PAPI_RES_STL, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_TLB_TL", PSC_PAPI_TLB_TL, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_TOT_IIS", PSC_PAPI_TOT_IIS, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_L2_TCM", PSC_PAPI_L2_TCM, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_L2_TCH", PSC_PAPI_L2_TCH, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_COUNTER,             "PAPI_FP_INS", PSC_PAPI_FP_INS, CID_NO_CODE );
    // papi

    //NP Metrics
    setMetricInEventList( GROUP_PAPI_NEHALEM_COUNTER,     "CPU_CLK_UNHALTED:THREAD_P", PSC_NP_THREAD_P, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_NEHALEM_COUNTER,     "UOPS_EXECUTED:PORT015", PSC_NP_UOPS_EXECUTED_PORT015, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_NEHALEM_COUNTER,     "UOPS_ISSUED:FUSED", PSC_NP_UOPS_ISSUED_FUSED, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_NEHALEM_COUNTER,     "UOPS_ISSUED:ANY", PSC_NP_UOPS_ISSUED_ANY, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_NEHALEM_COUNTER,     "UOPS_RETIRED:ANY", PSC_NP_UOPS_RETIRED_ANY, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_NEHALEM_COUNTER,     "SQ_FULL_STALL_CYCLES", PSC_NP_STALL_CYCLES, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_NEHALEM_COUNTER,     "RESOURCE_STALLS:ANY", PSC_NP_RESOURCE_STALLS_ANY, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_NEHALEM_COUNTER,     "INSTRUCTION_RETIRED", PSC_NP_INSTRUCTION_RETIRED, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_NEHALEM_COUNTER,     "MEM_INST_RETIRED:LOADS", PSC_NP_MEM_INST_RETIRED_LOADS, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_NEHALEM_COUNTER,     "MEM_INST_RETIRED:STORES", PSC_NP_MEM_INST_RETIRED_STORES, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_NEHALEM_COUNTER,     "DTLB_MISSES:ANY", PSC_NP_DTLB_MISSES_ANY, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_NEHALEM_COUNTER,     "DTLB_LOAD_MISSES:ANY", PSC_NP_DTLB_LOAD_MISSES_ANY, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_NEHALEM_COUNTER,     "DTLB_MISSES:WALK_COMPLETED", PSC_NP_DTLB_MISSES_WALK_COMPLETED, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_NEHALEM_COUNTER,     "ITLB_MISSES:ANY", PSC_NP_ITLB_MISSES_ANY, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_NEHALEM_COUNTER,     "PARTIAL_ADDRESS_ALIAS", PSC_NP_PARTIAL_ADDRESS_ALIAS, CID_NO_CODE );
    setMetricInEventList( GROUP_PAPI_NEHALEM_COUNTER,     "UOPS_DECODED:MS", PSC_NP_UOPS_DECODED_MS, CID_NO_CODE );

    //OPENCL execution time Metrics
    setMetricInEventList( GROUP_OPENCL_TIME_MEASUREMENT,  "OPENCL_TRANSFER_EXECUTION_TIME", PSC_OPENCL_TRANSFER_EXECUTION_TIME, CID_NO_CODE );
    setMetricInEventList( GROUP_OPENCL_TIME_MEASUREMENT,  "OPENCL_KERNEL_EXECUTION_TIME", PSC_OPENCL_KERNEL_EXECUTION_TIME, CID_NO_CODE );

    //CUDA execution time Metric
    setMetricInEventList( GROUP_CUDA_TIME_MEASUREMENT,    "CUDA_TRANSFER_EXECUTION_TIME", PSC_CUDA_TRANSFER_EXECUTION_TIME, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_TIME_MEASUREMENT,    "CUDA_KERNEL_EXECUTION_TIME", PSC_CUDA_KERNEL_EXECUTION_TIME, CID_NO_CODE );

    //CUDA byte transfer Metric
    setMetricInEventList( GROUP_CUDA_BYTES_TRANSFERRED,   "PSC_CUDA_BYTES_TRANSFERRED", PSC_CUDA_BYTES_TRANSFERRED, CID_NO_CODE );

    //CUDA_COUNTER Metrics
    setMetricInEventList( GROUP_CUDA_COUNTER,             "sm_cta_launched", PSC_CUPTI_DA_SM_CTA_LAUNCHED, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "l1_local_load_hit", PSC_CUPTI_DA_L1_LOCAL_LOAD_HIT, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "l1_local_load_miss", PSC_CUPTI_DA_L1_LOCAL_LOAD_MISS, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "l1_local_store_hit", PSC_CUPTI_DA_L1_LOCAL_STORE_HIT, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "l1_local_store_miss", PSC_CUPTI_DA_L1_LOCAL_STORE_MISS, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "l1_global_load_hit", PSC_CUPTI_DA_L1_GLOBAL_LOAD_HIT, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "l1_global_load_miss", PSC_CUPTI_DA_L1_GLOBAL_LOAD_MISS, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "uncached_global_load_transaction", PSC_CUPTI_DA_UNCACHED_GLOBAL_LOAD_TRANSACTION, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "global_store_transaction", PSC_CUPTI_DA_GLOBAL_STORE_TRANSACTION, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "l1_shared_bank_conflict", PSC_CUPTI_DA_L1_SHARED_BANK_CONFLICT, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "tex0_cache_sector_queries", PSC_CUPTI_DA_TEX0_CACHE_SECTOR_QUERIES, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "tex0_cache_sector_misses", PSC_CUPTI_DA_TEX0_CACHE_SECTOR_MISSES, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "tex1_cache_sector_queries", PSC_CUPTI_DA_TEX1_CACHE_SECTOR_QUERIES, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "tex1_cache_sector_misses", PSC_CUPTI_DA_TEX1_CACHE_SECTOR_MISSES, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "l2_subp0_write_sector_misses", PSC_CUPTI_DB_L2_SUBP0_WRITE_SECTOR_MISSES, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "l2_subp1_write_sector_misses", PSC_CUPTI_DB_L2_SUBP1_WRITE_SECTOR_MISSES, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "l2_subp0_read_sector_misses", PSC_CUPTI_DB_L2_SUBP0_READ_SECTOR_MISSES, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "l2_subp1_read_sector_misses", PSC_CUPTI_DB_L2_SUBP1_READ_SECTOR_MISSES, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "l2_subp0_write_sector_queries", PSC_CUPTI_DB_L2_SUBP0_WRITE_SECTOR_QUERIES, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "l2_subp1_write_sector_queries", PSC_CUPTI_DB_L2_SUBP1_WRITE_SECTOR_QUERIES, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "l2_subp0_read_sector_queries", PSC_CUPTI_DB_L2_SUBP0_READ_SECTOR_QUERIES, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "l2_subp1_read_sector_queries", PSC_CUPTI_DB_L2_SUBP1_READ_SECTOR_QUERIES, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "l2_subp0_read_hit_sectors", PSC_CUPTI_DB_L2_SUBP0_READ_HIT_SECTORS, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "l2_subp1_read_hit_sectors", PSC_CUPTI_DB_L2_SUBP1_READ_HIT_SECTORS, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "l2_subp0_read_tex_sector_queries", PSC_CUPTI_DB_L2_SUBP0_READ_TEX_SECTOR_QUERIES, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "l2_subp1_read_tex_sector_queries", PSC_CUPTI_DB_L2_SUBP1_READ_TEX_SECTOR_QUERIES, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "l2_subp0_read_tex_hit_sectors", PSC_CUPTI_DB_L2_SUBP0_READ_TEX_HIT_SECTORS, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "l2_subp1_read_tex_hit_sectors", PSC_CUPTI_DB_L2_SUBP1_READ_TEX_HIT_SECTORS, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "fb_subp0_read_sectors", PSC_CUPTI_DB_FB_SUBP0_READ_SECTORS, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "fb_subp1_read_sectors", PSC_CUPTI_DB_FB_SUBP1_READ_SECTORS, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "fb_subp0_write_sectors", PSC_CUPTI_DB_FB_SUBP0_WRITE_SECTORS, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "fb_subp1_write_sectors", PSC_CUPTI_DB_FB_SUBP1_WRITE_SECTORS, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "fb0_subp0_read_sectors", PSC_CUPTI_DB_FB0_SUBP0_READ_SECTORS, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "fb0_subp1_read_sectors", PSC_CUPTI_DB_FB0_SUBP1_READ_SECTORS, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "fb0_subp0_write_sectors", PSC_CUPTI_DB_FB0_SUBP0_WRITE_SECTORS, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "fb0_subp1_write_sectors", PSC_CUPTI_DB_FB0_SUBP1_WRITE_SECTORS, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "fb1_subp0_read_sectors", PSC_CUPTI_DB_FB1_SUBP0_READ_SECTORS, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "fb1_subp1_read_sectors", PSC_CUPTI_DB_FB1_SUBP1_READ_SECTORS, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "fb1_subp0_write_sectors", PSC_CUPTI_DB_FB1_SUBP0_WRITE_SECTORS, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "fb1_subp1_write_sectors", PSC_CUPTI_DB_FB1_SUBP1_WRITE_SECTORS, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "gld_inst_8bit", PSC_CUPTI_DC_GLD_INST_8BIT, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "gld_inst_16bit", PSC_CUPTI_DC_GLD_INST_16BIT, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "gld_inst_32bit", PSC_CUPTI_DC_GLD_INST_32BIT, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "gld_inst_64bit", PSC_CUPTI_DC_GLD_INST_64BIT, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "gld_inst_128bit", PSC_CUPTI_DC_GLD_INST_128BIT, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "gst_inst_8bit", PSC_CUPTI_DC_GST_INST_8BIT, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "gst_inst_16bit", PSC_CUPTI_DC_GST_INST_16BIT, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "gst_inst_32bit", PSC_CUPTI_DC_GST_INST_32BIT, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "gst_inst_64bit", PSC_CUPTI_DC_GST_INST_64BIT, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "gst_inst_128bit", PSC_CUPTI_DC_GST_INST_128BIT, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "branch", PSC_CUPTI_DD_BRANCH, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "divergent_branch", PSC_CUPTI_DD_DIVERGENT_BRANCH, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "warp_launched", PSC_CUPTI_DD_WARPS_LAUNCHED, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "threads_launched", PSC_CUPTI_DD_THREADS_LAUNCHED, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "active_warps", PSC_CUPTI_DD_ACTIVE_WARPS, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "active_cycles", PSC_CUPTI_DD_ACTIVE_CYCLES, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "local_load", PSC_CUPTI_DD_LOCAL_LOAD, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "local_store", PSC_CUPTI_DD_LOCAL_STORE, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "gld_request", PSC_CUPTI_DD_GLD_REQUEST, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "gst_request", PSC_CUPTI_DD_GST_REQUEST, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "shared_load", PSC_CUPTI_DD_SHARED_LOAD, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "shared_store", PSC_CUPTI_DD_SHARED_STORE, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "prof_trigger_00", PSC_CUPTI_DD_PROF_TRIGGER_00, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "prof_trigger_01", PSC_CUPTI_DD_PROF_TRIGGER_01, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "prof_trigger_02", PSC_CUPTI_DD_PROF_TRIGGER_02, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "prof_trigger_03", PSC_CUPTI_DD_PROF_TRIGGER_03, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "prof_trigger_04", PSC_CUPTI_DD_PROF_TRIGGER_04, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "prof_trigger_05", PSC_CUPTI_DD_PROF_TRIGGER_05, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "prof_trigger_06", PSC_CUPTI_DD_PROF_TRIGGER_06, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "prof_trigger_07", PSC_CUPTI_DD_PROF_TRIGGER_07, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "inst_issued", PSC_CUPTI_DD_INST_ISSUED, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "inst_issued1_0", PSC_CUPTI_DD_INST_ISSUED1_0, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "inst_issued2_1", PSC_CUPTI_DD_INST_ISSUED2_0, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "inst_issued1_1", PSC_CUPTI_DD_INST_ISSUED1_1, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "inst_issued2_2", PSC_CUPTI_DD_INST_ISSUED2_1, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "inst_executed", PSC_CUPTI_DD_INST_EXECUTED, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "thread_inst_executed_0", PSC_CUPTI_DD_THREAD_INST_EXECUTED_0, CID_NO_CODE );
    setMetricInEventList( GROUP_CUDA_COUNTER,             "thread_inst_executed_1", PSC_CUPTI_DD_THREAD_INST_EXECUTED_1, CID_NO_CODE );

    // ENOPT metrics
    setMetricInEventList( GROUP_UNDEFINED,                "ENOPT_START", PSC_ENOPT_START, CID_NO_CODE );
    setMetricInEventList( GROUP_ENOPT_ENERGY_COUNTER,     "ENOPT_ALL_CORES", PSC_ENOPT_ALL_CORES, ENOPT_ALL_CORES );
    setMetricInEventList( GROUP_ENOPT_ENERGY_COUNTER,     "ENOPT_ALL_UNCORES", PSC_ENOPT_ALL_UNCORES, ENOPT_ALL_UNCORES );
    setMetricInEventList( GROUP_ENOPT_ENERGY_COUNTER,     "ENOPT_ALL_SOCKETS", PSC_ENOPT_ALL_SOCKETS, ENOPT_ALL_SOCKETS );
    setMetricInEventList( GROUP_ENOPT_ENERGY_COUNTER,     "ENOPT_ALL_DRAMS", PSC_ENOPT_ALL_DRAMS, ENOPT_ALL_DRAMS );
    setMetricInEventList( GROUP_ENOPT_ENERGY_COUNTER,     "ENOPT_NODE", PSC_ENOPT_NODE, ENOPT_NODE );
    setMetricInEventList( GROUP_ENOPT_ENERGY_COUNTER,     "ENOPT_PDU", PSC_ENOPT_PDU, ENOPT_PDU );
    setMetricInEventList( GROUP_ENOPT_ENERGY_COUNTER,     "ENOPT_CORES_1", PSC_ENOPT_CORES_1, ENOPT_CORES_1 );
    setMetricInEventList( GROUP_ENOPT_ENERGY_COUNTER,     "ENOPT_CORES_2", PSC_ENOPT_CORES_2, ENOPT_CORES_2 );
    setMetricInEventList( GROUP_ENOPT_ENERGY_COUNTER,     "ENOPT_UNCORES_1", PSC_ENOPT_UNCORES_1, ENOPT_UNCORES_1 );
    setMetricInEventList( GROUP_ENOPT_ENERGY_COUNTER,     "ENOPT_UNCORES_2", PSC_ENOPT_UNCORES_2, ENOPT_UNCORES_2 );
    setMetricInEventList( GROUP_ENOPT_ENERGY_COUNTER,     "ENOPT_SOCKET_1", PSC_ENOPT_SOCKET_1, ENOPT_SOCKET_1 );
    setMetricInEventList( GROUP_ENOPT_ENERGY_COUNTER,     "ENOPT_SOCKET_2", PSC_ENOPT_SOCKET_2, ENOPT_SOCKET_2 );
    setMetricInEventList( GROUP_ENOPT_ENERGY_COUNTER,     "ENOPT_DRAM_1", PSC_ENOPT_DRAM_1, ENOPT_DRAM_1 );
    setMetricInEventList( GROUP_ENOPT_ENERGY_COUNTER,     "ENOPT_DRAM_2", PSC_ENOPT_DRAM_2, ENOPT_DRAM_2 );
    setMetricInEventList( GROUP_UNDEFINED,                "ENOPT_END", PSC_ENOPT_END, CID_NO_CODE );

    // Vienna pattern metrics
    setMetricInEventList( GROUP_VPATTERN_COUNTER,         "PIPE_STAGEEXEC_TIME", PSC_PIPE_STAGEEXEC_TIME, CID_NO_CODE );
    setMetricInEventList( GROUP_VPATTERN_COUNTER,         "PIPE_BUFINP_TIME", PSC_PIPE_BUFINP_TIME, CID_NO_CODE );
    setMetricInEventList( GROUP_VPATTERN_COUNTER,         "PIPE_BUFOUTP_TIME", PSC_PIPE_BUFOUTP_TIME, CID_NO_CODE );
    setMetricInEventList( GROUP_VPATTERN_COUNTER,         "PIPE_BUF_SIZE", PSC_PIPE_BUF_SIZE, CID_NO_CODE );
    setMetricInEventList( GROUP_VPATTERN_COUNTER,         "PIPE_EXEC_TIME", PSC_PIPE_EXEC_TIME, CID_NO_CODE );

    // Configurable metrics
    {
        const int baseSize                                            = sizeof( "CONFIGURABLE_MEASURE" );
        char      measureName[ sizeof( "CONFIGURABLE_MEASURE" ) + 3 ] = "CONFIGURABLE_MEASURE";
        Metric    index;
        measureName[ baseSize - 1 ] = '_';
        measureName[ baseSize + 1 ] = 0;
        for( index = PSC_CONFIGURABLE_MEASURE; index <= PSC_CONFIGURABLE_MEASURE_LAST; index++ ) {
            measureName[ baseSize ] = 'A' + index - PSC_CONFIGURABLE_MEASURE;
            setMetricInEventList( GROUP_CONFIGURABLE_COUNTER, measureName, index, CID_NO_CODE );
        }
    }

    // Other metrics
    setMetricInEventList( GROUP_OTHER,                    "TOTAL_ISSUED_INSTRUCTIONS", PSC_TOTAL_ISSUED_INSTRUCTIONS, CID_NO_CODE );
    setMetricInEventList( GROUP_OTHER,                    "INSTANCES", PSC_INSTANCES, CID_NO_CODE );

    //MPI metrics
    setMetricInEventList( GROUP_MPI,                      "MPI", PSC_MPI, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_TIME_SPENT", PSC_MPI_TIME_SPENT, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_CALL_COUNT", PSC_MPI_CALL_COUNT, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_CALL_COUNT_REMOTE", PSC_MPI_CALL_COUNT_REMOTE, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_REMOTE_SITE_COUNT", PSC_MPI_REMOTE_SITE_COUNT, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_AGGREGATE_MESSAGE_SIZE", PSC_MPI_AGGREGATE_MESSAGE_SIZE, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_SGI_COUNTERS", PSC_MPI_SGI_COUNTERS, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_LATE_SEND", PSC_MPI_LATE_SEND, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_EARLY_RECV", PSC_MPI_EARLY_RECV, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_LATE_RECV", PSC_MPI_LATE_RECV, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_EARLY_BCAST", PSC_MPI_EARLY_BCAST, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_LATE_BCAST", PSC_MPI_LATE_BCAST, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_EARLY_SCATTER", PSC_MPI_EARLY_SCATTER, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_LATE_SCATTER", PSC_MPI_LATE_SCATTER, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_LATE_GATHER", PSC_MPI_LATE_GATHER, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_LATE_REDUCE", PSC_MPI_LATE_REDUCE, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_LATE_ALLREDUCE", PSC_MPI_LATE_ALLREDUCE, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_LATE_ALLGATHER", PSC_MPI_LATE_ALLGATHER, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_LATE_ALLTOALL", PSC_MPI_LATE_ALLTOALL, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_LATE_BARRIER", PSC_MPI_LATE_BARRIER, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_LATE_SCAN", PSC_MPI_LATE_SCAN, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_MSG_FREQ_2K", PSC_MPI_MSG_FREQ_2K, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_MSG_FREQ_4K", PSC_MPI_MSG_FREQ_4K, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_MSG_FREQ_8K", PSC_MPI_MSG_FREQ_8K, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_MSG_FREQ_16K", PSC_MPI_MSG_FREQ_16K, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_MSG_FREQ_32K", PSC_MPI_MSG_FREQ_32K, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_MSG_FREQ_64K", PSC_MPI_MSG_FREQ_64K, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_MSG_P2P_TOT", PSC_MPI_MSG_P2P_TOT, CID_NO_CODE );
    setMetricInEventList( GROUP_MPI,                      "MPI_MSG_P2P_THR", PSC_MPI_MSG_P2P_THR, CID_NO_CODE );

    //OMP metrics
    setMetricInEventList( GROUP_OMP,                      "IMPLICIT_BARRIER_TIME", PSC_IMPLICIT_BARRIER_TIME, CID_NO_CODE );
    setMetricInEventList( GROUP_OMP,                      "CRITICAL_REGION_CYCLE", PSC_CRITICAL_REGION_CYCLE, CID_NO_CODE );
    setMetricInEventList( GROUP_OMP,                      "CRITICAL_BODY_CYCLE", PSC_CRITICAL_BODY_CYCLE, CID_NO_CODE );
    setMetricInEventList( GROUP_OMP,                      "SINGLE_REGION_CYCLE", PSC_SINGLE_REGION_CYCLE, CID_NO_CODE );
    setMetricInEventList( GROUP_OMP,                      "SINGLE_BODY_CYCLE", PSC_SINGLE_BODY_CYCLE, CID_NO_CODE );
    setMetricInEventList( GROUP_OMP,                      "MASTER_BODY_CYCLE", PSC_MASTER_BODY_CYCLE, CID_NO_CODE );
    setMetricInEventList( GROUP_OMP,                      "PARALLEL_REGION_CYCLE", PSC_PARALLEL_REGION_CYCLE, CID_NO_CODE );
    setMetricInEventList( GROUP_OMP,                      "PARALLEL_REGION_BODY_CYCLE", PSC_PARALLEL_REGION_BODY_CYCLE, CID_NO_CODE );
    setMetricInEventList( GROUP_OMP,                      "OMP_BARRIER_CYCLE", PSC_OMP_BARRIER_CYCLE, CID_NO_CODE );
    setMetricInEventList( GROUP_OMP,                      "ORDERED_REGION_CYCLE", PSC_ORDERED_REGION_CYCLE, CID_NO_CODE );
    setMetricInEventList( GROUP_OMP,                      "OMP_ATOMIC_CYCLE", PSC_OMP_ATOMIC_CYCLE, CID_NO_CODE );
    setMetricInEventList( GROUP_OMP,                      "OMP_SECTIONS_REGION_CYCLE", PSC_OMP_SECTIONS_REGION_CYCLE, CID_NO_CODE );
    setMetricInEventList( GROUP_OMP,                      "OMP_SECTION_BODY_CYCLE", PSC_OMP_SECTION_BODY_CYCLE, CID_NO_CODE );
    setMetricInEventList( GROUP_OMP,                      "OMP_DO_REGION_CYCLE", PSC_OMP_DO_REGION_CYCLE, CID_NO_CODE );
    setMetricInEventList( GROUP_OMP,                      "TASK_REGION_CYCLE", PSC_TASK_REGION_CYCLE, CID_NO_CODE );
    setMetricInEventList( GROUP_OMP,                      "TASK_REGION_BODY_CYCLE", PSC_TASK_REGION_BODY_CYCLE, CID_NO_CODE );
    setMetricInEventList( GROUP_OMP,                      "TASKS_CREATED", PSC_TASKS_CREATED, CID_NO_CODE );
    setMetricInEventList( GROUP_OMP,                      "TASKS_EXECUTED", PSC_TASKS_EXECUTED, CID_NO_CODE );
    setMetricInEventList( GROUP_OMP,                      "FLUSH_CYCLES", PSC_FLUSH_CYCLES, CID_NO_CODE );

    //hdeem
    setMetricInEventList( GROUP_HDEEM,                    "hdeem/BLADE/E", PSC_SYNC_HDEEM_BLADE, CID_NO_CODE );

    printf( "Metric Plugin Name: %s\n", metric_plugin_name );
    //energy
    if( strcmp( metric_plugin_name, "hdeem_sync_plugin" ) == 0 )
    {
        setMetricInEventList( GROUP_ENERGY,                    "hdeem/BLADE/E", PSC_NODE_ENERGY, CID_NO_CODE );
        setMetricInEventList( GROUP_ENERGY,                    "hdeem/CPU0/E", PSC_CPU0_ENERGY, CID_NO_CODE );
        setMetricInEventList( GROUP_ENERGY,                    "hdeem/CPU1/E", PSC_CPU1_ENERGY, CID_NO_CODE );

    }
    else
    {
        setMetricInEventList( GROUP_ENERGY,                    "x86_energy/BLADE/E", PSC_NODE_ENERGY, CID_NO_CODE );
        setMetricInEventList( GROUP_ENERGY,                    "x86_energy/CORE0/E", PSC_CPU0_ENERGY, CID_NO_CODE );
        setMetricInEventList( GROUP_ENERGY,                    "x86_energy/CORE1/E", PSC_CPU1_ENERGY, CID_NO_CODE );

    }


    //Overhead metrics
    setMetricInEventList( GROUP_LIBCALLS_OVERHEAD_COUNTER, "OVERHEAD", PSC_MRI_OVERHEAD, CID_NO_CODE );
    setMetricInEventList( GROUP_LIBCALLS_OVERHEAD_COUNTER, "LIBCALLS", PSC_MRI_LIBCALLS, CID_NO_CODE );
    setMetricInEventList( GROUP_LIBCALLS_OVERHEAD_COUNTER, "OVERHEAD", PSC_MRI_EXCL_OVERHEAD, CID_NO_CODE );


    for( i = 1; i < PSC_LAST_METRIC - 1; i++ ) {
        if( EventList[ i ].EventMetric == PSC_UNDEFINED_METRIC ) {
            printf( "ERROR: Metric %d not initialized in EventList array.\n", i );
            exit( 1 );
        }
    }
}

void finalizeEventlist( void ) {
    free( EventList );
}
