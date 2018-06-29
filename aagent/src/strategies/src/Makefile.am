lib_LIBRARIES += libstrategies.a

libstrategies_a_CXXFLAGS = ${global_compiler_flags}                                           \
                           -D_REENTRANT                                                       \
                           -std=c++14                                                        \
                           ${PSC_ACE_CPPFLAGS}                                                \
                           ${PSC_BOOST_CPPFLAGS}                                              \
                           ${PSC_ENOPT}                                                       \
                           ${PSC_OPENCL}                                                      \
                           ${PSC_CUPTI}                                                       \
                           ${PSC_VPATTERN}                                                    \
                           -I../include                                                       \
                           -I$(top_srcdir)/aagent/src/properties/include/configurableProperty \
                           -I$(top_srcdir)/aagent/src/properties/include/mpi                  \
                           -I$(top_srcdir)/aagent/src/properties/include/openmp               \
                           -I$(top_srcdir)/aagent/src/properties/include/overhead             \
                           -I$(top_srcdir)/aagent/src/properties/include/BenchmarkingProps    \
                           -I$(top_srcdir)/aagent/src/properties/include/Westmere             \
                           -I$(top_srcdir)/aagent/src/properties/include/GPUTesting           \
                           -I$(top_srcdir)/aagent/src/properties/include/OpenCL               \
                           -I$(top_srcdir)/aagent/src/properties/include/Energy               \
                           -I$(top_srcdir)/aagent/src/properties/include/Pipeline             \
                           -I$(top_srcdir)/aagent/src/properties/include/tuning               \
                           -I$(top_srcdir)/aagent/src/properties/include/BGP                  \
                           -I$(top_srcdir)/aagent/src/properties/include/p6                   \
                           -I$(top_srcdir)/aagent/src/properties/include/ia64                 \
                           -I$(top_srcdir)/aagent/src/properties/include/CPProps              \
                           -I$(top_srcdir)/aagent/src/properties/include/PerfDynamics         \
                           -I$(top_srcdir)/aagent/src/properties/include/Hdeem                \
                           -I$(top_srcdir)/aagent/src/properties/include/InterphaseProps      \
                           -I$(top_srcdir)/aagent/include                                     \
                           -I$(top_srcdir)/aagent/src/strategies/include                      \
                           -I$(top_srcdir)/autotune/datamodel/include                         \
                           -I$(top_srcdir)/mrimonitor/include                                 \
                           -I$(top_srcdir)/util/include                                       \
                           -I$(top_srcdir)/quality_expressions/include                        \
                           -I$(top_srcdir)/registry/include

libstrategies_a_SOURCES = aagent/src/strategies/src/ScorepMPIStrategy.cc    \
                          aagent/src/strategies/src/OpenMPAnalysisScoreP.cc \
                          aagent/src/strategies/src/Importance.cc           \
                          aagent/src/strategies/src/ConfigAnalysis.cc       \
                          aagent/src/strategies/src/EnergyGranularityBF.cc  \
                          aagent/src/strategies/src/Tuning.cc

if PSC_TDA_ENABLED
libstrategies_a_CXXFLAGS += ${PSC_TDA_CPPFLAGS}
libstrategies_a_SOURCES += aagent/src/strategies/src/PerfDynamicsAnalysis.cc
endif

# All strategies were disabled during the integration of the Score-P support
# To reinable strategies have to be adapted to the new measurement requesting
# and retrieval mechanism implemented by the DataProvider
#
#                          aagent/src/strategies/src/BGPStrategyDF.cc           \
#                          aagent/src/strategies/src/GPUTestingStrategy.cc      \
#                          aagent/src/strategies/src/OCLStrategy.cc             \
#                          aagent/src/strategies/src/OpenCLStrategy.cc          \
#                          aagent/src/strategies/src/EnergyStrategy.cc          \
#                          aagent/src/strategies/src/PipelineStrategy.cc        \
#                          aagent/src/strategies/src/p575Strategy.cc            \
#                          aagent/src/strategies/src/p575StrategyBF.cc          \
#                          aagent/src/strategies/src/RegionNestingStrategy.cc   \
#                          aagent/src/strategies/src/SCP_StrategyBF.cc          \
#                          aagent/src/strategies/src/WestmereBreadthFirst.cc    \
#                          aagent/src/strategies/src/WestmereDepthFirst.cc      \
#                          aagent/src/strategies/src/ScorepMPIStrategy.cc       \
#                          aagent/src/strategies/src/BenchmarkingStrategy.cc    \
#                          aagent/src/strategies/src/MPIStrategy.cc             \
#                          aagent/src/strategies/src/InstrumentationStrategy.cc \
#                          aagent/src/strategies/src/OpenMPAnalysis.cc          \
#                          aagent/src/strategies/src/RegionNestingStrategy.cc   \
#                          aagent/src/strategies/src/Tuning.cc                  \
#                          aagent/src/strategies/src/EnergyGranularity.cc       \
#                          aagent/src/strategies/src/ConfigAnalysis.cc          \
#                          aagent/src/strategies/src/Importance.cc
