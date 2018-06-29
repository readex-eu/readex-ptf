TESTS += test_derivedtuningparameter
check_PROGRAMS += test_derivedtuningparameter

test_derivedtuningparameter_CXXFLAGS = -I$(top_srcdir)/autotune/plugins/compilerflags/include  ${autotune_test_base_cxxflags}

test_derivedtuningparameter_SOURCES = test/autotune/datamodel/derivedtuningparameter/CFSTuningParameter.cc
test_derivedtuningparameter_LDADD = $(autotune_test_base_ldadd)
test_derivedtuningparameter_DEPENDENCIES = ${autotune_test_base_dependencies}
