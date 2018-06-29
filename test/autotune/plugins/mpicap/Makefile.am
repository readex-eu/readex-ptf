TESTS += test_mpicap
check_PROGRAMS += test_mpicap

test_mpicap_CXXFLAGS = -I$(top_srcdir)/autotune/plugins/mpicap/include  ${autotune_test_base_cxxflags}

test_mpicap_SOURCES = test/autotune/plugins/mpicap/MPICAPPlugin.cc
test_mpicap_LDADD = $(autotune_test_base_ldadd)
test_mpicap_DEPENDENCIES = $(autotune_test_base_dependencies)
