/**
   @file    PropertyID.h
   @ingroup AnalysisAgent
   @brief   Performance property types definition
   @verbatim
    Revision:       $Revision$
    Revision date:  $Date$
    Committed by:   $Author$

    This file is part of the Periscope performance measurement tool.
    See http://www.lrr.in.tum.de/periscope for details.

    Copyright (c) 2005-2011, Technische Universitaet Muenchen, Germany
    See the COPYING file in the base directory of the package for details.
   @endverbatim
 */

#ifndef PROPERTYIDS_H_
#define PROPERTYIDS_H_

enum PropertyID {
    LC2DMISSRATE,
    STALLCYCLES,
    STALLCYCLESINTEGERLOADS,
    L2MISS,
    L3MISS,
    DOMINATINGL2MISS,
    DOMINATINGL3MISS,
    MPIPROP,
    LOADIMBALANCEOMPREGION,
    CONVERSIONINTFLOAT,
    ADVANCESPECULATIVELOAD,
    MISSINGPREFETCH,
    POTENTIALBUNDLEBANKCONFLICT,
    BADCODE,
    SHORTSWPLOOP,

    //Scalability-based properties
    PROPERTYWITHINCREASINGSEVERITYACROSSCONFIGURATIONS, //15
    PROPERTYOCCURRINGINALLCONFIGURATIONS,               //16
    SUPERLINEARSPEEDUPFORALLCONFIGURATIONS,             //17
    LINEARSPEEDUPFORALLCONFIGURATIONS,                  //18
    LINEARSPEEDUPFAILEDFORTHEFIRSTTIME,                 //19
    SEQUENTIALCOMPUTATION,                              //20
    CODEREGIONWITHTHELOWESTSPEEDUPINTHERUN,             //21
    SPEEDUPDECREASING,                                  //22
    LOWSPEEDUP,                                         //23
    //OpenMP-based properties
    CRITICALREGIONOVERHEAD_OMPREGION,                   //24
    SERIALIZATIONCRITICALREGIONOVERHEAD_OMPREGION,      //25
    SEQUENTIALINSINGLE,                                 //26
    SEQUENTIALINMASTER,                                 //27
    SEQUENTIALINORDEREDLOOP,                            //28
    THREADS_IDLE_IN_MASTER,                             //29
    STARTUPSHUTDOWNOVERHEAD,                            //30
    TASKREGIONOVERHEAD,                                 //31
    TOOFINEGRANULARTASKS,                               //32
    EMPTYTASKINTASKREGION,                              //33
    OVERHEADDUETOSMALLTASK,                             //34
    IMBALANCEDTASKREGION,                               //35
    IMBALANCEDUETOUNEVENDISTRIBUTIONOFTASKS,            //36
    NUMBEROFTASKSSMALLERTHANNUMBEROFTHREADS,            //37
    IMBALANCEOMPBARRIER,                                //38
    IMBALANCE_IN_ORDERED_LOOP_OMP,                      //39
    FREQUENT_ATOMIC_OMP,                                //40
    NOT_ENOUGH_SECTION_OMP,                             //41
    UNEVEN_SECTION_OMP,                                 //42
    LOADIMBALANCEPARALLELLOOPNOWAIT,                    //43
    LOADIMBALANCEPARALLELREGION,                        //44
    LOADIMBALANCEPARALLELLOOP,                          //45
    LOADIMBALANCEWORKSHAREREGION,                       //46
    LOADIMBALANCEPARALLELSECTION,                       //47
    SERIALIZATIONINPARALLELPROGRAM,                     //48
    PARALLELREGIONEXECTIME,                             //49
    USERREGIONEXECTIME,                                 //50
    MAINREGIONEXECTIME,                                 //51
    OMPFORREGIONEXECTIME,                               //52
    CALLREGIONEXECTIME,                                 //53
    SUBREGIONEXECTIME,                                  //54
    ATOMICREGIONOVERHEAD_OMPREGION,                     //55
    FLUSHREGIONOVERHEAD_OMPREGION,                      //56

//p575 new properties begin
    p575PropBegin,                                     // relative prop number = absolute number - p575PropBegin
    P6HOTSPOT,                                         //01
    P6DPUHELDFXMULDIVTOT,                              //02
    P6DPUHELDFXMULTNOPIPE,                             //03
    P6DPUHELDFXMULTGPR,                                //04
    P6DPUHELDGPR,                                      //05
    P6L1MISSCYC,                                       //06
    P6AVGL1MISSCYC,                                    //07
    P6L1DEMANDMISSRATE,                                //08
    P6PREFL1,                                          //09
    P6L2DEMANDMISSRATE,                                //10
    P6PREFL2LD,                                        //11
    P6L3DEMANDMISSRATE,                                //12
    P6L2TOTALMISSRATE,                                 //13
    P6DERATMISSCYC,                                    //14
    P6DERATMISS4K,                                     //15
    P6DERATMISS64K,                                    //16
    P6DERATMISS16M,                                    //17
    P6DERATMISS16G,                                    //18
    P6SRQFULL,                                         //19
    P6STOREMISS,                                       //20
    P6STORECHAINED,                                    //21
    P6PREFL2ST,                                        //22
    P6DPUHELDFPQ,                                      //23
    P6FPUDIVSQRT,                                      //24
    P6FPUISSUEDIVOVLP,                                 //25
    P6FPUISSUESTALLFPR,                                //26
    P6FPUISSUESTALLST,                                 //27
    P6FPUFMARATE,                                      //28
    P6DPUHELDFPUCR,                                    //29
    P6DPUHELDSMT,                                      //30
    P6DPUSTALL,                                        //31
    P6DPUSTALLICACHE,                                  //32
    P6DPUSTALLOTHER,                                   //33
    P6LSULHSREJ,                                       //34
    p6HIGHLEVELMEM,
    p575PropEnd,

//p575 end new properties
//Cross platform props
    CPPL1MISS,
    CPPL2MISS,
    CPPL3MISS,
    CPPTLBMISS,
    OVERHEAD,
    HIGHINCLOVERHEAD,
    HIGHEXCLOVERHEAD,
    REQUIREDREGION,
    BENCHALLPROPS,

//Westmere properties
    EXECUTIONSTALL,
    MISPREDICTION,
    INSTRUCTION_STARVATION,
    DATA_CACHE,
    L1DMETRIC,
    L2MISSES,
    BRANCH_MISPREDICTION,
    DTLB_MISSES,
    ADDRESS_ALIASING,
    LONG_LATENCY_INSTRUCTION_EXCEPTION,
    WM_L1MISSRATE,
    WM_L2MISSRATE,
    WM_TLBMISSRATE,

