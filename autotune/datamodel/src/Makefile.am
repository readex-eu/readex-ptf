lib_LTLIBRARIES += libdatamodel.la

libdatamodel_la_CXXFLAGS = ${AM_CXXFLAGS}                              \
                           ${global_compiler_flags}                    \
                           -D_REENTRANT                                \
                           -I$(top_srcdir)/autotune/datamodel/include  \
                           -I$(top_srcdir)/util/include                \
                           -I$(top_srcdir)/aagent/include              \
                           -I$(top_srcdir)/registry/include            \
                           -I$(top_srcdir)/quality_expressions/include \
                           ${PSC_ACE_CPPFLAGS}                         \
                           ${PSC_BOOST_CPPFLAGS}

libdatamodel_la_SOURCES = autotune/datamodel/src/Constraint.cc             \
                          autotune/datamodel/src/AnalysisResultsPool.cc    \
                          autotune/datamodel/src/Restriction.cc            \
                          autotune/datamodel/src/Scenario.cc               \
                          autotune/datamodel/src/SearchSpace.cc            \
                          autotune/datamodel/src/ScenarioPool.cc           \
                          autotune/datamodel/src/ScenarioResultsPool.cc    \
                          autotune/datamodel/src/TuningConfiguration.cc    \
                          autotune/datamodel/src/TuningParameter.cc        \
                          autotune/datamodel/src/TuningValue.cc            \
                          autotune/datamodel/src/Variant.cc                \
                          autotune/datamodel/src/VariantSpace.cc           \
                          autotune/datamodel/src/VariantSpaceConstraint.cc \
                          autotune/datamodel/src/PropertyRequest.cc        \
                          autotune/datamodel/src/ProgramSignature.cc       \
                          autotune/datamodel/src/ScenarioPoolSet.cc        \
                          autotune/datamodel/src/Advice.cc                 \
                          autotune/datamodel/src/TuningSpecification.cc

libdatamodel_la_LDFLAGS = -version-info 1:0:0
