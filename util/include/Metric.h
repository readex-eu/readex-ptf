#ifndef METRIC_H_
#define METRIC_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __cplusplus
#include <string>
#endif

#include <stdlib.h>




/** @def PSC_MAX_CONFIGURABLE_MEASURE
    @brief Define globally and statically the maximum number of concurrent quality expression evaluation
 */
#define PSC_MAX_CONFIGURABLE_MEASURE 16

typedef enum GroupEnum {
    GROUP_PAPI_COUNTER              =   1, /* Handled in KPCL_Interface from the MRIMonitor */
    GROUP_PAPI_POWER6_COUNTER       =   2, /* Handled in KPCL_Interface from the MRIMonitor */
    GROUP_PAPI_NEHALEM_COUNTER      =   3, /* Handled in KPCL_Interface from the MRIMonitor */
    GROUP_PERISCOPE_COUNTER         =   4, /* Handled in KPCL_Interface from the MRIMonitor */
    GROUP_OPENCL_TIME_MEASUREMENT   =   5, /* Handled in OCLCL_Interface from the MRIMonitor */
    GROUP_CUDA_TIME_MEASUREMENT     =   6, /* Handled in GPUCL_Interface from the MRIMonitor */
    GROUP_CUDA_BYTES_TRANSFERRED    =   7, /* Handled in GPUCL_Interface from the MRIMonitor */
    GROUP_CUDA_COUNTER              =   8, /* Handled in GPUCL_Interface from the MRIMonitor */
    GROUP_CONFIGURABLE_COUNTER      =  10, /* Handled in QUALEXPRCL_Interface from the MRIMonitor */
    GROUP_ENOPT_ENERGY_COUNTER      =  11, /* Handled in ENCL_Interface from the MRIMonitor */
    GROUP_VPATTERN_COUNTER          =  12, /* Handled by external library which returns result in return_summary_data in the MRIMonitor */
    GROUP_LIBCALLS_OVERHEAD_COUNTER =  13, /* Handled in LCNT_Interface from the MRIMonitor */
    GROUP_TIME_MEASUREMENT          =  14, /* Handled in ETCL_Interface from the MRIMonitor */
    GROUP_MPI                       =  15, /* Handled in mpiProfiler from the MRIMonitor */
    GROUP_OMP                       =  16, /* Handled in DataProvider from the Analysis Agent */
    GROUP_HDEEM                     =  17, /* Handled in ??? */
    GROUP_ENERGY                            =  18, /* Handled in DataProvider from the Analysis Agent*/
    GROUP_OTHER                     =  19, /* Handled in DataProvider from the Analysis Agent*/
    GROUP_UNDEFINED                 = 999
}Group;

