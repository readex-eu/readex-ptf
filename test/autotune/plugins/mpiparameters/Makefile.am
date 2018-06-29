TESTS += test_mpiparameters
check_PROGRAMS += test_mpiparameters

test_mpiparameters_CXXFLAGS = -I$(top_srcdir)/autotune/plugins/mpiparameters/include  ${autotune_test_base_cxxflags}

test_mpiparameters_SOURCES = test/autotune/plugins/mpiparameters/MPIParametersPlugin.cc
test_mpiparameters_LDADD = $(autotune_test_base_ldadd)
test_mpiparameters_DEPENDENCIES = $(autotune_test_base_dependencies)

