TESTS += test_compilerflags
check_PROGRAMS += test_compilerflags

test_compilerflags_CXXFLAGS = -I$(top_srcdir)/autotune/plugins/compilerflags/include  ${autotune_test_base_cxxflags}

test_compilerflags_SOURCES = test/autotune/plugins/compilerflags/CompilerFlagsPlugin.cc
test_compilerflags_LDADD = $(autotune_test_base_ldadd)
test_compilerflags_DEPENDENCIES = $(autotune_test_base_dependencies)