typedef enum MetricEnum {
    PSC_EXECUTION_TIME =  1,
    PSC_PAPI_L1_TCM,
    PSC_PAPI_L1_DCM,
    PSC_PAPI_L1_DCA,
    PSC_PAPI_L1_ICM,
    PSC_PAPI_L2_DCM,
    PSC_PAPI_L2_DCA,
    PSC_PAPI_L3_TCM,
    PSC_PAPI_L3_TCA,
    PSC_PAPI_TOT_INS,                                   // issued instructions on Itanium
    PSC_PAPI_INT_INS,
    PSC_PAPI_FP_OPS,
    PSC_PAPI_LD_INS,
    PSC_PAPI_SR_INS,
    PSC_PAPI_TLB_DM,
    PSC_PAPI_TLB_IM,
    PSC_PAPI_LST_INS,
    PSC_PAPI_L2_ICR,
    PSC_PAPI_L2_ICH,
    PSC_PAPI_TOT_CYC,
    PSC_AVX_INSTS_CALC,
    PSC_BACK_END_BUBBLE_ALL,
    PSC_BACK_END_BUBBLE_FE,
    PSC_BE_EXE_BUBBLE_FRALL,
    PSC_BE_EXE_BUBBLE_GRALL,
    PSC_BE_EXE_BUBBLE_GRGR,
    PSC_BE_FLUSH_BUBBLE_BRU,
    PSC_BE_FLUSH_BUBBLE_XPN,
    PSC_BE_L1D_FPU_BUBBLE_FPU,
    PSC_BE_L1D_FPU_BUBBLE_L1D,
    PSC_BE_L1D_FPU_BUBBLE_L1D_FULLSTBUF,
    PSC_BE_L1D_FPU_BUBBLE_L1D_DCURECIR,
    PSC_BE_L1D_FPU_BUBBLE_L1D_HPW,
    PSC_BE_L1D_FPU_BUBBLE_L1D_TLB,
    PSC_BE_L1D_FPU_BUBBLE_L1D_L2BPRESS,
    PSC_BE_RSE_BUBBLE_ALL,
    PSC_FP_FALSE_SIRSTALL,
    PSC_FP_FLUSH_TO_ZERO,
    PSC_FP_TRUE_SIRSTALL,
    PSC_L1D_READ_MISSES_ALL,
    PSC_L2_DATA_REFERENCES_L2_DATA_READS,
    PSC_L2_FORCE_RECIRC_FILL_HIT,
    PSC_L2_FORCE_RECIRC_L1W,
    PSC_L2_FORCE_RECIRC_OZQ_MISS,
    PSC_L2_FORCE_RECIRC_SAME_INDEX,
    PSC_L2_OZQ_CANCELS1_BANK_CONF,
    PSC_L3_READS_DATA_READ_ALL,
    PSC_L3_READS_DATA_READ_HIT,
    PSC_L3_READS_DATA_READ_MISS,
    PSC_L2_OZQ_FULL_THIS,
    PSC_BE_FLUSH_BUBBLE_ALL,
    PSC_BE_L1D_FPU_BUBBLE_ALL,
    PSC_BE_EXE_BUBBLE_ALL,


    PSC_DATA_EAR_CACHE_LAT4,
    PSC_DATA_EAR_CACHE_LAT8,
    PSC_DATA_EAR_CACHE_LAT16,
    PSC_DATA_EAR_CACHE_LAT32,
    PSC_DATA_EAR_CACHE_LAT64,
    PSC_DATA_EAR_CACHE_LAT128,
    PSC_DATA_EAR_CACHE_LAT256,
    PSC_DATA_EAR_CACHE_LAT512,
    PSC_DATA_EAR_CACHE_LAT1024,
    PSC_DATA_EAR_CACHE_LAT2048,
    PSC_DATA_EAR_CACHE_LAT4096,

//p575 - Power Metric
    PSC_PM_START,
    PSC_PM_RUN_CYC,
    PSC_PM_DPU_HELD,
    PSC_PM_DPU_HELD_FP_FX_MULT,
    PSC_PM_DPU_HELD_MULT_GPR,
    PSC_PM_FPU_FXMULT,
    PSC_PM_FPU_FXDIV,
    PSC_PM_FXU_PIPELINED_MULT_DIV,
    PSC_PM_LD_MISS_L1_CYC,
    PSC_PM_DATA_FROM_L2_CYC,
    PSC_PM_DATA_FROM_L21_CYC,
    PSC_PM_DATA_FROM_L3_CYC,
    PSC_PM_LSU_LMQ_FULL_CYC,
    PSC_PM_DC_PREF_OUT_OF_STREAMS,
    PSC_PM_DC_PREF_STREAM_ALLOC,
    PSC_PM_LSU_DERAT_MISS_CYC,
    PSC_PM_DERAT_MISS_4K,
    PSC_PM_DERAT_MISS_64K,
    PSC_PM_DERAT_MISS_16M,
    PSC_PM_DERAT_MISS_16G,
    PSC_PM_DERAT_REF_4K,
    PSC_PM_DERAT_REF_64K,
    PSC_PM_DERAT_REF_16M,
    PSC_PM_DERAT_REF_16G,
    PSC_PM_L2_ST_REQ_DATA,
    PSC_PM_L2_ST_MISS_DATA,
    PSC_PM_LSU_REJECT_STQ_FULL,
    PSC_PM_ST_REF_L1,
    PSC_PM_LSU_ST_CHAINED,
    PSC_PM_L2_PREF_LD,
    PSC_PM_L1_PREF,
    PSC_PM_LD_REF_L1,
    PSC_PM_L2_PREF_ST,
    PSC_PM_DPU_WT,
    PSC_PM_BR_MPRED,
    PSC_PM_BR_PRED,
    PSC_PM_DPU_WT_IC_MISS,
    PSC_PM_LSU_REJECT_LHS,
    PSC_PM_DPU_HELD_GPR,
    PSC_PM_DPU_HELD_FPQ,
    PSC_PM_DPU_HELD_FPU_CR,
    PSC_PM_DPU_HELD_SMT,
    PSC_PM_FPU_FCONV,
    PSC_PM_FPU_FSQRT_FDIV,
    PSC_PM_FPU_FMA,
    PSC_PM_DPU_HELD_FXU_MULTI,
    PSC_PM_DPU_HELD_INT,
    PSC_PM_DPU_HELD_CR_LOGICAL,
    PSC_PM_L3SA_REF,
    PSC_PM_L3SA_MISS,
    PSC_PM_L3SA_HIT,
    PSC_PM_L3SB_REF,
    PSC_PM_L3SB_MISS,
    PSC_PM_L3SB_HIT,
    PSC_PM_FPU_ISSUE_0,
    PSC_PM_FPU_ISSUE_1,
    PSC_PM_FPU_ISSUE_2,
    PSC_PM_FPU_ISSUE_DIV_SQRT_OVERLAP,
    PSC_PM_FPU_ISSUE_STALL_FPR,
    PSC_PM_FPU_ISSUE_STALL_ST,
    PSC_PM_FPU_FLOP,
    PSC_PM_DATA_FROM_L2MISS,
    PSC_PM_DATA_FROM_L2,
    PSC_PM_DATA_FROM_L3MISS,
    PSC_PM_DATA_FROM_L3,
    PSC_PM_DATA_FROM_L21,
    PSC_PM_DATA_FROM_LMEM,
    PSC_PM_L2_LD_REQ_DATA,
    PSC_PM_LD_REQ_L2,
    PSC_PM_L2_LD_MISS_DATA,
    PSC_PM_INST_DISP,
    PSC_PM_RUN_INST_CMPL,
    PSC_PM_1PLUS_PPC_CMPL,
    PSC_PM_1PLUS_PPC_DISP,
    PSC_PM_LD_MISS_L1,
    PSC_PM_END,
//end p575

//added for benchmarking strategy
    PSC_PAPI_BR_CN,
    PSC_PAPI_BR_INS,
    PSC_PAPI_BR_MSP,
    PSC_PAPI_L1_DCH,
    PSC_PAPI_L2_DCH,
    PSC_PAPI_L3_DCA,
    PSC_PAPI_L3_DCH,
    PSC_PAPI_L3_DCM,
    PSC_PAPI_RES_STL,
    PSC_PAPI_TLB_TL,
    PSC_PAPI_TOT_IIS,

    PSC_PAPI_L2_TCM,                                    //added for Nehalem
    PSC_PAPI_L2_TCH,                                    //added for Nehalem
    PSC_PAPI_FP_INS,                                    //added for Nehalem/Sandy Bridge
//end PAPI

/* NP Metrics - Nehalem property*/
    PSC_NP_THREAD_P,                                    // Cycles when thread is not halted (programmable counter)
    PSC_NP_UOPS_EXECUTED_PORT015,                       // Counts number of Uops executed that where issued on port 0, 1, or 5. use cmask = 1, invert = 1 to count stall cycles
    PSC_NP_UOPS_ISSUED_FUSED,                           // Counts the number of fused Uops that were issued from the Register Allocation Table to the Reservation Station
    PSC_NP_UOPS_ISSUED_ANY,                             // Counts the number of Uops issued by the Register Allocation Table to the Reservation Station, i.e. the UOPs issued from the front end to the back end
    PSC_NP_UOPS_RETIRED_ANY,                            // Counts the number of micro-ops retired, (macro-fused = 1, micro-fused = 2, others = 1; maximum count of 8 per cycle). Most instructions are composed of one or two micro-ops. Some instructions are decoded into longer sequences such as repeat instructions, floating point transcendental instructions, and assists. Use cmask = 1 and invert to count active cycles or stalled cycles
    PSC_NP_STALL_CYCLES,                                // Cycles stalled no issued uops
    PSC_NP_RESOURCE_STALLS_ANY,                         // Counts the number of Allocator resource related stalls. Includes register renaming buffer entries, memory buffer entries
    PSC_NP_INSTRUCTION_RETIRED,                         // Count the number of instructions at retirement
    PSC_NP_MEM_INST_RETIRED_LOADS,                      // Counts the number of instructions with an architecturally-visible store retired on the architected path
    PSC_NP_MEM_INST_RETIRED_STORES,                     // Counts the number of instructions with an architecturally-visible store retired on the architected path
    PSC_NP_DTLB_MISSES_ANY,                             // Counts the number of misses in the STLB which causes a page walk
    PSC_NP_DTLB_LOAD_MISSES_ANY,                        // Counts all load misses that cause a page walk
    PSC_NP_DTLB_MISSES_WALK_COMPLETED,                  // Counts number of misses in the STLB which resulted in a completed page walk
    PSC_NP_ITLB_MISSES_ANY,                             // Counts the number of misses in all levels of the ITLB which causes a page walk
    PSC_NP_PARTIAL_ADDRESS_ALIAS,                       // False dependencies due to partial address aliasing
    PSC_NP_UOPS_DECODED_MS,                             // Counts the number of Uops decoded by the Microcode Sequencer, MS. The MS delivers uops when the instruction is more than 4 uops long or a microcode assist is occurring

/* OpenCL metrics */
    PSC_OPENCL_TRANSFER_EXECUTION_TIME,                 // Execution time for the transfers
    PSC_OPENCL_KERNEL_EXECUTION_TIME,                   // Execution time of the kernel
/* CUDA metrics */
    PSC_CUDA_TRANSFER_EXECUTION_TIME,                   // Execution time for the transfers
    PSC_CUDA_BYTES_TRANSFERRED,                         // Number of bytes transfered
    PSC_CUDA_KERNEL_EXECUTION_TIME,                     // Execution time of the kernel

///* NVIDIA CUDA Capable device with Compute Capability 1.x */
//  PSC_CUPTI_DA_TEX_CACHE_HIT                          = 200,
//  PSC_CUPTI_DA_TEX_CACHE_MISS                         = 201,
//  PSC_CUPTI_DB_BRANCH                                 = 202,
//  PSC_CUPTI_DB_DIVERGENT_BRANCH                       = 203,
//  PSC_CUPTI_DB_INSTRUCTIONS                           = 204,
//  PSC_CUPTI_DB_WARP_SERIALIZE                         = 205,
//  PSC_CUPTI_DB_GLD_INCOHERENT                         = 206,
//  PSC_CUPTI_DB_GLD_COHERENT                           = 207,
//  PSC_CUPTI_DB_GLD_32B                                = 208,
//  PSC_CUPTI_DB_GLD_64B                                = 209,
//  PSC_CUPTI_DB_GLD_128                                = 210,
//  PSC_CUPTI_DB_GST_INCOHERENT                         = 211,
//  PSC_CUPTI_DB_GST_COHERENT                           = 212,
//  PSC_CUPTI_DB_GST_32B                                = 213,
//  PSC_CUPTI_DB_GST_64B                                = 214,
//  PSC_CUPTI_DB_GST_128                                = 215,
//  PSC_CUPTI_DB_LOCAL_LOAD                             = 216,
//  PSC_CUPTI_DB_LOCAL_STORE                            = 217,
//  PSC_CUPTI_DB_CTA_LAUNCHED                           = 218,
//  PSC_CUPTI_DB_SM_CTA_LAUNCHED                        = 219,
//  PSC_CUPTI_DB_PROF_TRIGGER_00                        = 220,
//  PSC_CUPTI_DB_PROF_TRIGGER_01                        = 221,
//  PSC_CUPTI_DB_PROF_TRIGGER_02                        = 222,
//  PSC_CUPTI_DB_PROF_TRIGGER_03                        = 223,
//  PSC_CUPTI_DB_PROF_TRIGGER_04                        = 224,
//  PSC_CUPTI_DB_PROF_TRIGGER_05                        = 225,
//  PSC_CUPTI_DB_PROF_TRIGGER_06                        = 226,
//  PSC_CUPTI_DB_PROF_TRIGGER_07                        = 227,
//
///* NVIDIA CUDA Capable device with Compute Capability 2.x */
//  PSC_CUPTI_DA_SM_CTA_LAUNCHED                        = 228,
//  PSC_CUPTI_DA_L1_LOCAL_LOAD_HIT                      = 229,
//  PSC_CUPTI_DA_L1_LOCAL_LOAD_MISS                     = 230,
//  PSC_CUPTI_DA_L1_LOCAL_STORE_HIT                     = 231,
//  PSC_CUPTI_DA_L1_LOCAL_STORE_MISS                    = 232,
//  PSC_CUPTI_DA_L1_GLOBAL_LOAD_HIT                     = 233,
//  PSC_CUPTI_DA_L1_GLOBAL_LOAD_MISS                    = 234,
//  PSC_CUPTI_DA_UNCACHED_GLOBAL_LOAD_TRANSACTION       = 235,
//  PSC_CUPTI_DA_GLOBAL_STORE_TRANSACTION               = 236,
//  PSC_CUPTI_DA_L1_SHARED_BANK_CONFLICT                = 237,
//  PSC_CUPTI_DA_TEX0_CACHE_SECTOR_QUERIES              = 238,
//  PSC_CUPTI_DA_TEX0_CACHE_SECTOR_MISSES               = 239,
//  PSC_CUPTI_DA_TEX1_CACHE_SECTOR_QUERIES              = 240,
//  PSC_CUPTI_DA_TEX1_CACHE_SECTOR_MISSES               = 241,
//  PSC_CUPTI_DB_L2_SUBP0_WRITE_SECTOR_MISSES           = 242,
//  PSC_CUPTI_DB_L2_SUBP1_WRITE_SECTOR_MISSES           = 243,
//  PSC_CUPTI_DB_L2_SUBP0_READ_SECTOR_MISSES            = 244,
//  PSC_CUPTI_DB_L2_SUBP1_READ_SECTOR_MISSES            = 245,
//  PSC_CUPTI_DB_L2_SUBP0_WRITE_SECTOR_QUERIES          = 246,
//  PSC_CUPTI_DB_L2_SUBP1_WRITE_SECTOR_QUERIES          = 247,
//  PSC_CUPTI_DB_L2_SUBP0_READ_SECTOR_QUERIES           = 248,
//  PSC_CUPTI_DB_L2_SUBP1_READ_SECTOR_QUERIES           = 249,
//  PSC_CUPTI_DB_L2_SUBP0_READ_HIT_SECTORS              = 250,
//  PSC_CUPTI_DB_L2_SUBP1_READ_HIT_SECTORS              = 251,
//  PSC_CUPTI_DB_L2_SUBP0_READ_TEX_SECTOR_QUERIES       = 252,
//  PSC_CUPTI_DB_L2_SUBP1_READ_TEX_SECTOR_QUERIES       = 253,
//  PSC_CUPTI_DB_L2_SUBP0_READ_TEX_HIT_SECTORS          = 254,
//  PSC_CUPTI_DB_L2_SUBP1_READ_TEX_HIT_SECTORS          = 255,
//  PSC_CUPTI_DB_FB_SUBP0_READ_SECTORS                  = 256,
//  PSC_CUPTI_DB_FB_SUBP1_READ_SECTORS                  = 257,
//  PSC_CUPTI_DB_FB_SUBP0_WRITE_SECTORS                 = 258,
//  PSC_CUPTI_DB_FB_SUBP1_WRITE_SECTORS                 = 259,
//  PSC_CUPTI_DB_FB0_SUBP0_READ_SECTORS                 = 260,
//  PSC_CUPTI_DB_FB0_SUBP1_READ_SECTORS                 = 261,
//  PSC_CUPTI_DB_FB0_SUBP0_WRITE_SECTORS                = 262,
//  PSC_CUPTI_DB_FB0_SUBP1_WRITE_SECTORS                = 263,
//  PSC_CUPTI_DB_FB1_SUBP0_READ_SECTORS                 = 264,
//  PSC_CUPTI_DB_FB1_SUBP1_READ_SECTORS                 = 265,
//  PSC_CUPTI_DB_FB1_SUBP0_WRITE_SECTORS                = 266,
//  PSC_CUPTI_DB_FB1_SUBP1_WRITE_SECTORS                = 267,
//  PSC_CUPTI_DC_GLD_INST_8BIT                          = 268,
//  PSC_CUPTI_DC_GLD_INST_16BIT                         = 269,
//  PSC_CUPTI_DC_GLD_INST_32BIT                         = 270,
//  PSC_CUPTI_DC_GLD_INST_64BIT                         = 271,
//  PSC_CUPTI_DC_GLD_INST_128BIT                        = 272,
//  PSC_CUPTI_DC_GST_INST_8BIT                          = 273,
//  PSC_CUPTI_DC_GST_INST_16BIT                         = 274,
//  PSC_CUPTI_DC_GST_INST_32BIT                         = 275,
//  PSC_CUPTI_DC_GST_INST_64BIT                         = 276,
//  PSC_CUPTI_DC_GST_INST_128BIT                        = 277,
//  PSC_CUPTI_DD_BRANCH                                 = 278,
//  PSC_CUPTI_DD_DIVERGENT_BRANCH                       = 279,
//  PSC_CUPTI_DD_WARPS_LAUNCHED                         = 280,
//  PSC_CUPTI_DD_THREADS_LAUNCHED                       = 281,
//  PSC_CUPTI_DD_ACTIVE_WARPS                           = 282,
//  PSC_CUPTI_DD_ACTIVE_CYCLES                          = 283,
//  PSC_CUPTI_DD_LOCAL_LOAD                             = 284,
//  PSC_CUPTI_DD_LOCAL_STORE                            = 285,
//  PSC_CUPTI_DD_GLD_REQUEST                            = 286,
//  PSC_CUPTI_DD_GST_REQUEST                            = 287,
//  PSC_CUPTI_DD_SHARED_LOAD                            = 288,
//  PSC_CUPTI_DD_SHARED_STORE                           = 289,
//  PSC_CUPTI_DD_PROF_TRIGGER_00                        = 290,
//  PSC_CUPTI_DD_PROF_TRIGGER_01                        = 291,
//  PSC_CUPTI_DD_PROF_TRIGGER_02                        = 292,
//  PSC_CUPTI_DD_PROF_TRIGGER_03                        = 293,
//  PSC_CUPTI_DD_PROF_TRIGGER_04                        = 294,
//  PSC_CUPTI_DD_PROF_TRIGGER_05                        = 295,
//  PSC_CUPTI_DD_PROF_TRIGGER_06                        = 296,
//  PSC_CUPTI_DD_PROF_TRIGGER_07                        = 297,
//  PSC_CUPTI_DD_INST_ISSUED                            = 298,
//  PSC_CUPTI_DD_INST_ISSUED1_0                         = 299,
//  PSC_CUPTI_DD_INST_ISSUED2_0                         = 300,
//  PSC_CUPTI_DD_INST_ISSUED1_1                         = 301,
//  PSC_CUPTI_DD_INST_ISSUED2_1                         = 302,
//  PSC_CUPTI_DD_INST_EXECUTED                          = 303,
//  PSC_CUPTI_DD_THREAD_INST_EXECUTED_0                 = 304,
//  PSC_CUPTI_DD_THREAD_INST_EXECUTED_1                 = 305,
//
///* NVIDIA CUDA Capable device with Compute Capability 3.x */
//  PSC_CUPTI_DA_TEX0_CACHE_SECTOR_QUERIES              = 306,
//  PSC_CUPTI_DA_TEX0_CACHE_SECTOR_MISSES               = 307,
//  PSC_CUPTI_DA_TEX1_CACHE_SECTOR_QUERIES              = 308,
//  PSC_CUPTI_DA_TEX1_CACHE_SECTOR_MISSES               = 309,
//  PSC_CUPTI_DA_TEX2_CACHE_SECTOR_QUERIES              = 306,
//  PSC_CUPTI_DA_TEX2_CACHE_SECTOR_MISSES               = 307,
//  PSC_CUPTI_DA_TEX3_CACHE_SECTOR_QUERIES              = 308,
//  PSC_CUPTI_DA_TEX3_CACHE_SECTOR_MISSES               = 309,
//  PSC_CUPTI_DB_L2_SUBP0_WRITE_SECTOR_MISSES           = 310,
//  PSC_CUPTI_DB_L2_SUBP1_WRITE_SECTOR_MISSES           = 311,
//  PSC_CUPTI_DB_L2_SUBP2_WRITE_SECTOR_MISSES           = 312,
//  PSC_CUPTI_DB_L2_SUBP3_WRITE_SECTOR_MISSES           = 313,
//  PSC_CUPTI_DB_L2_SUBP0_READ_SECTOR_MISSES            = 314,
//  PSC_CUPTI_DB_L2_SUBP1_READ_SECTOR_MISSES            = 315,
//  PSC_CUPTI_DB_L2_SUBP2_READ_SECTOR_MISSES            = 316,
//  PSC_CUPTI_DB_L2_SUBP3_READ_SECTOR_MISSES            = 317,
//  PSC_CUPTI_DB_L2_SUBP0_WRITE_L1_SECTOR_QUERIES       = 318,
//  PSC_CUPTI_DB_L2_SUBP1_WRITE_L1_SECTOR_QUERIES       = 319,
//  PSC_CUPTI_DB_L2_SUBP2_WRITE_L1_SECTOR_QUERIES       = 320,
//  PSC_CUPTI_DB_L2_SUBP3_WRITE_L1_SECTOR_QUERIES       = 321,
//  PSC_CUPTI_DB_L2_SUBP0_READ_L1_SECTOR_QUERIES        = 322,
//  PSC_CUPTI_DB_L2_SUBP1_READ_L1_SECTOR_QUERIES        = 323,
//  PSC_CUPTI_DB_L2_SUBP2_READ_L1_SECTOR_QUERIES        = 324,
//  PSC_CUPTI_DB_L2_SUBP3_READ_L1_SECTOR_QUERIES        = 325,
//  PSC_CUPTI_DB_L2_SUBP0_READ_L1_HIT_SECTORS           = 326,
//  PSC_CUPTI_DB_L2_SUBP1_READ_L1_HIT_SECTORS           = 327,
//  PSC_CUPTI_DB_L2_SUBP2_READ_L1_HIT_SECTORS           = 328,
//  PSC_CUPTI_DB_L2_SUBP3_READ_L1_HIT_SECTORS           = 329,
//  PSC_CUPTI_DB_L2_SUBP0_READ_TEX_SECTOR_QUERIES       = 330,
//  PSC_CUPTI_DB_L2_SUBP1_READ_TEX_SECTOR_QUERIES       = 331,
//  PSC_CUPTI_DB_L2_SUBP2_READ_TEX_SECTOR_QUERIES       = 332,
//  PSC_CUPTI_DB_L2_SUBP3_READ_TEX_SECTOR_QUERIES       = 333,
//  PSC_CUPTI_DB_L2_SUBP0_READ_TEX_HIT_SECTORS          = 334,
//  PSC_CUPTI_DB_L2_SUBP1_READ_TEX_HIT_SECTORS          = 335,
//  PSC_CUPTI_DB_L2_SUBP2_READ_TEX_HIT_SECTORS          = 336,
//  PSC_CUPTI_DB_L2_SUBP3_READ_TEX_HIT_SECTORS          = 337,
//  PSC_CUPTI_DB_FB_SUBP0_READ_SECTORS                  = 338,
//  PSC_CUPTI_DB_FB_SUBP1_READ_SECTORS                  = 339,
//  PSC_CUPTI_DB_FB_SUBP0_WRITE_SECTORS                 = 340,
//  PSC_CUPTI_DB_FB_SUBP1_WRITE_SECTORS                 = 341,
///* Identical with Compute capability 2.x */
//  PSC_CUPTI_DC_GLD_INST_8BIT                          = 342,
//  PSC_CUPTI_DC_GLD_INST_16BIT                         = 343,
//  PSC_CUPTI_DC_GLD_INST_32BIT                         = 344,
//  PSC_CUPTI_DC_GLD_INST_64BIT                         = 345,
//  PSC_CUPTI_DC_GLD_INST_128BIT                        = 346,
//  PSC_CUPTI_DC_GST_INST_8BIT                          = 347,
//  PSC_CUPTI_DC_GST_INST_16BIT                         = 348,
//  PSC_CUPTI_DC_GST_INST_32BIT                         = 349,
//  PSC_CUPTI_DC_GST_INST_64BIT                         = 350,
//  PSC_CUPTI_DC_GST_INST_128BIT                        = 351,
//  PSC_CUPTI_DD_PROF_TRIGGER_00                        = 352,
//  PSC_CUPTI_DD_PROF_TRIGGER_01                        = 353,
//  PSC_CUPTI_DD_PROF_TRIGGER_02                        = 354,
//  PSC_CUPTI_DD_PROF_TRIGGER_03                        = 355,
//  PSC_CUPTI_DD_PROF_TRIGGER_04                        = 356,
//  PSC_CUPTI_DD_PROF_TRIGGER_05                        = 357,
//  PSC_CUPTI_DD_PROF_TRIGGER_06                        = 358,
//  PSC_CUPTI_DD_PROF_TRIGGER_07                        = 359,
//  PSC_CUPTI_DD_WARPS_LAUNCHED                         = 360,
//  PSC_CUPTI_DD_THREADS_LAUNCHED                       = 361,
///* END */
//  PSC_CUPTI_DD_INST_ISSUED1                           = 362,
//  PSC_CUPTI_DD_INST_ISSUED2                           = 363,
///* Identical with Compute capability 2.x */
//  PSC_CUPTI_DD_INST_EXECUTED                          = 303,
///* END */
//  PSC_CUPTI_DD_INST_EXECUTED_LSU_SIZE_128             = 304,
//  PSC_CUPTI_DD_INST_EXECUTED_LSU_SIZE_64              = 305,
//  PSC_CUPTI_DD_INST_EXECUTED_LSU_SIZE_32              = 306,
//  PSC_CUPTI_DD_INST_EXECUTED_LSU_SUB_SIZE_32          = 307,
//  PSC_CUPTI_DD_NOT_PREDICTED_OFF_THREAD_INST_EXECUTED = 308,
//  PSC_CUPTI_DD_WARP_CANT_ISSUE_BARRIER                = 309,
///* Identical with Compute capability 2.x */
//  PSC_CUPTI_DD_LOCAL_LOAD                             = 310,
//  PSC_CUPTI_DD_LOCAL_STORE                            = 311,
//  PSC_CUPTI_DD_GLD_REQUEST                            = 312,
//  PSC_CUPTI_DD_GST_REQUEST                            = 313,
//  PSC_CUPTI_DD_SHARED_LOAD                            = 314,
//  PSC_CUPTI_DD_SHARED_STORE                           = 315,
//  /* END */
//  PSC_CUPTI_DD_L1_LOCAL_LOAD_TRANSACTIONS             = 316,
//  PSC_CUPTI_DD_L1_LOCAL_STORE_TRANSACTIONS            = 317,
//  PSC_CUPTI_DD_L1_SHARED_LOAD_TRANSACTIONS            = 318,
//  PSC_CUPTI_DD_L1_SHARED_STORE_TRANSACTIONS           = 319,
//  PSC_CUPTI_DD_L1_GLOBAL_LOAD_TRANSACTIONS            = 320,
//  PSC_CUPTI_DD_L1_GLOBAL_STORE_TRANSACTIONS           = 321,
//  PSC_CUPTI_DD_L1_LOCAL_LOAD_HIT                      = 322,
//  PSC_CUPTI_DD_L1_LOCAL_LOAD_MISS                     = 323,
//  PSC_CUPTI_DD_L1_LOCAL_STORE_HIT                     = 324,
//  PSC_CUPTI_DD_L1_LOCAL_STORE_MISS                    = 325,
//  PSC_CUPTI_DD_L1_GLOBAL_LOAD_HIT                     = 326,
//  PSC_CUPTI_DD_L1_GLOBAL_LOAD_MISS                    = 327,
//  PSC_CUPTI_DD_UNCACHED_GLOBAL_LOAD_TRANSACTION       = 328,
//  PSC_CUPTI_DD_GLOBAL_STORE_TRANSACTION               = 329,
//  PSC_CUPTI_DD_LOCAL_LD_MEM_DIVERGENCE_REPLAYS        = 330,
//  PSC_CUPTI_DD_LOCAL_ST_MEM_DIVERGENCE_REPLAYS        = 331,
//  PSC_CUPTI_DD_GLOBAL_LD_MEM_DIVERGENCE_REPLAYS       = 332,
//  PSC_CUPTI_DD_GLOBAL_ST_MEM_DIVERGENCE_REPLAYS       = 333,
//  PSC_CUPTI_DD_SHARED_LOAD_BANK_CONFLICT              = 334,
//  PSC_CUPTI_DD_SHARED_STORE_BANK_CONFLICT             = 335,
///* Identical with Compute capability 2.x */
//  PSC_CUPTI_DD_BRANCH                                 = 336,
//  PSC_CUPTI_DD_DIVERGENT_BRANCH                       = 337,
//  PSC_CUPTI_DD_ACTIVE_WARPS                           = 338,
//  PSC_CUPTI_DD_ACTIVE_CYCLES                          = 339,
///* END */
//  PSC_CUPTI_DD_SM_CTA_LAUNCHED                        = 340,

    PSC_CUPTI_DA_SM_CTA_LAUNCHED,
    PSC_CUPTI_DA_L1_LOCAL_LOAD_HIT,
    PSC_CUPTI_DA_L1_LOCAL_LOAD_MISS,
    PSC_CUPTI_DA_L1_LOCAL_STORE_HIT,
    PSC_CUPTI_DA_L1_LOCAL_STORE_MISS,
    PSC_CUPTI_DA_L1_GLOBAL_LOAD_HIT,
    PSC_CUPTI_DA_L1_GLOBAL_LOAD_MISS,
    PSC_CUPTI_DA_UNCACHED_GLOBAL_LOAD_TRANSACTION,
    PSC_CUPTI_DA_GLOBAL_STORE_TRANSACTION,
    PSC_CUPTI_DA_L1_SHARED_BANK_CONFLICT,
    PSC_CUPTI_DA_TEX0_CACHE_SECTOR_QUERIES,
    PSC_CUPTI_DA_TEX0_CACHE_SECTOR_MISSES,
    PSC_CUPTI_DA_TEX1_CACHE_SECTOR_QUERIES,
    PSC_CUPTI_DA_TEX1_CACHE_SECTOR_MISSES,
    PSC_CUPTI_DB_L2_SUBP0_WRITE_SECTOR_MISSES,
    PSC_CUPTI_DB_L2_SUBP1_WRITE_SECTOR_MISSES,
    PSC_CUPTI_DB_L2_SUBP0_READ_SECTOR_MISSES,
    PSC_CUPTI_DB_L2_SUBP1_READ_SECTOR_MISSES,
    PSC_CUPTI_DB_L2_SUBP0_WRITE_SECTOR_QUERIES,
    PSC_CUPTI_DB_L2_SUBP1_WRITE_SECTOR_QUERIES,
    PSC_CUPTI_DB_L2_SUBP0_READ_SECTOR_QUERIES,
    PSC_CUPTI_DB_L2_SUBP1_READ_SECTOR_QUERIES,
    PSC_CUPTI_DB_L2_SUBP0_READ_HIT_SECTORS,
    PSC_CUPTI_DB_L2_SUBP1_READ_HIT_SECTORS,
    PSC_CUPTI_DB_L2_SUBP0_READ_TEX_SECTOR_QUERIES,
    PSC_CUPTI_DB_L2_SUBP1_READ_TEX_SECTOR_QUERIES,
    PSC_CUPTI_DB_L2_SUBP0_READ_TEX_HIT_SECTORS,
    PSC_CUPTI_DB_L2_SUBP1_READ_TEX_HIT_SECTORS,
    PSC_CUPTI_DB_FB_SUBP0_READ_SECTORS,
    PSC_CUPTI_DB_FB_SUBP1_READ_SECTORS,
    PSC_CUPTI_DB_FB_SUBP0_WRITE_SECTORS,
    PSC_CUPTI_DB_FB_SUBP1_WRITE_SECTORS,
    PSC_CUPTI_DB_FB0_SUBP0_READ_SECTORS,
    PSC_CUPTI_DB_FB0_SUBP1_READ_SECTORS,
    PSC_CUPTI_DB_FB0_SUBP0_WRITE_SECTORS,
    PSC_CUPTI_DB_FB0_SUBP1_WRITE_SECTORS,
    PSC_CUPTI_DB_FB1_SUBP0_READ_SECTORS,
    PSC_CUPTI_DB_FB1_SUBP1_READ_SECTORS,
    PSC_CUPTI_DB_FB1_SUBP0_WRITE_SECTORS,
    PSC_CUPTI_DB_FB1_SUBP1_WRITE_SECTORS,
    PSC_CUPTI_DC_GLD_INST_8BIT,
    PSC_CUPTI_DC_GLD_INST_16BIT,
    PSC_CUPTI_DC_GLD_INST_32BIT,
    PSC_CUPTI_DC_GLD_INST_64BIT,
    PSC_CUPTI_DC_GLD_INST_128BIT,
    PSC_CUPTI_DC_GST_INST_8BIT,
    PSC_CUPTI_DC_GST_INST_16BIT,
    PSC_CUPTI_DC_GST_INST_32BIT,
    PSC_CUPTI_DC_GST_INST_64BIT,
    PSC_CUPTI_DC_GST_INST_128BIT,
    PSC_CUPTI_DD_BRANCH,
    PSC_CUPTI_DD_DIVERGENT_BRANCH,
    PSC_CUPTI_DD_WARPS_LAUNCHED,
    PSC_CUPTI_DD_THREADS_LAUNCHED,
    PSC_CUPTI_DD_ACTIVE_WARPS,
    PSC_CUPTI_DD_ACTIVE_CYCLES,
    PSC_CUPTI_DD_LOCAL_LOAD,
    PSC_CUPTI_DD_LOCAL_STORE,
    PSC_CUPTI_DD_GLD_REQUEST,
    PSC_CUPTI_DD_GST_REQUEST,
    PSC_CUPTI_DD_SHARED_LOAD,
    PSC_CUPTI_DD_SHARED_STORE,
    PSC_CUPTI_DD_PROF_TRIGGER_00,
    PSC_CUPTI_DD_PROF_TRIGGER_01,
    PSC_CUPTI_DD_PROF_TRIGGER_02,
    PSC_CUPTI_DD_PROF_TRIGGER_03,
    PSC_CUPTI_DD_PROF_TRIGGER_04,
    PSC_CUPTI_DD_PROF_TRIGGER_05,
    PSC_CUPTI_DD_PROF_TRIGGER_06,
    PSC_CUPTI_DD_PROF_TRIGGER_07,
    PSC_CUPTI_DD_INST_ISSUED,
    PSC_CUPTI_DD_INST_ISSUED1_0,
    PSC_CUPTI_DD_INST_ISSUED2_0,
    PSC_CUPTI_DD_INST_ISSUED1_1,
    PSC_CUPTI_DD_INST_ISSUED2_1,
    PSC_CUPTI_DD_INST_EXECUTED,
    PSC_CUPTI_DD_THREAD_INST_EXECUTED_0,
    PSC_CUPTI_DD_THREAD_INST_EXECUTED_1,

    //ENOPT metrics
    PSC_ENOPT_START,
    PSC_ENOPT_ALL_CORES,
    PSC_ENOPT_ALL_UNCORES,
    PSC_ENOPT_ALL_SOCKETS,
    PSC_ENOPT_ALL_DRAMS,
    PSC_ENOPT_NODE,
    PSC_ENOPT_PDU,
    PSC_ENOPT_CORES_1,
    PSC_ENOPT_CORES_2,
    PSC_ENOPT_UNCORES_1,
    PSC_ENOPT_UNCORES_2,
    PSC_ENOPT_SOCKET_1,
    PSC_ENOPT_SOCKET_2,
    PSC_ENOPT_DRAM_1,
    PSC_ENOPT_DRAM_2,
    PSC_ENOPT_END,

    //Vienna pattern metrics
    PSC_PIPE_STAGEEXEC_TIME,
    PSC_PIPE_BUFINP_TIME,
    PSC_PIPE_BUFOUTP_TIME,
    PSC_PIPE_BUF_SIZE,
    PSC_PIPE_EXEC_TIME,

    PSC_CONFIGURABLE_MEASURE,
    PSC_CONFIGURABLE_MEASURE_LAST = PSC_CONFIGURABLE_MEASURE + PSC_MAX_CONFIGURABLE_MEASURE,

    // # of region executions - this information is always gathered by Score-P - don't request explicitly
    PSC_INSTANCES,

    PSC_TOTAL_ISSUED_INSTRUCTIONS,

    //MPI
    PSC_MPI,                                            ///< Generic MPI Property
    PSC_MPI_TIME_SPENT,                                 ///< Excessive MPI communication time
    PSC_MPI_CALL_COUNT,
    PSC_MPI_CALL_COUNT_REMOTE,
    PSC_MPI_REMOTE_SITE_COUNT,
    PSC_MPI_AGGREGATE_MESSAGE_SIZE,
    PSC_MPI_SGI_COUNTERS,                               ///< SGI MPI retry counters property
    PSC_MPI_LATE_SEND,
    PSC_MPI_EARLY_RECV,
    PSC_MPI_LATE_RECV,
    PSC_MPI_EARLY_BCAST,
    PSC_MPI_LATE_BCAST,
    PSC_MPI_EARLY_SCATTER,
    PSC_MPI_LATE_SCATTER,
    PSC_MPI_LATE_GATHER,
    PSC_MPI_LATE_REDUCE,
    PSC_MPI_LATE_ALLREDUCE,
    PSC_MPI_LATE_ALLGATHER,
    PSC_MPI_LATE_ALLTOALL,
    PSC_MPI_LATE_BARRIER,
    PSC_MPI_LATE_SCAN,
    PSC_MPI_MSG_FREQ_2K,
    PSC_MPI_MSG_FREQ_4K,
    PSC_MPI_MSG_FREQ_8K,
    PSC_MPI_MSG_FREQ_16K,
    PSC_MPI_MSG_FREQ_32K,
    PSC_MPI_MSG_FREQ_64K,
    PSC_MPI_MSG_P2P_TOT,
    PSC_MPI_MSG_P2P_THR,


    PSC_MRI_OVERHEAD,
    PSC_MRI_LIBCALLS,
    PSC_MRI_EXCL_OVERHEAD,

/* OpenMP Metrics */
    PSC_IMPLICIT_BARRIER_TIME,
    PSC_CRITICAL_REGION_CYCLE,
    PSC_CRITICAL_BODY_CYCLE,
    PSC_SINGLE_REGION_CYCLE,
    PSC_SINGLE_BODY_CYCLE,
    PSC_MASTER_BODY_CYCLE,
    PSC_PARALLEL_REGION_CYCLE,
    PSC_PARALLEL_REGION_BODY_CYCLE,
    PSC_OMP_BARRIER_CYCLE,
    PSC_ORDERED_REGION_CYCLE,
    PSC_OMP_ATOMIC_CYCLE,
    PSC_OMP_SECTIONS_REGION_CYCLE,
    PSC_OMP_SECTION_BODY_CYCLE,
    PSC_OMP_DO_REGION_CYCLE,
    PSC_TASK_REGION_CYCLE,
    PSC_TASK_REGION_BODY_CYCLE,
    PSC_TASKS_CREATED,
    PSC_TASKS_EXECUTED,
    PSC_FLUSH_CYCLES,

/* Taurus hdeem*/
    PSC_SYNC_HDEEM_BLADE,

/* PTF energy*/
    PSC_NODE_ENERGY,
    PSC_CPU0_ENERGY,
    PSC_CPU1_ENERGY,

    PSC_UNDEFINED_METRIC,
    PSC_LAST_METRIC
}Metric;

