/**
   @file    PropertyThresholdConfig.h
   @ingroup AnalysisAgent
   @brief   Performance property types definition
   @author  Ventsislav Petkov
   Apr 26, 2012
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

#ifndef PROPERTYTHRESHOLDCONFIG_H_
#define PROPERTYTHRESHOLDCONFIG_H_

/**
 * For gcc < 6 gnu compatible use const for non-integral variable
 * For gcc > 6 g++ compiler requires GNU_CONST_DECLARATION
 */

#if defined(__ICC) || defined(__INTEL_COMPILER)
#define GNU_CONST_DECLARATION const
#elif defined(__GNUC__) || defined(__GNUG__)
#define GNU_CONST_DECLARATION constexpr
#else
#define GNU_CONST_DECLARATION ""
#endif

struct MPI_PROPS_THRESHOLD {
    static GNU_CONST_DECLARATION double MPIexcessiveProp         = 5.0;
    static GNU_CONST_DECLARATION double MpiLateProp              = 5.0;
    static GNU_CONST_DECLARATION double MPIlargeMessagesProp     = 5.0;
    static GNU_CONST_DECLARATION double MPImanySmallMessagesProp = 5.0;
    static GNU_CONST_DECLARATION double MpiMessageSizeProp       = 5.0;
    static GNU_CONST_DECLARATION double MpiSGICountersProp       = 5.0;
    static GNU_CONST_DECLARATION double EagerLimitDependent      = 10.0;
};

struct WESTMERE_PROPS_THRESHOLD {
    static GNU_CONST_DECLARATION double AddressAliasing                 = 1.0;
    static GNU_CONST_DECLARATION double DataCache                       = 1.0;
    static GNU_CONST_DECLARATION double DTLB_Misses                     = 1.0;
    static GNU_CONST_DECLARATION double ExecutionStall                  = 1.0;
    static GNU_CONST_DECLARATION double InstructionStarvation           = 1.0;
    static GNU_CONST_DECLARATION double LongLatencyInstructionException = 1.0;
    static GNU_CONST_DECLARATION double Misprediction                   = 1.0;
    static GNU_CONST_DECLARATION double WM_L1MissRate                   = 1.0;
    static GNU_CONST_DECLARATION double WM_L2MissRate                   = 1.0;
    static GNU_CONST_DECLARATION double WM_TLBMissRate                  = 1.0;
};

struct BENCHMARKING_PROPS_THRESHOLD {
    static GNU_CONST_DECLARATION double BenchmarkingAllProps = 0.0;
    static GNU_CONST_DECLARATION double BranchMisprediction  = 1.0; // not used
    static GNU_CONST_DECLARATION double L1D                  = 1.0; // not used
    static GNU_CONST_DECLARATION double L2Misses             = 1.0; // not used
};

struct IA64_PROPS_THRESHOLD {
    static GNU_CONST_DECLARATION double StallCyclesProp                 = 10.0;
    static GNU_CONST_DECLARATION double StallCyclesIntegerLoadsProp     = 30.0;
    static GNU_CONST_DECLARATION double AdvanceSpeculativeLoadProp      = 0.0;
    static GNU_CONST_DECLARATION double BadCodeProp                     = 1.2;
    static GNU_CONST_DECLARATION double ConvIntFloatProp                = 0.0;
    static GNU_CONST_DECLARATION double DominatingL2MissProp            = 30.0;
    static GNU_CONST_DECLARATION double DominatingL3MissProp            = 30.0;
    static GNU_CONST_DECLARATION double L2MissProp                      = 30.0;
    static GNU_CONST_DECLARATION double L3MissProp                      = 30.0;
    static GNU_CONST_DECLARATION double LC2DMissRateProp                = 0.1;
    static GNU_CONST_DECLARATION double MissingPrefetchProp             = 0.0;
    static GNU_CONST_DECLARATION double PotentialBundleBankConflictProp = 0.0;
    static GNU_CONST_DECLARATION double ShortSoftwarePipelinedLoopProp  = 0.0; // not used
};

