test_LTLIBRARIES       = libptftest.la
testdir = ${plugindir}/test/

libptftest_la_CXXFLAGS = ${autotune_plugin_base_cxxflags}\
                         -I$(top_srcdir)/autotune/plugins/test/include

libptftest_la_SOURCES  = autotune/plugins/test/src/TestPlugin.cc
libptftest_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0