    EXECTIME,

    //DynamicProperties
    DYNPERF,
    FASTCHANGES,
    SLOWCHANGES,
    DYNENERGYLOCALIZED,
    PERFDEGRADATION,
    DEGRADATIONPEAKS,

//OpenCL properties
    OPENCL_KERNELEXECTIME,

//CUDA properties
    CUDA_ACHIEVEDOCCUPANCY,
    CUDA_ACTIVECYCLES,
    CUDA_BRANCHEFFICIENCY,
    CUDA_BYTESTRANSFERED,
    CUDA_EXECUTIONTIME,
    CUDA_TRANSFERTIME,
    CUDA_INSTRUCTIONSEXECUTED,

//Energy properties
    ENERGY_CONSUMPTION,
    ENERGY_INEFFICIENT,
    MEMORY_BOUND,
    ENERGY_SUITED,
    POWER_USED,

//Hdeem properties
    HDEEM_ENERGY_CONSUMTION_BLADE,

//Inter-phase properties for phase features
    INTERPHASE_PROPS,

//pipeline properties
    PIPESTAGEEXECTIME,
    PIPEEXECTIME,
    PIPESTAGEBUFWAITTIME,

//Configurable properties
    CONFIGURABLEPROPERTY,

//Tuning Objectives
    ACTIVECYCLESOBJECTIVE,

//Specialized MPI
    MPITIME,
    EAGERLIMITDEPENDENT,

//Master Worker Barcelona
    OVERLOADEDMASTER,

    EXECTIMEIMPORTANCE,

    HPCCONDITIONAL,

    PERFORMANCECOUNTERS,

// Performance Dynamics
    PERFORMANCEDYNAMICS,
    DEGRADATION
};

#endif /*PROPERTYIDS_H_*/