typedef enum ENOPTMetricEnum {
    ENOPT_ALL_CORES = 1,
    ENOPT_ALL_UNCORES,
    ENOPT_ALL_SOCKETS,
    ENOPT_ALL_DRAMS,
    ENOPT_NODE,
    ENOPT_PDU,
    ENOPT_CORES_1,
    ENOPT_CORES_2,
    ENOPT_UNCORES_1,
    ENOPT_UNCORES_2,
    ENOPT_SOCKET_1,
    ENOPT_SOCKET_2,
    ENOPT_DRAM_1,
    ENOPT_DRAM_2
}ENOPTMetric;

/*----------------------------------------------------------------------------*/
/* CID = CounterId */
/*  Event Symbol                         Event Code   Long Description */

#define CID_BACK_END_BUBBLE_ALL              0x40000003   //Full Pipe Bubbles in Main Pipe -- Front-end, RSE, EXE, FPU/L1D stall or a pipeline flush due to an exception/branch misprediction
#define CID_BACK_END_BUBBLE_FE               0x40000004   //Full Pipe Bubbles in Main Pipe -- front-end
#define CID_BE_EXE_BUBBLE_FRALL              0x4000000f   //Full Pipe Bubbles in Main Pipe due to Execution Unit Stalls -- Back-end was stalled by exe due to FR/FR or FR/load dependency
#define CID_BE_EXE_BUBBLE_GRALL              0x40000010   //Full Pipe Bubbles in Main Pipe due to Execution Unit Stalls -- Back-end was stalled by exe due to GR/GR or GR/load dependency
#define CID_BE_EXE_BUBBLE_GRGR               0x40000011   //Full Pipe Bubbles in Main Pipe due to Execution Unit Stalls -- Back-end was stalled by exe due to GR/GR dependency
#define CID_BE_FLUSH_BUBBLE_BRU              0x40000014   //Full Pipe Bubbles in Main Pipe due to Flushes. -- Back-end was stalled due to a branch misprediction flush
#define CID_BE_FLUSH_BUBBLE_XPN              0x40000015   //Full Pipe Bubbles in Main Pipe due to Flushes. -- Back-end was stalled due to an exception/interruption flush
#define CID_BE_L1D_FPU_BUBBLE_FPU            0x40000017   //Full Pipe Bubbles in Main Pipe due to FPU or L1D Cache -- Back-end was stalled by FPU.
#define CID_BE_L1D_FPU_BUBBLE_L1D            0x40000018   //Full Pipe Bubbles in Main Pipe due to FPU or L1D Cache -- Back-end was stalled by L1D. This includes all stalls caused by the L1 pipeline (created in the L1D stage of the L1 pipeline which corresponds to the DET stage of the main pipe).
#define CID_BE_L1D_FPU_BUBBLE_L1D_FULLSTBUF  0x4000001c   //Full Pipe Bubbles in Main Pipe due to FPU or L1D Cache -- Back-end was stalled by L1D due to store buffer being full
#define CID_BE_L1D_FPU_BUBBLE_L1D_DCURECIR   0x4000001a   //Full Pipe Bubbles in Main Pipe due to FPU or L1D Cache -- Back-end was stalled by L1D due to DCU recirculating
#define CID_BE_L1D_FPU_BUBBLE_L1D_HPW        0x4000001d   //Full Pipe Bubbles in Main Pipe due to FPU or L1D Cache -- Back-end was stalled by L1D due to Hardware Page Walker
#define CID_BE_L1D_FPU_BUBBLE_L1D_TLB        0x40000024   //Full Pipe Bubbles in Main Pipe due to FPU or L1D Cache -- Back-end was stalled by L1D due to L2DTLB to L1DTLB transfer
#define CID_BE_L1D_FPU_BUBBLE_L1D_L2BPRESS   0x4000001e   //Full Pipe Bubbles in Main Pipe due to FPU or L1D Cache -- Back-end was stalled by L1D due to L2 Back Pressure
#define CID_BE_RSE_BUBBLE_ALL                0x40000031   //Full Pipe Bubbles in Main Pipe due to RSE Stalls -- Back-end was stalled by RSE
#define CID_FP_FALSE_SIRSTALL                0x40000107   //SIR Stall Without a Trap
#define CID_FP_FLUSH_TO_ZERO                 0x40000108   //FP Result Flushed to Zero
#define CID_FP_TRUE_SIRSTALL                 0x4000010a   //SIR stall asserted and leads to a trap
#define CID_L1D_READ_MISSES_ALL              0x40000131   //L1 Data Cache Read Misses -- all L1D read misses will be counted.
#define CID_L2_DATA_REFERENCES_L2_DATA_READS 0x4000015c   //Data Read/Write Access to L2 -- count only data read and semaphore operations.
#define CID_L2_FORCE_RECIRC_FILL_HIT         0x40000160   //Forced Recirculates -- count only those caused by an L2 miss which hit in the fill buffer.
#define CID_L2_FORCE_RECIRC_L1W              0x40000163   //Forced Recirculates -- count only those caused by forced limbo
#define CID_L2_FORCE_RECIRC_OZQ_MISS         0x40000164   //Forced Recirculates -- caused by an L2 miss when an OZQ miss already existed
#define CID_L2_FORCE_RECIRC_SAME_INDEX       0x40000165   //Forced Recirculates -- caused by an L2 miss when a miss to the same index already existed
#define CID_L2_OZQ_CANCELS1_BANK_CONF        0x40000190   //L2 OZQ Cancels (Specific Reason Set 1) -- bank conflicts
#define CID_L3_READS_DATA_READ_ALL           0x400001b7   //L3 Reads -- L3 Load References (excludes reads for ownership used to satisfy stores)
#define CID_L3_READS_DATA_READ_HIT           0x400001b8   //L3 Reads -- L3 Load Hits (excludes reads for ownership used to satisfy stores)
#define CID_L3_READS_DATA_READ_MISS          0x400001b9   //L3 Reads -- L3 Load Misses (excludes reads for ownership used to satisfy stores)
#define CID_L2_OZQ_FULL_THIS                 0x400001ac   //L2D OZQ Is Full -- L2D OZQ is full
#define CID_BE_FLUSH_BUBBLE_ALL              0x40000013   //Full Pipe Bubbles in Main Pipe due to Flushes. -- Back-end was stalled due to either an exception/interruption or branch misprediction flush
#define CID_BE_L1D_FPU_BUBBLE_ALL            0x40000016   //Full Pipe Bubbles in Main Pipe due to FPU or L1D Cache -- Back-end was stalled by L1D or FPU
#define CID_BE_EXE_BUBBLE_ALL                0x4000000a   //Full Pipe Bubbles in Main Pipe due to Execution Unit Stalls -- Back-end was stalled by exe
#define CID_DATA_EAR_CACHE_LAT4              0x400000d3   //Cache accesses taking longer than n cycles
#define CID_DATA_EAR_CACHE_LAT8              0x400000d7
#define CID_DATA_EAR_CACHE_LAT16             0x400000cf
#define CID_DATA_EAR_CACHE_LAT32             0x400000d2
#define CID_DATA_EAR_CACHE_LAT64             0x400000d6
#define CID_DATA_EAR_CACHE_LAT128            0x400000ce
#define CID_DATA_EAR_CACHE_LAT256            0x400000d1
#define CID_DATA_EAR_CACHE_LAT512            0x400000d5
#define CID_DATA_EAR_CACHE_LAT1024           0x400000cd
#define CID_DATA_EAR_CACHE_LAT2048           0x400000d0
#define CID_DATA_EAR_CACHE_LAT4096           0x400000d4
#define CID_NO_CODE                          0xffffffff

typedef struct Event {
    Group       EventGroup;
    const char* EventName;
    Metric      EventMetric;
    int         CodeID;
}EventType;


extern EventType* EventList;


#ifdef __cplusplus
extern "C" {
#endif

void initEventlist(const char* metric_plugin_name);

void finalizeEventlist( void );

void printEventlist( void );

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
std::string metric2PAPIName( Metric m );

std::string metric2string( Metric m );

bool checkMetricAvail( Metric m );

#else
char* metric2PAPIName( Metric m );

char* metric2string( Metric m );

int checkMetricAvail( Metric m );

#endif /*__cplusplus */

#endif /*METRIC_H_*/
