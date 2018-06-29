lib_LIBRARIES += libpscproperties.a

libpscproperties_a_CXXFLAGS =                                          \
    ${global_compiler_flags}                                           \
    -std=c++14                                                        \
    -D_REENTRANT                                                       \
    -I../include                                                       \
    ${PSC_ACE_CPPFLAGS}                                                \
    ${PSC_BOOST_CPPFLAGS}                                              \
    -I$(top_srcdir)/aagent/include                                     \
    -I$(top_srcdir)/aagent/src/strategies/include                      \
    -I$(top_srcdir)/aagent/src/properties/include/configurableProperty \
    -I$(top_srcdir)/aagent/src/properties/include/BenchmarkingProps    \
    -I$(top_srcdir)/aagent/src/properties/include/Energy               \
    -I$(top_srcdir)/aagent/src/properties/include/GPUTesting           \
    -I$(top_srcdir)/aagent/src/properties/include/mpi                  \
    -I$(top_srcdir)/aagent/src/properties/include/OpenCL               \
    -I$(top_srcdir)/aagent/src/properties/include/openmp               \
    -I$(top_srcdir)/aagent/src/properties/include/overhead             \
    -I$(top_srcdir)/aagent/src/properties/include/Pipeline             \
    -I$(top_srcdir)/aagent/src/properties/include/skeleton             \
    -I$(top_srcdir)/aagent/src/properties/include/tuning               \
    -I$(top_srcdir)/aagent/src/properties/include/Westmere             \
    -I$(top_srcdir)/aagent/src/properties/include/PerfDynamics         \
    -I$(top_srcdir)/aagent/src/properties/include/Hdeem                \
    -I$(top_srcdir)/aagent/src/properties/include/InterphaseProps      \
    -I$(top_srcdir)/quality_expressions/include                        \
    -I$(top_srcdir)/registry/include                                   \
    -I$(top_srcdir)/util/include

libpscproperties_a_SOURCES =                                              \
    aagent/src/properties/src/BenchmarkingProps/BranchMisprediction.cc    \
    aagent/src/properties/src/BenchmarkingProps/BenchmarkingAllProps.cc   \
    aagent/src/properties/src/BenchmarkingProps/L1D.cc                    \
    aagent/src/properties/src/BenchmarkingProps/L2Misses.cc               \
    aagent/src/properties/src/BenchmarkingProps/ExecTimeImportance.cc     \
    aagent/src/properties/src/BenchmarkingProps/PerformanceCounters.cc    \
    aagent/src/properties/src/Energy/CostOwnership.cc                     \
    aagent/src/properties/src/Energy/EnergyConsumption.cc                 \
    aagent/src/properties/src/Energy/EnergyInefficient.cc                 \
    aagent/src/properties/src/Energy/MemoryBound.cc                       \
    aagent/src/properties/src/Energy/PowerUsed.cc                         \
    aagent/src/properties/src/Energy/SuitedForEnergyConfiguration.cc      \
    aagent/src/properties/src/GPUTesting/AchievedOccupancy.cc             \
    aagent/src/properties/src/GPUTesting/ActiveCycles.cc                  \
    aagent/src/properties/src/GPUTesting/BranchEfficiency.cc              \
    aagent/src/properties/src/GPUTesting/BytesTransfered.cc               \
    aagent/src/properties/src/GPUTesting/ExecutionTime.cc                 \
    aagent/src/properties/src/GPUTesting/InstructionsExecuted.cc          \
    aagent/src/properties/src/GPUTesting/TransferTime.cc                  \
    aagent/src/properties/src/mpi/MpiLate.cc                              \
    aagent/src/properties/src/mpi/MPIexcessive.cc                         \
    aagent/src/properties/src/mpi/EagerLimitDependent.cc                  \
    aagent/src/properties/src/mpi/MpiMessageSizeProp.cc                   \
    aagent/src/properties/src/mpi/OverloadedMasterProp.cc                 \
    aagent/src/properties/src/OpenCL/KernelExecutionTime.cc               \
    aagent/src/properties/src/openmp/CriticalRegionOverhead.cc            \
    aagent/src/properties/src/openmp/ImbalanceInOrderedLoop.cc            \
    aagent/src/properties/src/openmp/ImbalanceInParSections.cc            \
    aagent/src/properties/src/openmp/ImbalanceOMPBarrier.cc               \
    aagent/src/properties/src/openmp/LoadImbalanceOMPRegion.cc            \
    aagent/src/properties/src/openmp/StartupShutdownOverhead.cc           \
    aagent/src/properties/src/openmp/SerializationCriticalRegion.cc       \
    aagent/src/properties/src/openmp/SequentialInMaster.cc                \
    aagent/src/properties/src/openmp/SequentialInOrderedLoop.cc           \
    aagent/src/properties/src/openmp/SequentialInSingle.cc                \
    aagent/src/properties/src/openmp/HotRegionExecTime.cc                 \
    aagent/src/properties/src/openmp/EmptyTaskinTaskRegion.cc             \
    aagent/src/properties/src/openmp/OverheadDueToSmallTask.cc            \
    aagent/src/properties/src/openmp/OverheadDueToTaskCreation.cc         \
    aagent/src/properties/src/openmp/TooFineGranularTasks.cc              \
    aagent/src/properties/src/openmp/ImbalancedTaskRegion.cc              \
    aagent/src/properties/src/overhead/OverheadProp.cc                    \
    aagent/src/properties/src/overhead/RequiredRegionProp.cc              \
    aagent/src/properties/src/overhead/HighInclOverheadProp.cc            \
    aagent/src/properties/src/overhead/HighExclOverheadProp.cc            \
    aagent/src/properties/src/Pipeline/PipelineStageExecutionTime.cc      \
    aagent/src/properties/src/Pipeline/PipelineStageBufWaitTime.cc        \
    aagent/src/properties/src/Pipeline/PipelineExecutionTime.cc           \
    aagent/src/properties/src/skeleton/SkeletonProp.cc                    \
    aagent/src/properties/src/tuning/ExecTime.cc                          \
    aagent/src/properties/src/tuning/HPCConditional.cc                    \
    aagent/src/properties/src/tuning/ActiveCyclesObjective.cc             \
    aagent/src/properties/src/Westmere/AddressAliasing.cc                 \
    aagent/src/properties/src/Westmere/DataCache.cc                       \
    aagent/src/properties/src/Westmere/DTLB_Misses.cc                     \
    aagent/src/properties/src/Westmere/ExecutionStall.cc                  \
    aagent/src/properties/src/Westmere/InstructionStarvation.cc           \
    aagent/src/properties/src/Westmere/LongLatencyInstructionException.cc \
    aagent/src/properties/src/Westmere/Misprediction.cc                   \
    aagent/src/properties/src/Westmere/WM_L1MissRate.cc                   \
    aagent/src/properties/src/Westmere/WM_L2MissRate.cc                   \
    aagent/src/properties/src/Westmere/WM_TLBMissRate.cc                  \
    aagent/src/properties/src/Hdeem/HdeemProp.cc                          \
    aagent/src/properties/src/InterphaseProps/InterphaseProps.cc
    
if PSC_TDA_ENABLED
libpscproperties_a_CXXFLAGS += ${PSC_TDA_CPPFLAGS} 
libpscproperties_a_SOURCES += aagent/src/properties/src/PerfDynamics/PerformanceDynamics.cc \
                              aagent/src/properties/src/PerfDynamics/Degradation.cc
endif