struct BGP_PROPS_THRESHOLD {
    static GNU_CONST_DECLARATION double LC2DMissRateProp = 0.0;
};

struct OMP_PROPS_THRESHOLD {
    static GNU_CONST_DECLARATION double LoadImbalanceOMPRegionProp           = 5.0;
    static GNU_CONST_DECLARATION double CriticalRegionOverheadProp           = 5.0;
    static GNU_CONST_DECLARATION double EmptyTasksinTaskRegionProp           = 5.0;
    static GNU_CONST_DECLARATION double FrequentAtomicProp                   = 5.0;
    static GNU_CONST_DECLARATION double HotRegionExecTimeProp                = 5.0; // not used
    static GNU_CONST_DECLARATION double ImbalancedTaskRegionProp             = 5.0;
    static GNU_CONST_DECLARATION double ImbalanceDueToTaskingProp            = 5.0;
    static GNU_CONST_DECLARATION double ImbalanceInOrderedLoopProp           = 5.0;
    static GNU_CONST_DECLARATION double ImbalanceInParSectionsProp           = 5.0;
    static GNU_CONST_DECLARATION double ImbalanceOMPBarrierProp              = 5.0;
    static GNU_CONST_DECLARATION double OverheadDueToSmallTaskProp           = 5.0;
    static GNU_CONST_DECLARATION double OverheadDueToTaskCreationProp        = 5.0;
    static GNU_CONST_DECLARATION double SequentialInMasterProp               = 5.0;
    static GNU_CONST_DECLARATION double SequentialInOrderedLoopProp          = 5.0;
    static GNU_CONST_DECLARATION double SequentialInSingleProp               = 5.0;
    static GNU_CONST_DECLARATION double SerializationCriticalRegionProp      = 5.0;
    static GNU_CONST_DECLARATION double SerializationInParallelProgramProp   = 5.0; // not used
    static GNU_CONST_DECLARATION double StartupShutdownOverheadProp          = 5.0;
    static GNU_CONST_DECLARATION double Task_StartupAndShutdown_OverheadProp = 5.0;
    static GNU_CONST_DECLARATION double TooFineGranularTasksProp             = 5.0;
};

struct OVERHEAD_PROPS_THRESHOLD {
    static GNU_CONST_DECLARATION double HighExclOverheadProp = 5.0;
    static GNU_CONST_DECLARATION double HighInclOverheadProp = 5.0;
    static GNU_CONST_DECLARATION double OverheadProp         = 0.1;
    static GNU_CONST_DECLARATION double RequiredRegionProp   = 0.0; // not used
};

struct CPPROPS_PROPS_THRESHOLD {
    static GNU_CONST_DECLARATION double L1MissRate  = 1.0;
    static GNU_CONST_DECLARATION double L2MissRate  = 1.0;
    static GNU_CONST_DECLARATION double L3MissRate  = 1.0;
    static GNU_CONST_DECLARATION double TLBMissRate = 1.0;
};

struct ENERGY_GRANULARITY_THRESHOLD {
    static GNU_CONST_DECLARATION double RAPLGranularity        = 0.001; // 0.001 is 1 ms. MSR register responsible for RAPL Energy measurement is updated every ~1ms.
    static GNU_CONST_DECLARATION double PaddleCardsGranularity = 1.0;   // Check the minimum resolution for the Paddle cards
    static GNU_CONST_DECLARATION double iPDUGranularity        = 10.0;  // Check the minimum resolution for the iPDU
    static GNU_CONST_DECLARATION double EnergyInefficiency     = 5;     // Determine minimum energy that makes sense to tune
    static GNU_CONST_DECLARATION double MemoryBound            = 5;     // Determine minimum energy that makes sense to tune
};

#endif /* PROPERTYTHRESHOLDCONFIG_H_ */
