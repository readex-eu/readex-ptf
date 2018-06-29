foo_LTLIBRARIES             = libptffoo.la
foodir = ${plugindir}/foo/

libptffoo_la_CXXFLAGS = ${autotune_plugin_base_cxxflags} \
                                   -I$(top_srcdir)/autotune/plugins/foo/include

libptffoo_la_SOURCES  = autotune/plugins/foo/src/Foo.cc
libptffoo_la_LDFLAGS  = ${autotune_plugin_base_ldflags} -version-info 1:0:0
